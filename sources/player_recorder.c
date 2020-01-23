#include "player_recorder.h"

// for measurement of sound length
#include "timelib.h"

// for starting sound
#include "soundlib.h"

// timer config
#include "LPC17xx.h"

struct {
	unsigned start;
	int frequency;
	char is_recording;
	char unfinnished_record;
} recorder_state;

#ifndef PLAYER_TIMER_NO
#define PLAYER_TIMER_NO 2
#endif

#define PLAYER_TIMER_LPC LPC_TIM_EVAL(PLAYER_TIMER_NO)
#define PLAYER_TIMER_IRQn TIMER_IRQn_EVAL(PLAYER_TIMER_NO)
#define PLAYER_TIMER_Handler TIMER_IRQHandler_EVAL(PLAYER_TIMER_NO)

static char is_player_playing_val;

// end on frequency = 0;
static Sound saved_sounds[128];
static Sound* current_position;

static void reset_eeprom_position(void);
static void copy_ram_to_flash(void);
static void copy_flash_to_ram(void);

void player_recorder_init(void) {
	recorder_state.start = 0;
	recorder_state.frequency = NO_SOUND;
	recorder_state.is_recording = 0;
	recorder_state.unfinnished_record = 0;

	is_player_playing_val = 0;


	// PLAYER
	// timer configuration for player
	
	// turning on timer
	LPC_SC->PCONP |= PLAYER_TIMER_NO < 2 ? 1 << (1 + PLAYER_TIMER_NO) : 1 << (20 + PLAYER_TIMER_NO);

	// peripherial clock set to CCLK/4
	// no need - default value

	// enable interrupt for timer
	NVIC_EnableIRQ(PLAYER_TIMER_IRQn);

	// TODO prepare eeprom


	reset_next_sound_to_start();

	reset_eeprom_position();
	copy_flash_to_ram();
}

void start_record(int set_frequency) {
	if (recorder_state.unfinnished_record)
		end_record();
	recorder_state.is_recording = 1;
	recorder_state.start = millis();
	recorder_state.frequency = set_frequency;
	recorder_state.unfinnished_record = 1;
}

// TODO implemet it
static void append_to_eeprom(Sound to_save) {
	current_position++;
	*current_position = to_save;
	current_position[1].frequency = NO_SOUND;

}

void end_record(void) {
	if (!recorder_state.unfinnished_record)
		return;
	Sound to_save = {recorder_state.frequency, millis() - recorder_state.start};
	append_to_eeprom(to_save);
	recorder_state.unfinnished_record = 0;
	start_record(SILENCE);

}

void end_recording(void) {
	recorder_state.is_recording = 0;
	end_record();
	Sound end = {NO_SOUND, 0};
	append_to_eeprom(end);
	reset_next_sound_to_start();
	copy_ram_to_flash();
}


static Sound read_next_from_eeprom(void){
	// TODO read some valid data
	current_position++;
	return *current_position;
}

Sound read_next_sound(void) {
	return read_next_from_eeprom();
}

static void reset_eeprom_position(void) {
	current_position = saved_sounds - 1;

	// TODO actual reseting
	

}

void reset_next_sound_to_start(void) {
	reset_eeprom_position();
}

char has_next_sound() {
	return current_position[1].frequency != NO_SOUND;
}

char is_recording(void) {
	return recorder_state.is_recording;
}

// TODO implemet it
void erase_saved(void) {

	for (int iter = 0; iter < sizeof(saved_sounds)/sizeof(Sound); ++iter) {
		saved_sounds[iter].frequency = NO_SOUND;
	}
	reset_next_sound_to_start();
	copy_ram_to_flash();
}


// PLAYER

static void start_next_sound(void) {
	stop_sound();
	if (!has_next_sound()) {
		stop_playing_from_memory();
		return;
	}

	// turn on interrupts
	PLAYER_TIMER_LPC->MCR = 1 << 0;

	// set prescaler for 1 ms
	int prescaler = (SystemCoreClock/4)/1000;

	// set prescaller for timer
	// remember that it starts counting from 0
	PLAYER_TIMER_LPC->PR = prescaler - 1;

	Sound to_play = read_next_sound();

	// setting match register - numer of tick
	// to run handler
	PLAYER_TIMER_LPC->MR0 = to_play.length_in_millis;
	PLAYER_TIMER_LPC->TC = 0;

	// turn on counter
	PLAYER_TIMER_LPC->TCR = 1 << 0;

	if (to_play.frequency > 0) {
		start_sound(to_play.frequency);
	}
	is_player_playing_val = 1;
}

void PLAYER_TIMER_Handler(void) {
	start_next_sound();
	// clear interrupt flag
	PLAYER_TIMER_LPC->IR = 1 << 0;
}



void play_from_memory(void) {
	reset_next_sound_to_start();
	start_next_sound();
}

void stop_playing_from_memory(void) {
	// turn off counter
	PLAYER_TIMER_LPC->CCR = 0;
	is_player_playing_val = 0;
	reset_next_sound_to_start();
}

char is_player_playing(void) {
	return is_player_playing_val;
}

// IAP
#define IAP_LOCATION 0x1FFF1FF1U
#define IAP_START_MEMORY 0x00078000U

unsigned long command[5];
unsigned long output[5];

typedef void (*IAP)(unsigned long command [], unsigned long output[]);

IAP iap_entry = (IAP) IAP_LOCATION;

static void copy_ram_to_flash(void) {
	// prepare sector
	command[0] = 50;
	command[1] = 0x1D;
	command[2] = 0x1D;
	iap_entry(command, output);

	// erase 
	command[0] = 52;
	command[1] = 0x1D;
	command[2] = 0x1D;
	command[3] = SystemCoreClock/1000;
	iap_entry(command, output);
	
	// prepare sector
	command[0] = 50;
	command[1] = 0x1D;
	command[2] = 0x1D;
	iap_entry(command, output);
	
	// command number
	SystemCoreClockUpdate();
	command[0] = 51;
	command[1] = IAP_START_MEMORY;
	command[2] = (unsigned long)saved_sounds;
	command[3] = sizeof(saved_sounds);
	command[4] = SystemCoreClock/1000;
	iap_entry(command, output);

}

static void copy_flash_to_ram(void) {
	for (int iter = 0; iter < sizeof(saved_sounds)/sizeof(Sound); ++iter) {
		saved_sounds[iter] = ((Sound*)IAP_START_MEMORY)[iter];
	}
}
