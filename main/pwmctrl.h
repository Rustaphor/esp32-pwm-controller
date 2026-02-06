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
        CPwmControl() = default;
        // explicit CPwmControl(/* parameters like pin, frequency, duty_cycle */);
        ~CPwmControl() { setDutyCycle(0); };

        // Public methods
        void initialize();
        void initialize(float dc, uint32_t freqHz = PWM_FREQ_DEFAULT, uint8_t pin = PWM_OUTPUT_IO_DEFAULT, ledc_timer_t timerNum = PWM_TIMER_DEFAULT, ledc_channel_t channel = PWM_CHANNEL);

        esp_err_t setDutyCycle(float percent);

        esp_err_t setFrequency(uint32_t freqHz);

        /// Enable PWM output on the configured pin
        esp_err_t enable() { return setDutyCycle(duty_cycle_); };

        /// Disable PWM output (turns off signal)
        [[nodiscard]]
        void disable() { setDutyCycle(0); }

        /// Get current duty cycle in percent
        [[nodiscard]]
        float getDutyCycle() { return duty_cycle_; }

        /// Get current frequency in Hz
        [[nodiscard]]
        uint32_t getFrequency() { return frequency_; }

        /// Get the GPIO pin assigned for PWM
        [[nodiscard]]
        int getPin() const { return pin_; }

    protected:
            esp_err_t update();

    private:
        int pin_{PWM_OUTPUT_IO_DEFAULT};
        ledc_timer_t ldc_timer_{PWM_TIMER_DEFAULT};
        ledc_channel_t ldc_channel_{PWM_CHANNEL};
        uint32_t frequency_{PWM_FREQ_DEFAULT};  // Default 1kHz
        float duty_cycle_{0.0f};    // 0-100%
        mutable recursive_mutex rm_;
};

#endif  // PWM_CONTROL_H
