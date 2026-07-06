#pragma once

#include "AConsole2Cmd.h"
#include <string>

using namespace std;

/**
 * @brief Команда info - краткие сведения об архитектуре ESP32
 */
class CInfoCmd : public AConsole2Cmd {

public:
    CInfoCmd();

    /**
     * @brief Функция выполнения команды
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @return 0 при успехе
     */
    int exec_func_cb(int argc, char* argv[]) override;

};
