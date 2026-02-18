#ifndef CONSOLEEXEC_H
#define CONSOLEEXEC_H

#include "pwmctrl.h"
#include "esp_console.h"
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


using namespace std;

class CConsoleExecutor {

    friend esp_err_t esp_console_register_pwmcontrol_command();

    public:
        CConsoleExecutor(int argc, char **argv) { this->getOpts(argc, argv); }

        inline map<char,string> &getOpts() { return this->mArgs; };

        // Парсер параметров и ключей, начинающихся символом '-'
        map<char,string> &getOpts(int argc, char **argv);

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

inline esp_err_t esp_console_register_pwmcontrol_command() {
    return esp_console_cmd_register(&CConsoleExecutor::_pwmcmd);
}

#endif // CONSOLEEXEC_H
