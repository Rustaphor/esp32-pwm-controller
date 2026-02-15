/* Код взят из примера Wi-Fi SoftAP & Station Example в составе ESP-IDF SDK */
#include "esp-wifi-ctrl.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
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
#include "esp_event.h"
#include "nvs_flash.h"

/* WARNING! В виду особенностей инициализации структур, перенос кода в C++ невозможен! */


static const char* TAG_AP = "WiFi SoftAP";
static const char* TAG_STA = "WiFi STA";

EventGroupHandle_t s_wifi_event_group;
esp_netif_t *pEsp_netif_sta, *pEsp_netif_ap;
int s_retry_num = 0;



void setSoftAP_DNS_addr(esp_netif_t *esp_netif_ap, esp_netif_t *esp_netif_sta)
{
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(esp_netif_sta,ESP_NETIF_DNS_MAIN,&dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));
}


 void callback_STA_ConnectTimeout(TimerHandle_t xTimer)
 {
    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

    /* xEventGroupGetBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_REMOTE_AP_SSID, CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }

    /* Set sta as the default interface */
    esp_netif_set_default_netif(pEsp_netif_sta);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(pEsp_netif_ap) != ESP_OK) {
        ESP_LOGE(TAG_STA, "NAPT not enabled on the netif: %p", pEsp_netif_ap);
    }
 }



void callback_WifiEventHandler(void *pArg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base != WIFI_EVENT) return;

    switch (event_id){

        case WIFI_EVENT_AP_STACONNECTED:
            wifi_event_ap_staconnected_t *pEventCon = (wifi_event_ap_staconnected_t *) event_data;
            ESP_LOGI(TAG_AP, "Station "MACSTR" joined, AID=%d", MAC2STR(pEventCon->mac), pEventCon->aid);
            break;
        
        case WIFI_EVENT_AP_STADISCONNECTED:
            wifi_event_ap_stadisconnected_t *pEventDiscnt = (wifi_event_ap_stadisconnected_t *) event_data;
            ESP_LOGI(TAG_AP, "Station "MACSTR" left, AID=%d, reason:%d", MAC2STR(pEventDiscnt->mac), pEventDiscnt->aid, pEventDiscnt->reason);
            break;

        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            ESP_LOGI(TAG_STA, "Station started");       
            break;
    }
}

void callback_IPEventHandler(void *pArg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base != IP_EVENT) return;

    switch (event_id){

        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
            ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_REMOTE_AP_SSID, CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD);
            setSoftAP_DNS_addr(pEsp_netif_ap,pEsp_netif_sta);
            break;
    }
}


esp_netif_t *configure_STA(void)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_REMOTE_AP_SSID,
            .password = CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = CONFIG_ESP_MAXIMUM_STA_RETRY,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
            * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    return esp_netif_sta;
}

esp_netif_t *configure_SoftAP(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = CONFIG_ESP_WIFI_AP_SSID,
            .ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID),
            .channel = CONFIG_ESP_WIFI_AP_CHANNEL,
            .password = CONFIG_ESP_WIFI_AP_PASSWORD,
            .max_connection = CONFIG_ESP_MAX_STA_CONN_AP,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_ESP_WIFI_AP_SSID, CONFIG_ESP_WIFI_AP_PASSWORD, CONFIG_ESP_WIFI_AP_CHANNEL);

    return esp_netif_ap;
}


void wifi_initialize(wifi_mode_t wmode)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Initialize NVS if switched on
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize event group */
    s_wifi_event_group = xEventGroupCreate();

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    callback_WifiEventHandler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    callback_IPEventHandler,
                    NULL,
                    NULL));

    /* Initialize WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(wmode));

    switch (wmode){
        case WIFI_MODE_STA:
            /* Initialize STA */
            ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
            pEsp_netif_sta = configure_STA();
            break;
        case WIFI_MODE_AP:
            /* Initialize AP */
            ESP_LOGI(TAG_AP, "ESP_WIFI_MODE_AP");
            pEsp_netif_ap = configure_SoftAP();
            break;
        case WIFI_MODE_APSTA:
            pEsp_netif_ap = configure_SoftAP();
            pEsp_netif_sta = configure_STA();
            break;
        default:
            return;
    }

    /* Start WiFi */
    ESP_ERROR_CHECK(esp_wifi_start());

    // Start Software Timer to check STA Connection
    TimerHandle_t hTimer = xTimerCreate("STA_connect_timeout", portMAX_DELAY, pdFALSE, (void*) 0, callback_STA_ConnectTimeout);
    if( xTimerStart(hTimer, 0 ) != pdPASS ) {
        ESP_LOGE(TAG_STA,"Error starting connection timeout timer.");
    }
}

void wifi_loop_events()
{
    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

    /* xEventGroupGetBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT) { 
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_REMOTE_AP_SSID, CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD);
        setSoftAP_DNS_addr(pEsp_netif_ap,pEsp_netif_sta);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_REMOTE_AP_SSID, CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }

    /* Set sta as the default interface */
    esp_netif_set_default_netif(pEsp_netif_sta);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(pEsp_netif_ap) != ESP_OK) {
        ESP_LOGE(TAG_STA, "NAPT not enabled on the netif: %p", pEsp_netif_ap);
    }
}
