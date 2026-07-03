#pragma once

#include "AConsole2Cmd.h"

// Команда в консоли
#define CONSOLE_PWMCTRL_CMD_ON "on"
#define CONSOLE_PWMCTRL_CMD_OFF "off"
#define CONSOLE_PWMCTRL_COMMAND "pwm"
#define CONSOLE_PWMCTRL_HINT " <none> [CONSOLE_PWMCTRL_CMD_ON|CONSOLE_PWMCTRL_CMD_OFF] [-f <XX>] [-d <1-90%>] [-i]\n \
    <none> - display current PWM status. \n \
    TODO: needs to finish desc..."
#define CONSOLE_PWMCTRL_HELP "\
    Pulse Width Modulation control command. Controll special output pin to generate PWM.\n \
     command - changes DC level from 0% to last state\n \
     command - changes DC level from current to 0%\n \
    -f - frequency (30-100) (kHz)\n \
    -d - duty cycle \n \
    -i - inverted output signal \n \
    Example: \n \
    \t pwmcontrol on -if 50 -d 50 \n \
    Created by Vladimir Inshakov, 2026, markoni48@yandex.ru \
    TODO: fill detailed description!"


class CPwmCtrl : public AConsole2Cmd {

    constexpr static const char* _command = CONSOLE_PWMCTRL_COMMAND;
    constexpr static const char* _help = CONSOLE_PWMCTRL_HELP;
    constexpr static const char* _hint = CONSOLE_PWMCTRL_HINT;

public:

    CPwmCtrl() : AConsole2Cmd(_command, _help, _hint) {};
    int exec_func_cb(int argc, char* argv[]) override;

};
