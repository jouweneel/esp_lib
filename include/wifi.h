#ifndef _WIFI_H_
#define _WIFI_H_

#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Start WiFi connection
 * Configured via make menuconfig -> Networking configuration
*/
void wifi_start();

#ifdef __cplusplus
}
#endif

#endif
