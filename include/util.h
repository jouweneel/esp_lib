#ifndef _UTIL_H_
#define _UTIL_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void esp_init();
void status_blink_init();
void status_blink(uint32_t on_time);
void status_blink_start(uint32_t on_time, uint32_t off_time);
void status_blink_stop();

#ifdef __cplusplus
}
#endif

#endif
