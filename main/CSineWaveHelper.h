#pragma once
#include "acmotor_defs.h"
#include <utility>


namespace helper {

using namespace std;

class CSineWaveHelper final {

    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> table_buff_;
    float angle_max_degreese_;

public:
    CSineWaveHelper(pair<const mot_pwm_val_t*, const mot_pwm_val_t*> &hPair, float max_angle_degrees) : table_buff_{hPair}, angle_max_degreese_{max_angle_degrees} {};
    CSineWaveHelper(const mot_pwm_val_t* const pStart, unsigned int num_elements, float max_angle_degrees);

    inline unsigned int getOffset() {
        return table_buff_.second - table_buff_.first;
    }

    unsigned int fillSineWave(mot_pwm_val_t max_value);
    void copyBuffer(const mot_pwm_val_t* pOutBuff);
    
};

};

