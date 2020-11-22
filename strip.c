#include <fastmath.h>
#include <string.h>
#include "esp_log.h"

#include "strip.h"
#include "fx.h"

#include "esp_timer.h"

static const char *TAG = "[strip]";

FxSineCfg_t sine = {
  .index = 0,
  .speed = 0.01,

  .period = 1,
  .offset = 0.75,
  .scalar = 0.25,
  .phase = 0
};
FxFadeCfg_t fade = {
  .start = { 0, 0, 0 },
  .end = { 0, 0, 0 }
};

void hsv2rgb(uint8_t *hsv, uint8_t *rgb) {
  float h = hsv[0]/255.0;
  float s = hsv[1]/255.0;
  float v = hsv[2]/255.0;
  float r = 0; float g = 0; float b = 0;

  uint32_t i = (uint32_t)(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  switch(i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }
  rgb[0] = (uint8_t)(r*255);
  rgb[1] = (uint8_t)(g*255);
  rgb[2] = (uint8_t)(b*255);
}

void strip_write(StripData_t *strip) {
  uint8_t rgb[3];
  if (strip->type == STRIP_RGB) {
    rgb[0] = strip->color[0];
    rgb[1] = strip->color[1];
    rgb[2] = strip->color[2];
  } else {
    hsv2rgb(strip->color, rgb);
  }

  float brightness = (float)(strip->brightness) / 255.0;
  uint8_t r = (strip->power == 0) ? 0 : (uint8_t)round(rgb[0] * brightness);
  uint8_t g = (strip->power == 0) ? 0 : (uint8_t)round(rgb[1] * brightness);
  uint8_t b = (strip->power == 0) ? 0 : (uint8_t)round(rgb[2] * brightness);

  ws2812_write_color(strip, r, g, b);
}

void strip_power(StripData_t *strip, uint8_t power) {
  strip->power = power;
  strip_write(strip);
}

void strip_fade_power(StripData_t *strip, uint8_t power) {
  if (power == strip->power) {
    return;
  }

  fade.start[0] = strip->color[0];
  fade.start[1] = strip->color[1];
  fade.start[2] = power ? 0 : strip->color[2];

  fade.end[0] = strip->color[0];
  fade.end[1] = strip->color[1];
  fade.end[2] = power ? strip->color[2] : 0;

  strip->fx->calcFn = fadeCalc;
  strip->fx->calcCfg = &fade;
  strip->fx->channel = 3;
  strip->fx->steps = 50;

  start_fx(strip);

  strip->power = power;
}

void strip_brightness(StripData_t *strip, uint8_t brightness) {
  strip->brightness = brightness;
  strip_write(strip);
}

void strip_color(StripData_t *strip, uint8_t color[3]) {
  strip->color[0] = color[0];
  strip->color[1] = color[1];
  strip->color[2] = color[2];

  strip_write(strip);
}

void strip_fade_color(StripData_t *strip, uint8_t *target) {
  fade.start[0] = strip->color[0];
  fade.start[1] = strip->color[1];
  fade.start[2] = strip->color[2];

  fade.end[0] = target[0];
  fade.end[1] = target[1];
  fade.end[2] = target[2];

  strip->fx->calcFn = fadeCalc;
  strip->fx->calcCfg = &fade;
  strip->fx->channel = 3;
  strip->fx->steps = 50;

  start_fx(strip);

  strip->color[0] = target[0];
  strip->color[1] = target[1];
  strip->color[2] = target[2];
}

void strip_reset(StripData_t *strip) {
   if (strip->fx->handle != NULL) {
    TaskHandle_t tmp = strip->fx->handle;
    strip->fx->handle = NULL;

    strip_write(strip);  
    vTaskDelete(tmp);
  } else {
    strip_write(strip);
  }
}

void strip_colors(StripData_t *strip, uint8_t *colors) {
  float brightness = (float)(strip->brightness) / 255.0;

  for (uint32_t i = 0; i < 3 * strip->size; i++) {
    strip->leds[i] = (uint8_t)((float)(colors[i]) * brightness);
  }
  ws2812_write(strip, strip->leds);
}

void strip_sine_fx(StripData_t *strip, float *params) {
  strip->fx->base[0] = (uint8_t)(params[0]);
  strip->fx->base[1] = (uint8_t)(params[1]);
  strip->fx->base[2] = (uint8_t)(params[2]);
  strip->fx->channel = (uint8_t)(params[3]);
  strip->fx->steps = (uint32_t)(params[4]);

  sine.index = (uint8_t)(params[5]);
  sine.speed = params[6];
  sine.period = params[7];
  sine.offset = params[8];
  sine.scalar = params[9];

  strip->fx->calcCfg = &sine;
  strip->fx->calcFn = sineCalc;

  start_fx(strip);
}

StripData_t *strip_init(StripConfig_t *cfg) {
  void *ctx = ws2812_init(cfg);

  uint8_t *leds = (uint8_t *)malloc(3 * cfg->size);

  StripData_t *strip = (StripData_t *)malloc(sizeof(StripData_t));
  FxCfg_t *fx = (FxCfg_t *)malloc(sizeof(FxCfg_t));
  
  strip->brightness = cfg->brightness;
  strip->color[0] = cfg->color[0];
  strip->color[1] = cfg->color[1];
  strip->color[2] = cfg->color[2];
  strip->power = cfg->power;

  strip->type = cfg->type;
  strip->size = cfg->size;
  strip->leds = leds;
  strip->ctx = ctx;

  fx->buf = (uint8_t *)malloc(3 * strip->size);
  fx->base[0] = cfg->color[0];
  fx->base[1] = cfg->color[1];
  fx->base[2] = cfg->color[2];
  fx->loopDelay = 1;
  fx->channel = 2;
  fx->steps = 0;

  fx->calcFn = sineCalc;
  fx->calcCfg = &sine;
  fx->handle = NULL;

  strip->fx = fx;

  strip_write(strip);
  ESP_LOGI(TAG, "initialized");

  return strip;
}
