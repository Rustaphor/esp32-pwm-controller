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

typedef int mot_err_t;
typedef enum {
    AC_MOTOR_IS_STOPPED = 1,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
    AC_MOTOR_IS_BUSY,
    AC_MOTOR_INITIALIZED,
    AC_MOTOR_NOT_INITIALIZED
} mot_state_t;


using namespace std;


/**
* @brief Абстрактрый класс управления мотором переменного тока с фазосдвигающим конденсатором для вентиллятора
*/
class AacFanMotor {

public:

    /**
     * @brief Конструктор задания начальных параметров мотора
     * @param sine_wave_freq - требуемая частота синусоидальной волны мотора в Гц (например, 50)
     * @param amplitude - максимальное значение (амплитуда) ШИМ-счетчика, при коэффициенте заполнения при DC=100%
     */
    AacFanMotor(mot_sine_freq_t sine_wave_freq, mot_pwm_val_t amplitude, float powerOut = 0.0f) : _sine_freq{sine_wave_freq}, _amplitude{amplitude}, m_pwrOut{powerOut}{}

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Используется обычно при первичной инициализации и восстановлении энергопотребления. После выделения памяти вызывает метод hw_init()
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    mot_err_t initialize();

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Используется обычно при переходе в спязий режим. Метод должен быть помещен в десткуртор финального класса.
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    mot_err_t deinitialize();

  
    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    mot_err_t run();

    /**
     * @brief Получение текущего состояния мотора. Потокобезопасно.
     * @retval AC_MOTOR_IS_STOPPED - мотор остановлен, AC_MOTOR_IS_RUNNING - мотор запущен, AC_MOTOR_IN_FAILURE - ошибка
     */
    mot_state_t getCurrentState() { return m_status; };

    /**
     * @brief Set motor speed/power
     * @param powerOut - power output as percentage (0-100)
     * @return AC_MOTOR_OK on success
     */
    mot_err_t setPower(float powerOut);


protected:

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Метод вызывается из метода initialize()
    */
    virtual mot_err_t hw_init() = 0;

    /**
    * @brief Деинициализация оборудования, например, при переходе в спящий режим
    * @details Метод вызывается из метода deinitialize()
    */
    virtual mot_err_t hw_deinit() = 0;

    /**
     * @brief Функция вычисления длинны массива (буфера) значений синуса
     * @example PWM_FREQ / MOTOR_WAVE_FREQ; PWM_FREQ / sine_wave_freq 
     */
    virtual size_t calc_SineBufferLength(mot_sine_freq_t sine_wave_freq) noexcept = 0;

    /**
     * @brief Запуск мотора с заданной скоростью
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    // virtual mot_err_t run(float powerOut) = 0;

    // virtual void stop() = 0;
    // mot_err_t setMotorFreq(mot_sine_freq_t sine_wave_freq);


    size_t fill_SineWaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hBuff, mot_pwm_val_t max_value, float max_angle = ACMOTOR_SINE_MAX_ANGLE) noexcept;

    __always_inline
    const pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& getCurrentSineBuff() noexcept { return _hCurrentBuff; }


    mutable binary_semaphore sem{1};
    // mot_pwm_val_t* pCurrentSineValue = nullptr;


private:

    _GLIBCXX_NODISCARD
    inline optional<const mot_pwm_val_t*> alloc_WaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hArray, size_t buff_length) noexcept {
        hArray.first = (mot_pwm_val_t*) malloc(buff_length * sizeof(mot_pwm_val_t));
        hArray.second = hArray.first + buff_length;
        return hArray.first;
    };

    _GLIBCXX_NODISCARD
    optional<const mot_pwm_val_t*> realloc_SineWaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hArray, size_t buff_length) noexcept;

    mot_sine_freq_t _sine_freq;
    mot_pwm_val_t _amplitude;       // TODO: Убрать и перенести в параметр генерации синусоидального массива
    float m_pwrOut = 0.0f;

    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> _hSineValArray1;
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> _hSineValArray2;
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& _hCurrentBuff{_hSineValArray1};
    // const pair<pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&, pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&> _dblBuf{_hSineValArray1, _hSineValArray2};
    mot_state_t m_status = AC_MOTOR_NOT_INITIALIZED;
};
