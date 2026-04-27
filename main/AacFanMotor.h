// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "acmotor_defs.h"
#include <optional>
#include <utility>
#include <semaphore>
#include <cstdlib>

// TODO: Debug only: delete after
#include <iostream>

typedef int acmot_err_t;
typedef enum {
    AC_MOTOR_IS_STOPPED = 1,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
    AC_MOTOR_IS_BUSY,
    AC_MOTOR_INITIALIZED,
    AC_MOTOR_NOT_INITIALIZED
} AcMotorState;


using namespace std;


/**
* @brief Абстрактрый класс управления мотором переменного тока с фазосдвигающим конденсатором для вентиллятора
*/
class AacFanMotor {

public:

    /**
     * @brief Конструктор задания начальных параметров мотора
     * @param sine_wave_freq - частота синусоидальной волны при инициализации мотора в Гц (например, 50)
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    AacFanMotor(acmot_sinefreq_t sine_wave_freq, float powerOut = 0.0f) : _sine_freq{sine_wave_freq}, _relPwrOut{powerOut}{}

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Используется обычно при первичной инициализации и восстановлении энергопотребления. После выделения памяти вызывает метод hw_init()
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    acmot_err_t initialize();

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Используется обычно при переходе в спязий режим. Метод должен быть помещен в десткуртор финального класса.
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    acmot_err_t deinitialize();

  
    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    acmot_err_t run();

    /**
     * @brief Получение текущего состояния мотора. Потокобезопасно.
     * @retval AC_MOTOR_IS_STOPPED - мотор остановлен, AC_MOTOR_IS_RUNNING - мотор запущен, AC_MOTOR_IN_FAILURE - ошибка
     */
    AcMotorState getCurrentState() { return _m_status; };

    /**
     * @brief Set motor speed/power
     * @param powerOut - power output as percentage (0-100)
     * @return AC_MOTOR_OK on success
     * @synchronized
     */
    acmot_err_t setPower(float powerOut);


    acmot_err_t setFrequency(acmot_sinefreq_t sine_wave_freq);


protected:

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Метод вызывается из метода initialize()
    */
    virtual acmot_err_t hw_init() = 0;

    /**
    * @brief Деинициализация оборудования, например, при переходе в спящий режим
    * @details Метод вызывается из метода deinitialize()
    */
    virtual acmot_err_t hw_deinit() = 0;

    /**
     * @brief Функция вычисления длинны массива (буфера) значений синуса
     * @example \code{.cpp} PWM_FREQ / MOTOR_WAVE_FREQ; PWM_FREQ / sine_wave_freq \endcode
     */
    virtual size_t calc_SineBufferLength(acmot_sinefreq_t sine_wave_freq) noexcept = 0;

    /**
     * @brief Запуск мотора с заданной скоростью
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    // virtual acmot_err_t run(float powerOut) = 0;

    // virtual void stop() = 0;
    // acmot_err_t setMotorFreq(acmot_sinefreq_t sine_wave_freq);

    size_t fill_SineWaveBuffer(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff, acmot_sineval_t max_value, float max_angle = ACMOTOR_SINE_MAX_ANGLE) noexcept;

    __always_inline
    const pair<const acmot_sineval_t*, const acmot_sineval_t*>& getCurrentSineBuffer() noexcept { return _hSineWaveBuffer; }


    mutable binary_semaphore sem{1};

private:

    _GLIBCXX_NODISCARD
    inline optional<const acmot_sineval_t*> _allocWaveBuffer(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hArray, size_t buff_length) noexcept {
        hArray.first = (acmot_sineval_t*) malloc(buff_length * sizeof(acmot_sineval_t));
        hArray.second = hArray.first + buff_length;
        return hArray.first;
    };

    acmot_sineval_t _setPowerOutFast(float powerOut) noexcept;

    _GLIBCXX_NODISCARD
    optional<const acmot_sineval_t*> _reAllocSineWaveBuffer(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hArray, size_t buff_length) noexcept;

    acmot_sinefreq_t _sine_freq;
    acmot_sineval_t _amplitude;       // TODO: Убрать и перенести в параметр генерации синусоидального массива
    float _relPwrOut = 0.0f;

    pair<const acmot_sineval_t*, const acmot_sineval_t*> _hSineWaveMinFreqBuff;
    pair<const acmot_sineval_t*, const acmot_sineval_t*> _hSineWaveBuffer;
    AcMotorState _m_status = AC_MOTOR_NOT_INITIALIZED;
};
