#include "CSineBuffHelper.h"
#include <math.h>


// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"

float CSineBuffHelper::amplitude2Percents(acmot_sineval_t amplitude) noexcept
{
    _iq iampl;
    if(amplitude < _MIN_PWM_VALUE) amplitude = _MIN_PWM_VALUE;
    else if (amplitude >_MAX_PWM_VALUE) amplitude -= _MIN_PWM_VALUE;
    const _iq mp = _IQdiv(_IQ(100.0f),_IQ(_MAX_PWM_VALUE));
    iampl = _IQmpy(_IQ(amplitude), mp);
    return _IQtoF(iampl);
}

_GLIBCXX_NODISCARD acmot_sineval_t CSineBuffHelper::percents2Amplitude(float prcnt) noexcept
{
    const _iq mp = _IQdiv(_IQ(prcnt),_IQ(100.0f));
    _iq amp = _IQmpy(_IQ(_MAX_PWM_VALUE), mp);
    return (acmot_sineval_t) _IQint(amp);
}

size_t CSineBuffHelper::fill_buffer(acmot_sineval_t amplitude) noexcept
{
    size_t buff_len = _hBuff.second - _hBuff.first;

    const _iq offset = _IQ(_MIN_PWM_VALUE);
    _iq MaxVal;
    if (amplitude >= _MAX_PWM_VALUE) {
        amplitude = _MAX_PWM_VALUE;                                                 // limit to max pwm value
        MaxVal = _IQ(amplitude) - offset;
    } else {
        MaxVal = _IQ(amplitude);
    }
    _iq dAngleRad = _IQmpy(_IQ(_max_angle/buff_len), _IQ(M_PI/180.0f));             // Convert dAlpha angle to dAlphaRad (radians)
    _iq CurAngleRad = 0, val;
    int t2v;
 
    acmot_sineval_t* pCurrent = const_cast<acmot_sineval_t*>(_hBuff.first);        // Set pointer to start of buffer
    while (pCurrent < _hBuff.second) {
        val = offset + _IQmpy(_IQsin(CurAngleRad), MaxVal);
        t2v = _IQint(val);
        if (t2v < 0) t2v = 0;  // Values must be only posivive
        *pCurrent = (acmot_sineval_t) t2v;
        CurAngleRad += dAngleRad;
        pCurrent++;
    }

    return buff_len;
}
