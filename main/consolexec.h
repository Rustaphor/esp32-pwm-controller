#ifndef CONSOLEEXEC_H
#define CONSOLEEXEC_H

#include "pwmctrl.h"
#include "esp_console.h"
#include "cmd_system.h"
#include "esp-wifi-ctrl.h"
#include <string>
#include <map>


// Команда в консоли
#define CONSOLE_PWMCTRL_CMD_ON "on"
#define CONSOLE_PWMCTRL_CMD_OFF "off"
#define CONSOLE_PWMCTRL_COMMAND "pwm"
#define CONSOLE_PWMCTRL_HINT " <none> ["CONSOLE_PWMCTRL_CMD_ON"|"CONSOLE_PWMCTRL_CMD_OFF"] [-f <XX>] [-d <1-90%>] [-i]\n \
    <none> - display current PWM status. \n \
    TODO: needs to finish desc..."
#define CONSOLE_PWMCTRL_HELP "\
    Pulse Width Modulation control command. Controll special output pin to generate PWM.\n \
    "CONSOLE_PWMCTRL_CMD_ON" command - changes DC level from 0% to last state\n \
    "CONSOLE_PWMCTRL_CMD_OFF" command - changes DC level from current to 0%\n \
    -f - frequency (30-100) (kHz)\n \
    -d - duty cycle \n \
    -i - inverted output signal \n \
    Example: \n \
    \t pwmcontrol on -if 50 -d 50 \n \
    Created by Vladimir Inshakov, 2026, markoni48@yandex.ru \
    TODO: fill detailed description!"
#define PROMPT_STR CONFIG_IDF_TARGET


/*
 * Предупреждение бессмысленности включения консоли UART, при задействованном компоненте 'GDB Stub'
 * для программной отладки.
 */
#if CONFIG_ESP_SYSTEM_GDBSTUB_RUNTIME && CONFIG_CONSOLE_UART_ENABLED
#warning "A serial console is not useful when using 'GDB Stub' component. Please disable it in menuconfig."
#define CONFIG_ESP_CONSOLE_UART_DEFAULT false
#endif

/*
 * We warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif

using namespace std;

class CConsoleExecutor {

    public:
        CConsoleExecutor(int argc, char **argv) { this->getOpts(argc, argv); }

        inline map<char,string> &getOpts() { return this->mArgs; };

        // Парсер параметров и ключей, начинающихся символом '-'
        map<char,string> &getOpts(int argc, char **argv);

        // Первичная инициализация консольной системы
        static inline void init() {
            // Initialize Console system
            esp_console_repl_t *repl = NULL;
            esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

            /* Prompt to be printed before each line.
            * This can be customized, made dynamic, etc. */
            repl_config.prompt = PROMPT_STR ">";
            repl_config.max_cmdline_length = CONFIG_CONSOLE_MAX_COMMAND_LINE_LENGTH;

            /* Register commands */
            esp_console_register_help_command();
            register_system_common();
            esp_console_cmd_register(&CConsoleExecutor::_pwmcmd);

        #if CONFIG_CONSOLE_UART_ENABLED
        #if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
            esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
        #endif
        #endif

            /* Run Console Thread */
            ESP_ERROR_CHECK(esp_console_start_repl(repl));
        }

    private:
        static int exec_pwmctrl_callback(int argc, char **argv);

        constexpr static const esp_console_cmd_t _pwmcmd = {
            .command = CONSOLE_PWMCTRL_COMMAND,
            .help = CONSOLE_PWMCTRL_HELP,           // Развернутое описание по команде help <cmd>
            .hint = CONSOLE_PWMCTRL_HINT,           // Краткая подсказка по командам
            .func = exec_pwmctrl_callback
        };

    protected:
        map<char,string> mArgs;
};

#endif // CONSOLEEXEC_H
