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
} recorder_state;

#ifndef PLAYER_TIMER_NO
#define PLAYER_TIMER_NO 2
#endif

#define PLAYER_TIMER LPC_TIM2

static char is_player_playing_val;

void player_recorder_init(void) {
	recorder_state.start = 0;
	recorder_state.frequency = NO_SOUND;
	recorder_state.is_recording = 0;

	is_player_playing_val = 0;


	// PLAYER
	// timer configuration for player
	
	// turning on timer
	LPC_SC->PCONP |= PLAYER_TIMER_NO < 2 ? 1 << (1 + PLAYER_TIMER_NO) : 1 << (20 + PLAYER_TIMER_NO);

	// peripherial clock set to CCLK/4
	// no need - default value

	// enable interrupt for timer
	NVIC_EnableIRQ(TIMER2_IRQn);


	// TODO prepare eeprom

}

void start_record(int set_frequency) {
	if (recorder_state.is_recording)
		end_record();
	else
		recorder_state.is_recording = 1;
	recorder_state.start = millis();
	recorder_state.frequency = set_frequency;
}

// TODO implemet it
static void append_to_eeprom(Sound to_save) {
	
}

void end_record(void) {
	Sound to_save = {millis() - recorder_state.start,
		recorder_state.frequency};
	append_to_eeprom(to_save);
	if (recorder_state.is_recording)
		start_record(NO_SOUND);
}

void end_recording(void) {
	recorder_state.is_recording = 0;
	end_record();
}


static Sound read_next_from_eeprom(void){
	// TODO read some valid data
	Sound result = {-1, 100};
	return result;
}

Sound read_next_sound(void) {
	return read_next_from_eeprom();
}

static void reset_eeprom_position(void) {
	// TODO actual reseting

}

void reset_next_sound_to_start(void) {
	reset_eeprom_position();
}

char has_next_sound() {
	return 0;
}

// TODO implemet it
void erase_saved(void) {
	
}


// PLAYER

static void start_next_sound(void) {
	stop_sound();
	if (!has_next_sound()) {
		stop_playing_from_memory();
		return;
	}

	// turn on interrupts
	PLAYER_TIMER->MCR = 1 << 0;

	// set prescaler for 1 ms
	int prescaler = (SystemCoreClock/4)/1000;

	// set prescaller for timer
	// remember that it starts counting from 0
	PLAYER_TIMER->PR = prescaler - 1;

	Sound to_play = read_next_sound();

	// setting match register - numer of tick
	// to run handler
	PLAYER_TIMER->MR0 = to_play.length_in_millis;

	// turn on counter
	PLAYER_TIMER->TCR = 1 << 0;

	if (to_play.frequency > 0) {
		start_sound(to_play.frequency);
		is_player_playing_val = 1;
	}
		
}

void TIMER2_IRQHandler(void) {
	start_next_sound();
	// clear interrupt flag
	PLAYER_TIMER->IR = 1 << 0;
}



void play_from_memory(void) {
	reset_next_sound_to_start();
	start_next_sound();
}

void stop_playing_from_memory(void) {
	// turn off counter
	PLAYER_TIMER->CCR = 0;
	is_player_playing_val = 0;
	reset_next_sound_to_start();
}

char is_player_playing(void) {
	return is_player_playing_val;
}
