#ifndef WIFICONTROLLER_H
#define WIFICONTROLLER_H
#pragma once



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"



#include "esp_wifi.h"
#include "esp_event.h"

#include "freertos/event_groups.h"

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


/* DHCP server option */
#define DHCPS_OFFER_DNS             0x02


class CWiFiController {
    
public:

    static CWiFiController& getInstance() {
        static CWiFiController _self = CWiFiController();
        return _self;
    }

    void initialize(wifi_mode_t wmode = WIFI_MODE_APSTA) noexcept;

    // Example member function (can be modified as needed)
    void doSomething();


private:

    static void callback_WifiEventHandler(void *pArg, esp_event_base_t event_base,
        int32_t event_id, void *event_data);


    esp_netif_t* configure_STA(void);
    esp_netif_t* configure_SoftAP(void);
    void setSoftAP_DNS_addr(esp_netif_t *esp_netif_ap, esp_netif_t *esp_netif_sta);

    // Private Constructor
    explicit CWiFiController();

    wifi_mode_t _wifiMode;
};

#endif // WIFICONTROLLER_H
