#pragma once

#include "AConsole2Cmd.h"


class CMotorCtrl : public AConsole2Cmd {

    constexpr static const char* _command = "motor";
    constexpr static const char* _help = "\
        AC Motor control command. Controll special pins to generate PWM sine wave.\n \
        --help to usage information\n \
        Created by Vladimir Inshakov <markoni48@yandex.ru>, 2026\n";

public:

    CMotorCtrl() : AConsole2Cmd(_command, _help) {};
    int exec_func_cb(int argc, char* argv[]) override;

};
