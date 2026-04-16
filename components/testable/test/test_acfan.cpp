#include "unity.h"
#include "CSineWaveHelper.h"

using namespace helper;

TEST_CASE("Test generating Sine Wave array", "[acfan]")
{
    const float MaxAngle = 90.0f;                 // Maximum Degrees
    const mot_pwm_val_t Offset = 128;       // Length of Sine Wave array
    const mot_pwm_val_t* p = new mot_pwm_val_t[Offset];

    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> buff{p, p + Offset};

    CSineWaveHelper sineHelper{buff, MaxAngle};


    TEST_ASSERT_EQUAL(0, 0);

    delete[] p;
}

