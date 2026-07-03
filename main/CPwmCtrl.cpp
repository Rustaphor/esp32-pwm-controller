#include "CPwmCtrl.h"
#include "esp_log.h"



// Реализация виртуальной функции
int CPwmCtrl::exec_func_cb(int argc, char* argv[]) {
    // Здесь будет реализация команды
    // Пока просто возвращаем 0 (успех)
    ESP_LOGI("CPwmCtrl", "exec_func_cb");
    return 0;
}
