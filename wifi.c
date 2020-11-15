#include "config.h"
#include "wifi.h"

#include <string.h>
#include "esp_log.h"

#if WIFI_WPA2 == 1
  #include "esp_wpa2.h"
#endif

#define WIFI_RETRY_DELAY 500
#define WIFI_AP_MAX_CONNECTIONS 4

static const char *TAG = "[wifi]";

static int8_t wifi_status = 0;

#if WIFI_BLOCK_CONNECT == 1
static SemaphoreHandle_t s_wifi_connecting = NULL;
#endif

/**
 * esp system WIFI_EVENT handler
 */
static void event_handler(
  void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data
) {
  switch(event_id) {
    case WIFI_EVENT_STA_START:
      esp_wifi_connect();
      return;
    case WIFI_EVENT_AP_STOP:
    case WIFI_EVENT_STA_DISCONNECTED:
      wifi_status = 0;
      vTaskDelay(WIFI_RETRY_DELAY);
      ESP_LOGI(TAG, "Wifi reconnecting");
      esp_wifi_connect();
      return;
    case WIFI_EVENT_AP_START:
    case IP_EVENT_STA_GOT_IP:
      wifi_status = 1;
#if WIFI_BLOCK_CONNECT == 1
      xSemaphoreGive(s_wifi_connecting);
#endif
      ESP_LOGI(TAG, "Wifi connected");
      return;
    case WIFI_EVENT_STA_CONNECTED:
      return;
    default:
      ESP_LOGW(TAG, "Unhandled WiFi event %i", event_id);
  }
}

void wifi_start() {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

#if defined WIFI_MODE_AP
  ESP_LOGI(TAG, "Starting AP \"%s\"", CONFIG.WIFI_SSID);

  wifi_config_t wifi_config = {};
  wifi_config.ap.ssid_len = strlen(CONFIG.WIFI_SSID);
  wifi_config.ap.max_connection = WIFI_AP_MAX_CONNECTIONS;
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  strcpy((char *)wifi_config.ap.ssid, CONFIG.WIFI_SSID);
  if (strlen(CONFIG.WIFI_PASS) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  } else {
    strcpy((char *)wifi_config.ap.password, CONFIG.WIFI_PASS);
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

#else // WIFI_MODE_STA
  // ESP_LOGI(TAG, "Connecting to \"%s\"", CONFIG.WIFI_SSID);
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  wifi_config_t wifi_config = {};
  strcpy((char *)wifi_config.sta.ssid, CONFIG.WIFI_SSID);
  strcpy((char *)wifi_config.sta.password, CONFIG.WIFI_PASS);

  esp_netif_create_default_wifi_sta();
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

  #if WIFI_WPA2 == 1
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_WPA2_IDENTITY, strlen(WIFI_WPA2_IDENTITY)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_password((uint8_t *)CONFIG.WIFI_PASS, strlen(CONFIG.WIFI_PASS)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_ca_cert(NULL, 0));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_cert_key(NULL, 0, NULL, 0, NULL, 0));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_enable());
  #endif

  ESP_ERROR_CHECK(esp_wifi_start());
#endif

  #if WIFI_BLOCK_CONNECT == 1
    ESP_LOGI(TAG, "waiting for connection...");
    s_wifi_connecting = xSemaphoreCreateBinary();
    xSemaphoreTake(s_wifi_connecting, portMAX_DELAY);
    xSemaphoreGive(s_wifi_connecting);
  #endif

}
