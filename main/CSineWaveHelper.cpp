#include "CSineWaveHelper.h"
#include <math.h>

// Use IQ18 type, range [-8,192 8,191.999 996 185]
// (This definition should be added before including "IQmathLib.h")
#define GLOBAL_IQ               18
#include "IQmathLib.h"

helper::CSineWaveHelper::CSineWaveHelper(pair<const mot_pwm_val_t *, const mot_pwm_val_t *> &hPair, float max_angle_degrees)
: angle_max_degreese_{max_angle_degrees}, table_buff_{hPair} {}

helper::CSineWaveHelper::CSineWaveHelper(const mot_pwm_val_t *const pStart, unsigned int offset, float max_angle_degrees) : angle_max_degreese_{max_angle_degrees} {
    table_buff_.first = pStart;
    table_buff_.second = pStart + offset;
}


unsigned int helper::CSineWaveHelper::fillSineWave(mot_pwm_val_t amplitude)
{

    unsigned int length = getOffset();

    _iq dAngle = _IQdiv(_IQ(angle_max_degreese_), _IQ(length));     // Минимальный дискретный угол в градусах length / maxAngleDeg
    _iq dAngleRad = _IQmpy(dAngle, _IQ(M_PI / 180.0f));             // Convert dAlpha angle to dAlphaRad (radians)

    _iq CurAngleRad = 0, dcMaxVal = _IQdiv(_IQ(amplitude), 2), val;
    
    mot_pwm_val_t* pCurrent = const_cast<mot_pwm_val_t*>(table_buff_.first); // Set pointer to start of buffer

    while (pCurrent <= table_buff_.second) {
        val = _IQmpy(_IQsin(CurAngleRad), dcMaxVal);
        *pCurrent = (mot_pwm_val_t) _IQtoF(val);
        CurAngleRad += dAngleRad; pCurrent++;
    }

    return length;
}
