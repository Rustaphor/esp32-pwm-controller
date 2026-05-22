#include "AConsole2.h"
#include <stdlib.h>
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

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

esp_err_t AConsole2::_init_console_library(void)
{

    esp_err_t result;

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = CONSOLE2_MAX_CMDLINE_LENGTH,
            .max_cmdline_args = CONSOLE2_MAX_CMDLINE_ARGS,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };

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
