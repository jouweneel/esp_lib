#include "driver/timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.h"
#include "gpio.h"

static const char *TAG = "[util]";

static GpioOutData_t *status_led = NULL;
static TaskHandle_t status_blink_task_handle = NULL;
static uint32_t *delays = NULL;

void esp_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void status_blink_task(void *params) {
  uint32_t *delays = (uint32_t *)params;

  while(true) {
    gpio_write(status_led, 1);
    vTaskDelay(delays[0]);
    gpio_write(status_led, 0);
    vTaskDelay(delays[1]);
  }
}

void status_blink_init() {
  delays = (uint32_t *)malloc(2 * sizeof(uint32_t));
  status_led = gpio_output(STATUS_LED_PIN, 0);
}

void status_blink(uint32_t on_time) {
  gpio_write(status_led, 1);
  vTaskDelay(on_time);
  gpio_write(status_led, 0);
}

void status_blink_start(uint32_t on_time, uint32_t off_time) {
  delays[0] = on_time;
  delays[1] = off_time;

  if (status_blink_task_handle == NULL) {
    xTaskCreatePinnedToCore(
      status_blink_task, "status_blink", configMINIMAL_STACK_SIZE,
      (void *)delays, 10, &status_blink_task_handle, 1
    );
  }
}

void status_blink_stop() {
  if (status_blink_task_handle != NULL) {
    vTaskDelete(status_blink_task_handle);
    gpio_write(status_led, 0);
    status_blink_task_handle = NULL;
  }
}
