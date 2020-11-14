#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "fx.h"

static const char *TAG = "fx";

float sineCalc(FxCfg_t *fx, float idx, float time_delta) {
  FxSineCfg_t *cfg = fx->fxCfg;

  float res = cfg->offset + cfg->scalar * cos( 2 * M_PI / cfg->period * (
    (fx->index ? idx : 0) +
    cfg->phase + time_delta * (fx->invert ? 1 : -1)
  ));
  return res;
}

void fx(void *fxCfg) {
  FxCfg_t *cfg = (FxCfg_t *)fxCfg;

  uint8_t *buf = (uint8_t *)malloc(3 * cfg->size);

  volatile uint32_t step = 0;

  while(true) {
    memset(buf, 255, 3 * cfg->size);
    float time_delta = (cfg->speed ? cfg->speed * step : 0);

    for (uint32_t i = 0; i < cfg->size; i++) {
      float idx = (float)i / (float)(cfg->size);

      float res = (cfg->calcFn)(cfg, idx, time_delta);
      uint8_t val = (res < 0) ? 0 : (res > 1) ? 255 : (uint8_t)(255.0 * res);
      buf[3 * i + cfg->channel] = val;
    }

    // ESP_LOGI(TAG, "%.2f: %u", time_delta, buf[0]);
    if (cfg->type == STRIP_HSV) {
      for (uint32_t i = 0; i < cfg->size; i++) {
        uint8_t *led = &(buf[3 * i]);
        uint8_t rgb[3];
        hsv2rgb(led, rgb);

        led[0] = rgb[0];
        led[1] = rgb[1];
        led[2] = rgb[2];
      }
    }

    strip_colors(cfg->strip, buf);
    step += 1;
    vTaskDelay(cfg->loopDelay);
  }
}
