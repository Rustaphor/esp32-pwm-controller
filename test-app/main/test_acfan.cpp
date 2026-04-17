#include "unity.h"
#include "CSineWaveHelper.h"
#include <math.h>

using namespace std;
using namespace helper;

TEST_CASE("Test generating Sine Wave array", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 90.0f;                 // Maximum Degrees
    const mot_pwm_val_t Amplitude = 16000;        // Амплитуда квантования
    const mot_pwm_val_t Offset = 128U;          // Length of Sine Wave array
    

    // Создание тестового буфера и заполнение его синусом
    mot_pwm_val_t* p1 = new mot_pwm_val_t[Offset];
    pair<mot_pwm_val_t*, const mot_pwm_val_t*> testBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;
    float Alpha = 0.0f;
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        testBuff.first[i] = sinf(Alpha) * Amplitude;
        // cout << testBuff.first + i << "  " << testBuff.first[i] << endl;
    }

    // Тестирование класса массива синусоидной волны
    mot_pwm_val_t* p2 = new mot_pwm_val_t[Offset];
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> buff{p2, p2 + Offset};
    CSineWaveHelper sineHelper{buff, MaxAngle};
    sineHelper.fillSineWave(Amplitude);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        cout << buff.first + i << "  " << buff.first[i] << endl;
    }


    TEST_ASSERT_EQUAL(0, 0);

    delete[] p2;
    delete[] p1;
}

