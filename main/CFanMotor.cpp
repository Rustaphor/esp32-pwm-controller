#include "CFanMotor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hal/mcpwm_ll.h"
#include "mcpwm_private.h"
#include "freertos/semphr.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

SemaphoreHandle_t hxSemMot;
StaticSemaphore_t hxSemMotBuff;

// Обработчик прерывания таймера ШИМ-счетчика
bool IRAM_ATTR pwmtimer_onupdate_isr_cb(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx) {
    // DRAM_ATTR (для констант)

    static int inc_dec = 1;
    BaseType_t task_yield = pdFALSE;
    CFanMotor* pMotor = (CFanMotor*) user_ctx;

    // Назначение DC значения ШИМ

    // Высокоуровневый более медленный вариант изменений компаратора
    // mcpwm_comparator_set_compare_value(pMotor->hComparator_, *pMotor->_pCurVal);
    mcpwm_ll_operator_set_compare_value(MCPWM_LL_GET_HW(MOTOR_DRV_GROUP_ID), 
        pMotor->hComparator_->oper->oper_id,
        pMotor->hComparator_->cmpr_id,
        *pMotor->_pCurVal);

    pMotor->_pCurVal += inc_dec;

    if (pMotor->_pCurVal >= pMotor->getCurrentSineBuffer().second) {
        inc_dec = -1;
        pMotor->_pCurVal = const_cast<acmot_sineval_t*>(pMotor->getCurrentSineBuffer().second) - 1;
    } else if (pMotor->_pCurVal < pMotor->getCurrentSineBuffer().first) {
        inc_dec = 1;
        pMotor->_pCurVal = const_cast<acmot_sineval_t*>(pMotor->getCurrentSineBuffer().first);
        xSemaphoreGiveFromISR(pMotor->hxSem, &task_yield);
    }

    return task_yield;
}

acmot_err_t CFanMotor::hw_init()
{
    esp_err_t result;

    /*
    * Блок для управления пином Shutdown MOSFET-драйвера (если есть)
    */
#ifdef MOTOR_DRV_EN_PIN
    ESP_LOGD(tag, "Disable MOSFET gate driver");
    gpio_config_t drv_en_config = {
        .pin_bit_mask = 1ULL << MOTOR_DRV_EN_PIN,
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK(gpio_config(&drv_en_config));
    ESP_ERROR_CHECK(gpio_set_level(MOTOR_DRV_EN_PIN, 0));
#endif


#ifdef MOTOR_DRV_FAULT_PIN
    mcpwm_gpio_fault_config_t gpio_fault_config = {
        .group_id = MOTOR_DRV_GROUP_ID,
        .gpio_num = MOTOR_DRV_FAULT_PIN,
        .flags { 
            .active_level = 0,      // low level means fault, refer to DRV8302 datasheet
            .pull_up = true         // internally pull up
        }
    };
#endif

    mcpwm_timer_config_t timer_config = {
        .group_id = MOTOR_DRV_GROUP_ID,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MOTOR_MCPWM_TIMER_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN, //UP_DOWN mode will generate center align PWM wave, which can reduce MOSFET switch times on same effect, extend life
        .period_ticks = MOTOR_MCPWM_PERIOD
    };
    mcpwm_timer_event_callbacks_t cbs = {
        .on_full = pwmtimer_onupdate_isr_cb,
    };

    mcpwm_operator_config_t oper_config = {
        .group_id = MOTOR_DRV_GROUP_ID
    };

    mcpwm_comparator_config_t compare_config = {
        .flags{ .update_cmp_on_tez = true }
    };

    mcpwm_generator_config_t gen_config = {};

    ESP_LOGD(tag, "Create MCPWM operator");
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_timer(&timer_config, &hTimer_));
    if (result) {
        ESP_LOGD(tag, "Error #%d: failed create new MCPWM timer.", result);
        goto exit_error_init;
    }

    ESP_LOGD(tag, "Create MCPWM operator");
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_operator(&oper_config, &hOperator_));
    if (result) goto exit_error_init;
    ESP_LOGD(tag, "Connect operator to the MCPWM driver");
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_operator_connect_timer(hOperator_, hTimer_));
    if (result) goto exit_error_init;


    ESP_LOGD(tag, "Create comparators");
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_comparator(hOperator_, &compare_config, &hComparator_));
    if (result) goto exit_error_init;
    // set compare value to 0, we will adjust the speed in a period timer callback
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_comparator_set_compare_value(hComparator_, 0));
    if (result) goto exit_error_init;

    
