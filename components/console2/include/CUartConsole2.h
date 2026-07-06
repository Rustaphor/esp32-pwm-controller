#pragma once

#include "AConsole2.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"


/*
 * Warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif

#define CONSOLE2_UART_NUM UART_NUM_0
#define CONSOLE2_UART_BUFF_SIZE 256
#define CONSOLE2_UART_PATTERT ENTER


/**
 * @brief Класс UART-консоли на основе FreeRTOS.
 *
 * @details Реализует интерактивную консольную оболочку через UART с использованием
 *          FreeRTOS для обработки событий. Данный класс наследуется от AConsole2
 *          и предоставляет метод dispatch() для неблокирующего опроса событий UART.
 */
class CUartConsole2 : public AConsole2{

    TaskHandle_t _xTaskHandle;
    QueueHandle_t _hUartQueue = NULL;
    uart_event_t _event;
    unsigned char _ch[1];
    size_t _buffered_size;

public:
    /**
     * @brief Конструктор UART-консоли.
     *
     * @details Инициализирует объект консоли. Физическая инициализация периферии
     *          происходит при вызове метода initialize().
     */
    CUartConsole2();

    /**
     * @brief Неблокирующий опрос событий UART.
     *
     * @details Должна вызываться в бесконечном цикле задачи (обычно в idle-задаче)
     *          для обработки событий UART. Проверяет наличие данных в буфере UART
     *          и вызывает обработку при нажатии Enter.
     *
     * @note Метод не блокирует выполнение и может быть вызван из любой задачи.
     */
    inline void dispatch(void){

        // Check UART_DATA event data (start console) without blocking
        if (_hUartQueue && xQueueReceive(_hUartQueue, (void*) &_event, 0) == pdPASS) {

            if (conStatus != CONSOLE_STATUS_INITIALIZED) return;

            // Данные в буфере
            if (_event.type == UART_DATA) {
                _ch[0] = 0;
                uart_read_bytes(CONSOLE2_UART_NUM, _ch, sizeof(_ch), portMAX_DELAY);

                // Проверка клавиши 'Enter'
                if (_ch[0] == ENTER) {
                    run();
                }

                uart_flush_input(CONSOLE2_UART_NUM);
                // xQueueReset(_hUartQueue);
            }
        }
    }

    /**
     * @brief Остановка консоли.
     *
     * @details Останавливает работу консоли и освобождает ресурсы.
     *          Перегружает виртуальный метод AConsole2::stop().
     *
     * @return ESP_OK при успешной остановке, иная ошибка в противном случае.
     */
    esp_err_t stop(void) noexcept;

protected:

    /**
     * @brief Запуск консоли (переопределяемый метод).
     *
     * @details Вызывается из AConsole2::run() для запуска консоли.
     *          Переопределяется в производных классах для настройки периферии.
     *
     * @return ESP_OK при успешном запуске, иная ошибка в противном случае.
     */
    esp_err_t start(void) override;

    /**
     * @brief Инициализация периферии UART (переопределяемый метод).
     *
     * @details Вызывается из AConsole2::initialize() для настройки UART.
     *          Переопределяется в производных классах для настройки конкретного
     *          оборудования.
     *
     * @return ESP_OK при успешной инициализации, иная ошибка в противном случае.
     */
    esp_err_t init_periph(void) override;

private:

    /**
     * @brief Статическая функция задачи консоли.
     *
     * @details Основная задача FreeRTOS для обработки консольного ввода.
     *          Создается при вызове AConsole2::run().
     *
     * @param[in] pvParameters Параметры задачи (указатель на контекст).
     */
    static void _vConsole2Task(void* pvParameters);

    /**
     * @brief Инициализация библиотеки консоли.
     *
     * @details Настраивает библиотеку linenoise и регистрирует все
     *          зарегистрированные команды.
     *
     * @return ESP_OK при успешной инициализации, иная ошибка в противном случае.
     */
    esp_err_t _init_console_library(void);

    /**
     * @brief Инициализация и включение прерываний UART.
     *
     * @details Настраивает UART на генерацию прерываний при получении данных.
     *
     * @return ESP_OK при успешной настройке, иная ошибка в противном случае.
     */
    inline esp_err_t _init_n_enable_isr(void);

    /**
     * @brief Отключение прерываний UART.
     *
     * @details Отключает генерацию прерываний от UART.
     *
     * @return ESP_OK при успешном отключении, иная ошибка в противном случае.
     */
    inline esp_err_t _disable_isr(void) { return uart_disable_rx_intr(CONSOLE2_UART_NUM); }

    /**
     * @brief Регистрация пользовательских команд.
     *
     * @details Регистрирует все команды из списка AConsole2 в библиотеке esp_console.
     */
    inline void _register_custom_cmds() {
        auto it_cmd { AConsole2::getCommandList().cbegin() };
        while (it_cmd != AConsole2::getCommandList().cend()){
            esp_console_cmd_register((*it_cmd)->getCommand());
            ++it_cmd;
        }
    };
};
