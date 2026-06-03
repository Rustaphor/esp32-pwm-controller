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
} console2_status;

/* Console command history can be stored to and loaded from a memory.
 * TODO: Реализовать историю в память
 */
#if CONFIG_CONSOLE_STORE_HISTORY


#endif // CONFIG_CONSOLE_STORE_HISTORY


using namespace std;

// Component Log Tag
static const char logConsole2Tag[] = "Console";

enum CONSOLE2_KEY_ACTION {
	CTRL_C = 3,         /* Ctrl-c */
    CTRL_X = 24,        /* Ctrl-x */
	CTRL_D = 4,         /* Ctrl-d */
	ENTER = 13,         /* Enter */
	ESC = 27            /* Escape */
};


class AConsole2 {

    static list<AConsole2Cmd&> _commands;   // Подобрать список уникальных значений
    
public:

    /**
     * Инициализация и деинициализация консоли
     * 
     * @details Инициали
     */
    esp_err_t initialize(void);

    /**
     * Деинициализация консоли
     * 
     * @details Метод вызывается по команде выхода из консоли (`exit`, `Ctrl-c`, `Esc`, `Ctrl-x`)
     */
    esp_err_t deinitialize(void);

    /**
     * Регистрация команды в консоли
     */
    static esp_err_t registerCommand(AConsole2Cmd& command) noexcept;

    /**
     * Запуск консоли из режима ожидания
     * 
     * @details Команда должна создавать поток (задачу) в которой будет работать библиотека консоли *linenoise*
     */
    virtual esp_err_t run(void) = 0;


protected:

    // Вызывается из initialize()
    virtual esp_err_t init_periph(void) = 0;

    /**
     * Деинициализация периферии консоли
     * 
     * @details Метод должен освобожлать все ресурсы, работать противоположно методу `run`. По результату консоль должна вернуть в режим повторного запуска.
     */
    virtual esp_err_t deinit_periph(void) = 0;
    // virtual boolean init_commands(void) {

    char* setup_prompt(const char* prompt_str = NULL);

    console2_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;
    static char prompt[CONSOLE2_PROMPT_MAX_LENGTH];

private:

    esp_err_t _init_console_library(void);
};

