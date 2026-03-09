// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once

#include <inttypes.h>

typedef enum {
    AC_MOTOR_OK = 0,
    AC_MOTOR_FAIL = 0x20,
    AC_MOTOR_INIT_FALURE = 0x22
} mot_err_t;

typedef enum {
    AC_MOTOR_IS_STOPPED = 1,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
    AC_MOTOR_IS_BUSY,
    AC_MOTOR_NOT_INITIALIZED
} mot_status_t;


/**
* @brief A typical C++ class declaration
*/
class AacFanMotor {

protected:

    // /**
    // * @brief Первичная инициалиация оборудования для упраления мотором
    // * @details Данный метод вызывается из метода initialize()
    // */
    // virtual mot_err_t hw_init() = 0;

    /**
    * @brief Деинициализация оборудования, например, при переходе в спящий режим
    * @details Данный метод вызывается из метода deinitialize()
    */
    virtual mot_err_t hw_deinit() = 0;

    /**
     * @brief Запуск мотора с заданной скоростью
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    // virtual mot_err_t run(float powerOut) = 0;

    // virtual void stop() = 0;


public:


    AacFanMotor(); // Default initialize constructor

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Потоконебезопасно. Используется обычно при первичной инициализации и восстановлении энергопотребления
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    virtual mot_err_t initialize() = 0;

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Потоконебезопасно. Используется обычно при переходе в спязий режим
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    mot_err_t deinitialize();

    ~AacFanMotor();

    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    mot_err_t run();

    /**
     * @brief Получение текущего статуса мотора. Потокобезопасно.
     * @retval AC_MOTOR_IS_STOPPED - мотор остановлен, AC_MOTOR_IS_RUNNING - мотор запущен, AC_MOTOR_IN_FAILURE - ошибка
     */
    mot_status_t get_status();

private:

    mot_status_t m_status = AC_MOTOR_NOT_INITIALIZED;
    float m_pwrOut = 0.0f;
    
};

