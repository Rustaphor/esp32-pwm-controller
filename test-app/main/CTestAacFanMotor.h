// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "AacFanMotor.h"


#define MOTOR_MCPWM_TIMER_RESOLUTION_HZ 80000000     // 80MHz Частота выходе первого делителя главного тактового генератора
#define MOTOR_MCPWM_AMPLITUDE           942          // 84.925KHz (фактически диапазон значений установки ШИМ DC 0-100%: 0...MOTOR_MCPWM_PERIOD/2)
#define MOTOR_WAVE_FREQ                 50           // 50Hz Single phase AC

/**
 * @brief Simple implementation of AacFanMotor for educational purposes
 * This class provides a basic implementation of the abstract AacFanMotor class
 * using GPIO and simple PWM control.
 */
class CTestAacFanMotor : public AacFanMotor {

public:
    CTestAacFanMotor(mot_sine_freq_t sine_wave_freq, mot_pwm_val_t amplitude) : AacFanMotor{sine_wave_freq,amplitude,0.0f}{};

    size_t calc_SineBufferLength(mot_sine_freq_t sine_wave_freq) noexcept override;

    pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& get_SineBuffer() noexcept {
        return const_cast<pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&>(getCurrentSineBuff());
    };

    void test_fillSineBuffer(mot_pwm_val_t max_value, float max_angle) noexcept {
        fill_SineWaveBuffer(const_cast<pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&>(getCurrentSineBuff()),max_value,max_angle);
    };

protected:
    mot_err_t hw_init() override;
    mot_err_t hw_deinit() override;
};