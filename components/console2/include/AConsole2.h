#pragma once
#include <vector>
#include "esp_err.h"
#include "AConsole2Cmd.h"
#include "freertos/semphr.h"


#define CONSOLE2_PROMPT_MAX_LENGTH 16

typedef enum {
    CONSOLE_STATUS_NOT_INITIALIZED = 0x100,
    CONSOLE_STATUS_INITIALIZED,
    CONSOLE_STATUS_RUNNED
} console2_stat;

/* Console command history can be stored to and loaded from a memory. */
#if CONFIG_CONSOLE_STORE_HISTORY
 // TODO: Реализовать историю в память
#endif // CONFIG_CONSOLE_STORE_HISTORY


using namespace std;


class AConsole2 {
    

public:


    /**
     * @brief Получить текущее состояние консоли.
     *
     * @return Текущее состояние консоли (console2_stat).
     */
    console2_stat getState(void) noexcept { return conStatus; }

    /**
     * @brief Инициализация консоли.
     *
     * @details Инициализирует периферию консоли. Данный метод является потокобезопасным.
     *
     * @return ESP_OK при успешной инициализации, иная ошибка в противном случае.
     */
    esp_err_t initialize(void) noexcept;

    /**
     * @brief Регистрация команды в консоли.
     *
     * @param[in] command Ссылка на объект команды (AConsole2Cmd), который необходимо зарегистрировать.
     */
    static void registerCommand(AConsole2Cmd& command) noexcept;

    /**
     * @brief Удаление команды из списка зарегистрированных в консоли.
     *
     * @param[in] command Ссылка на объект команды (AConsole2Cmd), который необходимо удалить.
     */
    static void unregisterCommand(AConsole2Cmd& command) noexcept;

    /**
     * @brief Запуск консоли пользователем.
     *
     * @details Запускает работу консоли. В случае UART-консоли создается отдельная задача,
     *          и запускается библиотека консоли *linenoise* для обработки ввода.
     *          Метод является потокобезопасным.
     *
     * @return ESP_OK при успешном запуске, иная ошибка в противном случае.
     */
    esp_err_t run(void) noexcept;

    /**
     * @brief Остановка консоли.
     *
     * @details Останавливает работу консоли. В случае UART-консоли отправляется сигнал
     *          о завершении потока. Метод является потокобезопасным.
     *
     * @return ESP_OK при успешной остановке, иная ошибка в противном случае.
     */
    esp_err_t stop(void) noexcept;


protected:

    SemaphoreHandle_t hSem = NULL;
    console2_stat conStatus = CONSOLE_STATUS_NOT_INITIALIZED;
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
     * @brief Возвращает строку приглашения, если она установлена, иначе возвращает строку приглашения по умолчанию.
     *
     * @param[in] prompt_str Необязательная строка приглашения. Если NULL, возвращается текущая строка приглашения.
     * @return Указатель на строку приглашения.
     */
    const char* setup_prompt(const char* prompt_str = NULL);

};
