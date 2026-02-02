#ifndef HELPERPWMCONSOLE_H
#define HELPERPWMCONSOLE_H

#include "pwmctrl.h"
#include "esp_console.h"

// Команда в консоли
#define CONSOLE_PWMCTRL_COMMAND "pwmcontrol"
#define CONSOLE_PWMCTRL_HINT "\
    <none> [start|stop] [-freq|freq xx] [-d|dc <1-90%>]\n \
    <none> - display current PWM status.\n \
    TODO: needs to finish desc..."
#define CONSOLE_PWMCTRL_HELP "\
    Pulse Width Modulation control command. Controll special output pin to generate PWM. \
    start command - changes DC level from 0% to last state \
    stop command - changes DC level from current to 0% \
    Created by Vladimir Inshakov, 2026, markoni48@yandex.ru \
    TODO: fill detailed description!"


class CPwmCtrlConsoleHelper {

friend esp_err_t esp_console_register_pwmcontrol_command();

public:
    CPwmCtrlConsoleHelper() = default;

private:
    static int command_callback(int argc, char **argv);

    constexpr static const esp_console_cmd_t _pwmcmd = {
        .command = CONSOLE_PWMCTRL_COMMAND,
        .help = CONSOLE_PWMCTRL_HELP,      // Развернутое описание по команде help <cmd>
        .hint = CONSOLE_PWMCTRL_HINT,          // Краткая подсказка по командам
        .func = command_callback};

};

inline esp_err_t esp_console_register_pwmcontrol_command() {
    return esp_console_cmd_register(&CPwmCtrlConsoleHelper::_pwmcmd);
};

#endif // HELPERPWMCONSOLE_H
