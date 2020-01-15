
#include "lcdlib.h"
#include "soundlib.h"
#include "timelib.h"
#include "piano.h"
#include "player_recorder.h"


// action on touchscreen occured recently
volatile char action_on_touchscreen;

void interrupt_handler(void) {
	action_on_touchscreen = 1;
}

void main_setup(void) {
	lcd_init();
	dac_init();
	timer_init();
	player_recorder_init();

	piano_init();
	draw_piano();

	action_on_touchscreen = 0;

	set_interrupt_on_touch(interrupt_handler);
}

void main_loop(void) {
	while (!action_on_touchscreen)
		sleep_for_millis(10);
	Point pt = get_position_with_precision(5);
	piano_action(pt, is_touchpanel_pressed());
}


int main(void) {
	main_setup();
	while(1) {
		main_loop();
		sleep_for_millis(10);
	}
}
