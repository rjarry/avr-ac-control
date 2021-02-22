/* Copyright 2021 (c) Robin Jarry */

#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>

#include "protocol.h"

enum usb_string_descriptor_id {
	STRING_ID_LANGUAGE = 0, /**< supported languages ID (must be zero) */
	STRING_ID_MANUFACTURER = 1,
	STRING_ID_PRODUCT = 2,
	STRING_ID_SERIAL = 3,
};

static const USB_Descriptor_Device_t PROGMEM device_descriptor = {
	.Header = {
		.Size = sizeof(USB_Descriptor_Device_t),
		.Type = DTYPE_Device,
	},

	.USBSpecification = VERSION_BCD(1,1,0),
	.Class = USB_CSCP_VendorSpecificClass,
	.SubClass = USB_CSCP_NoDeviceSubclass,
	.Protocol = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID = OUTLET_USB_VENDOR_ID,
	.ProductID = OUTLET_USB_PRODUCT_ID,
	.ReleaseNumber = VERSION_BCD(2,0,0),

	.ManufacturerStrIndex = STRING_ID_MANUFACTURER,
	.ProductStrIndex = STRING_ID_PRODUCT,
	.SerialNumStrIndex = STRING_ID_SERIAL,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS,
};

struct usb_descriptor_config {
	USB_Descriptor_Configuration_Header_t config;
	USB_Descriptor_Interface_t interface;
};

static const struct usb_descriptor_config PROGMEM descriptor_config = {
	.config = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Configuration_Header_t),
			.Type = DTYPE_Configuration,
		},
		.TotalConfigurationSize = sizeof(struct usb_descriptor_config),
		.TotalInterfaces = 1,
		.ConfigurationNumber = 1,
		.ConfigurationStrIndex = NO_DESCRIPTOR,
		.ConfigAttributes = USB_CONFIG_ATTR_RESERVED,
		.MaxPowerConsumption = USB_CONFIG_POWER_MA(500),
	},
	.interface = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Interface_t),
			.Type = DTYPE_Interface,
		},
		.InterfaceNumber = 0,
		.AlternateSetting = 0,
		.TotalEndpoints = 0,
		.Class = USB_CSCP_VendorSpecificClass,
		.SubClass = 0x00,
		.Protocol = 0x00,
		.InterfaceStrIndex = NO_DESCRIPTOR,
	},
};

static const USB_Descriptor_String_t PROGMEM language_string = {
	.Header = {
		.Size = USB_STRING_LEN(1),
		.Type = DTYPE_String,
	},
	.UnicodeString = {LANGUAGE_ID_ENG},
};

static const USB_Descriptor_String_t PROGMEM
	manufacturer_string = USB_STRING_DESCRIPTOR(L"Sugar Sound");

static const USB_Descriptor_String_t PROGMEM
	product_string = USB_STRING_DESCRIPTOR(L"Filtered AC Relay Board");

static const USB_Descriptor_String_t PROGMEM
	serial_string = USB_STRING_DESCRIPTOR(L"23061981");

/*
 * Called by LUFA framework when the host requests a descriptor.
 * Must be an exported symbol (i.e. not static).
 */
uint16_t
CALLBACK_USB_GetDescriptor(const uint16_t value, const uint16_t index,
		const void** const descriptor_address)
{
	const uint8_t descriptor_type = (value >> 8);
	const uint8_t string_id = (value & 0xff);
	uint16_t size = NO_DESCRIPTOR;
	const void* addr = NULL;

	switch (descriptor_type) {
	case DTYPE_Device:
		addr = &device_descriptor;
		size = sizeof(device_descriptor);
		break;
	case DTYPE_Configuration:
		addr = &descriptor_config;
		size = sizeof(descriptor_config);
		break;
	case DTYPE_String:
		switch (string_id)
		{
		case STRING_ID_LANGUAGE:
			addr = &language_string;
			size = pgm_read_byte(&language_string.Header.Size);
			break;
		case STRING_ID_MANUFACTURER:
			addr = &manufacturer_string;
			size = pgm_read_byte(&manufacturer_string.Header.Size);
			break;
		case STRING_ID_PRODUCT:
			addr = &product_string;
			size = pgm_read_byte(&product_string.Header.Size);
			break;
		case STRING_ID_SERIAL:
			addr = &serial_string;
			size = pgm_read_byte(&serial_string.Header.Size);
			break;
		}
		break;
	}

	*descriptor_address = addr;

	return size;
}
