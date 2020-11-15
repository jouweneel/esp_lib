#include <string.h>
#include <math.h>
#include "esp_log.h"

#include "fx.h"

static const char *TAG = "fx";

float sineCalc(FxCfg_t *fxCfg, float idx, uint32_t step) {
  FxSineCfg_t *cfg = (FxSineCfg_t *)(fxCfg->calcCfg);

  return cfg->offset + cfg->scalar * cos( 2 * M_PI / cfg->period * (
    (cfg->index ? idx : 0) +
    cfg->phase + step * cfg->speed
  ));
}

float fadeCalc(FxCfg_t *fxCfg, float idx, uint32_t step) {
  FxFadeCfg_t *cfg = (FxFadeCfg_t *)(fxCfg->calcCfg);

  float start = (float)((cfg->start)[fxCfg->channel]) / 255.0;
  float end = (float)((cfg->end)[fxCfg->channel]) / 255.0;
  float delta = (end - start) / fxCfg->steps;

  return start + (delta * step);
}

float (*calcFns[])(FxCfg_t *fxCfg, float idx, uint32_t step) = {
  sineCalc, fadeCalc
};

void fx(void *fxCfg) {
  FxCfg_t *cfg = (FxCfg_t *)fxCfg;
  uint8_t *buf = cfg->buf;
  uint32_t size = cfg->strip->size;

  volatile uint32_t step = 0;

  for (uint32_t i = 0; i < size; i++) {
    uint8_t *led = &(buf[3 * i]);
    led[0] = cfg->base[0];
    led[1] = cfg->base[1];
    led[2] = cfg->base[2];
  }

  while(cfg->steps ? step < cfg->steps : true) {
    for (uint32_t i = 0; i < size; i++) {
      float idx = (float)i / (float)(size);

      uint8_t *buf_led = &(buf[3 * i]);
      uint8_t *led = &((cfg->strip->leds)[3 * i]);

      if (cfg->channel < 3) {
        float res = (cfg->calcFn)(cfg, idx, step);
        uint8_t val = (res < 0) ? 0 : (res > 1) ? 255 : (uint8_t)(255.0 * res);
        buf_led[cfg->channel] = val;
      } else {
        for (uint8_t c = 0; c < 3; c++) {
          cfg->channel = c;
          float res = (cfg->calcFn)(cfg, idx, step);
          uint8_t val = (res < 0) ? 0 : (res > 1) ? 255 : (uint8_t)(255.0 * res);
          buf_led[c] = val;
        }
        cfg->channel = 3;
      }

      if (cfg->type == STRIP_HSV) {
        hsv2rgb(buf_led, led); 
      } else {
        led[0] = buf_led[0];
        led[1] = buf_led[1];
        led[2] = buf_led[2];
      }
    }

    strip_colors(cfg->strip, cfg->strip->leds);
    step += 1;

    vTaskDelay(cfg->loopDelay);
  }

  strip_write(cfg->strip);
  TaskHandle_t tmp = cfg->handle;
  cfg->handle = NULL;
  free(cfg->buf);
  vTaskDelete(tmp);
}

void start_fx(FxCfg_t *cfg) {
  if (cfg->handle != NULL) {
    vTaskDelete(cfg->handle);
    free(cfg->buf);
  }
  cfg->buf = (uint8_t *)malloc(3 * cfg->strip->size);
  xTaskCreatePinnedToCore(fx, "fx", 4096, (void *)cfg, 5, &(cfg->handle), 1);
}

void stop_fx(FxCfg_t *cfg) {
  strip_write(cfg->strip);
  TaskHandle_t tmp = cfg->handle;
  cfg->handle = NULL;
  free(cfg->buf);
  vTaskDelete(tmp);
}
