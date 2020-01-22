#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

static const int SILENCE = -1;
static const int NO_SOUND = 0;

// RECORDER

typedef struct {
	int frequency;
	int length_in_millis;
} Sound;

// setup for recorder
// no special order of calling
void player_recorder_init(void);

// start record/sound and if not recording start recording
void start_record(int frequency);
// call at the end of current sound
void end_record(void);
// completely end recording
// calling start_record continues recording
void end_recording(void);
// clear data in eeprom and start recording from beginning
void erase_saved(void);
// read_next_sound will start from the beginning
void reset_next_sound_to_start(void);
// returns next sound saved in eeprom
Sound read_next_sound(void);
// if any sound left to play returns 1
// 0 otherwise
char has_next_sound(void);

char is_recording(void);

// PLAYER

// always starts from beginning
void play_from_memory(void);
void stop_playing_from_memory(void);
char is_player_playing(void);



#endif
