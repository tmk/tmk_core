#ifndef WAIT_H
#define WAIT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AVR__)
#   include <util/delay.h>
#   define wait_ms(ms)  _delay_ms(ms)
#   define wait_us(us)  _delay_us(us)
static inline void wait_ms_variable(unsigned int ms) {
    while(ms--) {
        _delay_ms(1);
    }
}
#elif defined(PROTOCOL_CHIBIOS) /* __AVR__ */
#   include "ch.h"
#   define wait_ms(ms) chThdSleepMilliseconds(ms)
#   define wait_us(us) chThdSleepMicroseconds(us)
#   define wait_ms_variable(ms) wait_ms(ms)
#elif defined(__arm__) /* __AVR__ */
#   include "wait_api.h"
#   define wait_ms_variable(ms) wait_ms(ms)
#endif /* __AVR__ */

#ifdef __cplusplus
}
#endif

#endif
