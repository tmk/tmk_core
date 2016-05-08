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

#ifndef HOST_DRIVER_H
#define HOST_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "report.h"


typedef struct {
    // Called at the start of the driver connection
    // Should start to connect to the host. Note that
    // if the host_driver_configuration asks for a non-parallel
    // connection, connect might never be called.
    void (*connect)(void);
    // Should return true, when the driver is connected
    bool (*is_connected)(void);
    // Should return true, when the host is suspended from
    // the driver's point of view
    bool (*is_suspended)(void);
    // Called regularly, during connection, suspend and normal
    // keyboard loop. This is automatically called for all drivers
    // which have got the connect called automatically, even if they
    // have failed to connect.
    // So it might not be called when using non-parallel connections
    // It's also not called for any drivers, that are not part of the
    // host_driver_configuration.
    void (*poll)(void);
    // Should return true when host remote wakeup is supported
    bool (*is_remote_wakeup_supported)(void);
    // Send remote wakeup to the host
    void (*send_remote_wakeup)(void);
    uint8_t (*keyboard_leds)(void);
    void (*send_keyboard)(report_keyboard_t *);
    void (*send_mouse)(report_mouse_t *);
    void (*send_system)(uint16_t);
    void (*send_consumer)(uint16_t);
} host_driver_t;

// Represents the host driver configuration of the keyboard
// Can consist of a single driver, or multiple driver in case
// there are several ways to contact the host, for example
// both USB and Bluetooth.
// Override hook_get_driver_configuration, to return your custom
// configuration.
typedef struct {
    int num_drivers;
    // The wait time between connection retries
    int connection_delay;
    // There are four modes of connection
    // If you need to customize the behavior, you could write a
    // "virtual driver", which internally combines two or more drivers
    // but handles the connection logic and prioritization internally
    // 1. The connection_timeout is 0 and try_connect_all is false
    //    - All drivers are being connected in parallel
    //      and the first driver that connects gets selected.
    // 2. The connection timeout is 0 and try_connect_all is true
    //    - All drivers are bing connected in parallel
    //    - This continues until all drivers are connected
    //    - The first driver in the list gets selected
    // 3. The connection_timeout is >0 and try_connect_all is false
    //    - Try to connect the first driver until connection_timout
    //    - If it succeeds before that, select the driver
    //    - If it fails to connect, then try to connect the second driver
    //    - Continue with following drivers until a driver gets selected
    //    - The last driver will not have timeout, and the code will block
    //      until it connects.
    // 4. The connection_timout is >0 and try_connect_all is true
    //    - Try to connect all drivers in parallel, until the timeout
    //    - If all drivers connect within the timeout, select the first in the list
    //      and don't wait until the end of the timeout
    //    - Otherwise wait until the timeout, and select the first one in the list
    //      that has successfully connected.
    //    - If no drivers have connected when the timeout expires, wait until the
    //      one of them connects, and select it immediately
    int connection_timeout;
    bool try_connect_all;
    host_driver_t* drivers[4];
} host_driver_configuration_t;

#endif
