#include "AacFanMotor.h"
#include <math.h>


// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"


acmot_err_t AacFanMotor::run() {
    if (!this->_currentAmplitude) {
        _m_status = AC_MOTOR_IS_STOPPED;
    }
    return AC_MOTOR_OK;
}

acmot_err_t AacFanMotor::setPower(float powerOut) noexcept
{
    sem.acquire();
    acmot_err_t result = AC_MOTOR_OK;

    // Проверка инициализирован ли мотор
    if (_m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_set_power;
    }

    // Проверка корректности входных данных
    if (powerOut < 0 || powerOut > 100) {
        result = AC_ERR_MOTOR_INVALID_POWER;
        goto end_set_power;
    }

    _setPowerOutImmediately(powerOut);

end_set_power:
    sem.release();
    return result;
}

float AacFanMotor::getPowerOutPercent() noexcept
{
    // TODO: Добавить мьютекс чтения
    sem.acquire();
    float result = 0.0f; _iq cur_pwr;
    constexpr const _iq mp = _IQ(100.0f/ACMOTOR_SINE_MAX_VALUE);
    if (_currentAmplitude == 0) goto end_get_power;
    cur_pwr = _IQmpy(_IQ(_currentAmplitude), mp);
    result = _IQtoF(cur_pwr);

end_get_power:
    sem.release();
    return result;
}

acmot_err_t AacFanMotor::setFrequency(acmot_sinefreq_t sine_wave_freq)
{
    sem.acquire();
    acmot_err_t result = AC_MOTOR_OK; size_t sine_buff_len;
    if (_m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_set_freq;
    }

    if (sine_wave_freq < ACMOTOR_SINE_MIN_FREQ) {
        result = AC_ERR_MOTOR_INVALID_FREQ;
        goto end_set_freq;
    }

    sine_buff_len = calc_SineBufferLength(sine_wave_freq);
    if (!sine_buff_len || sine_buff_len > _hSineWaveMinFreqBuff.second - _hSineWaveMinFreqBuff.first) {
        result = AC_ERR_MOTOR_FAIL;
        goto end_set_freq;
    }
    _hSineWaveBuffer = {_hSineWaveMinFreqBuff.first, _hSineWaveMinFreqBuff.first + sine_buff_len};
    _currentSineFreq = sine_wave_freq;

    // TODO: Вставить функцию пересчета синуса

end_set_freq:
    sem.release();
    return result;
}

acmot_err_t AacFanMotor::initialize()
{
    sem.acquire();
    acmot_err_t result;

    optional<const acmot_sineval_t*> op1, op2;
    uint16_t sine_array_len = calc_SineBufferLength(_currentSineFreq);
    op1 = _allocWaveBuffer(_hSineWaveMinFreqBuff, sine_array_len);
    if (!op1.has_value()) {
        result = AC_ERR_MOTOR_NO_MEMORY;
        goto end_init;
    }
    _hSineWaveBuffer = {_hSineWaveMinFreqBuff};

    _setPowerOutImmediately(_currentAmplitude);

    result = this->hw_init();
    if (result != AC_MOTOR_OK) { goto end_init_memfree; }
    _m_status = AC_MOTOR_INITIALIZED;
    goto end_init;

end_init_memfree:
    free((void*) _hSineWaveMinFreqBuff.first);
    _hSineWaveMinFreqBuff = {};       
end_init:
    sem.release();
    return result;
}

acmot_err_t AacFanMotor::deinitialize()
{
    sem.acquire();
    acmot_err_t result;

    // Проверка инициализирован ли мотор
    if (_m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_deinit;
    }
    // TODO: (else if) Выполнить проврки других состояний мотора и если мотор не остановлен, выполнить процедуру аваринйной остановки

    result = hw_deinit();
    if (_hSineWaveMinFreqBuff.first) {
        free(const_cast<acmot_sineval_t*>(_hSineWaveMinFreqBuff.first));
        _hSineWaveMinFreqBuff = {};
        _hSineWaveBuffer = {};
    }
    _m_status = AC_MOTOR_NOT_INITIALIZED;

end_deinit:
    sem.release();
    return result;
}

_GLIBCXX_NODISCARD acmot_sineval_t AacFanMotor::_calcMaxSineValue(float powerOut) noexcept
{
    _iq amp = _IQmpy(_IQ(ACMOTOR_SINE_MAX_VALUE), _IQ(powerOut / 100.0f));
    acmot_sineval_t max_val = _IQtoF(amp);
    return max_val;
}

acmot_sineval_t AacFanMotor::_setPowerOutImmediately(acmot_sineval_t powerOut) noexcept
{
    if (_currentAmplitude == powerOut) return powerOut;
    fill_SineWaveBuffer(_hSineWaveBuffer, powerOut);
    _currentAmplitude = powerOut;
    return powerOut;
}

optional<const acmot_sineval_t *> AacFanMotor::_reAllocSineWaveBuffer(pair<const acmot_sineval_t *, const acmot_sineval_t *> &hArray, size_t buff_length) noexcept
{
    hArray.first = (acmot_sineval_t*) realloc((void*) hArray.first, buff_length * sizeof(acmot_sineval_t));
    hArray.second = hArray.first + buff_length;
    return hArray.first;
}

size_t AacFanMotor::fill_SineWaveBuffer(pair<const acmot_sineval_t *, const acmot_sineval_t *> &hBuff, acmot_sineval_t max_value, float max_angle) noexcept
{
    size_t length = hBuff.second - hBuff.first;

    _iq dAngleRad = _IQmpy(_IQ(max_angle/length), _IQ(M_PI / 180.0f));             // Convert dAlpha angle to dAlphaRad (radians)
    _iq CurAngleRad = 0, dcMaxVal = _IQ(max_value), val;
 
    acmot_sineval_t* pCurrent = const_cast<acmot_sineval_t*>(hBuff.first);             // Set pointer to start of buffer

    while (pCurrent < hBuff.second) {
        val = _IQmpy(_IQsin(CurAngleRad), dcMaxVal);
        *pCurrent = (acmot_sineval_t) _IQtoF(val);
        CurAngleRad += dAngleRad;
        pCurrent++;
    }

    return length;
}
