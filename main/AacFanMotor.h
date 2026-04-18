// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#pragma once
#include "acmotor_defs.h"
#include <optional>
#include <utility>
#include <semaphore>


typedef int mot_err_t;
typedef enum {
    AC_MOTOR_IS_STOPPED = 1,
    AC_MOTOR_IS_RUNNING,
    AC_MOTOR_IN_FAILURE,
    AC_MOTOR_IS_BUSY,
    AC_MOTOR_INITIALIZED,
    AC_MOTOR_NOT_INITIALIZED
} mot_status_t;


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
    AacFanMotor(uint8_t sine_wave_freq, mot_pwm_val_t amplitude) : _sine_freq{sine_wave_freq}, _amplitude{amplitude} {}


    ~AacFanMotor() { deinitialize(); }


    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Используется обычно при первичной инициализации и восстановлении энергопотребления. После выделения памяти вызывает метод hw_init()
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    mot_err_t initialize();

    /**
    * @brief Деинициализация мотора, например, при переходе в спящий режим
    * @details Используется обычно при переходе в спязий режим
    * @retval AC_MOTOR_OK - успех, иначе код ошибки
    */
    mot_err_t deinitialize();

  
    /**
     * @brief Запуск мотора с прежней (ранее установленной) мощностью
     */
    mot_err_t run();

    /**
     * @brief Получение текущего статуса мотора. Потокобезопасно.
     * @retval AC_MOTOR_IS_STOPPED - мотор остановлен, AC_MOTOR_IS_RUNNING - мотор запущен, AC_MOTOR_IN_FAILURE - ошибка
     */
    mot_status_t get_status();


protected:

    /**
    * @brief Первичная инициалиация оборудования для упраления мотором
    * @details Данный метод вызывается из метода initialize()
    */
    virtual mot_err_t hw_init() = 0;

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

    virtual uint16_t sineArrayLength(uint8_t sine_wave_freq) noexcept = 0;

    mot_err_t make_SineQuaterWaveArray(uint8_t sine_wave_freq);


    __always_inline
    const pair<const mot_pwm_val_t*, const mot_pwm_val_t*> getSineValArray() noexcept { return _hSineValArray1; }


    binary_semaphore sem{1};
    // mot_pwm_val_t* pCurrentSineValue = nullptr;

private:

    // _GLIBCXX_NODISCARD
    static optional<const mot_pwm_val_t*> helper_memAllocDoubleBuffer(const pair<const mot_pwm_val_t*, uint16_t>& array) noexcept;
    
    _GLIBCXX_NODISCARD
    optional<const mot_pwm_val_t*> alloc_SineWaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hArray, uint8_t sine_wave_freq, uint32_t pwm_freq) noexcept;

    _GLIBCXX_NODISCARD
    inline optional<const mot_pwm_val_t*> alloc_SineWaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hArray, uint16_t sine_array_len) noexcept {
        hArray.first = (mot_pwm_val_t*) malloc(sine_array_len * sizeof(mot_pwm_val_t));
        hArray.second = hArray.first + sine_array_len;
        return hArray.first;
    };

    _GLIBCXX_NODISCARD
    optional<const mot_pwm_val_t*> realloc_SineWaveBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>& hArray, uint16_t sine_array_len) noexcept;


    uint8_t _sine_freq;
    mot_pwm_val_t _amplitude;       // TODO: Убрать и перенести в параметр генерации синусоидального массива
    float m_pwrOut = 0.0f; 

    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> _hSineValArray1;
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> _hSineValArray2;
    const pair<pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&, pair<const mot_pwm_val_t*, const mot_pwm_val_t*>&> _dblBuf{_hSineValArray1, _hSineValArray2};
    mot_status_t m_status = AC_MOTOR_NOT_INITIALIZED;
};
