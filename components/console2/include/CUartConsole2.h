#pragma once


#include "AConsole2.h"
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"
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

// CONFIG_ESP_CONSOLE_UART_NUM

extern const char* logCons2TAG;


class CUartConsole2 : public AConsole2{

    static QueueHandle_t _huart_queue;

public:
    CUartConsole2();
    ~CUartConsole2();

    inline void dispatchFromTask(void){
        uart_event_t event;
        uint8_t pByte[2];

        // Wait until UART_DATA event is triggered
        if (xQueueReceive(_huart_queue, (void*) &event, (TickType_t) portMAX_DELAY)) {

            ESP_LOGI(logCons2TAG, "Interrupt type is: %d", event.type);

            
            // Данные в буфере
            if (event.type == UART_DATA) {

                    ESP_LOGI(logCons2TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(CONSOLE2_UART_NUM, pByte, sizeof(pByte), portMAX_DELAY);

                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(CONSOLE2_UART_NUM);
            }
        }
    }

protected:

    esp_err_t init_periph(void) override;

    esp_err_t on(void);
    esp_err_t off(void);

private:

    inline esp_err_t _init_isr(void) {
        // Configure a UART interrupt threshold and timeout
        const uart_intr_config_t uart_intr = {
            .intr_enable_mask = UART_INTR_CMD_CHAR_DET,
            .rx_timeout_thresh = 100,
            .txfifo_empty_intr_thresh = 100,
            .rxfifo_full_thresh = 100
        };
        return uart_intr_config(CONSOLE2_UART_NUM, &uart_intr);
    }

    inline esp_err_t _disable_isr(void) { return uart_disable_rx_intr(CONSOLE2_UART_NUM); }
    
};
