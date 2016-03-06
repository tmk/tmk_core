/*
Copyright 2016 Jun Wako <wakojun@gmail.com>

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

#ifndef _HOOKS_H_
#define _HOOKS_H_

#include "keyboard.h"
#include "led.h"

/* -------------------------------------
 *       Hardware / one-off hooks
 * ------------------------------------- */

/* Called once, before initialising USB. */
/* Default behaviour: do nothing. */
void early_init_hook(void);

/* Called once, after USB is connected and keyboard initialised. */
/* Default behaviour: do nothing. */
void late_init_hook(void);

/* Called once, on getting SUSPEND event from USB. */
/* Default behaviour: enables sleep LED breathing. */
void suspend_entry_hook(void);

/* Called repeatedly during the SUSPENDed state. */
/* Default behaviour: power down and periodically check
 * the matrix, cause wakeup if needed. */
void suspend_loop_hook(void);

/* Called once, on getting WAKE event from USB. */
/* Default behaviour: disables sleep LED breathing and restores 
 * the "normal" indicator LED status by default. */
void wakeup_hook(void);

/* Called once, on checking the bootmagic combos. */
/* Default behaviour: do nothing. */
void bootmagic_hook(void);

/* -------------------------------------
 *       Keyboard / periodic hooks
 * ------------------------------------- */

/* Called periodically from the matrix scan loop (very often!) */
/* Default behaviour: do nothing. */
void scan_loop_hook(void);

/* Called on matrix state change event (every keypress => often!) */
/* Default behaviour: do nothing. */
void matrix_change_hook(keyevent_t event);

/* Called on layer state change event. */
/* Default behaviour: do nothing. */
void layer_change_hook(uint8_t layer_state);

/* Called on tap event. */
/* Default behaviour: do nothing. */
// void tap_event_hook(void);
/* TODO */

/* Called on hold event. */
/* Default behaviour: do nothing. */
// void hold_event_hook(void);
/* TODO */

/* Called on indicator LED update event (when reported from host). */
/* Default behaviour: calls led_set (for compatibility). */
void led_update_hook(uint8_t led_status);

#endif /* _HOOKS_H_ */
