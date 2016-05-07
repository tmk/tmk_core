/*
 * (c) 2015 flabberast <s3+flabbergast@sdfeu.org>
 *
 * Based on the following work:
 *  - Guillaume Duc's raw hid example (MIT License)
 *    https://github.com/guiduc/usb-hid-chibios-example
 *  - PJRC Teensy examples (MIT License)
 *    https://www.pjrc.com/teensy/usb_keyboard.html
 *  - hasu's TMK keyboard code (GPL v2 and some code Modified BSD)
 *    https://github.com/tmk/tmk_keyboard/
 *  - ChibiOS demo code (Apache 2.0 License)
 *    http://www.chibios.org
 *
 * Since some GPL'd code is used, this work is licensed under
 * GPL v2 or later.
 */

#include "ch.h"
#include "hal.h"

#include "usb_main.h"

/* TMK includes */
#include "report.h"
#include "host.h"
#include "host_driver.h"
#include "keyboard.h"
#include "action.h"
#include "action_util.h"
#include "mousekey.h"
#include "led.h"
#include "sendchar.h"
#include "debug.h"
#ifdef SLEEP_LED_ENABLE
#include "sleep_led.h"
#endif
#include "suspend.h"
#include "hook.h"
#include "chibios.h"


/* -------------------------
 *   TMK host driver defs
 * -------------------------
 */

/* declarations */
void init_driver(void);
bool is_usb_connected(void);
bool is_usb_suspended(void);
bool is_remote_wakeup_supported(void);
static void usb_poll(void) {}

uint8_t keyboard_leds(void);
void send_keyboard(report_keyboard_t *report);
void send_mouse(report_mouse_t *report);
void send_system(uint16_t data);
void send_consumer(uint16_t data);

/* host struct */
host_driver_t chibios_usb_driver = {
  init_driver,
  is_usb_connected,
  is_usb_suspended,
  usb_poll,
  is_remote_wakeup_supported,
  send_remote_wakeup,
  keyboard_leds,
  send_keyboard,
  send_mouse,
  send_system,
  send_consumer
};

host_driver_configuration_t chibios_driver_configuration = {
  .num_drivers = 1,
  .connection_delay = 50,
  .connection_timeout = 0,
  .drivers = {&chibios_usb_driver}
};

/* Default hooks definitions. */
__attribute__((weak))
void hook_early_init(void) {}

__attribute__((weak))
void hook_late_init(void) {}

__attribute__((weak))
void hook_usb_suspend_loop(void) {
  /* Do this in the suspended state */
  suspend_power_down(); // on AVR this deep sleeps for 15ms
}

__attribute__((weak))
host_driver_configuration_t* hook_get_driver_configuration(void) {
    return &chibios_driver_configuration;
}

void hook_platform_init(void) {
  /* ChibiOS/RT init */
  halInit();
  chSysInit();
  init_printf(NULL,sendchar_pf);
}

/* TESTING
 * Amber LED blinker thread, times are in milliseconds.
 */
/* set this variable to non-zero anywhere to blink once */
// uint8_t blinkLed = 0;
// static THD_WORKING_AREA(waBlinkerThread, 128);
// static THD_FUNCTION(blinkerThread, arg) {
//   (void)arg;
//   chRegSetThreadName("blinkOrange");
//   while(true) {
//     if(blinkLed) {
//       blinkLed = 0;
//       palSetPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
//       chThdSleepMilliseconds(100);
//       palClearPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
//     }
//     chThdSleepMilliseconds(100);
//   }
// }


