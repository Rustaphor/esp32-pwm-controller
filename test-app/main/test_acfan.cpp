#include "unity.h"
#include <math.h>
#include "CTestAacFanMotor.h"
#include <utility>
#include <iostream>
#include <stdio.h>
#include "helpfuns.h"
#include <inttypes.h>
#include <memory>

using namespace std;

CTestAacFanMotor motor{MOTOR_WAVE_FREQ, 100.0f};    // Default motor

// Запускается перед каждым тестом
void motor_init(CTestAacFanMotor &mot = motor){
    TEST_ASSERT_EQUAL_MESSAGE(AC_MOTOR_NOT_INITIALIZED, mot.getCurrentState(), "Error state. Motor is already initialized.");
    mot.initialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_INITIALIZED, mot.getCurrentState());
}

// Запускается после каждого теста
void motor_deinit(CTestAacFanMotor &mot = motor){
    TEST_ASSERT_TRUE_MESSAGE(AC_MOTOR_NOT_INITIALIZED != mot.getCurrentState(), "Error state! Motor is already not initialized.");
    motor.deinitialize();
    TEST_ASSERT_EQUAL(AC_MOTOR_NOT_INITIALIZED, motor.getCurrentState());
}


TEST_CASE("Test AacFanMotor correct calculation Sine Values 0-90 degs", "[acfan]")
{
    // Условия теста
    const float MaxAngle = ACMOT_SINE_MAX_ANGLE;           // Maximum Degrees
    const acmot_sineval_t MaxValue = ACMOT_SINE_MAX_VALUE; // Масимальное число счетчика ШИМ
    const acmot_sineval_t Amplitude = MaxValue;              // Амплитуда квантования
    const float relTolerance = 2.0f;                         // Точность расхождения в процентах

    CTestAacFanMotor mot{ACMOT_SINE_MIN_FREQ, 100.0f};
    motor_init(mot);

    const acmot_sineval_t Offset = mot.calcSineBufferLength(ACMOT_SINE_MIN_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    shared_ptr<acmot_sineval_t[]> pBuff { make_shared<acmot_sineval_t[]>(Offset) };    // массив из n элементов, равных 0
    pair<acmot_sineval_t*, const acmot_sineval_t*> expBuff{pBuff.get(), pBuff.get() + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы) или M_PI/6.0f
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expBuff.first[i] = MaxValue/2 + Amplitude*(sinf(Alpha) - 0.5);
    }

    // Заполнение его синусом тестируемого объекта
    mot.test_fillSineBuffer(MaxValue, MaxAngle);

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(checkValuesByTolPercents(expBuff.first[i], mot.get_SineBuffer().first[i], relTolerance));
    }

    motor_deinit(mot);
}


TEST_CASE("Test AacFanMotor correct calculation Sine Values 0-180 degs", "[acfan]")
{
    // Условия теста
    const float MaxAngle = 180.0f;                            // Maximum Degrees
    const acmot_sineval_t MaxValue = ACMOT_SINE_MAX_VALUE;  // Амплитуда квантования
    const acmot_sineval_t Amplitude = MaxValue;              // Амплитуда квантования
    const acmot_sineval_t max_error_tol = 5;                          // Точность расхождения в процентах (в библиетеке IQmath набегающая погрешность)
    const acmot_sinefreq_t MOTOR_SINE_FREQ = MOTOR_WAVE_FREQ;

    CTestAacFanMotor mot{MOTOR_SINE_FREQ, 100.0f};
    motor_init(mot);

    const acmot_sineval_t Offset = mot.calcSineBufferLength(MOTOR_SINE_FREQ);
    
    // Создание тестового буфера и заполнение его синусом
    shared_ptr<acmot_sineval_t[]> pBuff { make_shared<acmot_sineval_t[]>(Offset) };    // массив из n элементов, равных 0
    pair<acmot_sineval_t*, const acmot_sineval_t*> expBuff{pBuff.get(), pBuff.get() + Offset};
    const float dAlpha = M_PI/180.0f * MaxAngle/Offset;     // Дискретный угол (радианы)
    float Alpha = 0.0f;                                     // Начальный угол (радианы)
    for (int i = 0; i < Offset; ++i, Alpha += dAlpha) {
        expBuff.first[i] = MaxValue/2 + Amplitude*(sinf(Alpha) - 0.5);
    }

#if DISABLED_FOR_TARGETS(linux)
    // Start the timer
    ccomp_timer_start();
#endif
    // Заполнение его синусом тестируемого объекта
    mot.test_fillSineBuffer(MaxValue, MaxAngle);
#if DISABLED_FOR_TARGETS(linux)
    int64_t t = ccomp_timer_stop();
    cout << "Time: " << t << " us";
#endif

    // Сравнение результатов
    for (int i = 0; i < Offset; ++i) {
        TEST_ASSERT_TRUE(check2ValuesByDelta(expBuff.first[i], mot.get_SineBuffer().first[i], max_error_tol));
        printf("Expect: %d\tGot: %d\t Matched: %d\n", expBuff.first[i], mot.get_SineBuffer().first[i], check2ValuesByDelta(expBuff.first[i], mot.get_SineBuffer().first[i], max_error_tol));
    }

    motor_deinit(mot);
}


