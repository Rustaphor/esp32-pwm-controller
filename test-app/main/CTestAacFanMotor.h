// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "AacFanMotor.h"


#define MOTOR_MCPWM_TIMER_RESOLUTION_HZ 80000000     // 80MHz Частота выходе первого делителя главного тактового генератора
#define MOTOR_MCPWM_PERIOD              942          // 84.925KHz (диапазон значений ШИМ DC 0-100%: 0...MOTOR_MCPWM_PERIOD/2)
#define MOTOR_WAVE_FREQ                 50           // 50Hz Single phase AC

/**
 * @brief Simple implementation of AacFanMotor for educational purposes
 * This class provides a basic implementation of the abstract AacFanMotor class
 * using GPIO and simple PWM control.
 */
class CTestAacFanMotor : public AacFanMotor {

public:
    CTestAacFanMotor() : AacFanMotor{MOTOR_WAVE_FREQ,MOTOR_MCPWM_PERIOD}{};

protected:
    mot_err_t hw_init() override;
    mot_err_t hw_deinit() override;

    uint16_t calc_SineArrayLength(mot_sine_freq_t sine_wave_freq) noexcept override;
};