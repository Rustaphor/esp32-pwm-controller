#pragma once


#include "AConsole2.h"
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
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

// CONFIG_ESP_CONSOLE_UART_NUM


class CUartConsole2 : public AConsole2{

    QueueHandle_t _hUartQueue;
    uart_event_t event;
    unsigned char ch[1];

public:
    CUartConsole2() {};

    // Dispatch infinitive Loop from any Task (no blocking)
    inline void dispatch_loop(void){

        // Wait until UART_DATA event is triggered
        if (xQueueReceive(_hUartQueue, (void*) &event, 0)) {

            // Данные в буфере
            if (event.type == UART_DATA && conStatus == CONSOLE_STATUS_INITIALIZED) {
                uart_read_bytes(CONSOLE2_UART_NUM, ch, sizeof(ch), portMAX_DELAY);

                // Команда запуска консоли
                if (ch[0] == ENTER) {
                    run();
                } else if (ch[0] == CTRL_C || ch[0] == CTRL_X) {
                    // TODO: Реализовать выход из консоли
                }

                // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                // record the position. We should set a larger queue size.
                // Directly flush the rx buffer here.
                uart_flush_input(CONSOLE2_UART_NUM);
            }

        }
    }

protected:

    esp_err_t start(void) override;
    esp_err_t init_periph(void) override;

private:

    inline esp_err_t _init_n_enable_isr(void);
    inline esp_err_t _disable_isr(void) { return uart_disable_rx_intr(CONSOLE2_UART_NUM); }
};
