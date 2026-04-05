#pragma once
#include "acmotor_defs.h"
#include <utility>


namespace helper {

using namespace std;

class CSineWaveHelper final {

    float max_angle_degs;

public:
    CSineWaveHelper(pair<const mot_pwm_val_t*, const mot_pwm_val_t*>, float max_angle_degrees);

    uint32_t getOffset();
    void fillSineWave(mot_pwm_val_t amplitude);
    const mot_pwm_val_t* copyBuffer(pair<const mot_pwm_val_t*, const mot_pwm_val_t*> out_buff);

};

};

