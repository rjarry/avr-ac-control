#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "button.h"
#include "relay.h"
#include "led.h"

int main(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division when in sleep mode */
	clock_prescale_set(clock_div_1);
	set_sleep_mode(SLEEP_MODE_IDLE);

	/* Hardware Initialization */
	led_init();
	led_on();
	//button_init();
	relay_init();
	USB_Init();

	GlobalInterruptEnable();
	led_off();

	for (;;) {
		sleep_mode();
	}

	/* unreached */
	return 0;
}
