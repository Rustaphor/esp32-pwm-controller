#include "unity.h"
#include "CUartConsole2.h"
#include "CPwmCtrl.h"

using namespace std;


CUartConsole2 con2;

TEST_CASE("Test Console2 initialization", "[console2]")
{

    CPwmCtrl pwm_cmd;

    // Первая инициализация
    TEST_ASSERT_FALSE_MESSAGE(con2.initialize(),"Error: fail Console2 initialize.");


    con2.registerCommand(pwm_cmd);

    // Повторная инициализация (должна закончится с ошибкой)
    TEST_ASSERT_EQUAL(con2.initialize(), ESP_ERR_NOT_ALLOWED);
}
