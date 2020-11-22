#ifndef _STRIP_H_
#define _STRIP_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum color_type_t { STRIP_RGB, STRIP_HSV } color_type_t;

typedef struct StripConfig_t {
  uint8_t pin;
  uint8_t power;
  uint8_t brightness;
  uint8_t color[3];
  uint32_t size;
  color_type_t type;
} StripConfig_t;

typedef struct FxCfg_t {
  // Fx function / cfg
  float (*calcFn)(struct FxCfg_t *cfg, float idx, uint32_t step);
  void *calcCfg;
  uint8_t base[3];

  // Operates on - params
  uint8_t channel;
  uint32_t loopDelay;
  uint32_t steps; // # of iterations, 0 is infinite

  TaskHandle_t handle;
  uint8_t *buf;
} FxCfg_t;


typedef struct StripData_t {
  uint8_t power;
  uint8_t brightness;
  uint8_t color[3];
  uint32_t size;
  color_type_t type;

  uint8_t *leds;
  FxCfg_t *fx;
  void *ctx;
} StripData_t;

#ifdef __cplusplus
extern "C" {
#endif

void hsv2rgb(uint8_t *hsv, uint8_t *rgb);
void strip_write(StripData_t *strip);
void strip_power(StripData_t *strip, uint8_t power);
void strip_fade_power(StripData_t *strip, uint8_t power);
void strip_brightness(StripData_t *strip, uint8_t brightness);
void strip_color(StripData_t *strip, uint8_t color[3]);
void strip_fade_color(StripData_t *strip, uint8_t *target);
void strip_reset(StripData_t *strip);
void strip_colors(StripData_t *strip, uint8_t *colors);
void strip_sine_fx(StripData_t *strip, float *params);

StripData_t *strip_init(StripConfig_t *cfg);

/** Hardware-specific init, defined in platforms ws2812.c */
void *ws2812_init(StripConfig_t *cfg);
void ws2812_write(StripData_t *data, uint8_t *buf);
void ws2812_write_color(StripData_t *strip, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif
