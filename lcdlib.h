/*
 * Library for lcd and touch pannel support
 * for LPC1768 board
 */

#ifndef LCDLIB_H
#define LCDLIB_H

// for uint16_t
#include <stdint.h>

// Point struct
#include "common.h"

// display properties
#define DISPLAY_WIDTH (240)
#define DISPLAY_HEIGHT (320)

// x in <0, DISPLAY_WIDTH)
// y in <0, DISPLAY_HEIGHT)


// need to be called at the beginning of the program
void lcd_init(void);

// drawing on lcd
uint16_t get_colour(uint8_t red, uint8_t green, uint8_t blue);
void clear_screen_with(uint16_t colour);
void clear_screen(void); // using white colour
void draw_triangle(uint16_t colour, Point p1, Point p2, Point p3);
void draw_quardangle(uint16_t colour, Point p1, Point p2,
						Point p3, Point p4);
void draw_rectangle(uint16_t colour, Point p1, Point p2);
void draw_polygon(uint16_t colour, int number_of_points, ...); // Point objects
void draw_circle(uint16_t colour, Point center, int r);
// rotate - rotate by rotate * 90^o
void draw_char(uint16_t colour, unsigned char ascii, Point p, int rotate);
// fold - fold if runs out of screen
void draw_string(uint16_t colour, Point p, const char* str, int rotate, int fold);


// touch pannel
// this function return values corresponding to
// pixel position on lcd
Point get_position(void);
Point get_position_with_preciton(int iterations);

// this function sets handler for interrupts caused by
// touching the screen
// fp is not guaranteed to be called only twice per 1 press
void set_interrupt_on_touch(void (*fp)(void));
void disable_interrupt_on_touch(void);

char is_touchpanel_pressed(void);


#endif // LCDLIB_H
