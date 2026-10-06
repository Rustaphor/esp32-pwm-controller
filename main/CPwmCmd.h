// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once

#include "CFanMotor.h"
#include "AConsole2Cmd.h"

/**
 * @brief A class that combines CFanMotor functionality with console command interface
 */
class CPwmCmd : public AConsole2Cmd {

    constexpr static const char* _command = "pwm";
    constexpr static const char* _help = "\
        PWM test control command. Help function to test hardware MotorControl.\n \
        --help to usage information\n \
        Created by Vladimir Inshakov <markoni48@yandex.ru>, 2026\n";

    CFanMotor& _motor;  // Reference to the fan motor instance
    acmot_sineval_t _value;  // Current value of the sine wave for PWM

public:
    /**
     * @brief Constructor initializes the motor and console command
     */
    CPwmCmd(CFanMotor &motor) : AConsole2Cmd(_command, _help), _motor{motor} {}

    /**
     * @brief Console command callback function
     * @param argc Number of arguments
     * @param argv Array of argument strings
     * @return Return code (0 for success)
     */
    int exec_func_cb(int argc, char* argv[]) override;

protected:

    esp_err_t pwm_start() noexcept;
    esp_err_t pwm_stop() noexcept;
    esp_err_t set_counter_value(acmot_sineval_t val) noexcept;

private:

    void _disp_pwm_status() const;

    _GLIBCXX_NODISCARD
    bool _isMotorAvalableOrPrintError() const noexcept;

    _GLIBCXX_NODISCARD
    const char* _getMotorOnlyStateString(AcMotorState_t state) const noexcept;

    _GLIBCXX_NODISCARD
    const char* _getMotorSysStateString(devState_t state) const noexcept;
};

