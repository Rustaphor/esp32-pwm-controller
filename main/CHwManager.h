#pragma once

#include "ADevice.h"
#include <vector>
#include <memory>

using namespace std;

class CHwManager {
public:
    CHwManager() = default;
    ~CHwManager() = default;

    // Убираем копируемые операции, если менеджеры не должны дублироваться
    CHwManager(const CHwManager&) = delete;
    CHwManager& operator=(const CHwManager&) = delete;

    // Основные методы инициализации и деинициализации всех управляемых устройств
    void initAll();
    void deinitAll();

    // Дополнительно: метод для добавления устройства
    void addDevice(shared_ptr<ADevice> device);

private:
    vector<shared_ptr<ADevice>> m_devices;
};
