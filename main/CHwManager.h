#pragma once

#include "ADevice.h"
#include <vector>
#include <optional>

using namespace std;

class CHwManager {

public:

    /**
     * @brief Конструкторы всех устройств создаются тут
     */
    CHwManager();

    // Убираем копируемые операции, если менеджеры не должны дублироваться
    CHwManager(const CHwManager&) = delete;
    CHwManager& operator=(const CHwManager&) = delete;

    // Основные методы инициализации и деинициализации всех управляемых устройств
    void initAll();
    void deinitAll();

    // Дополнительно: метод для добавления устройства
    void addDevice(ADevice& device) {
        _m_devices.push_back(&device);
    };

    optional<const ADevice*> getDeviceByName(const char * name) const noexcept;

private:

    const char* TAG = "HwManager";
    vector<const ADevice*> _m_devices;
};
