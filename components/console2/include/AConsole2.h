#pragma once

#include <string>
#include "sdkconfig.h"
#include "esp_err.h"
#include "AConsole2Cmd.h"
#include <freertos/task.h>

#define CONSOLE2_PROMPT_MAX_LENGTH 16

typedef enum {
    CONSOLE_STATUS_NOT_INITIALIZED = 0x100,
    CONSOLE_STATUS_INITIALIZED,
    CONSOLE_STATUS_RUNNED
} console2_status;

/* Console command history can be stored to and loaded from a memory. */
#if CONFIG_CONSOLE_STORE_HISTORY
 // TODO: Реализовать историю в память
#endif // CONFIG_CONSOLE_STORE_HISTORY


using namespace std;


class AConsole2 {

    
public:

    /**
     * Получить текущее состояние консоли
     */
    console2_status getState(void) noexcept { return conStatus; }

    /**
     * Инициализация и деинициализация консоли
     * 
     * @details Инициали
     */
    esp_err_t initialize(void) noexcept;

    /**
     * Регистрация команды в консоли
     */
    static esp_err_t registerCommand(AConsole2Cmd& command) noexcept;

    /**
     * Запуск консоли пользователем
     * 
     * @details Команда должна создавать поток (задачу) в которой будет работать библиотека консоли *linenoise*. Вызывает метод `start()`
     */
    esp_err_t run(void) noexcept;


    esp_err_t stop(void) noexcept;


protected:

    // Вызывается из initialize()
    virtual esp_err_t init_periph(void) = 0;

    // Вызывается из run()
    virtual esp_err_t start(void) = 0;

    static const char* getHelp2EnterConsoleMsg(void) {
        static const char* greetings = "Press <ENTER> to enter command line interface.\n\r";
        return greetings;
    }

    /**
     * Return prompt string if it is set, otherwise return default prompt string
     */
    const char* setup_prompt(const char* prompt_str = NULL);

    console2_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;

private:

    TaskHandle_t _xTaskHandle;
    static void _vConsole2Task(void* pvParameters);
    char _prompt[CONSOLE2_PROMPT_MAX_LENGTH];
    esp_err_t _init_console_library(void);
};

