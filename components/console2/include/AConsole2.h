#pragma once

#include <string>
#include <vector>
#include "esp_err.h"
#include "AConsole2Cmd.h"
#include "freertos/semphr.h"


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
     * @details Инициализация периферии консоли. Потокобезопасно.
     */
    esp_err_t initialize(void) noexcept;

    /**
     * Регистрация команды в консоли
     */
    static void registerCommand(AConsole2Cmd& command) noexcept;

    /**
     * Запуск консоли пользователем
     * 
     * @details Команда запускает консоль, в случае UART, создается задача и работает библиотека консоли *linenoise*. Потокобезопасно.
     */
    esp_err_t run(void) noexcept;


    /**
     * Остановка консоли
     * 
     * @details Останавливает консоль, в случае UART, отправляется сигнал о завершении потока. Потокобезопасно.
     */
    esp_err_t stop(void) noexcept;


protected:

    SemaphoreHandle_t hSem = NULL;
    console2_status conStatus = CONSOLE_STATUS_NOT_INITIALIZED;
    char prompt[CONSOLE2_PROMPT_MAX_LENGTH];

    AConsole2() {
        vSemaphoreCreateBinary(hSem);
    }

    // Вызывается из initialize()
    virtual esp_err_t init_periph(void) = 0;

    // Вызывается из run()
    virtual esp_err_t start(void) = 0;

    // Вызывается из stop()
    virtual esp_err_t terminate(void) { return ESP_OK; };

    // Стандартные статические сообщения (подсказки)
    static const char* getMsgHelp2EnterConsole(void);
    static const char* getMsgStandartGreeting(void);
    static const vector<const AConsole2Cmd*>& getCommandList(void) noexcept;

    /**
     * Return prompt string if it is set, otherwise return default prompt string
     */
    const char* setup_prompt(const char* prompt_str = NULL);

};

