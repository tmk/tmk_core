/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

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
#include "usb_keyboard.h"
#include "usb_mouse.h"
#include "usb_extra.h"
#include "host_driver.h"
#include "pjrc.h"
#include "hook.h"
#include "suspend.h"
#include "sendchar.h"
#include "print.h"
#ifdef SLEEP_LED_ENABLE
#include "sleep_led.h"
#endif


/*------------------------------------------------------------------*
 * Host driver
 *------------------------------------------------------------------*/
static uint8_t keyboard_leds(void);
static void send_keyboard(report_keyboard_t *report);
static void send_mouse(report_mouse_t *report);
static void send_system(uint16_t data);
static void send_consumer(uint16_t data);

static bool is_connected(void) { return usb_configured(); }
static bool is_suspended(void) { return suspend; }
static void poll_usb(void) {}
static bool is_remote_wakeup_supported(void) {return remote_wakeup;}

static host_driver_t driver = {
        usb_init,
        is_connected,
        is_suspended,
        poll_usb,
        is_remote_wakeup_supported,
        usb_remote_wakeup,
        keyboard_leds,
        send_keyboard,
        send_mouse,
        send_system,
        send_consumer
};

static host_driver_configuration_t driver_configuration = {
  .num_drivers = 1,
  .connection_delay = 50,
  .drivers = {&driver}
};

host_driver_t *pjrc_driver(void)
{
    return &driver;
}

void protocol_early_init(void) {
    print_set_sendchar(sendchar);
}


__attribute__((weak))
host_driver_configuration_t* hook_get_driver_configuration(void) {
    return &driver_configuration;
}

static uint8_t keyboard_leds(void) {
    return usb_keyboard_leds;
}

static void send_keyboard(report_keyboard_t *report)
{
    usb_keyboard_send_report(report);
}

static void send_mouse(report_mouse_t *report)
{
#ifdef MOUSE_ENABLE
    usb_mouse_send(report->x, report->y, report->v, report->h, report->buttons);
#endif
}

static void send_system(uint16_t data)
{
#ifdef EXTRAKEY_ENABLE
    usb_extra_system_send(data);
#endif
}

static void send_consumer(uint16_t data)
{
#ifdef EXTRAKEY_ENABLE
    usb_extra_consumer_send(data);
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
