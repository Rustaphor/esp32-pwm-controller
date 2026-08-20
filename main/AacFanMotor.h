// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "IDevice.h"
#include <optional>
#include <utility>
#include <semaphore>
#include <cstdlib>
#include "CSineBuffHelper.h"

/**
 * Максимальное (амплитудное) значение размаха функции синуса
 * 
 * @details Глобальный конфигурационный параметр, определяющий максимально возможное значение расчетное значение синусоиды. Задается в конкретной реализации "железа"
 * @showinitializer
 */
#ifndef ACMOTOR_SINE_MAX_VALUE
#error ACMOTOR_SINE_MAX_VALUE must be defined before first include!
#endif


/**
 * Минимально допустимое значение генератора ШИМ-сигналов
 * 
 * @details Данный параметр определяется схемотехническими ограничениями. \
 *  Минимально допустимая длительность импульса управления ключами MOSFET.
 *  Этот параметр также влияет и на верхнее максимальное значение ШИМ, вычитается из максимального значения счетчика.
 * @showinitializer
 * @attention Параметр не должен быть 0. Слишкорм короткий импульс может привести к перегрузке и выходу драйвера из строя.
 */
#ifndef ACMOTOR_PWM_MIN_VALUE
#warning ACMOTOR_PWM_MIN_VALUE must be defined before first include! MOSFET switches are not working with too short pulses.
#define ACMOTOR_PWM_MIN_VALUE 0
#endif


/**
 * Угол расчета значений синусоидной волны. Обычно 1/4 (90 град) или 1/2 (180 град) волны.
 * @details Глобальный параметр, используемый для расчета таблицы значений (буфера) синусоидной волны.
 * @showinitializer
 */
#ifndef ACMOTOR_SINE_MAX_ANGLE
#define ACMOTOR_SINE_MAX_ANGLE    90.0f
#endif

/**
 * Минимальная частота синусоидальной генерируемой волны мотора в Гц (например, 25)
 * 
 * @details Глобальный параметр требуется для расчета максимального размера буфера значений синуса в сэмпле. Максимальное значение достигается при минимальной частоте.
 * @showinitializer
 */
#ifndef ACMOTOR_SINE_MIN_FREQ
#define ACMOTOR_SINE_MIN_FREQ     25
#endif


typedef int acmot_err_t;
typedef enum {
    AC_MOTOR_IS_STOPPED = 0x04,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
} AcMotorState_t;


using namespace std;


/**
* @brief Абстрактрый класс управления мотором переменного тока с фазосдвигающим конденсатором для вентиллятора
*/
class AacFanMotor : public IDevice {

public:

