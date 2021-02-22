/* Copyright 2021 (c) Robin Jarry */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "button.h"
#include "relay.h"

/* button pin numbers in port B */
static const uint8_t pins[4] = {
	_BV(PORTB2), _BV(PORTB3), _BV(PORTB4), _BV(PORTB5)};
/* debounced status per button */
static uint8_t debounced[4];

void button_init(void)
{
	for (uint8_t i = 0; i < sizeof(pins); i++) {
		/* configure B2, B3, B4, B5 pins as inputs */
		DDRB &= ~pins[i];
	}
	/* div = 256, the 8bit timer overflows every ~4ms @16MHz */
	TCCR0B = _BV(CS02);
	/* start timer */
	TIMSK0 |= (1 << TOIE0);
}

#define PRESS_MS 8 /* stable time before registering pressed */
#define RELEASE_MS 128 /* stable time before registering released */
#define POLL_MS 4 /* button pin polling period */
/* per button press counters */
static uint8_t counters[4] = {
	PRESS_MS / POLL_MS,
	PRESS_MS / POLL_MS,
	PRESS_MS / POLL_MS,
	PRESS_MS / POLL_MS,
};

ISR(TIMER1_OVF_vect)
{
	uint8_t raw_state;

	for (uint8_t i = 0; i < sizeof(pins); i++) {
		raw_state = PINB & pins[i];

		if (raw_state == debounced[i]) {
			/* reset counter */
			if (debounced[i]) {
				counters[i] = RELEASE_MS / POLL_MS;
			} else {
				counters[i] = PRESS_MS / POLL_MS;
			}
		} else {
			/* button has changed, wait for stable state */
			if (--counters[i] == 0) {
				/* timer expired accept change and reset */
				debounced[i] = raw_state;
				relay_status_toggle(i);
				if (debounced[i]) {
					counters[i] = RELEASE_MS / POLL_MS;
				} else {
					counters[i] = PRESS_MS / POLL_MS;
				}
			}
		}
	}
}
