/* Copyright 2021 (c) Robin Jarry */

#include <avr/eeprom.h>
#include <avr/io.h>

#include "protocol.h"
#include "relay.h"

/* relay pin numbers in port D */
static const uint8_t pins[4] = {_BV(0), _BV(1), _BV(2), _BV(3)};
#define RELAY_COUNT (sizeof(pins))
/* relay groups saved in eeprom to restore at power up
 * by default each relay is in its own group */
static uint8_t groups_save[4] EEMEM = {0, 1, 2, 3};
/* relay groups in ram used at runtime */
static uint8_t groups[4];

void relay_init(void)
{
	uint8_t mask = 0;
	for (uint8_t r = 0; r < RELAY_COUNT; r++) {
		mask |= pins[r];
	}
	/* configure pins as outputs */
	DDRD |= mask;
	/* turn off all relays */
	PORTD &= ~mask;
	/* get saved groups from eeprom at power up */
	eeprom_read_block(groups, groups_save, sizeof(groups));
}

uint8_t relay_count(void)
{
	return RELAY_COUNT;
}

uint8_t relay_status_get(uint8_t relay, uint8_t *status)
{
	if (relay >= RELAY_COUNT)
		return OUTLET_ERR_ARG;

	*status = !!(PIND & pins[relay]);

	return OUTLET_ERR_OK;
}

uint8_t relay_status_set(uint8_t relay, uint8_t status)
{
	uint8_t mask = 0;

	if (relay >= RELAY_COUNT)
		return OUTLET_ERR_ARG;

	/* change status of all relays that are in the same group */
	for (uint8_t r = 0; r < RELAY_COUNT; r++) {
		if (groups[r] == groups[relay]) {
			mask |= pins[r];
		}
	}
	if (status) {
		PORTD |= mask;
	} else {
		PORTD &= ~mask;
	}

	return OUTLET_ERR_OK;
}

uint8_t relay_status_toggle(uint8_t relay)
{
	uint8_t err, status;

	status = 0;
	err = relay_status_get(relay, &status);
	if (err)
		return err;

	return relay_status_set(relay, !status);
}

uint8_t relay_group_get(uint8_t relay, uint8_t *group)
{
	if (relay >= RELAY_COUNT)
		return OUTLET_ERR_ARG;

	*group = groups[relay];

	return OUTLET_ERR_OK;
}

uint8_t relay_group_set(uint8_t relay, uint8_t group)
{
	uint8_t status, err;

	if (relay >= RELAY_COUNT)
		return OUTLET_ERR_ARG;

	status = 0;

	for (uint8_t r = 0; r < RELAY_COUNT; r++) {
		if (groups[r] == group) {
			err = relay_status_get(r, &status);
			if (err)
				return err;
			break;
		}
	}

	groups[relay] = group;
	eeprom_update_byte(&groups_save[relay], group);

	return relay_status_set(relay, status);
}
