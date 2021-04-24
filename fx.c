#include <string.h>
#include <math.h>

#include "fx.h"

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

// float (*calcFns[])(FxCfg_t *fxCfg, float idx, uint32_t step) = {
//   sineCalc, fadeCalc
// };

static void fx(void *params) {
  StripData_t *strip = (StripData_t *)params;
  FxCfg_t *cfg = strip->fx;
  uint8_t *buf = cfg->buf;
  uint32_t size = strip->size;

  volatile uint32_t step = 0;

  for (uint32_t i = 0; i < size; i++) {
    uint8_t *led = &(buf[3 * i]);
    led[0] = cfg->base[0];
    led[1] = cfg->base[1];
    led[2] = cfg->base[2];
  }

  while(cfg->steps ? step < cfg->steps : true) {
    for (uint32_t i = 0; i < size; i++) {
      uint8_t *buf_led = &(buf[3 * i]);
      float idx = (float)i / (float)(size);

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

      // Write out
      uint8_t *led = &((strip->leds)[3 * i]);
      if (strip->type == STRIP_HSV) {
        hsv2rgb(buf_led, led); 
      } else {
        led[0] = buf_led[0];
        led[1] = buf_led[1];
        led[2] = buf_led[2];
      }
    }

    strip_colors(strip, strip->leds);
    step += 1;

    vTaskDelay(cfg->loopDelay);
  }

  strip->fx->handle = NULL;
  strip_write(strip);
  
  vTaskDelete(NULL);
}

void start_fx(StripData_t *strip) {
  if (strip->fx->handle != NULL) {
    vTaskDelete(strip->fx->handle);
  }
  xTaskCreatePinnedToCore(fx, "fx", 4096, (void *)strip, 5, &(strip->fx->handle), 1);
}
