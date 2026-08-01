// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef ACMOTOR_DEFS_H
#define ACMOTOR_DEFS_H

// Include necessary headers
#include <inttypes.h>

// Define error codes
#define AC_MOTOR_OK                     0
#define AC_ERR_MOTOR_FAIL               0x200
#define AC_ERR_MOTOR_IS_BUSY_NOW        0x201
#define AC_ERR_MOTOR_INIT_FALURE        0x202
#define AC_ERR_MOTOR_NO_MEMORY          0x203
#define AC_ERR_MOTOR_NOT_INITIALIZED    0x204
#define AC_ERR_MOTOR_INVALID_POWER      0x205
#define AC_ERR_MOTOR_INVALID_FREQ       0x206


// Type definitions
typedef uint16_t acmot_sineval_t;
typedef uint8_t acmot_sinefreq_t;


/**
 * Угол расчета значений синусоидной волны. Обычно 1/4 (90 град) или 1/2 (180 град) волны.
 * @details Глобальный параметр, используемый для расчета таблицы значений (буфера) синусоидной волны.
 * @showinitializer
 */
#ifndef ACMOTOR_SINE_MAX_ANGLE
#define ACMOTOR_SINE_MAX_ANGLE    90.0f
#endif

/**
 * Минимальная частота синусоидальной генерируемой волны мотора в Гц (например, 25)
 * 
 * @details Глобальный параметр требуется для расчета максимального размера буфера значений синуса в сэмпле. Максимальное значение достигается при минимальной частоте.
 * @showinitializer
 */
#ifndef ACMOTOR_SINE_MIN_FREQ
#define ACMOTOR_SINE_MIN_FREQ     25
#endif

/**
 * Максимальное (амплитудное) значение размаха функции синуса
 * 
 * @details Глобальный конфигурационный параметр, определяющий максимально возможное значение расчетное значение синусоиды. Задается в конкретной реализации "железа"
 * @showinitializer
 */
#define ACMOTOR_SINE_MAX_VALUE    (942 / 2)


#endif // ACMOTOR_DEFS_H
