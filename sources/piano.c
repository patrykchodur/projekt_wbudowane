#include "piano.h"

// drawing and Point
#include "lcdlib.h"
// playing sounds
#include "soundlib.h"

#include "player_recorder.h"

typedef enum {
	BLACK,
	WHITE,
} KeyType;


typedef struct {
	Point point1;
	Point point2;
	KeyType type;
	Note sound;
} PianoKey;

static PianoKey[8] white_keys;
static PianoKey[5] black_keys;

typedef struct {
	Point point1;
	Point point2;
} Button;

typedef struct {
	Point point1;
	Point point2;
	char switched_on;
} Switch;
	

static Switch play_stop_button;
static Switch record_stop_recording_button;
static Button erase_recorded_button;
static Button increase_volume_button;
static Button decrease_volume_button;

static int volume;


void piano_init(void) {
	//white_keys;
	Point start = {20, 20};
	int width_x = 180;
	int height_y = 25;
	int empty_y = 5;
	for (int iter = 0; iter < sizeof(white_keys)/sizeof(PianoKey); ++iter) {

		white_keys[iter] = {{start.x, start.y},
			{start.x + width_x, start.y + height_y + iter * (height_y + empty_y)}, WHITE, NO_NOTE};
	}

	white_keys[0].sound = C;
	white_keys[1].sound = D;
	white_keys[2].sound = E;
	white_keys[3].sound = F;
	white_keys[4].sound = G;
	white_keys[5].sound = A;
	white_keys[6].sound = B;
	white_keys[7].sound = C2;

	//black keys
	black_keys[0] = {{100, 35}, {200, 55}, BLACK, Cs};
	black_keys[1] = {{100, 70}, {200, 90}, BLACK, Ds};
	black_keys[2] = {{100, 130}, {200, 150}, BLACK, Fs};
	black_keys[3] = {{100, 160}, {200, 180}, BLACK, Gs};
	black_keys[4] = {{100, 190}, {200, 210}, BLACK, As};


	// buttons
	// TODO set propper values
	play_stop_button = {{50, 50}, {100, 100}, 0};
	record_stop_recording_button = {{50, 50}, {100, 100}, 0};
	erase_recorded_button = {{50, 50}, {100, 100}};
	increase_volume_button = {{50, 50}, {100, 100}};
	decrease_volume_button = {{50, 50}, {100, 100}};

	volume = 150;
	set_volume(volume);

}

void draw_play_stop_button(void) {
	// default state
	uint16_t black = get_colour(0, 0, 0);
	uint16_t white = get_colour(255, 255, 255);
	if (!play_stop_button.switched_on) {
		draw_rectangle(black, play_stop_button.point1, play_stop_button.point2);
		// TODO play button triangle
		Point p1[3] = {{50, 50}, {60, 60}, {70, 70}};
		draw_triangle(white, p1[0], p1[1], p[2]);
	}
	else {
		draw_rectangle(black, play_stop_button.point1, play_stop_button.point2);
		// TODO square in stop
		Point p1 = {50, 50};
		Point p2 = {60, 60};
		draw_rectangle(white, p1, p2);
	}
}

void draw_record_stop_recording_button(void){
	uint16_t black = get_colour(0, 0, 0);
	uint16_t white = get_colour(255, 255, 255);

	if (!record_stop_recording_button.switched_on) {
		draw_rectangle(black, record_stop_recording_button.point1, record_stop_recording_button.point2);
		// TODO record button circle
		Point center = {50, 50};
		draw_circle(white, center, r);
	}
	else {
		draw_rectangle(black, record_stop_recording_button.point1, record_stop_recording_button.point2);
		// TODO square in stop
		Point p1 = {50, 50};
		Point p2 = {60, 60};
		draw_rectangle(white, p1, p2);
	}
}

void draw_erase_recorded_button(void){
	uint16_t black = get_colour(0, 0, 0);
	uint16_t white = get_colour(255, 255, 255);
	draw_rectangle(black, erase_recorded_button.point1, erase_recorded_button.point2);

	// TODO draw trash can
	Point can[2] = {{50, 50}, {60, 60}};
	Point top[2] = {{50, 50}, {60, 60}};
	Point handle[2] = {{50, 50}, {60, 60}};

}

