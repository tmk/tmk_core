/*
Bluefruit Protocol for TMK firmware
Author: Benjamin Gould, 2013
Based on code Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "serial.h"
#include "keyboard.h"
#include "usb.h"
#include "host.h"
#include "timer.h"
#include "print.h"
#include "debug.h"
#include "sendchar.h"
#include "suspend.h"
#include "bluefruit.h"
#include "pjrc.h"

#define CPU_PRESCALE(n)    (CLKPR = 0x80, CLKPR = (n))

#define HOST_DRIVER_NOT_SET     0
#define BLUEFRUIT_HOST_DRIVER   1
#define PJRC_HOST_DRIVER        2

void hook_platform_init(void) {

    CPU_PRESCALE(0);

    DDRD  = _BV(PD5);
    DDRB  = _BV(PB0);
    
    PORTD = _BV(PD5);
    PORTB = _BV(PB0);

    print_set_sendchar(sendchar);
}

static host_driver_configuration_t driver_configuration = {
  .num_drivers = 2,
  .connection_delay = 50,
  .connection_timeout  = 2000,
  .drivers = {&pjrc_driver, &bluefruit_driver}
};

__attribute__((weak))
host_driver_configuration_t* hook_get_driver_configuration(void) {
    return &driver_configuration;
}

__attribute__((weak))
void hook_early_init(void) {}

__attribute__((weak))
void hook_late_init(void)
{
    // Enable leds based on the selected driver
    if(host_get_driver() == &bluefruit_driver) {
        // Green
        PORTD = ~_BV(PD5);
    }
    else {
        // Yellow
        PORTB = ~_BV(PB0);
    }
}

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
