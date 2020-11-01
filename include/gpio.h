#ifndef _GPIO_H_
#define _GPIO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GpioInData_t {
  void (*callback)(uint8_t value);
  char name[12];
  uint8_t pin;
  QueueHandle_t queue;
  uint8_t value[1];
} GpioInData_t;

typedef struct GpioOutData_t {
  uint8_t pin;
  uint8_t value;
} GpioOutData_t;

GpioInData_t *gpio_input(uint8_t pin, void (*callback)(uint8_t value));
GpioOutData_t *gpio_output(uint8_t pin, uint8_t value);

esp_err_t gpio_write(GpioOutData_t *output, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /** GPIO_H_ */