void draw_increase_volume_button(void){
	uint16_t black = get_colour(0, 0, 0);
	uint16_t white = get_colour(255, 255, 255);
	draw_rectangle(black, increase_volume_button.point1, increase_volume_button.point2);
	
	// TODO drawing plus sign
	Point p1 = {50, 50};
	draw_char(white, '+', p1, 1);
}
void draw_decrease_volume_button(void){
	uint16_t black = get_colour(0, 0, 0);
	uint16_t white = get_colour(255, 255, 255);

	draw_rectangle(black, decrease_volume_button.point1, decrease_volume_button.point2);
	
	// TODO drawing plus sign
	Point p1 = {50, 50};
	draw_char(white, '-', p1, 1);

}

void draw_piano(void) {
	uint16_t white = get_colour(255, 255, 255);
	for (int iter = 0; iter < sizeof(white_keys)/sizeof(PianoKey); ++iter) {
		draw_rectangle(white, white_keys[iter].point1, white_keys[iter].point2);
	}

	uint16_t black = get_colour(0, 0, 0);
	for (int iter = 0; iter < sizeof(black_keys)/sizeof(PianoKey); ++iter) {
		draw_rectangle(black, black_keys[iter].point1, black_keys[iter].point2);
	}
	

	// draw buttons
	draw_play_stop_button();
	draw_record_stop_recording_button();
	draw_erase_recorded_button();
	draw_increase_volume_button();
	draw_decrease_volume_button();

}

static void increase_volume(void) {
	volume *= 2;
	volume = volume < 1024 ? volume : 1023;
	set_volume(volume);
}

static void decrease_volume(void) {
	volume /= 2;
	volume = volume < 2 ? 2 : volume;
	set_volume(volume);
}

static Note get_note_from_point(Point pt) {
	for (int iter = 0; iter < sizeof(black_keys/sizeof(PianoKey)); ++iter) {
		if (inside_rect(pt, black_keys[iter].point1, black_keys[iter].point2))
			return black_keys[iter].sound;
	}
	for (int iter = 0; iter < sizeof(white_keys/sizeof(PianoKey)); ++iter) {
		if (inside_rect(pt, white_keys[iter].point1, white_keys[iter].point2))
			return white_keys[iter].sound;
	}
	return NO_NOTE;
}

static char is_key_playing(void) {
	return is_playing() && !is_player_playing();
}

static void stop_key_playing(void) {
	if (is_key_playing())
		stop_sound();
	if (record_stop_recording_button.switched_on)
		end_record();
}

void piano_action(Point pt, char pressed) {
	// button released
	if (is_player_playing()){
		// check stop button
		if (pressed && inside_rect(pt, play_stop_button.point1, play_stop_button.point2)) {
			play_stop_button.switched_on = 0;
			draw_play_stop_button();
			stop_playing_from_memory();
			return;
		}
		return;
	}
	if (!pressed && is_key_playing()) {
		stop_key_playing();
		return;
	}
	if (pressed) {
		Note tmp = get_note_from_point(pt);
		if (tmp != NO_NOTE) {
			start_note(tmp);
			if (record_stop_recording_button.switched_on)
				start_record(note_to_frequency(tmp));
			return;
		}

		// check buttons
		if (inside_rect(pt, increase_volume_button.point1, increase_volume_button.point2)) {
			increase_volume();
			return;
		}
		if (inside_rect(pt, decrease_volume_button.point1, decrease_volume_button.point2)) {
			decrease_volume();
			return;
		}
		if (!play_stop_button.switched_on && inside_rect(pt, play_stop_button.point1, play_stop_button.point2)) {
			play_from_memory();
			draw_play_stop_button();
			return;
		}
		if (!record_stop_recording_button.switched_on && inside_rect(pt, record_stop_recording_button.point1, record_stop_recording_button.point2)) {
			record_stop_recording_button.switched_on = 1;
			draw_record_stop_recording_button();
			return;
		}
		if (inside_rect(pt, erase_recorded_button.point1, erase_recorded_button.point2)) {
			erase_saved();
			return;
		}

	}
}


