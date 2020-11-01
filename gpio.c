#include "esp_log.h"
#include "gpio.h"

#include "freertos/task.h"

static const char *TAG = "[gpio]";

static void IRAM_ATTR input_handler(void * input_arg) {
  GpioInData_t *input = (GpioInData_t *)input_arg;
  uint8_t value = gpio_get_level(input->pin);
  xQueueSendToBackFromISR(input->queue, &value, NULL);
  return;
}

void input_task(void *pvParams) {
  GpioInData_t *input = (GpioInData_t *)pvParams;

  while(true) {
    BaseType_t res = xQueueReceive(
      input->queue, input->value, portMAX_DELAY
    );
    if (res == pdTRUE) {
      input->callback((input->value)[0]);
    }
    vTaskDelay(1);
  }
}

GpioInData_t *gpio_input(uint8_t pin, void (*callback)(uint8_t value)) {
  gpio_config_t cfg = {
#if defined CONFIG_IDF_TARGET_ESP8266
    .intr_type = GPIO_INTR_ANYEDGE,
#else
    .intr_type = GPIO_PIN_INTR_ANYEDGE,
#endif
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = ((1ULL << pin)),
    .pull_down_en = 1,
    .pull_up_en = 0
  };
  esp_err_t res = gpio_config(&cfg);
  if (res != ESP_OK) {
    return NULL;
  }

  // Create and initialize GpioInData_t struct
  GpioInData_t *input = (GpioInData_t *)malloc(sizeof(GpioInData_t));
  input->callback = callback;
  sprintf(input->name, "gpio_in_%u", input->pin);
  input->pin = pin;
  input->value[0] = gpio_get_level(input->pin);
  input->queue = xQueueCreate(1, sizeof(uint8_t));

  ESP_LOGI(TAG, "queue: %p", input->queue);
  if (input->queue == NULL) {
    free(input);
    return NULL;
  }

#if defined CONFIG_IDF_TARGET_ESP8266
  gpio_install_isr_service(0);
#else
  gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
#endif

  gpio_isr_handler_add(input->pin, input_handler, input);

  res = xTaskCreate(
    input_task, input->name, 8192, (void *)input, 20, NULL
  );
  if (res != pdPASS) {
    free(input);
    return NULL;
  }

  return input;
}

esp_err_t gpio_write(GpioOutData_t *output, uint8_t value) {
  esp_err_t res = gpio_set_level(output->pin, value);
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Setting pin %u to %u", output->pin, value);
  } else {
    output->value = value;
  }
  return res;
}

GpioOutData_t *gpio_output(uint8_t pin, uint8_t value) {
  gpio_config_t cfg = {
#if defined CONFIG_IDF_TARGET_ESP8266
    .intr_type = GPIO_INTR_DISABLE,
#else
    .intr_type = GPIO_PIN_INTR_DISABLE,
#endif
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = ((1ULL << pin)),
    .pull_down_en = 0,
    .pull_up_en = 0
  };

  esp_err_t res = gpio_config(&cfg);
  if (res != ESP_OK) {
    return NULL;
  }

  res = gpio_set_level(pin, value);
  if (res != ESP_OK) {
    return NULL;
  }

  GpioOutData_t *output = (GpioOutData_t *)malloc(sizeof(GpioOutData_t));
  output->pin = pin;
  output->value = value;
  
  return output;
}
