/* Copyright 2021 (c) Robin Jarry */

#ifndef _RELAY_H
#define _RELAY_H

#include <stdint.h>

void relay_init(void);
uint8_t relay_count(void);
uint8_t relay_status_get(uint8_t relay, uint8_t *status);
uint8_t relay_status_set(uint8_t relay, uint8_t status);
uint8_t relay_status_toggle(uint8_t relay);
uint8_t relay_group_get(uint8_t relay, uint8_t *group);
uint8_t relay_group_set(uint8_t relay, uint8_t group);

#endif
