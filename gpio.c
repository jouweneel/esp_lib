#include "esp_log.h"
#include "gpio.h"

#include "freertos/task.h"

static const char *TAG = "[gpio]";
static const int SAMPLES = 64;

void analog_task(void *pvParams) {
  AnalogInData_t *input = (AnalogInData_t *)pvParams;
  input->value = 0;

  while(true) {
    uint32_t total = 0;
    for (int i=  0; i < SAMPLES; i++) {
      total += adc1_get_raw(input->channel);
    }

    int value = total >> 10;

    if (value != input->value) {
      input->value = value;
      input->callback(value);
    }

    vTaskDelay(60 * 100); // 100 = 1s
  }
}

AnalogInData_t *analog_input(
  adc1_channel_t channel, void (*callback)(int value)
) {
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_12Bit));
  ESP_ERROR_CHECK(adc1_config_channel_atten(channel, ADC_ATTEN_11db));
  
  AnalogInData_t *input = (AnalogInData_t *)malloc(sizeof(AnalogInData_t));

  input->callback = callback;
  input->channel = channel;
  sprintf(input->name, "analog_in_%u", channel);

  ESP_LOGI(TAG, "%s initialized", input->name);

  xTaskCreate(
    analog_task, input->name, 4096, (void *)input, 20, NULL
  );

  return input;
}

/** Digital i/o **/
static void IRAM_ATTR d_input_handler(void * input_arg) {
  GpioInData_t *input = (GpioInData_t *)input_arg;
  xTaskNotify(input->task, 0, 0);
  return;
}

void d_input_task(void *pvParams) {
  GpioInData_t *input = (GpioInData_t *)pvParams;

  while(true) {
    ulTaskNotifyTake(1, portMAX_DELAY);
    uint8_t value = gpio_get_level(input->pin);

    if (value != input->value[0]) {
      // ESP_LOGI(TAG, "GPIO: %i", value);
      input->value[0] = value;
      input->callback(value);
    }
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
    .pull_down_en = 0,
    .pull_up_en = 1
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
  input->value[0] = 2;  // Invalid value -> forces update

#if defined CONFIG_IDF_TARGET_ESP8266
  gpio_install_isr_service(0);
#else
  gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
#endif

  gpio_isr_handler_add(input->pin, d_input_handler, input);

  res = xTaskCreate(
    d_input_task, input->name, 4096, (void *)input, 20, &(input->task)
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