#ifdef MOTOR_DRV_FAULT_PIN
    ESP_LOGD(tag, "Create over current fault detector");
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&gpio_fault_config, &hFaultPin_));
#endif

    ESP_LOGD(tag, "Create PWM generator(s)");
    gen_config.gen_gpio_num = MOTOR_PWM_HS_PIN;
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_generator(hOperator_, &gen_config, &hGenerator_[0]));
    if (result) goto exit_error_init;
    gen_config.gen_gpio_num = MOTOR_PWM_LS_PIN;
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_new_generator(hOperator_, &gen_config, &hGenerator_[1]));
    if (result) goto exit_error_init;

    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_generator_set_actions_on_compare_event(hGenerator_[0],
                                            MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, hComparator_, MCPWM_GEN_ACTION_LOW),
                                            MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, hComparator_, MCPWM_GEN_ACTION_HIGH),
                                            MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    if (result) goto exit_error_init;
    // result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_generator_set_actions_on_compare_event(hGenerator_[1],
    //                                         MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, hComparator_, MCPWM_GEN_ACTION_HIGH),
    //                                         MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, hComparator_, MCPWM_GEN_ACTION_LOW),
    //                                         MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    // if (result) goto exit_error_init;
    // Debugging begin
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_generator_set_action_on_timer_event(hGenerator_[1],
                                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_TOGGLE)));
    if (result) goto exit_error_init;
    // Debuggin end

    // Регистрация обработчика прерывания таймера
    ESP_LOGD(tag, "Register MCPWM Timer callback(s)");
    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_register_event_callbacks(hTimer_, &cbs, this));
    if (result) goto exit_error_init;

    _pCurVal = const_cast<acmot_sineval_t*>(getCurrentSineBuffer().first); // Reset pointer
    hxSemMot = xSemaphoreCreateMutexStatic(&hxSemMotBuff);

    ESP_LOGI(tag,"Motor driver initialize passed!");

    return DEVICE_OK;

exit_error_init:
    ESP_LOGE(tag,"Error #%d mcpwm-driver fail initialize", result);
    return DEVICE_INIT_FALURE;
}

acmot_err_t CFanMotor::hw_deinit()
{
    acmot_err_t result;

    result = mcpwm_del_generator(hGenerator_[1]);
    if (result) goto err_hwinit;
    result = mcpwm_del_generator(hGenerator_[0]);
    if (result) goto err_hwinit;


    result = mcpwm_del_comparator(hComparator_);
    if (result) goto err_hwinit;

    result = mcpwm_del_operator(hOperator_);
    if (result) goto err_hwinit;

    result = mcpwm_del_timer(hTimer_);
    if (result) goto err_hwinit;

    result = hw_set_enabled(false);
    if (result) goto err_hwinit;

    ESP_LOGD(tag,"Hardware de-initialized");
    vSemaphoreDelete(hxSemMot);
    return DEVICE_OK;

err_hwinit:
    // TODO: добавить статус/действие критической ошибки переинициализации
    ESP_LOGE(tag,"Error #%d mcpwm-driver fail de-initializing", result);
    return result;
}

size_t CFanMotor::calcSineBufferLength(acmot_sinefreq_t sine_wave_freq) noexcept
{
    _pCurVal = const_cast<acmot_sineval_t*>(getCurrentSineBuffer().first); // Reset pointer
    return (MOTOR_MCPWM_TIMER_RESOLUTION_HZ/(MOTOR_MCPWM_PERIOD * 4)) / sine_wave_freq;
}

acmot_err_t CFanMotor::hw_run(const acmot_sineval_t powerOut)
{
    mcpwm_comparator_set_compare_value(hComparator_, 0);

    auto result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_enable(hTimer_));
    if (result) {
        ESP_LOGE(tag, "Error #%d driver mcpwm timer enable failed", result);
        return result;
    }

    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_start_stop(hTimer_, MCPWM_TIMER_START_NO_STOP));
    if (result) {
        ESP_LOGE(tag, "Error #%d driver mcpwm timer start-stop failed", result);
    }

    ESP_LOGI(tag, "Motor driver started.");
    _pCurVal = const_cast<acmot_sineval_t*>(getCurrentSineBuffer().first);  // Reset pointer
    return result;
}

acmot_err_t CFanMotor::hw_stop()
{
    // Отправка команды таймеру-генератору ШИМ оставиться при следующем знанчении 0;
    auto result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_start_stop(hTimer_, MCPWM_TIMER_STOP_EMPTY));
    if (result) {
        ESP_LOGE(tag, "Error #%d driver mcpwm timer stopping", result);
        return result;
    }

    result = ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_timer_disable(hTimer_));
    if (result) {
        ESP_LOGE(tag, "Error #%d driver mcpwm timer disabling", result);
        return result;
    }

    ESP_LOGI(tag, "Motor driver stopped.");
    return DEVICE_OK;
}

acmot_err_t CFanMotor::hw_set_enabled(bool en)
{
    acmot_err_t result = DEVICE_OK;
#ifdef MOTOR_DRV_EN_PIN
    result = gpio_set_level(MOTOR_DRV_EN_PIN, en);
    ESP_LOGI(tag, "Mosfet gate driver is %s", en ? "Enable" : "Disable");
#endif
    return result;
}

