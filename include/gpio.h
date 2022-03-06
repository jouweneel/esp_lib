#ifndef _GPIO_H_
#define _GPIO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnalogInData_t {
  void (*callback)(int value);
  char name[12];
  adc1_channel_t channel;
  int value;
} AnalogInData_t;

typedef struct GpioInData_t {
  void (*callback)(uint8_t value);
  char name[12];
  uint8_t pin;
  TaskHandle_t task;
  uint8_t value[1];
} GpioInData_t;

typedef struct GpioOutData_t {
  uint8_t pin;
  uint8_t value;
} GpioOutData_t;

AnalogInData_t *analog_input(adc1_channel_t channel, void (*callback)(int value));
GpioInData_t *gpio_input(uint8_t pin, void (*callback)(uint8_t value));
GpioOutData_t *gpio_output(uint8_t pin, uint8_t value);

esp_err_t gpio_write(GpioOutData_t *output, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /** GPIO_H_ */
