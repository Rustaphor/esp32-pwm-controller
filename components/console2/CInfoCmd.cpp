#include "CInfoCmd.h"
#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_idf_version.h"
#include "soc/soc_caps.h"


CInfoCmd::CInfoCmd() : AConsole2Cmd(
    "info",
    "Display ESP32 architecture information"){};

int CInfoCmd::exec_func_cb(int argc, char* argv[]) {
    esp_chip_info_t chipnfo;
    esp_chip_info(&chipnfo);

    printf("\nESP32 Architecture Information\n");
    printf("==============================\n");

    // Chip model
    printf("Chip model: ESP32");
    switch(chipnfo.model) {
        case CHIP_ESP32S2:
            printf("-S2");
            break;
        case CHIP_ESP32S3:
            printf("-S3");
            break;
        case CHIP_ESP32C2:
            printf("-C2");
            break;
        case CHIP_ESP32C3:
            printf("-C3");
            break;
        case CHIP_ESP32C6:
            printf("-C6");
            break;
        case CHIP_ESP32H2:
            printf("-H2");
            break;
        case CHIP_ESP32P4:
            printf("-P4");
            break;
        case CHIP_ESP32C61:
            printf("-C61");
            break;
        case CHIP_ESP32C5:
            printf("-C5");
            break;
        case CHIP_ESP32H21:
            printf("-H21");
            break;
        case CHIP_ESP32H4:
            printf("-H4");
            break;
        case CHIP_POSIX_LINUX:
            printf("Linux");
            break;
        default:
            break;
    }

    // Cores
    printf(" Cores: %d\n", chipnfo.cores);

    // Features
    printf("Features:\n");
    if (chipnfo.features & CHIP_FEATURE_WIFI_BGN) {
        printf(" WiFi/BG\n");
    }
    if (chipnfo.features & CHIP_FEATURE_BT) {
        printf(" Bluetooth\n");
    }
    if (chipnfo.features & CHIP_FEATURE_BLE) {
        printf(" BLE\n");
    }
    if (chipnfo.features & CHIP_FEATURE_IEEE802154) {
        printf(" IEEE 802.15.4 LR-WPAN\n");
    }
    if (chipnfo.features & CHIP_FEATURE_EMB_FLASH) {
        printf(" Embedded Flash\n");
    }

    // Revision
    printf("Revision: %d\n", chipnfo.revision);

    // Flash info
    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("Flash size: %ld KB\n", flash_size / 1024);
    }

    // ESP-IDF version
    printf("ESP-IDF version: %d.%d.%d\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);

    // Build date
    printf("Build date: %s %s\n", __DATE__, __TIME__);

    return ESP_OK;
}
