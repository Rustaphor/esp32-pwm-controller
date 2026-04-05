#include "AacFanMotor.h"
#include "CSineWaveHelper.h"
#include <math.h>

// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"


// TODO: (Debug only) Удалить после отладки блок ниже
#include "esp_log.h"


using namespace helper;

mot_err_t AacFanMotor::run() {
    if (!this->m_pwrOut) {
        m_status = AC_MOTOR_IS_STOPPED;
    }
    return AC_MOTOR_OK;
}


mot_err_t AacFanMotor::deinitialize()
{
    sem.acquire();

    mot_err_t result;
    if (m_status == AC_MOTOR_NOT_INITIALIZED) {
        result = AC_ERR_MOTOR_NOT_INITIALIZED;
        goto end_deinit;
    }

    result = this->hw_deinit();
    if (!_pWave_array.first) {
        free(const_cast<mot_pwm_val_t*>(_pWave_array.first));
        _pWave_array = {};
    }
    m_status = AC_MOTOR_NOT_INITIALIZED;

end_deinit:
    sem.release();
    return result;
}


mot_status_t AacFanMotor::get_status()
{
    return mot_status_t();
}


mot_err_t AacFanMotor::make_SineQuaterWaveArray(uint8_t wave_freq)
{
    sem.acquire();
    mot_err_t result = AC_MOTOR_OK;

    // // optional<const mot_pwm_val_t*> pMem2;
    // auto pMem1 = helper_CreateNewSineDataArray(wave_freq, 90.0f);
    // if (!pMem1) {
    //     result = AC_ERR_MOTOR_NO_MEMORY;
    //     goto exit_sqwa;
    // }
    // _pWave_array.first = (mot_pwm_val_t*) pMem1.value_or(nullptr);

    // result = helper_memAllocDoubleBuffer(&pCurrentSineValue);
    // if (!pCurrentSineValue) {
    //      free((void*) pMem1.value());
    //      result = AC_ERR_MOTOR_NO_MEMORY;
    //      goto exit_sqwa;
    // }


// exit_sqwa:
    sem.release();
    return result;
}


optional<const mot_pwm_val_t*> AacFanMotor::helper_memAllocDoubleBuffer(const pair<const mot_pwm_val_t*, uint16_t>& array)
{
    void* pBuf = malloc(array.second);
    if (!pBuf) {
        return {};
    }

    mot_pwm_val_t* pBuff2 = (mot_pwm_val_t*) memcpy(pBuf,  array.first, array.second);
    return pBuff2;
}
