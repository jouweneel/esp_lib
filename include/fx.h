#ifndef _FX_H_
#define _FX_H_

#include <stdint.h>

#include "strip.h"

typedef struct FxCfg_t {
  // Fx function / cfg
  float (*calcFn)(void *cfg, float idx, float time_delta);
  void *fxCfg;
  StripData_t *strip;

  // Time params
  float speed;
  uint8_t invert;
  int loopDelay;

  // Operates on - params
  uint32_t size;  // Size of strip
  uint8_t channel;
  uint8_t index;  // Use indexing?
  color_type_t type;
} FxCfg_t;

typedef struct FxSineCfg_t {
  // Sine-wave params
  float period;
  float scalar;
  float offset;
  float phase;
} FxSineCfg_t;

#ifdef __cplusplus
extern "C" {
#endif

float sineCalc(FxCfg_t *fx, float idx, float time_delta);
void fx(void *fxCfg);

#ifdef __cplusplus
}
#endif

#endif