TEST_CASE("Test AacFanMotor setting Output Power Percents", "[acfan]"){
    motor_init();

    float in_range[] = {0.0f, 0.001f, 10.0f, 12.2f, 12.31f, 40.0f, 50.0f, 100.0f};
    float a,b;
    for (auto cur_val : in_range) {
        TEST_ASSERT_FALSE(motor.setPowerPercents(cur_val));
        a = motor.getPowerOutPercent();
        b = cur_val;
        TEST_ASSERT_TRUE(checkValuesByTolPercents(a, b, 0.30f));
        }
    
    motor_deinit();
}


TEST_CASE("Test AacFanMotor out of range setting power value", "[acfan]"){
    CTestAacFanMotor mot{MOTOR_WAVE_FREQ, 100.0f};    // Default motor
    motor_init(mot);

    float in_range[] = {-1.0f, 100.01f, 110.0f};
    for (auto cur_val : in_range) {
        TEST_ASSERT_EQUAL(ACMOT_ERR_INVALID_POWER, mot.setPowerPercents(cur_val));
    }
    
    motor_deinit(mot);
}


TEST_CASE("Test AacFanMotor run() and stop() methods", "[acfan]"){
    TEST_ASSERT_EQUAL(AC_ERR_MOTOR_NOT_INITIALIZED, motor.run());
    TEST_ASSERT_EQUAL(AC_ERR_MOTOR_NOT_INITIALIZED, motor.stop());
    motor_init();

    TEST_ASSERT_FALSE(motor.run());
    TEST_ASSERT_EQUAL(motor.getCurrentState(), AC_MOTOR_IS_RUNNING);
    TEST_ASSERT_FALSE(motor.stop());
    TEST_ASSERT_EQUAL(motor.getCurrentState(), AC_MOTOR_IS_STOPPED);
    
    motor_deinit();
}


TEST_CASE("Test AacFanMotor setting power 0\% as stop command", "[acfan]"){
    CTestAacFanMotor mot{MOTOR_WAVE_FREQ, 51.0f};
    motor_init(mot);

    TEST_ASSERT_EQUAL(AC_MOTOR_OK, mot.run());
    TEST_ASSERT_FALSE(mot.setPowerPercents(0.0f));
    TEST_ASSERT_EQUAL(mot.getCurrentState(), AC_MOTOR_IS_STOPPED);
    
    motor_deinit(mot);
}

#if DISABLED_FOR_TARGETS(linux)
TEST_CASE("Test CFanMotor ISR-handler", "[acfan]"){
    const unsigned long ISR_CALL_TIMES = 100UL * motor.calcSineBufferLength(MOTOR_WAVE_FREQ);       // Кол-полных ISR-handler кратных размеру массива синусоидных чисел

    motor_init();

    for (unsigned long i = 0; i < ISR_CALL_TIMES; ++i) {
        
    }
    
    motor_deinit();
}
#endif //!TEMPORARY_DISABLED_FOR_TARGETS(linux)