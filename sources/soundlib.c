#include "soundlib.h"

// SystemCoreClock and other lpc stuff
#include "LPC17xx.h"
// for pow function
#include <math.h>

#ifndef SOUNDLIB_TIMER_NO
#define SOUNDLIB_TIMER_NO 1
#endif

#define SOUNDLIB_TIMER_LPC LPC_TIM_EVAL(SOUNDLIB_TIMER_NO)
#define SOUNDLIB_TIMER_IRQn TIMER_IRQn_EVAL(SOUNDLIB_TIMER_NO)
#define SOUNDLIB_TIMER_Handler TIMER_IRQHandler_EVAL(SOUNDLIB_TIMER_NO)

volatile struct {
	int volume;
	char high;
	char on;
	char leave_handler;
} dac_state;

void dac_init(void) {
	// timer configuration
	
	// turning on timer
	LPC_SC->PCONP |= SOUNDLIB_TIMER_NO < 2 ? 1 << (1 + SOUNDLIB_TIMER_NO) : 1 << (20 + SOUNDLIB_TIMER_NO);

	// peripherial clock set to CCLK/4
	// no need - default value

	// enable interrupt for timer
	NVIC_EnableIRQ(SOUNDLIB_TIMER_IRQn);

	// dac configuration

	dac_state.volume = 0;
	dac_state.high = 0;
	dac_state.on = 0;
	dac_state.leave_handler = 0;

	// enable dac function on P0.26
	LPC_PINCON->PINSEL1 = 2 << 20;

	// setting bias
	LPC_DAC->DACR = 1 << 16;
}

static void configure_timer_for_playing_sound(int frequency) {

	// turn on interrupts and auto reset of counter
	SOUNDLIB_TIMER_LPC->MCR = 1 << 0 | 1 << 1;

	SystemCoreClockUpdate();

	// set prescaler for 10 us
	// (switch for A takes place about 1 time every ms)
	int prescaler = (SystemCoreClock/4)/100000;
	
	// set prescaller for timer
	// remember that it starts counting from 0
	SOUNDLIB_TIMER_LPC->PR = prescaler - 1;

	// setting match register - numer of tick
	// to run handler
	// 2 every period (turn on and off speaker)
	SOUNDLIB_TIMER_LPC->MR0 = 100000/(frequency * 2);

	// turn on counter
	SOUNDLIB_TIMER_LPC->TCR = 1 << 0;

}

static void stop_counter(void) {
	// turn off counter
	SOUNDLIB_TIMER_LPC->TCR = 0;
}

void start_sound(int frequency) {
	dac_state.on = 1;
	dac_state.high = 0;
	configure_timer_for_playing_sound(frequency);
}

int note_to_frequency(Note note) {
	return 440.f * powf(2.f, (note - A)/12.f);
}


void start_note(Note note) {
	start_sound(note_to_frequency(note));
}

void stop_sound(void) {
	stop_counter();
	dac_state.on = 0;
}

void set_volume(int volume) {
	// guard for bad volume level
	volume = volume > 0x3FF ? 0x3FF : volume;
	volume = volume < 0 ? 0 : volume;

	dac_state.volume = volume;
}

void set_volume_f(float volume) {
	set_volume(volume * (float)0x3FF);
}


void SOUNDLIB_TIMER_Handler(void) {
	if (!dac_state.on || dac_state.leave_handler) {
		if (dac_state.leave_handler)
			dac_state.leave_handler = 0;
		// clear interrupt flag
		SOUNDLIB_TIMER_LPC->IR = 1 << 0;
		return;
	}
	unsigned int value = dac_state.high ? 0 : (dac_state.volume & 0x3FF);
	LPC_DAC->DACR = (1 << 16) | (value << 6);
	dac_state.high = !dac_state.high;
	dac_state.leave_handler = 1;
	// clear interrupt flag
	SOUNDLIB_TIMER_LPC->IR = 1 << 0;
	NVIC_ClearPendingIRQ(SOUNDLIB_TIMER_IRQn);
}

char is_playing(void) {
	return dac_state.on;
}

