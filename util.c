#include "driver/timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.h"
#include "gpio.h"

static const char *TAG = "[util]";
EspConfig_t CONFIG = {};

static nvs_handle_t storage_handle;
static GpioOutData_t *status_led = NULL;
static TaskHandle_t status_blink_task_handle = NULL;
static uint32_t *delays = NULL;

static char * nvs_read_string(const char *key) {
  size_t length = 0;

  esp_err_t err = nvs_get_str(storage_handle, key, NULL, &length);
  ESP_ERROR_CHECK(err);

  char *target = (char *)malloc(length);
  err = nvs_get_str(storage_handle, key, target, &length);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "Key %s loaded: %s", key, target);
  return target;
}

static uint8_t nvs_read_u8(const char *key) {
  uint8_t res = 0;
  esp_err_t err = nvs_get_u8(storage_handle, key, &res);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "Key %s loaded: %u", key, res);
  return res;
}

static uint32_t nvs_read_u32(const char *key) {
  uint32_t res = 0;
  esp_err_t err = nvs_get_u32(storage_handle, key, &res);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "Key %s loaded: %u", key, res);
  return res;
}

void esp_init() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  err = nvs_open("storage", NVS_READWRITE, &storage_handle);
  ESP_ERROR_CHECK(err);

  CONFIG.ESP_ID = nvs_read_string("ESP_ID");
  CONFIG.ESP_NAME = nvs_read_string("ESP_NAME");
  CONFIG.WIFI_SSID = nvs_read_string("WIFI_SSID");
  CONFIG.WIFI_PASS = nvs_read_string("WIFI_PASS");
  CONFIG.OTA_URL = nvs_read_string("OTA_URL");
  CONFIG.MQTT_URL = nvs_read_string("MQTT_URL");
  
  CONFIG.LED_PIN = nvs_read_u8("LED_PIN");
  CONFIG.STRIP_PIN = nvs_read_u8("STRIP_PIN");
  CONFIG.INPUT_PIN = nvs_read_u8("INPUT_PIN");
  CONFIG.STRIP_PWR = nvs_read_u8("STRIP_PWR");
  CONFIG.STRIP_BRT = nvs_read_u8("STRIP_BRT");
  CONFIG.STRIP_H = nvs_read_u8("STRIP_H");
  CONFIG.STRIP_S = nvs_read_u8("STRIP_S");
  CONFIG.STRIP_V = nvs_read_u8("STRIP_V");
  CONFIG.STRIP_SIZE = nvs_read_u32("STRIP_SIZE");
  
  nvs_close(storage_handle);
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
  status_led = gpio_output(CONFIG.LED_PIN, 0);
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
