#ifndef _OUTLET_DEVICE_PROTOCOL_H
#define _OUTLET_DEVICE_PROTOCOL_H

#include <stdint.h>

#define OUTLET_USB_VENDOR_ID 0x2306
#define OUTLET_USB_PRODUCT_ID 0x1107

#define OUTLET_CMD_INVAL 0x00
#define OUTLET_CMD_COUNT 0x01
#define OUTLET_CMD_STATUS_GET 0x02
#define OUTLET_CMD_STATUS_SET 0x03
#define OUTLET_CMD_STATUS_TOGGLE 0x04
#define OUTLET_CMD_GROUP_GET 0x05
#define OUTLET_CMD_GROUP_SET 0x06
#define OUTLET_CMD_MAX 0x07

#define OUTLET_ERR_OK 0x00
#define OUTLET_ERR_CMD 0x01
#define OUTLET_ERR_ARG 0x02

struct outlet_cmd_response {
	uint8_t cmd;
	uint8_t err;
	uint8_t arg;
	uint8_t __unused;  /* reserved for future use */
};

#endif
