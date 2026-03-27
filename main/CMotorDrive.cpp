#include "CMotorDrive.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


const char* TAG = "MotorDriver";

bool tmr_onupdate_isr_callback(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx) {
    return false;
}

// Default constructor
CMotorDrive::CMotorDrive() : hTimer_{NULL} {
}

mot_err_t CMotorDrive::initialize()
{
    /*
    * Блок для управления пином Shutdown MOSFET-драйвера (если есть)
    */
    ESP_LOGI(TAG, "Start motor driver initializing");

    esp_err_t result;


#ifdef MOTOR_DRV_EN_PIN
    ESP_LOGD(TAG, "Disable MOSFET gate driver");
    gpio_config_t drv_en_config = {
        .pin_bit_mask = 1ULL << MOTOR_DRV_EN_PIN,
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK(gpio_config(&drv_en_config));
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_DRV_EN_PIN, 0));
#endif

    ESP_LOGD(TAG, "Create MCPWM timer");
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MOTOR_MCPWM_TIMER_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN, //UP_DOWN mode will generate center align PWM wave, which can reduce MOSFET switch times on same effect, extend life
        .period_ticks = MOTOR_MCPWM_PERIOD
    };
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_timer(&timer_config, &hTimer_));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;


    ESP_LOGD(TAG, "Create MCPWM operator");
    mcpwm_operator_config_t oper_config = {
        .group_id = 0
    };
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_operator(&oper_config, &hOperator_));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_operator_connect_timer(hOperator_, hTimer_));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;


    ESP_LOGD(TAG, "Create comparators");
    mcpwm_comparator_config_t compare_config = {
        .flags{ .update_cmp_on_tez = true }
    };
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_comparator(hOperator_, &compare_config, &hComparator_));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;
    // set compare value to 0, we will adjust the speed in a period timer callback
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_comparator_set_compare_value(hComparator_, 0));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;

    
#ifdef MOTOR_DRV_FAULT_PIN
    ESP_LOGD(TAG, "Create over current fault detector");
    mcpwm_gpio_fault_config_t gpio_fault_config = {
        .group_id = 0,
        .gpio_num = MOTOR_DRV_FAULT_PIN,
        .flags { 
            .active_level = 0,      // low level means fault, refer to DRV8302 datasheet
            .pull_up = true         // internally pull up
        }
    };
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&gpio_fault_config, &hFaultPin_));
#endif


    ESP_LOGD(TAG, "Create PWM generator(s)");
    mcpwm_generator_config_t gen_config = {
        .gen_gpio_num = MOTOR_PWM_PIN
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(hOperator_, &gen_config, &hGenerator_));


    ESP_LOGD(TAG, "Register MCPWM Timer callback(s)");
    mcpwm_timer_event_callbacks_t cbs = {
        .on_full = tmr_onupdate_isr_callback,
    };
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_register_event_callbacks(hTimer_, &cbs, this));
    if (!result) return AC_ERR_MOTOR_INIT_FALURE;

    
    ESP_LOGI(TAG,"Motor driver initialize passed!");
    return AC_MOTOR_OK;
}

mot_err_t CMotorDrive::hw_deinit()
{
    ESP_LOGD(TAG,"Motor Driver de-initialization started");
    return AC_ERR_MOTOR_INIT_FALURE;
}

// Destructor
CMotorDrive::~CMotorDrive() {
}

// Public method implementation
void CMotorDrive::doSomething() {
    helperFunction();
}

// Private helper function
void CMotorDrive::helperFunction() {
    // Implementation here
}