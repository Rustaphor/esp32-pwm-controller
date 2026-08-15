#include "CSineBuffHelper.h"
#include <math.h>


// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"

float CSineBuffHelper::amplitude2Percents(acmot_sineval_t amplitude) noexcept
{
    _iq iampl;
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

size_t CSineBuffHelper::fillSineWaveValues(acmot_sineval_t amplitude) noexcept
{
    size_t length = _hBuff.second - _hBuff.first;

    const _iq PwmMaxValue = _IQ(amplitude);                                        // Амплитудное значение мощности
    const _iq offset = _IQ(_MAX_PWM_VALUE / 2);
    _iq dAngleRad = _IQmpy(_IQ(_max_angle/length), _IQ(M_PI/180.0f));              // Convert dAlpha angle to dAlphaRad (radians)
    _iq CurAngleRad = 0, val;
 
    acmot_sineval_t* pCurrent = const_cast<acmot_sineval_t*>(_hBuff.first);             // Set pointer to start of buffer
    while (pCurrent < _hBuff.second) {
        // val = _IQmpy(_IQsin(CurAngleRad), dcMaxVal);
        val = offset + _IQmpy(_IQsin(CurAngleRad) - _IQ(0.5f), PwmMaxValue);
        *pCurrent = (acmot_sineval_t) _IQint(val);
        CurAngleRad += dAngleRad;
        pCurrent++;
    }

    return length;
}
