#include "unity.h"
#include <math.h>
#include "CSineWaveHelper.h"
#include "CTestAacFanMotor.h"
#include <iostream>

using namespace std;
using namespace helper;

// Функция проверки относительной точности
template<typename T>
static bool check2ValuesByTolerance(T val1, T val2, const float relTol) {
    if (val1 == val2) return true;
    else if (val1 > val2 && 100*(1 - (double) val2/(double) val1) <= relTol) {
        return true;
    }
    else if (val2 > val1 && 100*(1 - (double) val1/(double) val2) <= relTol) {
        return true;
    }

    return false;
}

// Функция проверки сравнительной точности
template<typename T>
static bool check2ValuesByDelta(T val1, T val2, const T max_delta) {
    if (val1 == val2) return true;
    else if (val1 > val2 && val1-val2 <= max_delta) {
        return true;
    }
    else if (val2 > val1 && val2-val1 <= max_delta) {
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
        TEST_ASSERT_TRUE(check2ValuesByTolerance(expectBuff.first[i], buff.first[i], relTolerance));
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
        TEST_ASSERT_TRUE(check2ValuesByTolerance(expectBuff.first[i], p2[i], relTolerance));
    }

    delete[] p2;
    delete[] p1;
}

TEST_CASE("Test AacFanMotor init-deinit", "[acfan]")
{
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ,MOTOR_MCPWM_AMPLITUDE};
    TEST_ASSERT_EQUAL(AC_MOTOR_NOT_INITIALIZED, motor.getCurrentState());

    motor.initialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_INITIALIZED, motor.getCurrentState());

    motor.deinitialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_NOT_INITIALIZED, motor.getCurrentState());
}

TEST_CASE("Test AacFanMotor correct calculation Sine Values 0-90 degs", "[acfan]")
{
    // Условия теста
    const float MaxAngle = ACMOTOR_SINE_MAX_ANGLE;  // Maximum Degrees
    const mot_pwm_val_t MaxValue = 8000U;           // Амплитуда квантования
    const mot_pwm_val_t MaxDelta = 2;               // Точность расхождения в процентах

    CTestAacFanMotor motor{MOTOR_WAVE_FREQ, MaxValue};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    const mot_pwm_val_t Offset = motor.calc_SineBufferLength(MOTOR_WAVE_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    mot_pwm_val_t* p1 = new mot_pwm_val_t[Offset];
    pair<mot_pwm_val_t*, const mot_pwm_val_t*> expectBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expectBuff.first[i] = sinf(Alpha) * MaxValue;
    }

    // Заполнение его синусом тестируемого объекта
    motor.test_fillSineBuffer(MaxValue, MaxAngle);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(check2ValuesByDelta(expectBuff.first[i], motor.get_SineBuffer().first[i], MaxDelta));
    }

    motor.deinitialize();
    delete[] p1;
}

TEST_CASE("Test AacFanMotor correct calculation Sine Values 0-180 degs", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 180.0f;                  // Maximum Degrees
    const mot_pwm_val_t MaxValue = 7890U;           // Амплитуда квантования
    const mot_pwm_val_t MaxDelta = 13;               // Точность расхождения в процентах
    const mot_sine_freq_t MOTOR_SINE_FREQ = 48;     // Частота мотора (Гц)

    CTestAacFanMotor motor{MOTOR_SINE_FREQ, MaxValue};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    const mot_pwm_val_t Offset = motor.calc_SineBufferLength(MOTOR_SINE_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    mot_pwm_val_t* p1 = new mot_pwm_val_t[Offset];
    pair<mot_pwm_val_t*, const mot_pwm_val_t*> expectBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expectBuff.first[i] = sinf(Alpha) * MaxValue;
    }

    // Заполнение его синусом тестируемого объекта
    motor.test_fillSineBuffer(MaxValue, MaxAngle);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        cout << "i = " << i << ": " << expectBuff.first[i] << " " << motor.get_SineBuffer().first[i] << " check=" << check2ValuesByDelta(expectBuff.first[i], motor.get_SineBuffer().first[i], MaxDelta) << endl;
        TEST_ASSERT_TRUE(check2ValuesByDelta(expectBuff.first[i], motor.get_SineBuffer().first[i], MaxDelta));
    }

    motor.deinitialize();
    delete[] p1;
}