    /**
     * @brief Конструктор задания начальных параметров мотора
     * @param freq - частота синусоидальной волны при инициализации мотора в Гц (например, 50)
     * @param powerOut - выходная мощность в процентах (мощность PWM) [1...100]
     */
    AacFanMotor(acmot_sinefreq_t freq, float powerOut = 0.0f) : _currentFreq{freq}{
        if (powerOut > 0 && powerOut <= 100) {
            auto sbh = CSineBuffHelper(_hSineWaveBuffer, _currentFreq, ACMOTOR_SINE_MAX_ANGLE, ACMOTOR_SINE_MAX_VALUE, ACMOTOR_PWM_MIN_VALUE);
            _currentPower = sbh.percents2Amplitude(powerOut);
        } else {
            _currentPower = 0;
        }
    }

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Используется обычно при первичной инициализации и восстановлении энергопотребления. После выделения памяти вызывает метод hw_init()
    * @retval DEVICE_OK - успех, иначе код ошибки
    */
    acmot_err_t initialize() override {
        sem.acquire();
        acmot_err_t result;

        optional<const acmot_sineval_t*> buff;
        uint16_t sine_array_len = calcSineBufferLength(ACMOTOR_SINE_MIN_FREQ);
        buff = _allocWaveBuffer(_hSineWaveBuffer, sine_array_len);
        if (!buff.has_value()) {
            result = AC_ERR_MOTOR_NO_BUFF_MEMORY;
            goto end_init;
        }

        _resizeWaveBufferAndFill(_hSineWaveBuffer, _currentFreq, _currentPower);

        result = this->hw_init();
        if (result != DEVICE_OK) { goto end_init_memfree; }
        _devState.sysState = DEV_INITIALIZED;
        goto end_init;

    end_init_memfree:
        free((void*) _hSineWaveBuffer.first);
        _hSineWaveBuffer = {};       
    end_init:
        sem.release();
        return result;
    };

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Используется обычно при переходе в спязий режим. Метод должен быть помещен в десткуртор финального класса.
    * @retval DEVICE_OK - успех, иначе код ошибки
    */
    acmot_err_t deinitialize() override {
        sem.acquire();
        acmot_err_t result;

        // Проверка инициализирован ли мотор
        if (_devState.sysState == DEV_NOT_INITIALIZED) {
            result = DEVICE_NOT_INITIALIZED;
            goto end_deinit;
        }
        // TODO: (else if) Выполнить проврки других состояний мотора и если мотор не остановлен, выполнить процедуру аваринйной остановки

        result = hw_deinit();
        if (_hSineWaveBuffer.first) {
            free(const_cast<acmot_sineval_t*>(_hSineWaveBuffer.first));
            _hSineWaveBuffer = {};
        }
        _devState.sysState = DEV_NOT_INITIALIZED;

    end_deinit:
        sem.release();
        return result;
    };
  
    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    acmot_err_t run() { 
        sem.acquire();
        acmot_err_t result = DEVICE_OK;

        // Проверка инициализирован ли мотор
        if (_devState.sysState == DEV_NOT_INITIALIZED) {
            result = DEVICE_NOT_INITIALIZED;
            goto end_run;
        } else if ((AcMotorState_t) _devState.reserved == AC_MOTOR_IS_RUNNING){  // Check if already running.
            goto end_run;
        }

        if (!_currentPower) {
            result = AC_ERR_MOTOR_INVALID_POWER;
            goto end_run;
        }

        result = this->hw_run(_currentPower);
        if (result == DEVICE_OK) _devState.reserved = (dev_state_reserved_t) AC_MOTOR_IS_RUNNING;

    end_run:
        sem.release();
        return result;
    }

    /**
    * @brief Остановка мотора
    */
    acmot_err_t stop() {
        sem.acquire();
        acmot_err_t result;

        if (_devState.sysState == DEV_NOT_INITIALIZED) {
            result = DEVICE_NOT_INITIALIZED;
            goto exit_stop;
        }

        switch ((AcMotorState_t) _devState.reserved)
        {
        case AC_MOTOR_IS_STOPPED:
            result = DEVICE_OK;
            break;

        case AC_MOTOR_IS_RUNNING:
            result = this->hw_stop();
            _devState.reserved = (dev_state_reserved_t) AC_MOTOR_IS_STOPPED;
            break;
        
        default:
            result = DEVICE_IS_BUSY_NOW;
            break;
        }

    exit_stop:
        sem.release();
        return result;
    }

    /**
     * @brief Получение текущего состояния мотора. Потокобезопасно.
     * @retval DEVICE_OK - мотор остановлен, остальные статусы
     */
    __always_inline
    devState_t getCurrentState() override { return _devState; }

    /**
     * @brief Set motor speed/power
     * @param powerOutPcnts - power output as percentage (0-100)
     * @return DEVICE_OK on success
     */
    acmot_err_t setPowerPercents(float powerOutPcnts) noexcept {

        sem.acquire();
        acmot_err_t result = DEVICE_OK;

        // Проверка инициализирован ли мотор
        if (_devState.sysState == DEV_NOT_INITIALIZED) {
            result = DEVICE_NOT_INITIALIZED;
            goto end_set_power;
        }

        // Проверка корректности входных данных
        if (powerOutPcnts < 0 || powerOutPcnts > 100) {
            result = AC_ERR_MOTOR_INVALID_POWER;
            goto end_set_power;
        }
        _currentPower = _setPowerOutImmediatelyLL(powerOutPcnts);

        // Если нуль, по факту остановка мотора
        if (_currentPower == 0 && _devState.reserved == (dev_state_reserved_t) AC_MOTOR_IS_RUNNING)
        {
            result = this->hw_stop();
            _devState.reserved = (dev_state_reserved_t) AC_MOTOR_IS_STOPPED;
        }

    end_set_power:
        sem.release();
        return result;
    };

