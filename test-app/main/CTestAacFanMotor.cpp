#include "CTestAacFanMotor.h"

mot_err_t CTestAacFanMotor::hw_init()
{
    return AC_MOTOR_OK;
}

mot_err_t CTestAacFanMotor::hw_deinit()
{
    return AC_MOTOR_OK;
}

size_t CTestAacFanMotor::calc_SineBufferLength(mot_sine_freq_t sine_wave_freq) noexcept
{
    return (MOTOR_MCPWM_TIMER_RESOLUTION_HZ / MOTOR_MCPWM_AMPLITUDE) / (sine_wave_freq * 4);
}
