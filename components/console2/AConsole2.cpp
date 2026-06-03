#include "AConsole2.h"
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "console2_defs.h"

static const char* logCons2TAG = "Console2";

esp_err_t AConsole2::initialize(void)
{
    esp_err_t result = ESP_OK;

    if (conStatus != CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        goto end_init;
    }

    result = init_periph();
    if (result) {
        ESP_LOGE(logCons2TAG, "Failed to initialize console peripherals!");
        goto end_init;
    }

    result = _init_console_library();
    if (result) {
        ESP_LOGE(logCons2TAG, "Failed to initialize console library!");
        goto end_init;
    }

    conStatus = CONSOLE_STATUS_INITIALIZED;
    ESP_LOGI(logCons2TAG, "Console2 component initialized");

end_init:
    return result;
}

esp_err_t AConsole2::registerCommand(AConsole2Cmd &command)
{
    // _commands.push_back(command);
    ESP_LOGI(logCons2TAG, "Registering console command: %s", command._console_cmd.command);
    return esp_console_cmd_register(&command._console_cmd);
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
