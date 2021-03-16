/* Copyright 2021 (c) Robin Jarry */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "button.h"
#include "relay.h"
#include "led.h"

/* button pin numbers in port B */
static const uint8_t pins[4] = {_BV(0), _BV(1), _BV(2), _BV(3)};
static const uint8_t pinmask = _BV(0) | _BV(1) | _BV(2) | _BV(3);

void button_init(void)
{
	/* configure pins as inputs */
	DDRB &= ~pinmask;
	/* enable pull-up resistors */
	PORTB |= pinmask;
	/* enable pin-change interrupt routine for pins B0 B1 B2 B3 */
	PCMSK0 |= _BV(PCINT0) | _BV(PCINT1) | _BV(PCINT2) | _BV(PCINT3);
	/* enable pin-change interrupts */
	PCICR |= _BV(PCIE0);
	/* div = 64, the 8bit timer overflows every ~1ms @16MHz */
	TCCR0B = _BV(CS01) | _BV(CS00);
	/* timer *not* started */
}

ISR(PCINT0_vect)
{
	/* disable pin-change interrupts */
	PCICR &= ~_BV(PCIE0);
	/* start timer to debounce buttons */
	TIMSK0 |= _BV(TOIE0);
	led_on();
	_delay_ms(200);
	led_off();
}

#define PRESS_MS 10 /* stable time before registering pressed */
#define RELEASE_MS 200 /* stable time before registering released */
/* per button press counters */
static volatile uint8_t counters[4] = {PRESS_MS, PRESS_MS, PRESS_MS, PRESS_MS};
/* debounced status per button */
static volatile uint8_t debounced[4] = {0, 0, 0, 0};

ISR(TIMER0_OVF_vect)
{
	uint8_t raw_state, all_released = 1;

	for (uint8_t i = 0; i < sizeof(pins); i++) {
		/* buttons are active=low (pull-up resistors) */
		raw_state = !(PINB & pins[i]);

		if (raw_state == debounced[i]) {
			/* reset counter */
			if (debounced[i]) {
				counters[i] = RELEASE_MS;
				all_released = 0;
			} else {
				counters[i] = PRESS_MS;
			}
		} else {
			/* button has changed, wait for stable state */
			if (--counters[i] == 0) {
				/* timer expired accept change and reset */
				debounced[i] = raw_state;
				if (debounced[i]) {
					relay_status_toggle(i);
					counters[i] = RELEASE_MS;
				} else {
					counters[i] = PRESS_MS;
					all_released = 0;
				}
			} else {
				all_released = 0;
			}
		}
	}
	if (all_released) {
		/* enable pin-change interrupts */
		PCICR |= _BV(PCIE0);
		/* stop timer */
		TIMSK0 &= ~_BV(TOIE0);
	}
}
