#pragma once
#include "acmotor_defs.h"
#include <utility>


namespace helper {

using namespace std;

class CSineWaveHelper final {

    float angle_max_degreese_;
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> table_buff_;

public:

    CSineWaveHelper(pair<const mot_pwm_val_t*, const mot_pwm_val_t*> &hPair, float max_angle_degrees);
    CSineWaveHelper(const mot_pwm_val_t* const pStart, unsigned int offset, float max_angle_degrees);

    inline unsigned int getOffset() {
        return table_buff_.second - table_buff_.first;
    }

    unsigned int fillSineWave(mot_pwm_val_t amplitude);
    void copyBuffer(const mot_pwm_val_t* pOutBuff);
    
};

};

