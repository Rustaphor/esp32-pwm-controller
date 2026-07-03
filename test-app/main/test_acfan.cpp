#include "unity.h"
#include <math.h>
#include "CTestAacFanMotor.h"
#include <utility>
#include <iostream>
#include "helpfuns.h"
#include <inttypes.h>



using namespace std;


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
    const acmot_sineval_t MaxValue = 8000U;           // Амплитуда квантования
    const float relTolerance = 2.0f;                // Точность расхождения в процентах

    CTestAacFanMotor motor{ACMOTOR_SINE_MIN_FREQ, 100.0f};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    const acmot_sineval_t Offset = motor.calcSineBufferLength(ACMOTOR_SINE_MIN_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    acmot_sineval_t* p1 = new acmot_sineval_t[Offset];
    pair<acmot_sineval_t*, const acmot_sineval_t*> expectBuff{p1, p1 + Offset};
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
    const acmot_sineval_t MaxValue = 7890U;           // Амплитуда квантования
    const float relTolerance = 6.5f;                // Точность расхождения в процентах (в библиетеке IQmath набегающая погрешность)
    const acmot_sinefreq_t MOTOR_SINE_FREQ = MOTOR_WAVE_FREQ;

    CTestAacFanMotor motor{MOTOR_SINE_FREQ, 100.0f};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    const acmot_sineval_t Offset = motor.calcSineBufferLength(MOTOR_SINE_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    acmot_sineval_t* p1 = new acmot_sineval_t[Offset];
    pair<acmot_sineval_t*, const acmot_sineval_t*> expectBuff{p1, p1 + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expectBuff.first[i] = sinf(Alpha) * MaxValue;
    }

#if DISABLED_FOR_TARGETS(linux)
    // Start the timer
    ccomp_timer_start();
#endif
    // Заполнение его синусом тестируемого объекта
    motor.test_fillSineBuffer(MaxValue, MaxAngle);
#if DISABLED_FOR_TARGETS(linux)
    int64_t t = ccomp_timer_stop();
    cout << "Time: " << t << " us";
#endif

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(check2ValuesByTolerance(expectBuff.first[i], motor.get_SineBuffer().first[i], relTolerance));
    }

    motor.deinitialize();
    delete[] p1;
}


TEST_CASE("Test AacFanMotor setting Output Power", "[acfan]"){
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ, 0.0f};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    float in_range[] = {0.0f, 0.001f, 10.0f, 12.2f, 12.31f, 40.0f, 50.0f, 100.0f};
    for (auto cur_val : in_range) {
        TEST_ASSERT_FALSE(motor.setPower(cur_val));
        TEST_ASSERT_TRUE(check2ValuesByTolerance(motor.getPowerOutPercent(), cur_val, 0.30f));
        }
    
    motor.deinitialize();
}


TEST_CASE("Test AacFanMotor out of range setting power value", "[acfan]"){
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ, 120.0f};
    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    float in_range[] = {-1.0f, 100.01f, 110.0f};
    for (auto cur_val : in_range) {
        TEST_ASSERT_EQUAL(AC_ERR_MOTOR_INVALID_POWER, motor.setPower(cur_val));
    }
    
    motor.deinitialize();
}


TEST_CASE("Test AacFanMotor run() and stop() methods", "[acfan]"){
    CTestAacFanMotor motor2{MOTOR_WAVE_FREQ, 100.0f};
    TEST_ASSERT_EQUAL(AC_ERR_MOTOR_NOT_INITIALIZED, motor2.run());
    TEST_ASSERT_EQUAL(AC_ERR_MOTOR_NOT_INITIALIZED, motor2.stop());
    motor2.initialize();

    TEST_ASSERT_FALSE(motor2.run());
    TEST_ASSERT_EQUAL(motor2.getCurrentState(), AC_MOTOR_IS_RUNNING);
    TEST_ASSERT_FALSE(motor2.stop());
    TEST_ASSERT_EQUAL(motor2.getCurrentState(), AC_MOTOR_IS_STOPPED);
    
    motor2.deinitialize();
}


TEST_CASE("Test AacFanMotor setting power 0\% as stop command", "[acfan]"){
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ, 80.0f};
    motor.initialize();

    motor.run();
    TEST_ASSERT_FALSE(motor.setPower(0.0f));
    TEST_ASSERT_EQUAL(motor.getCurrentState(), AC_MOTOR_IS_STOPPED);
    
    motor.deinitialize();
}

#if DISABLED_FOR_TARGETS(linux)
TEST_CASE("Test CMotorDrive ISR-handler", "[acfan]"){
    CTestAacFanMotor motor{MOTOR_WAVE_FREQ, 100.0f};
    const unsigned long ISR_CALL_TIMES = 100UL * motor.calcSineBufferLength(MOTOR_WAVE_FREQ);       // Кол-полных ISR-handler кратных размеру массива синусоидных чисел

    TEST_ASSERT_FALSE_MESSAGE(motor.initialize(),"Error: fail Motor initialize.");

    for (unsigned long i = 0; i < ISR_CALL_TIMES; ++i) {
        
    }
    
    motor.deinitialize();
}
#endif //!TEMPORARY_DISABLED_FOR_TARGETS(linux)