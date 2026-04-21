#include "AacFanMotor.h"
#include <math.h>


// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"


mot_err_t AacFanMotor::run() {
    if (!this->_relPwrOut) {
        m_status = AC_MOTOR_IS_STOPPED;
    }
    return AC_MOTOR_OK;
}

mot_err_t AacFanMotor::setPower(float powerOut)
{
    sem.acquire();
    mot_err_t result = AC_MOTOR_OK;
    if (m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_set_power;
    }

    if (powerOut < 0 || powerOut > 100) {
        result = AC_ERR_MOTOR_INVALID_POWER;
        goto end_set_power;
    }

    set_powerOutFast(powerOut);

end_set_power:
    sem.release();
    return result;
}

mot_err_t AacFanMotor::setFrequency(mot_sine_freq_t sine_wave_freq)
{
    sem.acquire();
    mot_err_t result = AC_MOTOR_OK; size_t sine_buff_len;
    if (m_status == AC_MOTOR_NOT_INITIALIZED) {
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
    _sine_freq = sine_wave_freq;

    // TODO: Вставить функцию пересчета синуса

end_set_freq:
    sem.release();
    return result;
}

mot_err_t AacFanMotor::initialize()
{
    sem.acquire();
    mot_err_t result;

    optional<const mot_pwm_val_t*> op1, op2;
    uint16_t sine_array_len = calc_SineBufferLength(_sine_freq);
    op1 = alloc_WaveBuffer(_hSineWaveMinFreqBuff, sine_array_len);
    if (!op1.has_value()) {
        result = AC_ERR_MOTOR_NO_MEMORY;
        goto end_init;
    }
    _hSineWaveBuffer = {_hSineWaveMinFreqBuff};

    set_powerOutFast(_relPwrOut);

    result = this->hw_init();
    if (result != AC_MOTOR_OK) { goto end_init_memfree; }
    m_status = AC_MOTOR_INITIALIZED;
    goto end_init;

end_init_memfree:
    free((void*) _hSineWaveMinFreqBuff.first);
    _hSineWaveMinFreqBuff = {};       
end_init:
    sem.release();
    return result;
}

mot_err_t AacFanMotor::deinitialize()
{
    sem.acquire();
    mot_err_t result;

    // Проверка инициализирован ли мотор
    if (m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_deinit;
    }
    // TODO: (else if) Выполнить проврки других состояний мотора и если мотор не остановлен, выполнить процедуру аваринйной остановки

    result = hw_deinit();
    if (_hSineWaveMinFreqBuff.first) {
        free(const_cast<mot_pwm_val_t*>(_hSineWaveMinFreqBuff.first));
        _hSineWaveMinFreqBuff = {};
        _hSineWaveBuffer = {};
    }
    m_status = AC_MOTOR_NOT_INITIALIZED;

end_deinit:
    sem.release();
    return result;
}

mot_pwm_val_t AacFanMotor::set_powerOutFast(float powerOut) noexcept
{
    _iq amp = _IQmpy(_IQ(ACMOTOR_SINE_MAX_VALUE), _IQ(powerOut / 100.0f));
    mot_pwm_val_t max_val = _IQtoF(amp);
    fill_SineWaveBuffer(_hSineWaveBuffer, max_val);
    _relPwrOut = powerOut;
    return max_val;
}

optional<const mot_pwm_val_t *> AacFanMotor::realloc_SineWaveBuffer(pair<const mot_pwm_val_t *, const mot_pwm_val_t *> &hArray, size_t buff_length) noexcept
{
    hArray.first = (mot_pwm_val_t*) realloc((void*) hArray.first, buff_length * sizeof(mot_pwm_val_t));
    hArray.second = hArray.first + buff_length;
    return hArray.first;
}

size_t AacFanMotor::fill_SineWaveBuffer(pair<const mot_pwm_val_t *, const mot_pwm_val_t *> &hBuff, mot_pwm_val_t max_value, float max_angle) noexcept
{
    size_t length = hBuff.second - hBuff.first;

    _iq dAngleRad = _IQmpy(_IQ(max_angle/length), _IQ(M_PI / 180.0f));             // Convert dAlpha angle to dAlphaRad (radians)
    _iq CurAngleRad = 0, dcMaxVal = _IQ(max_value), val;
 
    mot_pwm_val_t* pCurrent = const_cast<mot_pwm_val_t*>(hBuff.first);             // Set pointer to start of buffer

    while (pCurrent < hBuff.second) {
        val = _IQmpy(_IQsin(CurAngleRad), dcMaxVal);
        *pCurrent = (mot_pwm_val_t) _IQtoF(val);
        CurAngleRad += dAngleRad;
        pCurrent++;
    }

    return length;
}
