#include "CHwManager.h"
#include "CFanMotor.h"
#include "esp_log.h"


CFanMotor fanmot;

const char* TAG = "HwManager";


void CHwManager::initAll() {

    int errcode;

    for (auto& device : m_devices) {
        if (device) {
            if (!device->initialize()) {
                // Здесь можно решить, продолжать ли инициализацию остальных
                // или прерывать весь процесс
            }
        }
    }

    // TODO: Времянка
    errcode = fanmot.initialize();
    if(errcode != DEVICE_OK){
        ESP_LOGE(TAG,"Device %s init error 0x%X", fanmot.getName(), errcode);
    } else {
        ESP_LOGD(TAG, "Device %s initialized", fanmot.getName());
    }
    // end

    ESP_LOGI(TAG, "All devices has been initialized");
}

void CHwManager::deinitAll() {
    // Деинициализируем в обратном порядке
    for (auto it = m_devices.rbegin(); it != m_devices.rend(); ++it) {
        if (*it) {
            (*it)->deinitialize();
        }
    }
    // Очищаем вектор
    m_devices.clear();

    ESP_LOGI(TAG, "All devices has been uninitialized");
}

void CHwManager::addDevice(std::shared_ptr<ADevice> device) {
    if (device) {
        m_devices.push_back(device);
    }
}