#include <fastmath.h>
#include <string.h>
#include "esp_log.h"

#include "strip.h"

#include "esp_timer.h"

static const char *TAG = "[strip]";

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

  for (uint32_t i = 0; i < strip->size; i++) {
    uint8_t offset = 3 * i;
    strip->leds[offset] = r;
    strip->leds[offset + 1] = g;
    strip->leds[offset + 2] = b;
  }

  ws2812_write(strip, strip->leds);
}

void strip_color(StripData_t *strip, uint8_t color[3]) {
  strip->color[0] = color[0];
  strip->color[1] = color[1];
  strip->color[2] = color[2];

  ESP_LOGI(TAG, "[%u, %u, %u]", color[0], color[1], color[2]);

  strip_write(strip);
}

void strip_colors(StripData_t *strip, uint8_t *colors) {
  float brightness = (float)(strip->brightness) / 255.0;

  int64_t start = esp_timer_get_time();
  for (uint32_t i = 0; i < 3 * strip->size; i++) {
    strip->leds[i] = (uint8_t)((float)(colors[i]) * brightness);
  }
  ws2812_write(strip, strip->leds);
}

void strip_power(StripData_t *strip, uint8_t power) {
  strip->power = power;
  strip_write(strip);
}

void strip_brightness(StripData_t *strip, uint8_t brightness) {
  strip->brightness = brightness;
  strip_write(strip);
}

StripData_t *strip_init(StripConfig_t *cfg) {
  void *ctx = ws2812_init(cfg);

  uint8_t *leds = (uint8_t *)malloc(3 * cfg->size);
  StripData_t *strip = (StripData_t *)malloc(sizeof(StripData_t));
  strip->brightness = cfg->brightness;
  strip->color[0] = cfg->color[0];
  strip->color[1] = cfg->color[1];
  strip->color[2] = cfg->color[2];
  strip->power = cfg->power;

  strip->type = cfg->type;
  strip->size = cfg->size;
  strip->leds = leds;
  strip->ctx = ctx;

  strip_write(strip);
  ESP_LOGI(TAG, "initialized");

  return strip;
}
