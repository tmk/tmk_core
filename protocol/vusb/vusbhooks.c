/* Name: main.c
 * Project: hid-mouse, a very simple HID example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 790 2010-05-30 21:00:26Z cs $
 */
#include <avr/wdt.h>
#include "vusb.h"
#include "host.h"
#include "uart.h"
#include "hook.h"
#include "suspend.h"

#define UART_BAUD_RATE 115200

static host_driver_configuration_t driver_configuration = {
  .num_drivers = 1,
  .connection_delay = 1,
  .connection_timeout = 0,
  .drivers = {&vusb_driver}
};

__attribute__((weak))
host_driver_configuration_t* hook_get_driver_configuration(void) {
    return &driver_configuration;
}

void hook_platform_init(void) {
    CLKPR = 0x80, CLKPR = 0;
#ifndef NO_UART
    uart_init(UART_BAUD_RATE);
#endif
}

__attribute__((weak))
void hook_early_init(void) {}

__attribute__((weak))
void hook_late_init(void) {}

 __attribute__((weak))
void hook_usb_suspend_entry(void)
{
#ifdef SLEEP_LED_ENABLE
    sleep_led_enable();
#endif
}

__attribute__((weak))
void hook_usb_suspend_loop(void)
{
    suspend_power_down();
}

__attribute__((weak))
void hook_usb_wakeup(void)
{
#ifdef SLEEP_LED_ENABLE
    sleep_led_disable();
    // NOTE: converters may not accept this
    led_set(host_keyboard_leds());
#endif
}
