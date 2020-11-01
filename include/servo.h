#ifndef _SERVO_H_
#define _SERVO_H_

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ServoData_t {
  // Configurables
  mcpwm_unit_t unit;
  mcpwm_io_signals_t signals;
  int pin;

  uint32_t frequency;
  mcpwm_generator_t generator;
  mcpwm_timer_t timer;

  float pulse_0_us;           // us pulse width for 0 degrees
  float pulse_us_per_degree;  // us pulse width for 1 degree of turning

  uint32_t pw;                // Current pulse width (us)
  float angle;                // Current angle (degrees)
} ServoData_t;

void servo_init(ServoData_t *data);
void servo_set_angle(ServoData_t *data, float angle);
void servo_set_pw(ServoData_t *data, uint32_t pw);

#ifdef __cplusplus
}
#endif

#endif
