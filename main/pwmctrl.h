// Copyright (c) 2023 Your Name or Company. All rights reserved.
// SPDX-License-Identifier: MIT
//
// File: pwm_control.h
// Description: Interface for PWM (Pulse Width Modulation) control class.

#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

// Include necessary standard library headers
#include <cstdint>
// #include <functional>


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
        constexpr CPwmControl() = default;
        // explicit CPwmControl(/* parameters like pin, frequency, duty_cycle */);
        ~CPwmControl() = default;

        // Copy and move operations
        CPwmControl(const CPwmControl&) = default;
        CPwmControl& operator=(const CPwmControl&) = default;
        CPwmControl(CPwmControl&&) = default;
        CPwmControl& operator=(CPwmControl&&) = default;

        // Public methods
        bool initialize();
        void setDutyCycle(float percent);

        void setFrequency(uint32_t freqHz);

        /// Enable PWM output on the configured pin
        void enable();

        /// Disable PWM output (turns off signal)
        void disable();

        /// Get current duty cycle in percent
        [[nodiscard]] float getDutyCycle() const { return duty_cycle_; }

        /// Get current frequency in Hz
        [[nodiscard]] uint32_t getFrequency() const { return frequency_; }

        /// Check if PWM output is currently enabled
        [[nodiscard]] bool isEnabled() const { return is_enabled_; }

        /// Get the GPIO pin assigned for PWM
        [[nodiscard]] uint8_t getPin() const { return pin_; }

    private:
        uint8_t pin_{0};
        uint32_t frequency_{1000};  // Default 1kHz
        float duty_cycle_{0.0f};    // 0-100%
        bool is_enabled_{false};
};

#endif  // PWM_CONTROL_H
