#include <avr/io.h>

#include "led.h"

#define LED _BV(6)

void led_init(void)
{
	/* configure D6 port as output (on-board led) */
	DDRD |= LED;
	led_off();
}

void led_on(void)
{
	PORTD |= LED;
}

void led_off(void)
{
	PORTD &= ~LED;
}
