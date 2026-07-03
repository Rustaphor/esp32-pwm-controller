#include "CUartConsole2.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "esp_log.h"
#include "esp_console.h"
#include "cmd_system.h"
#include "soc/soc_caps.h"
#include "driver/uart_vfs.h"
#include "hal/uart_ll.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_vfs_cdcacm.h"
#include "linenoise/linenoise.h"
#include "console2_defs.h"


// System exit console command
esp_console_cmd_t cmd_exit = {
    .command = "exit",
    .help = "Exit from console",
    .hint = NULL,
    .func = nullptr,
    .argtable = nullptr,
    .func_w_context = [](void *context, int argc, char **argv) {
        return 0xDEAD; // CONSOLE2_SIG_TEMRINATE
    }
};


void CUartConsole2::_vConsole2Task(void *pvParameters)
{
    esp_err_t err = ESP_OK;
    char* line; int ret;
    CUartConsole2 *pConsole = (CUartConsole2*) pvParameters;

    /* Console Task Initializer */
    err = pConsole->_init_console_library();
    if (err) {
        ESP_LOGE(logConsole2Tag, "Failed to initialize console library!");
        goto err_task_exit;
    } 

    /* Register system commands */
    if (esp_console_register_help_command() != ESP_OK) {
        goto err_task_exit;
    }

    if (esp_console_cmd_register(&cmd_exit) != ESP_OK) {
        goto err_task_exit;
    }

    extern void register_system_common(void);
    register_system_common();

    // Блок регистрации команд пользователя
    pConsole->_register_custom_cmds();

    // Print Greeting message
    printf(getMsgStandartGreeting());

    if (linenoiseIsDumbMode()) {
        printf("\n \
Your terminal application does not support escape sequences.\n \
Line editing and history features are disabled.\n \
On Windows, try using Windows Terminal or Putty instead.\n");
    }

    /* Main loop */
    while(true) {

        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        line = linenoise(pConsole->prompt);

        if (line == NULL) { /* Ignore empty lines */
            continue;
        }

        /* Add the command to the history if not empty*/
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif // CONFIG_CONSOLE_STORE_HISTORY
        }

        /* Try to run the command */
        err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (ret == 0xDEAD) { // CONSOLE2_SIG_TEMRINATE
            linenoiseFree(line);
            break;
        } else if (err == ESP_OK && ret) {
            printf("Command returned non-zero error code: 0x%x\n", ret);
        } else if (err != ESP_OK) {
            printf("Internal error: %d\n", err);
        }

        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    xSemaphoreTake(pConsole->hSem, portMAX_DELAY);
    printf("Console terminated.\n");
    pConsole->conStatus = CONSOLE_STATUS_INITIALIZED;

#ifdef CONFIG_CONSOLE2_PRESSENTER_MSG
   printf("Press <ENTER> to enter command line interface.\n\r");
#endif

err_task_exit:
    esp_console_deinit();
    xSemaphoreGive(pConsole->hSem);
    vTaskDelete(NULL);
}

esp_err_t CUartConsole2::_init_console_library(void)
{
    /* Initialize the console */
    esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
    console_config.max_cmdline_length = CONFIG_CONSOLE2_MAX_COMMAND_LINE_LENGTH;
    console_config.max_cmdline_args = CONSOLE2_MAX_CMDLINE_ARGS;
#if CONFIG_LOG_COLORS
    console_config.hint_color = atoi(LOG_COLOR_CYAN);
#endif


    esp_err_t result = esp_console_init(&console_config);
    assert(result == ESP_OK);
    if (result) return result;

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(CONSOLE2_HISTORY_BUFF_SIZE);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

    /* Figure out if the terminal supports escape sequences */
    const int probe_status = linenoiseProbe();
    if (probe_status) {         /* zero indicates success */
        linenoiseSetDumbMode(1);
    }

    return ESP_OK;
}

CUartConsole2::CUartConsole2()
{
    cmd_exit.context = this;
}

esp_err_t CUartConsole2::stop(void) noexcept
{
    ESP_LOGW(logConsole2Tag, "Warning! Console stop operation is not supported!");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t CUartConsole2::start(void)
{
    esp_err_t result = ESP_OK;

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    // _disable_isr();
#endif
    
    if (xTaskCreate(_vConsole2Task, "console", CONFIG_CONSOLE2_TASK_HEAP_STACK_SIZE, this, tskIDLE_PRIORITY, &_xTaskHandle) != pdTRUE) {
        ESP_LOGE(logConsole2Tag, "Error: %d. Failed to run console task", result);
        result = ESP_FAIL;
    }
    xTaskNotify(_xTaskHandle, 0, eNoAction);

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
