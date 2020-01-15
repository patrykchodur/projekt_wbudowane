/*
 * very small library for time measurement on LPC1768 using SysTick
 * 1 ms resolution
 */

#ifndef TIMELIB_H
#define TIMELIB_H

// no special order of execution
void timer_init(void);
// milliseconds since beginning of the program
unsigned millis(void);
// seconds since beginning of the program
unsigned seconds(void);
// these functions make current thread sleep in wfi
void sleep_for_millis(unsigned);
void sleep_for_seconds(unsigned);




#endif // TIMELIB_H
