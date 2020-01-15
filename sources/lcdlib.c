#include "lcdlib.h"

// for variable number of draw_quadrangle function arguments
#include <stdarg.h>
// for ascii chars
#include "asciiLib.h"
// various stuff related to board
#include "LPC17xx.h"
// for init_ILI9325 function
#include "LCD_ILI9325.h"
// for lcdWriteReg and lcdConfiguration functions
#include "Open1768_LCD.h"
// touch pannel support
#include "TP_Open1768.h"

volatile static char interrupts_on_touch_enabled;
volatile static char is_holding;

void lcd_init(void) {
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();
	interrupts_on_touch_enabled = 0;
	is_holding = 0;
}

static uint16_t _get_colour(uint16_t red, uint16_t green, uint16_t blue) {
   uint16_t result = blue;
   result |= green << 5;
   result |= red << 11;
   return result;
}


uint16_t get_colour(uint8_t red, uint8_t green, uint8_t blue) {
	float newred = red/255.0 * 31.0;
	float newblue = blue/255.0 * 31.0;
	float newgreen = green/255.0 * 63.0;
	return _get_colour(newred, newgreen, newblue);
}

static int sign (Point p1, Point p2, Point p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static int point_in_triangle (Point pt, Point v1, Point v2, Point v3)
{
    int d1, d2, d3;
    int has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void clear_screen(void) {
	clear_screen_with((uint16_t)0xFFFF);
}

void clear_screen_with(uint16_t colour) {
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteIndex(DATA_RAM);
	for (int itery = 0; itery < DISPLAY_HEIGHT; ++itery) {
		for (int iterx = 0; iterx < DISPLAY_WIDTH; ++iterx) {
			lcdWriteData(colour);
		}
	}
}

void draw_triangle (uint16_t colour, Point p1, Point p2, Point p3) {
	// setting display
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteIndex(DATA_RAM);
	int valid_lcd_index = 1;

	for (int itery = 0; itery < DISPLAY_HEIGHT; ++itery) {
		for (int iterx = 0; iterx < DISPLAY_WIDTH; ++iterx) {
			Point current_point = {iterx, itery};

			if (point_in_triangle(current_point, p1, p2, p3)) {
				// small optimalization
				// if previous point was valid and screen autoincremented
				// in default direction there is no need for setting
				// coursor position
				if (valid_lcd_index) {
					lcdWriteData(colour);
				} else {
					lcdWriteReg(ADRX_RAM, iterx);
					lcdWriteReg(ADRY_RAM, itery);
					lcdWriteReg(DATA_RAM, colour);
					valid_lcd_index = 1;
				}
			} else {
				valid_lcd_index = 0;
			}
		}
	}
}

void draw_quadrangle(uint16_t colour, Point p1, Point p2, Point p3, Point p4) {
	draw_triangle(colour, p1, p2, p3);
	draw_triangle(colour, p3, p4, p1);
}

void draw_rectangle(uint16_t colour, Point p1, Point p2) {
	Point p3 = {p1.x, p2.y};
	Point p4 = {p2.x, p1.y};
	draw_quadrangle(colour, p1, p3, p2, p4);
}

/*
static int point_in_rectangle(Point pt, Point p1, Point p2) {
	int result = 1;
	if (p1.x < p2.x) {
		if (pt.x < p1.x || pt.x > p2.x)
			return 0;
	}
	else {
		if (pt.x < p2.x || pt.x > p1.x)
			return 0;
	}
	if (p1.y < p2.y) {
		if (pt.y < p1.y || pt.y > p2.y)
			return 0;
	}
	else {
		if (pt.y < p2.y || pt.y > p1.y)
			return 0;
	}
	return 1;
}


void draw_rectangle(uint16_t colour, Point p1, Point p2) {
	// setting display
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteIndex(DATA_RAM);
	int valid_lcd_index = 1;

	for (int itery = 0; itery < DISPLAY_HEIGHT; ++itery) {
		for (int iterx = 0; iterx < DISPLAY_WIDTH; ++iterx) {
			Point current_point = {iterx, itery};

			if (point_in_rectangle(current_point, p1, p2)) {
				// small optimalization
				// if previous point was valid and screen autoincremented
				// in default direction there is no need for setting
				// coursor position
				if (valid_lcd_index) {
					lcdWriteData(colour);
				} else {
					lcdWriteReg(ADRX_RAM, iterx);
					lcdWriteReg(ADRY_RAM, itery);
					lcdWriteReg(DATA_RAM, colour);
					valid_lcd_index = 1;
				}
			} else {
				valid_lcd_index = 0;
			}
		}
	}
}
*/

void draw_polygon(uint16_t colour, int number_of_points, ...) {
	Point p1;
	Point p2;
	Point p3;

	va_list argp;
	va_start(argp, number_of_points);

	// guard for bad memory usage
	if (number_of_points < 2)
		return;
	
	p1 = va_arg(argp, Point);
	p2 = va_arg(argp, Point);

	for (int iter = 0; iter < number_of_points - 2; ++iter) {
		p3 = va_arg(argp, Point);
		draw_triangle(colour, p1, p2, p3);
		p2 = p3;
	}


	va_end(argp);
}

static int point_in_circle(Point current_point, Point center, int r) {
	return ((current_point.x - center.x)*(current_point.x - center.x) + 
			(current_point.y - center.y)*(current_point.y - center.y)
			< r * r);
}




// this function is implemented sepparately for convinience
void draw_circle(uint16_t colour, Point center, int r) {
	// setting display
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteIndex(DATA_RAM);
	int valid_lcd_index = 1;

	for (int itery = 0; itery < DISPLAY_HEIGHT; ++itery) {
		for (int iterx = 0; iterx < DISPLAY_WIDTH; ++iterx) {
			Point current_point = {iterx, itery};

			if (point_in_circle(current_point, center, r)) {
				// small optimalization
				// if previous point was valid and screen autoincremented
				// in default direction there is no need for setting
				// coursor position
				if (valid_lcd_index) {
					lcdWriteData(colour);
				} else {
					lcdWriteReg(ADRX_RAM, iterx);
					lcdWriteReg(ADRY_RAM, itery);
					lcdWriteReg(DATA_RAM, colour);
					valid_lcd_index = 1;
				}
			} else {
				valid_lcd_index = 0;
			}
		}
	}
}

static void draw_point(uint16_t colour, Point p) {
	lcdWriteReg(ADRX_RAM, p.x);
	lcdWriteReg(ADRY_RAM, p.y);
	lcdWriteReg(DATA_RAM, colour);
}

// Point p is top left corner
void draw_char(uint16_t colour, unsigned char ascii, Point p, int rotate) {
	unsigned char buffer[16];
	GetASCIICode(0, buffer, ascii);
	// height of the letter
	for (int outer_iter = 0; outer_iter < 16; ++outer_iter) {
		// width of the letter
		for (int inner_iter = 0; inner_iter < 8; ++inner_iter) {
			if (inner_iter & buffer[outer_iter]) {
				Point to_draw;
				switch (rotate) {
					case 0:
						to_draw.x = p.x + inner_iter;
						to_draw.y = p.y + outer_iter;
						break;
					case 1:
						to_draw.x = p.x - outer_iter;
						to_draw.y = p.y + inner_iter;
						break;
					case 2:
						to_draw.x = p.x - inner_iter;
						to_draw.y = p.y - outer_iter;
						break;
					case 3:
						to_draw.x = p.x + outer_iter;
						to_draw.y = p.y - inner_iter;
						break;
					default:
						return;
				}
				draw_point(colour, to_draw);
			}
		}
	}
}

void draw_string(uint16_t colour, Point p, const char* str, int rotate, int fold) {
	while (*str) {
		draw_char(colour, *str, p, rotate);

		switch (rotate) {
			case 0:
				p.x += 8;
				if (p.x > DISPLAY_WIDTH - 16 && fold) {
					p.x = 8;
					p.y += 20;
				}
				break;
			case 1:
				p.y += 8;
				if (p.y > DISPLAY_HEIGHT - 16 && fold) {
					p.y = 8;
					p.x -= 20;
				}
				break;
			case 2:
				p.x -= 8;
				if (p.x < 16 && fold) {
					p.x = DISPLAY_WIDTH - 8;
					p.y -= 20;
				}
				break;
			case 3:
				p.y -= 8;
				if (p.y < 16 && fold) {
					p.y = DISPLAY_HEIGHT - 16;
					p.x += 20;
				}
				break;
			default:
				return;
		}
		++str;
	}
}

#define FRAME_SMALL (150)
#define FRAME_LARGE (500)
#define TP_MAX (4096)


Point get_position(void) {
	int x, y;
	Point result;
	if (interrupts_on_touch_enabled) {
		NVIC_DisableIRQ(EINT3_IRQn);
	}
	// NOTE x and y are reversed!
	touchpanelGetXY(&x, &y);
	if (interrupts_on_touch_enabled) {
		LPC_GPIOINT->IO0IntClr = 1 << 19;
		NVIC_ClearPendingIRQ(EINT3_IRQn);
		NVIC_EnableIRQ(EINT3_IRQn);
	}

	result.x = DISPLAY_WIDTH*(y - FRAME_SMALL)/(TP_MAX - 2 * FRAME_SMALL);
	result.y = DISPLAY_HEIGHT*(x - FRAME_SMALL)/(TP_MAX - FRAME_SMALL - FRAME_LARGE);
	result.x = result.x < 0 ? 0 : result.x;
	result.x = result.x >= DISPLAY_WIDTH ? DISPLAY_WIDTH - 1 : result.x;
	result.y = result.y < 0 ? 0 : result.y;
	result.y = result.y >= DISPLAY_HEIGHT ? DISPLAY_HEIGHT - 1 : result.y;
	return result;
}

Point get_position_with_precision(int iterations) {
	if (iterations < 1)
		iterations = 1;
	Point result = {0, 0};
	for (int iter = 0; iter < iterations; iter++) {
		Point tmp = get_position();
		result.x += tmp.x;
		result.y += tmp.y;
	}
	result.x /= iterations;
	result.y /= iterations;
	return result;
}


static void (*handler)(void);

void set_interrupt_on_touch(void (*fp)(void)) {
	handler = fp;
	is_holding = 0;

	// set gpio interrupt for rising edge on pin 19
	LPC_GPIOINT->IO0IntEnR = 1 << 19;

	// clear pending interrupts on pin 19
	LPC_GPIOINT->IO0IntClr = 1 << 19;
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

	interrupts_on_touch_enabled = 1;

}

void disable_interrupt_on_touch(void) {
	NVIC_DisableIRQ(EINT3_IRQn);
	interrupts_on_touch_enabled = 0;
	is_holding = 0;
}


char is_touchpanel_pressed(void) {
	return is_holding;
}

void EINT3_IRQHandler(void) {
	is_holding = !is_holding;
	handler();
	if (is_holding) {
		// set gpio interrupt for falling edge on pin 19
		LPC_GPIOINT->IO0IntEnF = 1 << 19;
	}
	else {
		// set gpio interrupt for rising edge on pin 19
		LPC_GPIOINT->IO0IntEnR = 1 << 19;
	}

	LPC_GPIOINT->IO0IntClr = 1 << 19;
}










