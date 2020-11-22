#ifndef _FX_H_
#define _FX_H_

#include <stdint.h>

#include "strip.h"

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

void start_fx(StripData_t *strip);

float fadeCalc(FxCfg_t *fxCfg, float idx, uint32_t step);
float sineCalc(FxCfg_t *fxCfg, float idx, uint32_t step);

#ifdef __cplusplus
}
#endif

#endif
