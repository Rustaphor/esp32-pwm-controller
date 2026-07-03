#include "CUartConsole2.h"
#include <stdio.h>
#include <fcntl.h>
#include "esp_log.h"
#include "esp_console.h"
#include "soc/soc_caps.h"
#include "driver/uart_vfs.h"
#include "hal/uart_ll.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_vfs_cdcacm.h"
#include "linenoise/linenoise.h"


// #include "console2_defs.h"


esp_err_t CUartConsole2::start(void)
{
    esp_err_t result = ESP_OK;

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    // _disable_isr();
#endif
    
    // TODO: Реализовать запуск консоли

    return result;
}


esp_err_t CUartConsole2::init_periph(void)
{
   /* Очистка буфера потока stdout перед реконфигурированием */
    fflush(stdout);
    fsync(fileno(stdout));

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
#if SOC_UART_SUPPORT_REF_TICK
            .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
            .source_clk = UART_SCLK_XTAL,
#endif
            .flags = {
                .allow_pd = 0,
                .backup_before_sleep = 0
            }
    };

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONSOLE2_UART_NUM, CONSOLE2_UART_BUFF_SIZE, 0, sizeof(_ch), &_hUartQueue, 0) );
    ESP_ERROR_CHECK( uart_param_config(CONSOLE2_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Enable interrupt to check _start_ command */
    if (_init_n_enable_isr()) {
        ESP_LOGE(logConsole2Tag, "Error initializing UART interrupt");
    }

#elif defined(CONFIG_ESP_CONSOLE_USB_CDC)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_cdcacm_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_cdcacm_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Enable blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Enable blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    usb_serial_jtag_driver_config_t jtag_config = {
        .tx_buffer_size = 256,
        .rx_buffer_size = 256,
    };

    /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( usb_serial_jtag_driver_install(&jtag_config));

    /* Tell vfs to use usb-serial-jtag driver */
    usb_serial_jtag_vfs_use_driver();

#else
#error Unsupported console type
#endif

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    return ESP_OK;
}

inline esp_err_t CUartConsole2::_init_n_enable_isr(void)
{
    // Configure a UART interrupt threshold and timeout
    const uart_intr_config_t uart_intr = {
        .intr_enable_mask = UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_FULL,
        .rx_timeout_thresh = 100,
        .txfifo_empty_intr_thresh = 100,
        .rxfifo_full_thresh = 100
    };
    esp_err_t result = uart_intr_config(CONSOLE2_UART_NUM, &uart_intr);
    if (!result) {
        result = uart_enable_rx_intr(CONSOLE2_UART_NUM);
    }

    return result;
}

