#pragma once

#include <string>
#include <list>
#include "sdkconfig.h"
#include "esp_err.h"
#include "AConsole2Cmd.h"


#define CONSOLE2_PROMPT_MAX_LENGTH 16

typedef enum {
    CONSOLE_STATUS_NOT_INITIALIZED = 0x100,
    CONSOLE_STATUS_INITIALIZED,
    CONSOLE_STATUS_RUNNING,
    CONSOLE_STATUS_SUSPENDED
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

    esp_err_t run(void);


protected:

    /**
     * Вызывается из initialize()
     */
    virtual esp_err_t init_periph(void) = 0;
    // virtual boolean init_commands(void) {

    char* setup_prompt(const char* prompt_str = NULL);

    console_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;
    static char prompt[CONSOLE2_PROMPT_MAX_LENGTH];

private:

    esp_err_t _init_console_library(void);
};

