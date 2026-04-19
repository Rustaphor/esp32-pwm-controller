#include "unity.h"
#include <math.h>
#include "CSineWaveHelper.h"
#include "CTestAacFanMotor.h"
#include <iostream>

using namespace std;
using namespace helper;

template<typename T>
static bool check2ValuesWithTolerance(T val1, T val2, const float relTol) {
    if (val1 == val2) return true;
    else if (val1 > val2 && (double)val2/(double)val1 <= relTol) {
        return true;
    }
    else if (val1 < val2 && (double)val1/(double)val2 <= relTol) {
        return true;
    }

    return false;
}


TEST_CASE("Test correct calculation Sine Wave Values 0-90 degreese", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 90.0f;                   // Maximum Degrees
    const mot_pwm_val_t MaxValue = 8000U;           // Амплитуда квантования
    const mot_pwm_val_t Offset = 425U;              // Length of Sine Wave array
    const float relTolerance = 1.0f;                // Точность расхождения в процентах
    

    // Создание тестового буфера и заполнение его синусом
    mot_pwm_val_t* p1 = new mot_pwm_val_t[Offset];
    pair<mot_pwm_val_t*, const mot_pwm_val_t*> expectBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expectBuff.first[i] = sinf(Alpha) * MaxValue;
    }

    // Создание массива чисел и заполнение его синусом тестируемого объекта
    mot_pwm_val_t* p2 = new mot_pwm_val_t[Offset];
    pair<const mot_pwm_val_t*, const mot_pwm_val_t*> buff{p2, p2 + Offset};
    CSineWaveHelper sineHelper{buff, MaxAngle};
    sineHelper.fillSineWave(MaxValue);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(check2ValuesWithTolerance(expectBuff.first[i], buff.first[i], relTolerance));
    }

    delete[] p2;
    delete[] p1;
}


TEST_CASE("Test correct calculation Sine Wave Values 0-180 degreese and 2d constructor", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 180.0f;                   // Maximum Degrees
    const mot_pwm_val_t MaxValue = 7293U;           // Амплитуда квантования
    const mot_pwm_val_t Offset = 520U;              // Length of Sine Wave array
    const float relTolerance = 1.0f;                // Точность расхождения в процентах
    

    // Создание тестового буфера и заполнение его синусом
    mot_pwm_val_t* p1 = new mot_pwm_val_t[Offset];
    pair<mot_pwm_val_t*, const mot_pwm_val_t*> expectBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expectBuff.first[i] = sinf(Alpha) * MaxValue;
    }

    // Создание массива чисел и заполнение его синусом тестируемого объекта
    mot_pwm_val_t* p2 = new mot_pwm_val_t[Offset];
    CSineWaveHelper sineHelper{p2, Offset, MaxAngle};
    sineHelper.fillSineWave(MaxValue);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(check2ValuesWithTolerance(expectBuff.first[i], p2[i], relTolerance));
    }

    delete[] p2;
    delete[] p1;
}

TEST_CASE("Test AacFanMotor class init-deinit", "[acfan]")
{
    CTestAacFanMotor motor;
    TEST_ASSERT_EQUAL(AC_MOTOR_NOT_INITIALIZED, motor.getCurrentState());

    motor.initialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_INITIALIZED, motor.getCurrentState());

    motor.deinitialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_NOT_INITIALIZED, motor.getCurrentState());
}