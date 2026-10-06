#include "CHwManager.h"
#include "CFanMotor.h"
#include "esp_log.h"


CFanMotor fanmot;

CHwManager::CHwManager()
{
    addDevice(fanmot);
}

void CHwManager::initAll()
{
    esp_err_t errcode;
    ADevice* pDevice;
    size_t num_devices = _m_devices.size();

    for (auto pDev : _m_devices) {
        if (pDev && pDev->getCurrentState().sysState == DEV_NOT_INITIALIZED) {
            pDevice = const_cast<ADevice*>(pDev);
            errcode = pDevice->initialize();
            if (errcode != DEVICE_OK) {
                ESP_LOGE(TAG,"Device %s init error 0x%X", pDevice->getDeviceName(), errcode);
            } else {
                ESP_LOGD(TAG, "Device %s initialized", pDevice->getDeviceName());
                num_devices--;
            }
        }
    }

    if (num_devices == 0) {
        ESP_LOGI(TAG, "All devices has been initialized");
    } else {
        ESP_LOGW(TAG, "%zu devices are not initialized", num_devices);
    }
}

void CHwManager::deinitAll() {
    esp_err_t errcode;
    ADevice* pDevice;
    size_t num_devices = _m_devices.size();

    for (auto pDev : _m_devices) {
        if (!pDev) {
            ESP_LOGW(TAG, "Null pointer in device list");
            continue;
        }
        if (pDev->getCurrentState().sysState == DEV_INITIALIZED || pDev->getCurrentState().sysState == DEV_FAILURE) {
            pDevice = const_cast<ADevice*>(pDev);
            errcode = pDevice->deinitialize();
            if (errcode != DEVICE_OK) {
                ESP_LOGE(TAG,"Device %s deinit error 0x%X", pDevice->getDeviceName(), errcode);
            } else {
                ESP_LOGD(TAG, "Device %s initialized", pDevice->getDeviceName());
                num_devices--;
            }
        }
    }

    if (num_devices == 0) {
        ESP_LOGI(TAG, "All devices has been uninitialized");
    } else {
        ESP_LOGW(TAG, "%zu devices are not uninitialized", num_devices);
    }
}

optional<const ADevice*> CHwManager::getDeviceByName(const char *name) const noexcept
{
    for (auto pDev : _m_devices) {
        if (pDev && strcmp(pDev->getDeviceName(), name) == 0) {
            return pDev;
        }
    }

    return nullopt;
}
