#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "led.h"
#include "protocol.h"
#include "relay.h"

static struct outlet_cmd_response response;

static void usb_send_response(void)
{
	/* mark request handled to avoid further processing */
	Endpoint_ClearSETUP();
	/* write data to endpoint */
	Endpoint_Write_8(response.cmd);
	Endpoint_Write_8(response.err);
	Endpoint_Write_8(response.arg);
	Endpoint_Write_8(0);
	/* send packet */
	Endpoint_ClearIN();
}

static void usb_outlet_count(void)
{
	response.cmd = OUTLET_CMD_COUNT;
	response.err = OUTLET_ERR_OK;
	response.arg = relay_count();
	usb_send_response();
}

static void usb_outlet_status_get(void)
{
	uint8_t outlet = USB_ControlRequest.wValue & 0xff;
	response.cmd = OUTLET_CMD_STATUS_GET;
	response.err = relay_status_get(outlet, &response.arg);
	usb_send_response();
}

static void usb_outlet_group_get(void)
{
	uint8_t outlet = USB_ControlRequest.wValue & 0xff;
	response.cmd = OUTLET_CMD_GROUP_GET;
	response.err = relay_group_get(outlet, &response.arg);
	usb_send_response();
}

static void usb_outlet_status_set(void)
{
	uint8_t outlet = USB_ControlRequest.wValue & 0xff;
	uint8_t status = USB_ControlRequest.wIndex & 0xff;
	response.cmd = OUTLET_CMD_STATUS_SET;
	response.err = relay_status_set(outlet, status);
	response.arg = 0;
	usb_send_response();
}

static void usb_outlet_status_toggle(void)
{
	uint8_t outlet = USB_ControlRequest.wValue & 0xff;
	response.cmd = OUTLET_CMD_STATUS_TOGGLE;
	response.err = relay_status_toggle(outlet);
	response.arg = 0;
	usb_send_response();
}

static void usb_outlet_group_set(void)
{
	uint8_t outlet = USB_ControlRequest.wValue & 0xff;
	uint8_t group = USB_ControlRequest.wIndex & 0xff;
	response.cmd = OUTLET_CMD_GROUP_SET;
	response.err = relay_group_set(outlet, group);
	response.arg = 0;
	usb_send_response();
}

#define REQUEST_TYPE (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQREC_DEVICE)

void EVENT_USB_Device_ControlRequest(void)
{
	if (USB_ControlRequest.bmRequestType == REQUEST_TYPE) {
		led_on();
		switch (USB_ControlRequest.bRequest) {
		case OUTLET_CMD_COUNT:
			usb_outlet_count();
			break;
		case OUTLET_CMD_STATUS_GET:
			usb_outlet_status_get();
			break;
		case OUTLET_CMD_GROUP_GET:
			usb_outlet_group_get();
			break;
		case OUTLET_CMD_STATUS_SET:
			usb_outlet_status_set();
			break;
		case OUTLET_CMD_STATUS_TOGGLE:
			usb_outlet_status_toggle();
			break;
		case OUTLET_CMD_GROUP_SET:
			usb_outlet_group_set();
			break;
		}
		led_off();
	}
}
