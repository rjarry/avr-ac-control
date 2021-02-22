#ifndef _LUFA_CONFIG_H
#define _LUFA_CONFIG_H

#define USE_STATIC_OPTIONS ( \
		/* full speed by default */ \
		USB_DEVICE_OPT_FULLSPEED | \
		/* enable 3.3V regulator */ \
		USB_OPT_REG_ENABLED | \
		/* let LUFA lib handle the hi-freq USB clock */ \
		USB_OPT_AUTO_PLL \
	)
#define USB_DEVICE_ONLY
/* store USB descriptors in flash memory instead of RAM */
#define USE_FLASH_DESCRIPTORS
/* reduce binary size */
#define FIXED_CONTROL_ENDPOINT_SIZE 8
/* store USB_DeviceState in GPIOR0 register for faster access */
#define DEVICE_STATE_AS_GPIOR 0
/* only one config, reduce binary size */
#define FIXED_NUM_CONFIGURATIONS 1
/* no other endpoints, this is enough for our use case */
#define CONTROL_ONLY_DEVICE
/* USB control reports are handled only by interrupts */
#define INTERRUPT_CONTROL_ENDPOINT

#endif
