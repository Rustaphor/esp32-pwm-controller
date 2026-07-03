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


class CUartConsole2 : public AConsole2{

    TaskHandle_t _xTaskHandle;
    QueueHandle_t _hUartQueue = NULL;
    uart_event_t _event;
    unsigned char _ch[1];
    size_t _buffered_size;

public:
    CUartConsole2();

    /**
     * Dispatch infinitive Loop from any Task (no blocking)
     * 
     * @details Функцию необходлимо постоянно опрашивать в бесконечном цикле какой-либо задачи (FreeRTOS Task). \
     * Наиболее удобно низкоприоритетных задачах бездействия (Idle).
     */
    inline void dispatch(void){

        // Check UART_DATA event data (start console) without blocking
        if (_hUartQueue && xQueueReceive(_hUartQueue, (void*) &_event, 0) == pdTRUE) {
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

    esp_err_t stop(void) noexcept;

protected:

    esp_err_t start(void) override;
    esp_err_t init_periph(void) override;

private:

    static void _vConsole2Task(void* pvParameters);
    esp_err_t _init_console_library(void);
    inline esp_err_t _init_n_enable_isr(void); 
    inline esp_err_t _disable_isr(void) { return uart_disable_rx_intr(CONSOLE2_UART_NUM); }
    inline void _register_custom_cmds() {
        auto it_cmd { AConsole2::getCommandList().cbegin() };
        while (it_cmd != AConsole2::getCommandList().cend()){
            esp_console_cmd_register((*it_cmd)->getCommand());
            ++it_cmd;
        }
    };
};
