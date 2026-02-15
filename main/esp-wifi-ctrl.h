#ifndef WIFICONTROLLER_H
#define WIFICONTROLLER_H



// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_mac.h"
// #include "esp_log.h"
// #include "esp_netif.h"
// #include "esp_netif_net_stack.h"
// #include "lwip/inet.h"
// #include "lwip/netdb.h"
// #include "lwip/sockets.h"
// #if IP_NAPT
// #include "lwip/lwip_napt.h"
// #endif
// #include "lwip/err.h"
// #include "lwip/sys.h"




#include "esp_wifi.h"
// #include "esp_event.h"


/* To change Wi-Fi default configuration, run SDK: idf.py menuconfig */
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WAPI_PSK
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* DHCP server option */
#define DHCPS_OFFER_DNS             0x02


#ifdef __cplusplus
extern "C" {
#endif

/* Инициализация Wi-Fi контроллера и автостарт
@param wmode:
    WIFI_MODE_STA - Wi-Fi station mode only
    WIFI_MODE_AP - Wi-Fi soft-AP mode only
    WIFI_MODE_APSTA - Dual Wi-Fi station + soft-AP mode
*/
void wifi_initialize(wifi_mode_t wmode);

/* Функция постоянного опроса событий WiFi
должна быть помещена в бесконечном цикле какого-либо потока */
void wifi_loop_events();

#ifdef __cplusplus
}
#endif

#endif // WIFICONTROLLER_H
