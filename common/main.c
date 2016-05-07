/*
Copyright 2016 Fred Sundvik

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

#include <stddef.h>
#include "hook.h"
#include "wait.h"
#include "action_util.h"
#include "sleep_led.h"
#include "suspend.h"
#include "print.h"
#include "timer.h"
#include "host.h"
#ifdef MOUSEKEY_ENABLE
#include "mousekey.h"
#endif

int main(void) {
  hook_platform_init();
  host_driver_configuration_t* dc = hook_get_driver_configuration();

  for (int i=0; i < dc->num_drivers; i++) {
    dc->drivers[i]->init();
    // Only initialize the first driver if a connection timeout is specified
    if (dc->connection_timeout > 0) {
        break;
    }
  }

  hook_early_init();
  keyboard_setup();

  /* Wait until the the host driver is connected */
  host_driver_t* selected_driver = NULL;
  int driver_nr = 0;
  timer_init();
  uint16_t last_time = timer_read();
  while(selected_driver == NULL){
    if (dc->connection_timeout > 0) {
        // If a connection timeout is specified, only try to one driver at a time
        dc->drivers[driver_nr]->poll();
        if (dc->drivers[driver_nr]->is_connected()) {
            selected_driver = dc->drivers[driver_nr];
        }
        else {
            // If there's a timeout, continue with the next driver
            uint16_t elapsed = timer_elapsed(last_time);
            if (elapsed > dc->connection_timeout) {
                if (driver_nr < dc->num_drivers  - 1) {
                    driver_nr++;
                    last_time = timer_read();
                }
            }
        }
    }
    else {
        // No connection timeout specified, try all drivers in parallel
        for (int i=0; i<dc->num_drivers; i++) {
            dc->drivers[i]->poll();
            if (dc->drivers[i]->is_connected()) {
                selected_driver = dc->drivers[i];
                break;
            }
        }
    }
    wait_ms_variable(dc->connection_delay);
  }

  /* On ChibiOS
   * Do need to wait here!
   * Otherwise the next print might start a transfer on console EP
   * before the USB is completely ready, which sometimes causes
   * HardFaults.
   * Does no harm on other platforms, so just always wait
   */
  wait_ms(50);

  print("USB configured.\n");

  /* init TMK modules */
  keyboard_init();
  host_set_driver(selected_driver);

#ifdef SLEEP_LED_ENABLE
  sleep_led_init();
#endif

  print("Keyboard start.\n");

  hook_late_init();

  /* Main loop */
  while(true) {

    host_driver_t* host_driver = host_get_driver();
    if (host_driver) {
      if(host_driver->is_suspended()) {
        print("[s]");
        while(host_driver->is_suspended()) {
          hook_usb_suspend_loop();
          for (int i=0; i<dc->num_drivers; i++) {
            dc->drivers[i]->poll();
          }
          /* Remote wakeup */
          if((host_driver->is_remote_wakeup_supported()) && suspend_wakeup_condition()) {
            host_driver->send_remote_wakeup();
          }
        }
        /* Woken up */
        // variables have been already cleared
        send_keyboard_report();
#ifdef MOUSEKEY_ENABLE
        mousekey_send();
#endif /* MOUSEKEY_ENABLE */
      }
    }

    keyboard_task();
    for (int i=0; i<dc->num_drivers; i++) {
      dc->drivers[i]->poll();
    }
  }
}
