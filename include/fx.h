#ifndef _FX_H_
#define _FX_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "strip.h"

typedef struct FxCfg_t {
  // Fx function / cfg
  float (*calcFn)(struct FxCfg_t *cfg, float idx, uint32_t step);
  void *calcCfg;
  uint8_t base[3];

  // Operates on - params
  uint8_t channel;
  uint32_t size;  // Size of strip
  color_type_t type;
  uint32_t loopDelay;
  uint32_t steps; // # of iterations, 0 is infinite

  StripData_t *strip;
  TaskHandle_t handle;
  uint8_t *buf;
} FxCfg_t;

typedef struct FxSineCfg_t {
  uint8_t index;
  float speed;

  float period;
  float scalar;
  float offset;
  float phase;
} FxSineCfg_t;

typedef struct FxFadeCfg_t {
  uint8_t start[3];
  uint8_t end[3];
} FxFadeCfg_t;

#ifdef __cplusplus
extern "C" {
#endif

void fx(void *fxCfg);
void start_fx(FxCfg_t *cfg);
void stop_fx(FxCfg_t *cfg);

float fadeCalc(FxCfg_t *fxCfg, float idx, uint32_t step);
float sineCalc(FxCfg_t *fxCfg, float idx, uint32_t step);

#ifdef __cplusplus
}
#endif

#endif
