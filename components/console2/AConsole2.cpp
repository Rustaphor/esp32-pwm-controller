#include "AConsole2.h"
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "console2_defs.h"

static const char* logTAG = "Console2";

esp_err_t AConsole2::initialize(void)
{
    esp_err_t result = ESP_OK;

    if (conStatus != CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        goto end_init;
    }

    result = init_periph();
    if (result) {
        ESP_LOGE(logTAG, "Failed to initialize console peripherals!");
        goto end_init;
    }

    result = _init_console_library();
    if (result) {
        ESP_LOGE(logTAG, "Failed to initialize console library!");
        goto end_init;
    }

    conStatus = CONSOLE_STATUS_INITIALIZED;
    ESP_LOGI(logTAG, "Console2 component initialized");

end_init:
    return result;
}

esp_err_t AConsole2::registerCommand(AConsole2Cmd &command)
{
    // _commands.push_back(command);
    ESP_LOGI(logTAG, "Registering console command: %s", command._console_cmd.command);
    return esp_console_cmd_register(&command._console_cmd);
}

esp_err_t AConsole2::run(void)
{
    esp_err_t result;
    if (conStatus != CONSOLE_STATUS_INITIALIZED) {
        return ESP_ERR_INVALID_STATE;
    }

    auto result = xTaskCreate(_vTaskConsole2, "uart_console2_task", CONFIG_CONSOLE2_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, &_proc_data_hdl);
    return ESP_OK;
}


void AConsole2::_vTaskConsole2(void *pvParameters)
{
    TaskHandle_t proc_data_task_hdl = (TaskHandle_t) pvParameters;

    ESP_LOGI(logTAG, "Start UART console task started");

    printf("\n"
           "This is an example of ESP-IDF console component.\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n"
           "Ctrl+C will terminate the console environment.\n");

    if (linenoiseIsDumbMode()) {
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Windows Terminal or Putty instead.\n");
    }

    /* Main loop */
    while(true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
        if (line == NULL) { /* Ignore empty lines */
            continue;;
        }
#else
        if (line == NULL) { /* Break on EOF or error */
            break;
        }
#endif // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

        /* Add the command to the history if not empty*/
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif // CONFIG_CONSOLE_STORE_HISTORY
        }

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    ESP_LOGE(logTAG, "Error or end-of-input, terminating console");

    vTaskDelete(proc_data_task_hdl);
}

esp_err_t AConsole2::_init_console_library(void)
{

    esp_err_t result;

    /* Initialize the console */
    esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
    console_config.max_cmdline_length = CONSOLE2_MAX_CMDLINE_LENGTH;
    console_config.max_cmdline_args = CONSOLE2_MAX_CMDLINE_ARGS;
#if CONFIG_LOG_COLORS
    console_config.hint_color = atoi(LOG_COLOR_CYAN);
#endif


    result = esp_console_init(&console_config);
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


char *AConsole2::setup_prompt(const char *prompt_str)
{
    /* set command line prompt */
    const char *prompt_temp = CONSOLE2_PROMPT_STR;
    if (prompt_str) {
        prompt_temp = prompt_str;
    }
    snprintf(prompt, CONSOLE2_PROMPT_MAX_LENGTH - 1, LOG_COLOR_I "%s " LOG_RESET_COLOR, prompt_temp);

    if (linenoiseIsDumbMode()) {
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the s_prompt.
         */
        snprintf(prompt, CONSOLE_PROMPT_MAX_LEN - 1, "%s ", prompt_temp);
#endif //CONFIG_LOG_COLORS
    }
    return prompt;
}
