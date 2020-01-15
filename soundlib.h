/*
 * library for playing square wave sound
 * on dac using one of LPC1768 timers through
 * hardware interrupts (1 interrupt per switch)
 */

#ifndef SOUNDLIB_H
#define SOUNDLIB_H

// Note declaration
#include "common.h"

// setting timer (timer 1 by default,
// define TIMER_NO if desired)
// and dac configuration
void dac_init(void);
// start sound with specific frequency
// this function setups timer so it finnishes quickly
void start_sound(int frequency);
// start_sound with some extra steps
void start_note(Note note);
// stop prevoiusly played sound
// no action if not playing
void stop_sound(void);
// volume in [0, 1024)
void set_volume(int volume);
// volume in [0, 1]
void set_volume_f(float volume);
// simple conversion function
int note_to_frequency(Note note);
// pretty self explainatory
char is_playing(void);


#endif // SOUNDLIB_H
