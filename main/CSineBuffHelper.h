#pragma once
#include "acmotor_defs.h"
#include <utility>

using namespace std;

/**
 * @class CSineBuffHelper
 * @brief Вспомогательный класс для заполнения буффера ШИМ-контроллера значениями синусоидального сигнала.
 * 
 * Предоставляет методы для преобразования амплитуды в проценты и обратно,
 * а также для заполнения буфера значениями синусоиды.
 */
class CSineBuffHelper final {

    pair<const acmot_sineval_t*, const acmot_sineval_t*> &_hBuff;
    acmot_sinefreq_t _sine_wave_freq;
    float _max_angle;
    acmot_sineval_t _MAX_PWM_VALUE;
    acmot_sineval_t _MIN_PWM_VALUE;

public:

    /**
     * @brief Конструктор класса.
     * @param hBuff Ссылка на пару указателей на буферы синусоидального сигнала.
     * @param freq Частота синусоидальной волны.
     * @param max_angle Максимальный расчетный угол фазы от 0 до данного значения.
     * @param max_pwm_value Максимальное аппаратное значение ШИМ.
     */
    CSineBuffHelper(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff, acmot_sinefreq_t freq, float max_angle, acmot_sineval_t max_pwm_value, acmot_sineval_t min_pwm_value) :
        _hBuff{hBuff},
        _sine_wave_freq{freq},
        _max_angle{max_angle},
        _MAX_PWM_VALUE{max_pwm_value},
        _MIN_PWM_VALUE{min_pwm_value} {};

    /**
     * @brief Устанавливает ссылку на буфер синусоидального сигнала.
     * @param hBuff Новая ссылка пара указателей на буферы.
     */
    void setBuffer(const pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff) {
        _hBuff = hBuff;
    };

    /**
     * @brief Устанавливает частоту синусоидальной волны.
     * @param freq Новая частота.
     */
    void setFreq(float freq) { _sine_wave_freq = freq; };

    /**
     * @brief Преобразует значение амплитуды в проценты.
     * @param amplitude Значение амплитуды.
     * @return Процентное значение [0...100%] амплитуды (отношение к max_pwm_value).
     */
    _GLIBCXX_NODISCARD
    float amplitude2Percents(acmot_sineval_t amplitude) noexcept;

    /**
     * @brief Преобразует процентное значение [0...100%] в абсолютную амплитуду.
     * @param prcnt Процентное значение.
     * @return Значение амплитуды в абсолютных единицах.
     */
    _GLIBCXX_NODISCARD
    acmot_sineval_t percents2Amplitude(float prcnt) noexcept;

    /**
     * @brief Заполняет буфер таблицей синусоидальными значениями.
     * @param amplitude Амплитуда синусоидального сигнала.
     * @return Количество заполненных элементов в буфере.
     */
    size_t fill_buffer(acmot_sineval_t amplitude) noexcept;
};