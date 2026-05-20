#pragma once

#include <string>
#include "sdkconfig.h"
#include "esp_err.h"

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
public:

    esp_err_t initialize(void);
    static esp_err_t registerCommand(const string& command, const string& help, const string& hint, int (*func)(int, char**));

protected:

    /**
     * Вызывается из initialize()
     */
    virtual esp_err_t init_periph(void) = 0;

    console_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;

private:

    esp_err_t _init_console_library(void);
};

