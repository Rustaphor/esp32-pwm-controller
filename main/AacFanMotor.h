// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "acmotor_defs.h"
#include "IDevice.h"
#include <optional>
#include <utility>
#include <semaphore>
#include <cstdlib>


typedef int acmot_err_t;
typedef enum {
    AC_MOTOR_IS_STOPPED = 1,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
    AC_MOTOR_IS_BUSY,
    AC_MOTOR_INITIALIZED,
    AC_MOTOR_NOT_INITIALIZED
} AcMotorState;

typedef union {
    enum {
        AC_MOTOR_IS_STOPPED = 1,
        AC_MOTOR_IS_RUNNING,
    } state;
    devState_t sys_state;
} acMotState_t;

using namespace std;


/**
* @brief Абстрактрый класс управления мотором переменного тока с фазосдвигающим конденсатором для вентиллятора
*/
class AacFanMotor : public IDevice {

public:

    /**
     * @brief Конструктор задания начальных параметров мотора
     * @param sine_wave_freq - частота синусоидальной волны при инициализации мотора в Гц (например, 50)
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    AacFanMotor(acmot_sinefreq_t sine_wave_freq, float powerOut = 0.0f) : _currentSineFreq{sine_wave_freq}{
        if (powerOut > 0 && powerOut <= 100) {
            _currentAmplitude = AacFanMotor::_calcMaxSineValueInPercents(powerOut);
        } else {
            _currentAmplitude = 0;
        }
    }

    int getState2() const { return DEVICE_IN_FAILURE; };

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Используется обычно при первичной инициализации и восстановлении энергопотребления. После выделения памяти вызывает метод hw_init()
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    acmot_err_t initialize() override;

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Используется обычно при переходе в спязий режим. Метод должен быть помещен в десткуртор финального класса.
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    acmot_err_t deinitialize() override;
  
    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    acmot_err_t run() { return _run(_currentAmplitude); }

    /**
    * @brief Остановка мотора
    */
    acmot_err_t stop();

    /**
     * @brief Получение текущего состояния мотора. Потокобезопасно.
     * @retval AC_MOTOR_IS_STOPPED - мотор остановлен, AC_MOTOR_IS_RUNNING - мотор запущен, AC_MOTOR_IN_FAILURE - ошибка
     */
    AcMotorState getCurrentState() { return _m_status; };

    /**
     * @brief Set motor speed/power
     * @param powerOut - power output as percentage (0-100)
     * @return AC_MOTOR_OK on success
     */
    acmot_err_t setPowerPercents(float powerOut) noexcept;

    constexpr static inline bool checkPowerPercentsValue(float powerVal) {
        if (powerVal < 0 || powerVal > 100) return false;
        return true;
    }

    /**
     * @brief Мощноость мотора в пересчитанных процентах
     * @return Мощность мотора в процентах (%)
     */
    float getPowerOutPercent() noexcept;


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
    _GLIBCXX_NODISCARD
    virtual size_t calcSineBufferLength(acmot_sinefreq_t sine_wave_freq) noexcept = 0;

    /**
     * @brief Запуск мотора с заданной мощностью или ищменение текущей мощности
     * @details Метод вызывается из метода run().
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]%
     */
    virtual acmot_err_t hw_run(const acmot_sineval_t powerOut) = 0;

    /**
     * @brief Остановка мотора
     * @details Метод вызывается из метода stop(). По-умолчанию устанавливает значние мошности в 0%
     */
    virtual acmot_err_t hw_stop() {
        _setPowerOutImmediatelyLL(0U);
        return AC_MOTOR_OK;
    }

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

    inline void _resizeWaveBufferAndFill(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff, acmot_sinefreq_t sine_wave_freq, acmot_sineval_t amplitude) noexcept {
        auto sine_buff_len = calcSineBufferLength(sine_wave_freq);
        if (!sine_buff_len || sine_buff_len > hBuff.second - hBuff.first) {
            return;
        }
        hBuff = {_hSineWaveMinFreqBuff.first, _hSineWaveMinFreqBuff.first + sine_buff_len};
        _currentSineFreq = sine_wave_freq;

        fill_SineWaveBuffer(hBuff, amplitude);
    }

    _GLIBCXX_NODISCARD
    static acmot_sineval_t _calcMaxSineValueInPercents(float powerout_percent) noexcept;

    acmot_sineval_t _setPowerOutImmediatelyLL(acmot_sineval_t powerOut) noexcept;
    acmot_err_t _run(acmot_sineval_t powerOut);

    acmot_sinefreq_t _currentSineFreq;
    acmot_sineval_t _currentAmplitude;

    pair<const acmot_sineval_t*, const acmot_sineval_t*> _hSineWaveMinFreqBuff;
    pair<const acmot_sineval_t*, const acmot_sineval_t*> _hSineWaveBuffer;
    AcMotorState _m_status = AC_MOTOR_NOT_INITIALIZED;
};
