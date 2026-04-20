// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef ACMOTOR_DEFS_H
#define ACMOTOR_DEFS_H

// Include necessary headers
#include <inttypes.h>

// Define error codes
#define AC_MOTOR_OK                     0
#define AC_ERR_MOTOR_FAIL               0x200
#define AC_ERR_MOTOR_INIT_FALURE        0x202
#define AC_ERR_MOTOR_NO_MEMORY          0x203
#define AC_ERR_MOTOR_NOT_INITIALIZED    0x204

// Type definitions
typedef uint16_t mot_pwm_val_t;
typedef uint8_t mot_sine_freq_t;

// Задание константы максимального угла расчета синуса. Обычно 1/4 (90 град) или 1/2 (180 град) волны.
#ifndef ACMOTOR_SINE_MAX_ANGLE
#define ACMOTOR_SINE_MAX_ANGLE          90.0f
#endif

/**
 * @brief Минимальная частота синусоидальной генерируемой волны мотора в Гц (например, 25)
 * @param ACMOTOR_SINE_MIN_FREQ
 * @details Глобальный параметр требуется для расчета максимального размера буфера значений синуса в сэмпле. Максимальное значение достигается при минимальной частоте.
 */
#ifndef ACMOTOR_SINE_MIN_FREQ
#define ACMOTOR_SINE_MIN_FREQ          25
#endif


#endif // ACMOTOR_DEFS_H
