#include "AConsole2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"
#include "cmd_system.h"
#include "linenoise/linenoise.h"
#include "console2_defs.h"


#define CONSOLE2_SIG_TEMRINATE 0xDEAD

// System exit console command
esp_console_cmd_t cmd_exit = {
    .command = "exit",
    .help = "Exit from console",
    .hint = NULL,
    .func = nullptr,
    .argtable = nullptr,
    .func_w_context = [](void *context, int argc, char **argv) {
        return CONSOLE2_SIG_TEMRINATE;
    }
};


esp_err_t AConsole2::initialize(void)
{
    esp_err_t result = ESP_OK;

    if (conStatus != CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        goto end_init;
    }

    result = init_periph();
    if (result) {
        ESP_LOGE(logConsole2Tag, "Failed to initialize console peripherals!");
        goto end_init;
    }

    conStatus = CONSOLE_STATUS_INITIALIZED;
    ESP_LOGD(logConsole2Tag, "Console2 component initialized");

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    setup_prompt(CONSOLE2_PROMPT_PREFIX ">");

    cmd_exit.context = this;

    // Строка приветствия в консоли
#ifdef CONFIG_CONSOLE2_PRESSENTER_MSG
    printf(getHelp2EnterConsoleMsg());
#endif

end_init:
    return result;
}

esp_err_t AConsole2::registerCommand(AConsole2Cmd &command)
{
    // _commands.push_back(command);
    ESP_LOGI(logConsole2Tag, "Registering console command: %s", command._console_cmd.command);
    return esp_console_cmd_register(&command._console_cmd);
}

esp_err_t AConsole2::run(void)
{
    esp_err_t result = ESP_OK;

    if (conStatus == CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        goto _err_exit;
    } else if (conStatus == CONSOLE_STATUS_RUNNED) {
        goto _err_exit;
    }

    result = _init_console_library();
    if (result) {
        // TODO: add deinit-periph here
        ESP_LOGE(logConsole2Tag, "Failed to initialize console library!");
        goto _err_exit;
    }

    /* Register system commands */
    esp_console_register_help_command();
    register_system_common();
    esp_console_cmd_register(&cmd_exit);

    result = start();
    if (result) {
        goto _deinit;
    }

    if (xTaskCreate(_vConsole2Task, "console", CONFIG_CONSOLE2_TASK_HEAP_STACK_SIZE, this, tskIDLE_PRIORITY, &_xTaskHandle) != pdTRUE) {
        result = ESP_FAIL;
        goto _deinit;
    }

    conStatus = CONSOLE_STATUS_RUNNED;
    return ESP_OK;   
    
_deinit:
    esp_console_deinit();
_err_exit:
    ESP_LOGE(logConsole2Tag, "Error: %d. Failed to start console!", result);
    return result;
}

esp_err_t AConsole2::stop(void) noexcept
{
    esp_err_t result;
    result = esp_console_deinit();

    if (result != ESP_OK) {
        ESP_LOGE(logConsole2Tag, "Error: %d. Failed to deinitialize console!", result);
    }

    conStatus = CONSOLE_STATUS_INITIALIZED;
    
    return result;
}

void AConsole2::_vConsole2Task(void *pvParameters)
{
    AConsole2 *pConsole = (AConsole2*) pvParameters;

    printf("\n"
            CONFIG_CONSOLE2_GREETINGS_MESSAGE "\n \
Type 'help' to get the list of commands.\n \
Use <UP>/<DOWN> arrows to navigate through command history.\n \
Press <TAB> when typing command name to auto-complete. \n \
Type 'exit' to terminate the console environment.\n");

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
        char* line = linenoise(pConsole->_prompt);

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
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (ret == CONSOLE2_SIG_TEMRINATE) {
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

    printf("Console terminated.\n");
    pConsole->stop();

#ifdef CONFIG_CONSOLE2_PRESSENTER_MSG
   printf(getHelp2EnterConsoleMsg());
#endif

    vTaskDelete(NULL);
}

esp_err_t AConsole2::_init_console_library(void)
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


const char *AConsole2::setup_prompt(const char *prompt_str)
{
    /* set command line prompt */
    memset(_prompt, 0, CONSOLE2_PROMPT_MAX_LENGTH);
    const char *prompt_temp = CONSOLE2_PROMPT_PREFIX;
    if (prompt_str) prompt_temp = prompt_str;
    snprintf(_prompt, CONSOLE2_PROMPT_MAX_LENGTH - 1, LOG_COLOR_I "%s " LOG_RESET_COLOR, prompt_temp);

    if (linenoiseIsDumbMode()) {
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the s_prompt.
         */
        snprintf(_prompt, CONSOLE_PROMPT_MAX_LEN - 1, "%s ", prompt_temp);
#endif //CONFIG_LOG_COLORS
    }
    return _prompt;
}
