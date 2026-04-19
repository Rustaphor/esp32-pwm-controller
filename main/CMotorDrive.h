// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef C_MOTOR_DRIVE_H
#define C_MOTOR_DRIVE_H

#include "AacFanMotor.h"
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"

#define MOTOR_MCPWM_TIMER_RESOLUTION_HZ 80000000     // 80MHz Частота выходе первого делителя главного тактового генератора
#define MOTOR_MCPWM_PERIOD              942          // 84.925KHz (диапазон значений ШИМ DC 0-100%: 0...MOTOR_MCPWM_PERIOD/2)
#define MOTOR_WAVE_FREQ                 50           // 50Hz Single phase AC
// #define MOTOR_SPEED_UPDATE_PERIOD_US    200000      // 200ms

#define MOTOR_DRV_EN_PIN        GPIO_NUM_16
#define MOTOR_DRV_FAULT_PIN     10
#define MOTOR_PWM_HS_PIN        GPIO_NUM_21
#define MOTOR_PWM_LS_PIN        GPIO_NUM_22

// Системный таймер (0 или 1)
#define MOTOR_DRV_GROUP_ID        0

// #define MOTOR_MCPWM_OP_INDEX_U     0
// #define MOTOR_MCPWM_OP_INDEX_V     1
// #define MOTOR_MCPWM_OP_INDEX_W     2
// #define MOTOR_MCPWM_GEN_INDEX_HIGH 0
// #define MOTOR_MCPWM_GEN_INDEX_LOW  1

/**
 * @brief A typical C++ class declaration
 */
class CMotorDrive : public AacFanMotor {

    // Прототип обработчика прерывания таймера ШИМ-контроллера
    friend bool pwmtimer_onupdate_isr_cb(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx);

public:

    uint32_t count = 0;
    SemaphoreHandle_t hxSem = xSemaphoreCreateCounting(1, 0);

    // Constructors
    CMotorDrive();

    mot_err_t initialize();

    
    // Destructor
    ~CMotorDrive();
    
    // // Assignment operator
    // CMotorDrive& operator=(const CMotorDrive& other);
    mot_err_t run();

    mot_err_t stop();

    mot_err_t setDC(uint16_t pwm_dc);


protected:
    mot_err_t hw_enable(bool en);
    mot_err_t hw_deinit() override;
    
private:
    mcpwm_timer_handle_t hTimer_;
    mcpwm_oper_handle_t hOperator_;
    mcpwm_cmpr_handle_t hComparator_;
    mcpwm_gen_handle_t hGenerator_[2];
#ifdef MOTOR_DRV_FAULT_PIN
    mcpwm_fault_handle_t hFaultPin_ = NULL;
#endif


    // Private methods
    void helperFunction();

};

#endif // MYCLASS_H