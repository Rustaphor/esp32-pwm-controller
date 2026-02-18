// Copyright (c) 2023 Your Name or Company. All rights reserved.
// SPDX-License-Identifier: MIT
//
// File: pwm_control.h
// Description: Interface for PWM (Pulse Width Modulation) control class.

#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

// Include necessary standard library headers
#include <cstdint>
#include <mutex>
#include <utility>
#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_TIMER_DEFAULT       LEDC_TIMER_0
#define PWM_OUTPUT_IO_DEFAULT   (17) // Define the output GPIO
#define PWM_CHANNEL             LEDC_CHANNEL_0
#define RWM_DUTY_RESOLUTION     LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define PWM_FREQ_DEFAULT        (50000)  // Frequency in Hertz. Set frequency at 50 kHz


using namespace std;

/**
 * @brief Controls PWM signal generation for hardware peripherals.
 *
 * This class provides an interface to configure and manage PWM signals
 * on microcontroller pins. It supports duty cycle adjustment, frequency
 * setting, and enabling/disabling output.
 */
class CPwmControl {
    public:

        // Constructors, destructors
        CPwmControl(int pin, ledc_timer_t timerNum = PWM_TIMER_DEFAULT, ledc_channel_t channel = PWM_CHANNEL);
        ~CPwmControl() { setDutyCycle(0); };

        // Public methods
        esp_err_t initialize(uint32_t freqHz = PWM_FREQ_DEFAULT);

        esp_err_t setDutyCycle(float percent);

        /// Enable PWM output on the configured pin
        esp_err_t enable();

        /// Disable PWM output (turns off signal)
        esp_err_t disable();

        bool isEnabled() { return enabled_; };

        /// Get current duty cycle in percent
        float getDutyCycle();

        pair<uint32_t, uint32_t> getSysDC();

        /// Get current frequency in Hz
        uint32_t getFrequency() { return frequency_; }

        /// Get the GPIO pin assigned for PWM
        int getPin() const { return pin_; }

        // Update to assign changes
        esp_err_t update();
    
    protected:
        esp_err_t setSysDC(uint32_t dc);

    private:
        int pin_;
        bool enabled_;
        ledc_timer_t ldc_timer_;
        ledc_channel_t ldc_channel_;
        uint32_t frequency_;
        uint32_t dc_;
        // float duty_cycle_{0.0f};                // 0-100%

        mutable recursive_mutex rm_ = recursive_mutex();
};

#endif  // PWM_CONTROL_H
