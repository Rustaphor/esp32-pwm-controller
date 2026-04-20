#include "AacFanMotor.h"
#include <math.h>


// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"


mot_err_t AacFanMotor::run() {
    if (!this->m_pwrOut) {
        m_status = AC_MOTOR_IS_STOPPED;
    }
    return AC_MOTOR_OK;
}

mot_err_t AacFanMotor::initialize()
{
    sem.acquire();
    mot_err_t result;

    optional<const mot_pwm_val_t*> op1, op2;
    uint16_t sine_array_len = calc_SineBufferLength(_sine_freq);
    op1 = alloc_WaveBuffer(_hSineWaveBuffer, sine_array_len);
    if (!op1.has_value()) {
        result = AC_ERR_MOTOR_NO_MEMORY;
        goto end_init;
    }

    result = this->hw_init();
    if (result != AC_MOTOR_OK) { goto end_init_memfree; }
    m_status = AC_MOTOR_INITIALIZED;
    goto end_init;

end_init_memfree:
    free((void*) _hSineWaveBuffer.first);
    _hSineWaveBuffer = {};       
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
    if (_hSineWaveBuffer.first) {
        free(const_cast<mot_pwm_val_t*>(_hSineWaveBuffer.first));
        _hSineWaveBuffer = {};
    }
    m_status = AC_MOTOR_NOT_INITIALIZED;

end_deinit:
    sem.release();
    return result;
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