    /**
     * @brief Мощноость мотора в пересчитанных процентах
     * @return Мощность мотора в процентах (%)
     */
    float getPowerOutPercent() noexcept {
        sem.acquire();
        auto sbh = CSineBuffHelper(_hSineWaveBuffer, _currentFreq, ACMOTOR_SINE_MAX_ANGLE, ACMOTOR_SINE_MAX_VALUE, ACMOTOR_PWM_MIN_VALUE);
        float powerOut = sbh.amplitude2Percents(_currentPower);
        sem.release();
        return powerOut;
    };

    // NOTE: Not implemented yet
    acmot_err_t setFrequency(acmot_sinefreq_t freq) {
        sem.acquire();
        acmot_err_t result = DEVICE_OK;
        if (_devState.sysState == DEV_NOT_INITIALIZED) {
            result = DEVICE_NOT_INITIALIZED;
            goto end_set_freq;
        }

        // TODO: Добавить валидатор установки частоты
        if (freq < ACMOTOR_SINE_MIN_FREQ) {
            result = AC_ERR_MOTOR_INVALID_FREQ;
            goto end_set_freq;
        }

        _currentFreq = freq;
        _resizeWaveBufferAndFill(_hSineWaveBuffer, _currentFreq, _currentPower);

    end_set_freq:
        sem.release();
        return result;
    };


protected:

    binary_semaphore sem{1};

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
     * @example \code{.cpp} PWM_FREQ / MOTOR_WAVE_FREQ; PWM_FREQ / freq \endcode
     */
    _GLIBCXX_NODISCARD
    virtual size_t calcSineBufferLength(acmot_sinefreq_t freq) noexcept = 0;

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
    virtual acmot_err_t hw_stop() noexcept = 0;

    __always_inline
    const pair<const acmot_sineval_t*, const acmot_sineval_t*>& getCurrentSineBuffer() noexcept { return _hSineWaveBuffer; }


private:

    _GLIBCXX_NODISCARD
    inline optional<const acmot_sineval_t*> _allocWaveBuffer(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hArray, size_t buff_length) noexcept {
        hArray.first = (acmot_sineval_t*) malloc(buff_length * sizeof(acmot_sineval_t));
        hArray.second = hArray.first + buff_length;
        return hArray.first;
    };

    inline void _resizeWaveBufferAndFill(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff, acmot_sinefreq_t freq, acmot_sineval_t amplitude) noexcept {
        auto sine_buff_len = calcSineBufferLength(freq);
        if (!sine_buff_len || sine_buff_len > hBuff.second - hBuff.first) {
            return;
        }
        hBuff = {_hSineWaveBuffer.first, _hSineWaveBuffer.first + sine_buff_len};
        _currentFreq = freq;

        auto sh = CSineBuffHelper(hBuff, freq, ACMOTOR_SINE_MAX_ANGLE, ACMOTOR_SINE_MAX_VALUE, ACMOTOR_PWM_MIN_VALUE);
        sh.fill_buffer(amplitude);
    }

    acmot_sineval_t _setPowerOutImmediatelyLL(float powerout_prcnts) noexcept {
        auto sbh = CSineBuffHelper(_hSineWaveBuffer, _currentFreq, ACMOTOR_SINE_MAX_ANGLE, ACMOTOR_SINE_MAX_VALUE, ACMOTOR_PWM_MIN_VALUE);
        acmot_sineval_t power_amplitude = sbh.percents2Amplitude(powerout_prcnts);
        if (_currentPower == power_amplitude) return _currentPower;
        sbh.fill_buffer(power_amplitude);
        return power_amplitude;
    };

    acmot_sinefreq_t _currentFreq;
    acmot_sineval_t _currentPower;
    pair<const acmot_sineval_t*, const acmot_sineval_t*> _hSineWaveBuffer;
    devState_t _devState = {
        .sysState{DEV_NOT_INITIALIZED},
        .reserved = (dev_state_reserved_t) AC_MOTOR_IS_STOPPED
    };
};
