// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#ifndef C_MOTOR_DRIVE_H
#define C_MOTOR_DRIVE_H

#include "AacFanMotor.h"

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

    mot_err_t hw_deinit();

    // Private methods
    void helperFunction();
};

#endif // MYCLASS_H