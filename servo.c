#include "esp_log.h"
#include "string.h"

#include "servo.h"

static const char *TAG = "Servo";

void servo_init(ServoData_t *data) {
  esp_err_t res = mcpwm_gpio_init(data->unit, data->signals, data->pin);
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Initializing GPIO: %i", res);
    return;
  }

  mcpwm_config_t *pwm_cfg = (mcpwm_config_t *)malloc(sizeof(mcpwm_config_t));
  pwm_cfg->cmpr_a = 0;
  pwm_cfg->cmpr_b = 0;
  pwm_cfg->counter_mode = MCPWM_UP_COUNTER;
  pwm_cfg->duty_mode = MCPWM_DUTY_MODE_0;
  pwm_cfg->frequency = data->frequency;

  res = mcpwm_init(data->unit, data->timer, pwm_cfg);
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Initializing PWM: %i", res);
    free(pwm_cfg);
    return;
  }

  servo_set_angle(data, data->angle);
}

void servo_set_angle(ServoData_t *data, float angle) {
  uint32_t pw = (uint32_t)(data->pulse_0_us + (data->pulse_us_per_degree * angle));
  esp_err_t res = mcpwm_set_duty_in_us(data->unit, data->timer, data->generator, pw);

  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Setting angle: %i", res);
  } else {
    data->angle = angle;
    data->pw = pw;
  }
  return;
}

void servo_set_pw(ServoData_t *data, uint32_t pw) {
  esp_err_t res = mcpwm_set_duty_in_us(data->unit, data->timer, data->generator, pw);

  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Setting pulse width: %i", res);
  } else {
    data->angle = ((float)pw - data->pulse_0_us) / data->pulse_us_per_degree;
    data->pw = pw;
  }
  return;
}

