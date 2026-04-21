#include "unity.h"
#include <math.h>
#include "CTestAacFanMotor.h"
#include <iostream>

using namespace std;

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

TEST_CASE("Test AacFanMotor init-deinit", "[acfan]")
{
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ,100.0f};
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
    const float relTolerance = 2.0f;                // Точность расхождения в процентах

    CTestAacFanMotor motor{ACMOTOR_SINE_MIN_FREQ, 100.0f};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    const mot_pwm_val_t Offset = motor.calc_SineBufferLength(ACMOTOR_SINE_MIN_FREQ);
    
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
        TEST_ASSERT_TRUE(check2ValuesByTolerance(expectBuff.first[i], motor.get_SineBuffer().first[i], relTolerance));
    }

    motor.deinitialize();
    delete[] p1;
}

TEST_CASE("Test AacFanMotor correct calculation Sine Values 0-180 degs", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 180.0f;                  // Maximum Degrees
    const mot_pwm_val_t MaxValue = 7890U;           // Амплитуда квантования
    const float relTolerance = 6.5f;                // Точность расхождения в процентах (в библиетеке IQmath набегающая погрешность)
    const mot_sine_freq_t MOTOR_SINE_FREQ = MOTOR_WAVE_FREQ;

    CTestAacFanMotor motor{MOTOR_SINE_FREQ, 100.0f};
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
        TEST_ASSERT_TRUE(check2ValuesByTolerance(expectBuff.first[i], motor.get_SineBuffer().first[i], relTolerance));
    }

    motor.deinitialize();
    delete[] p1;
}