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
#include "matrix.h"
#ifdef MOUSEKEY_ENABLE
#include "mousekey.h"
#endif

static uint8_t used_drivers = 0;

static inline bool is_serial_connection(host_driver_configuration_t* dc) {
    return dc->connection_timeout > 0 && dc->try_connect_all == false;
}

static void poll_drivers(host_driver_configuration_t* dc) {
    for (int i=0; i<dc->num_drivers; i++) {
        if (used_drivers & (1u << i)) {
            dc->drivers[i]->poll();
        }
    }
}

static host_driver_t* connect_driver(host_driver_configuration_t* dc) {
    const int timeout = dc->connection_timeout;
    /* Wait until the the host driver is connected */
    host_driver_t* selected_driver = NULL;
    int driver_nr = 0;
    timer_init();
    uint16_t last_time = timer_read();
    while(selected_driver == NULL){
        if (is_serial_connection(dc)) {
            // If we are doing a serial connection, try one driver after another,
            // until we have a connection
            // This matches case 3. in the host_driver_configuration_t description
            poll_drivers(dc);
            if (dc->drivers[driver_nr]->is_connected()) {
                selected_driver = dc->drivers[driver_nr];
            }
            else {
                // If there's a timeout, continue with the next driver
                uint16_t elapsed = timer_elapsed(last_time);
                if (elapsed > timeout) {
                    if (driver_nr < dc->num_drivers  - 1) {
                        driver_nr++;
                        // Since we didn't connect this driver at the start, we have to do it now
                        dc->drivers[driver_nr]->connect();
                        used_drivers |= (1u << driver_nr);
                        last_time = timer_read();
                    }
                }
            }
        }
        else {
            bool all_connected = true;
            host_driver_t* first_connected = NULL;
            for (int i=0; i<dc->num_drivers; i++) {
                dc->drivers[i]->poll();
                all_connected &= dc->drivers[i]->is_connected();
                if (!first_connected && dc->drivers[i]->is_connected()) {
                    first_connected = dc->drivers[i];
                }
            }
            // Case 1. in the host_driver_configuration_t description
            // timeout should be zero if this if is taken
            if (dc->try_connect_all == false && first_connected) {
                selected_driver = first_connected;
            }
            // Case 2. and 4. in the host_driver_configuration_t description
            // If all drivers are connected, then it's safe to continue
            // In case 2. when the timeout is zero, this is the only branch
            // that can be selected. For case 4. this branch can be selected
            // if all drivers get connected before the timeout.
            else if (all_connected) {
                selected_driver = first_connected;
            }
            // Case 4. in the host_driver_configuration_t description
            // If all drivers are not connected when the timeout happens
            // Then select the first driver that has connected.
            // Note that he branch is taken even if no driver has connected yet
            // this is safe, because first_connected will be NULL, and the
            // connection loop will continue
            else if(timeout > 0 && timer_elapsed(last_time) > timeout) {
                selected_driver = first_connected;
            }
        }
        wait_ms_variable(dc->connection_delay);
    }
    return selected_driver;
}

static void keyboard_loop(host_driver_configuration_t* dc) {
    // make sure that you always use host_get_driver()
    // since polling and keyboard task might change it
    while(true) {
        if (host_get_driver()) {
            if(host_get_driver()->is_suspended()) {
                print("[s]");
                matrix_power_down();
                while(host_get_driver() && host_get_driver()->is_suspended()) {
                    hook_usb_suspend_loop();
                    poll_drivers(dc);
                    /* Remote wakeup */
                    if(host_get_driver() && host_get_driver()->is_remote_wakeup_supported() && suspend_wakeup_condition()) {
                        host_get_driver()->send_remote_wakeup();
                    }
                }
                matrix_power_up();
                /* Woken up */
                // variables have been already cleared
                send_keyboard_report();
                #ifdef MOUSEKEY_ENABLE
                mousekey_send();
                #endif /* MOUSEKEY_ENABLE */
            }
        }

        keyboard_task();
        poll_drivers(dc);
    }
}

int main(void) {
    hook_platform_init();
    host_driver_configuration_t* dc = hook_get_driver_configuration();

    for (int i=0; i < dc->num_drivers; i++) {
        used_drivers |= (1u << i);
        dc->drivers[i]->connect();
        // Only initialize the first driver if a connection timeout is specified
        // and try_connect_all is false, we are going to do a serial connection in
        // that case
        if (is_serial_connection(dc)) {
            break;
        }
    }

    hook_early_init();
    keyboard_setup();

    host_driver_t* selected_driver = connect_driver(dc);
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

    keyboard_loop(dc);
}
