#include "HIDKeyboard.h"
#include "host.h"
#include "host_driver.h"
#include "mbed_driver.h"

HIDKeyboard keyboard;


/* Host driver */
static uint8_t keyboard_leds(void);
static void send_keyboard(report_keyboard_t *report);
static void send_mouse(report_mouse_t *report);
static void send_system(uint16_t data);
static void send_consumer(uint16_t data);
static void init_driver(void) {}
static bool is_connected(void) {return keyboard.configured();}
// The HIDKeyboard does not seem to expose the suspend state
static bool is_suspended(void) {return false;}
static void poll_usb(void) {}
static bool is_remote_wakeup_supported() {return false;}
static void send_remote_wakeup(void) {}

host_driver_t mbed_driver = {
    init_driver,
    is_connected,
    is_suspended,
    poll_usb,
    is_remote_wakeup_supported,
    send_remote_wakeup,
    keyboard_leds,
    send_keyboard,
    send_mouse,
    send_system,
    send_consumer
};


static uint8_t keyboard_leds(void)
{
    return keyboard.leds();
}
static void send_keyboard(report_keyboard_t *report)
{
    keyboard.sendReport(*report);
}
static void send_mouse(report_mouse_t *report)
{
}
static void send_system(uint16_t data)
{
}
static void send_consumer(uint16_t data)
{
}
