#include "pwmconhelp.h"
#include "esp_log.h"

// For Debug
#include <iostream>

static const char* logTAG = __FILE_NAME__;       // Локальный тег логирования модуля

int CPwmCtrlConsoleHelper::command_callback(int argc, char **argv)
{
        ESP_LOGI(logTAG,"Number of agguments: %d",argc);
        for (int i = 0; i < argc; ++i) {
            std::cout << argv[i] << "\n";
        }

        return ESP_OK;
}
