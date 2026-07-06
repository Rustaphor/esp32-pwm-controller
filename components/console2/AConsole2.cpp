#include "AConsole2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "console2_defs.h"


#define CONSOLE2_SIG_TEMRINATE 0xDEAD

static vector<const AConsole2Cmd*> cmdList;


esp_err_t AConsole2::initialize(void)
{
    esp_err_t result = ESP_OK;
    if (hSem == NULL) {
        result = ESP_ERR_INVALID_STATE;
        return result;
    }

    xSemaphoreTake(hSem, portMAX_DELAY);

    if (conStatus != CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        goto end_init;
    }

    result = init_periph();
    if (result) {
        ESP_LOGE(logConsole2Tag, "Failed to initialize console peripherals!");
        goto end_init;
    }

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    setup_prompt(CONSOLE2_PROMPT_PREFIX ">");

    conStatus = CONSOLE_STATUS_INITIALIZED;
    ESP_LOGD(logConsole2Tag, "Console2 component initialized");

    // Строка приветствия в консоли
#ifdef CONFIG_CONSOLE2_PRESSENTER_MSG
    printf(getMsgHelp2EnterConsole());
#endif

end_init:
    xSemaphoreGive(hSem);
    return result;
}

void AConsole2::registerCommand(AConsole2Cmd &command)
{
    // Проверка на поворное добавление одной и той же команды
    auto it { cmdList.begin() };
    while (it != cmdList.end()) {
        if (*it == &command) return;
        ++it;
    }

    cmdList.push_back(&command);
    ESP_LOGI(logConsole2Tag, "Registering console command: %s", command._console_cmd.command);
}

const vector<const AConsole2Cmd *> &AConsole2::getCommandList(void) noexcept
{
    return cmdList;
}

esp_err_t AConsole2::run(void)
{
    esp_err_t result = ESP_OK;
    if (hSem == NULL) {
        result = ESP_ERR_INVALID_STATE;
        return result;
    }
    xSemaphoreTake(hSem, portMAX_DELAY);

    if (conStatus == CONSOLE_STATUS_NOT_INITIALIZED) {
        result = ESP_ERR_NOT_ALLOWED;
        ESP_LOGW(logConsole2Tag, "Run failed. Console2 must be initilized before!");
        goto err_run_exit;
    } else if (conStatus == CONSOLE_STATUS_RUNNED) {
        goto good_run_exit;
    }

    result = start();
    if (result != ESP_OK) {
        goto err_run_exit;
    }
    conStatus = CONSOLE_STATUS_RUNNED;
    goto good_run_exit; 
    
err_run_exit:
    ESP_LOGE(logConsole2Tag, "Error: %d. Failed to run console!", result);

good_run_exit:
    xSemaphoreGive(hSem);
    return result;
}

esp_err_t AConsole2::stop(void) noexcept
{
    esp_err_t result = ESP_OK;
    if (hSem == NULL) {
        result = ESP_ERR_INVALID_STATE;
        return result;
    }
    xSemaphoreTake(hSem, portMAX_DELAY);

    if (conStatus != CONSOLE_STATUS_RUNNED) {
        goto err_stop_exit;
    }

    result = terminate();
    if (result != ESP_OK) {
        goto err_stop_exit;
    }

    conStatus = CONSOLE_STATUS_INITIALIZED;
    goto good_stop_exit; 

err_stop_exit:
    ESP_LOGE(logConsole2Tag, "Error: %d. Failed to stop console!", result);

good_stop_exit:
    xSemaphoreGive(hSem);
    return result;
}

const char *AConsole2::getMsgHelp2EnterConsole(void)
{
    static const char* msg = "Press <ENTER> to enter command line interface.\n\r";
    return msg;
}

const char *AConsole2::getMsgStandartGreeting(void)
{
    static const char* msg = "\n"
            CONFIG_CONSOLE2_GREETINGS_MESSAGE "\n \
Type 'help' to get the list of commands.\n \
Use <UP>/<DOWN> arrows to navigate through command history.\n \
Press <TAB> when typing command name to auto-complete. \n \
Type 'exit' to terminate the console environment.\n";

    return msg;
}

const char *AConsole2::setup_prompt(const char *prompt_str)
{
    /* set command line prompt */
    memset(prompt, 0, CONSOLE2_PROMPT_MAX_LENGTH);
    const char *prompt_temp = CONSOLE2_PROMPT_PREFIX;
    if (prompt_str) prompt_temp = prompt_str;
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
