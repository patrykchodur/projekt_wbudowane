#include "timelib.h"

#include <stdint.h>
#include "LPC17xx.h"

volatile static unsigned counter;


void timer_init(void) {
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	counter = 0;
}

unsigned millis(void) {
	return counter;
}

unsigned seconds(void) {
	return counter / 1000;
}

void sleep_for_millis(unsigned val) {
	unsigned start = millis();
	while (millis() - start < val)
		__WFI();
}

void sleep_for_seconds(unsigned val) {
	unsigned start = seconds();
	while (seconds() - start < val)
		__WFI();
}



