#pragma once
#include "acmotor_defs.h"
#include <utility>

using namespace std;

class CSineBuffHelper final {

    pair<const acmot_sineval_t*, const acmot_sineval_t*> &_hBuff;
    acmot_sinefreq_t _sine_wave_freq;
    float _max_angle;
    acmot_sineval_t _MAX_PWM_VALUE;

public:
    CSineBuffHelper(pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff, acmot_sinefreq_t freq, float max_angle, acmot_sineval_t max_pwm_value) :
        _hBuff{hBuff},
        _sine_wave_freq{freq},
        _max_angle{max_angle},
        _MAX_PWM_VALUE{max_pwm_value} {};

    void setBuffer(const pair<const acmot_sineval_t*, const acmot_sineval_t*>& hBuff) {
        _hBuff = hBuff;
    };

    void setFreq(float freq) { _sine_wave_freq = freq; };

    _GLIBCXX_NODISCARD
    float amplitude2Percents(acmot_sineval_t amplitude) noexcept;

    _GLIBCXX_NODISCARD
    acmot_sineval_t percents2Amplitude(float prcnt) noexcept;

    size_t fillSineWaveValues(acmot_sineval_t amplitude) noexcept;
};