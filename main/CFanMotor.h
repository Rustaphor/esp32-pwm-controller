// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef C_MOTOR_DRIVE_H
#define C_MOTOR_DRIVE_H

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"

/*
 * Настройка регистров MCPWM для управления мотором подробно описана в Chapter 29 в ESP32 Technical Reference Manual
*/
#define MOTOR_MCPWM_TIMER_RESOLUTION_HZ 80000000     // 80MHz Частота выходе первого делителя главного тактового генератора
#define MOTOR_MCPWM_PERIOD              942          // 84.925KHz (диапазон значений ШИМ DC 0-100%: 0...MOTOR_MCPWM_PERIOD/2)
#define ACMOT_SINE_MAX_VALUE            (MOTOR_MCPWM_PERIOD/2)
#define MOTOR_WAVE_FREQ                 50           // 50Hz Single phase AC
#define ACMOT_ERR_NO_MEMORY             ESP_ERR_NO_MEM
#define ACMOT_PWM_MIN_VALUE             2
#include "AacFanMotor.h"

#define MOTOR_DRV_EN_PIN            GPIO_NUM_16
// #define MOTOR_DRV_FAULT_PIN     GPIO_NUM_17
#define MOTOR_PWM_HS_PIN            GPIO_NUM_21
#define MOTOR_PWM_HS_PIN_ACTLVL     1
#define MOTOR_PWM_LS_PIN            GPIO_NUM_22
#define MOTOR_PWM_LS_PIN_ACTLVL     1

// Системный таймер (0 или 1)
#define MOTOR_DRV_GROUP_ID          0


using namespace std;

/**
 * @brief A typical C++ class declaration
 */
class CFanMotor : public AacFanMotor {

    const char* tag = "FanMotor";

    // Прототип обработчика прерывания таймера ШИМ-контроллера
    friend bool pwmtimer_onupdate_isr_cb(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx);

public:

    const char * getName() { return this->tag; };

    // Constructors
    CFanMotor() : AacFanMotor{MOTOR_WAVE_FREQ, 0.0f}, _direction{1}, hTimer_{NULL} {};

    // Destructor
    ~CFanMotor() {
        hw_deinit();
    }

    // Убираем копируемые операции, если менеджеры не должны дублироваться
    CFanMotor(const CFanMotor&) = delete;
    CFanMotor& operator=(const CFanMotor&) = delete;
    
protected:
    acmot_err_t hw_init() override;
    acmot_err_t hw_deinit() override;
    size_t calcSineBufferLength(acmot_sinefreq_t sine_wave_freq) noexcept override;
    acmot_err_t hw_run(const acmot_sineval_t powerOut) override;
    acmot_err_t hw_stop() noexcept override;
    acmot_err_t hw_set_enabled(bool en);

private:

    volatile acmot_sineval_t* _pCurVal = nullptr;
    int _direction;
    mcpwm_timer_handle_t hTimer_;
    mcpwm_oper_handle_t hOperator_;
    mcpwm_cmpr_handle_t hComparator_;
    mcpwm_gen_handle_t hGenerator_[2];
#ifdef MOTOR_DRV_FAULT_PIN
    mcpwm_fault_handle_t hFaultPin_ = NULL;
#endif


};

#endif // MYCLASS_H