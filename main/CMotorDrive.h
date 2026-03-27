// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef C_MOTOR_DRIVE_H
#define C_MOTOR_DRIVE_H

#include "AacFanMotor.h"
#include "driver/gpio.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"

#define MOTOR_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define MOTOR_MCPWM_PERIOD              500      // 50us, 20KHz
#define MOTOR_SPEED_UPDATE_PERIOD_US    200000   // 200ms

#define MOTOR_DRV_EN_PIN    GPIO_NUM_16
#define MOTOR_DRV_FAULT_PIN     10
#define MOTOR_PWM_PIN           21
#define MOTOR_PWM_UL_PIN        21

// #define MOTOR_MCPWM_OP_INDEX_U     0
// #define MOTOR_MCPWM_OP_INDEX_V     1
// #define MOTOR_MCPWM_OP_INDEX_W     2
// #define MOTOR_MCPWM_GEN_INDEX_HIGH 0
// #define MOTOR_MCPWM_GEN_INDEX_LOW  1

/**
 * @brief A typical C++ class declaration
 */
class CMotorDrive : public AacFanMotor {
public:
    // Constructors
    CMotorDrive();

    mot_err_t initialize();

    
    // Destructor
    ~CMotorDrive();
    
    // // Assignment operator
    // CMotorDrive& operator=(const CMotorDrive& other);
    
    // Public methods
    void doSomething();
    
private:
    mcpwm_timer_handle_t hTimer_;
    mcpwm_oper_handle_t hOperator_;
    mcpwm_cmpr_handle_t hComparator_;
    mcpwm_gen_handle_t hGenerator_;
#ifdef MOTOR_DRV_FAULT_PIN
    mcpwm_fault_handle_t hFaultPin_ = NULL;
#endif

    mot_err_t hw_deinit();

    // Private methods

    void helperFunction();
};

#endif // MYCLASS_H