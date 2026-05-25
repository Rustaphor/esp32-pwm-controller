#pragma once

#include <string>
#include <list>
#include "sdkconfig.h"
#include "esp_err.h"
#include "AConsole2Cmd.h"

// Строка приветствия
#define PROMPT_STR CONFIG_IDF_TARGET

typedef enum {
    CONSOLE_STATUS_OK = 0,
    CONSOLE_STATUS_NOT_INITIALIZED = 0x100,
    CONSOLE_STATUS_INITIALIZED,
    CONSOLE_STATUS_RUNNING
} console_status;

/* Console command history can be stored to and loaded from a memory.
 * TODO: Реализовать историю в память
 */
#if CONFIG_CONSOLE_STORE_HISTORY


#endif // CONFIG_CONSOLE_STORE_HISTORY


using namespace std;



class AConsole2 {

    static list<AConsole2Cmd&> _commands;   // Подобрать список уникальных значений
    
public:

    esp_err_t initialize(void);
    static esp_err_t registerCommand(AConsole2Cmd& command) noexcept;

protected:

    /**
     * Вызывается из initialize()
     */
    virtual esp_err_t init_periph(void) = 0;

    console_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;

private:

    esp_err_t _init_console_library(void);
};

