#include <string.h>
#include "wifi-ctrl.h"
#include "nvs_flash.h"




/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* TAG_AP = "WiFi SoftAP";
static const char* TAG_STA = "WiFi STA";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;


void CWiFiController::callback_WifiEventHandler(void *pArg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    CWiFiController* pSelf = (CWiFiController*) pArg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


esp_netif_t *CWiFiController::configure_STA(void)
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
            .threshold = {
                .authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,

            },
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    return esp_netif_sta;
}

esp_netif_t *CWiFiController::configure_SoftAP(void)
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
    }

    if (strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_ESP_WIFI_AP_SSID, CONFIG_ESP_WIFI_AP_PASSWORD, CONFIG_ESP_WIFI_AP_CHANNEL);

    return esp_netif_ap;
}

void CWiFiController::setSoftAP_DNS_addr(esp_netif_t *esp_netif_ap, esp_netif_t *esp_netif_sta)
{
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(esp_netif_sta,ESP_NETIF_DNS_MAIN,&dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));
}

// Constructor
CWiFiController::CWiFiController() {
    // Initialization code (if needed)
}

void CWiFiController::initialize(wifi_mode_t wmode) noexcept
{
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // //Initialize NVS
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    // /* Initialize event group */
    // s_wifi_event_group = xEventGroupCreate();

    // /* Register Event handler */
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    //                 ESP_EVENT_ANY_ID,
    //                 CWiFiController::callback_WifiEventHandler,
    //                 this,
    //                 NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    //                 IP_EVENT_STA_GOT_IP,
    //                 CWiFiController::callback_WifiEventHandler,
    //                 this,
    //                 NULL));

    // /*Initialize WiFi */
    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // ESP_ERROR_CHECK(esp_wifi_set_mode(wmode));

    // esp_netif_t *esp_netif_sta, *esp_netif_ap;
    // switch (wmode){
    //     case WIFI_MODE_STA:
    //         /* Initialize STA */
    //         ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
    //         esp_netif_sta = configure_STA();
    //         break;
    //     case WIFI_MODE_AP:
    //         /* Initialize AP */
    //         ESP_LOGI(TAG_AP, "ESP_WIFI_MODE_AP");
    //         esp_netif_ap = configure_SoftAP();
    //         break;
    //     case WIFI_MODE_APSTA:
    //         esp_netif_ap = configure_SoftAP();
    //         esp_netif_sta = configure_STA();
    //         break;
    // }
}

// Example member function definition
void CWiFiController::doSomething() {
    // Function implementation
}
