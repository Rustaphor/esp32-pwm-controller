#include "CUartConsole2.h"
#include <stdio.h>
#include <fcntl.h>
#include "esp_log.h"
#include "esp_console.h"
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "driver/uart_vfs.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_vfs_cdcacm.h"
// #include "console2_defs.h"

static const char* logTAG = __FILE_NAME__;

CUartConsole2::CUartConsole2() {
    ESP_LOGI(logTAG, "CUartConsole2 constructor called");
}

CUartConsole2::~CUartConsole2() {
    ESP_LOGI(logTAG, "CUartConsole2 destructor called");
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
#if SOC_UART_SUPPORT_REF_TICK
            .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
            .source_clk = UART_SCLK_XTAL,
#endif
    };

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

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

// esp_err_t CUartConsole2::init() {
//     if (_conStatus == CONSOLE_STATUS_INITIALIZED) {
//         ESP_LOGW(logTAG, "Console already initialized");
//         return ESP_ERR_NOT_ALLOWED;
//     }

//     // Initialize console peripherals
//     _init_console_periphs();
//     _init_console_library();
    
//     // Initialize console commands
//     esp_console_register_help_command();
//     register_system_common();
    
//     mInitialized = true;
//     ESP_LOGI(logTAG, "Console initialized");
// }

bool CUartConsole2::registerCommand(const string& command, const string& help, 
                                const string& hint, int (*func)(int, char**)) {
    if (!mInitialized) {
        ESP_LOGE(logTAG, "Console not initialized");
        return false;
    }
    
    esp_console_cmd_t cmd = {
        .command = command.c_str(),
        .help = help.c_str(),
        .hint = hint.c_str(),
        .func = func
    };
    
    esp_err_t err = esp_console_cmd_register(&cmd);
    if (err != ESP_OK) {
        ESP_LOGE(logTAG, "Failed to register command '%s': %s", 
                command.c_str(), esp_err_to_name(err));
        return false;
    }
    
    ESP_LOGI(logTAG, "Command '%s' registered successfully", command.c_str());
    return true;
}


