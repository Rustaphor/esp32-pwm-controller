// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef ACMOTOR_DEFS_H
#define ACMOTOR_DEFS_H

// Include necessary headers
#include <inttypes.h>

// Define error codes
#define AC_ERR_MOTOR_NO_BUFF_MEMORY     0x203
#define AC_ERR_MOTOR_INVALID_POWER      0x205
#define AC_ERR_MOTOR_INVALID_FREQ       0x206


// Type definitions
typedef uint16_t acmot_sineval_t;
typedef uint8_t acmot_sinefreq_t;



#endif // ACMOTOR_DEFS_H
