#include "pwmctrl.h"
#include "esp_log.h"

#define PWM_MAX_DUTY (2^LEDC_TIMER_8_BIT)


constexpr const char* logTAG = __FILE_NAME__;

void CPwmControl::initialize()
{
    scoped_lock<recursive_mutex> lock(rm_);

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .duty_resolution  = RWM_DUTY_RESOLUTION,
        .timer_num        = ldc_timer_,
        .freq_hz          = frequency_,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin_,
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = ldc_channel_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = ldc_timer_,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void CPwmControl::initialize(float dc, uint32_t freqHz, uint8_t pin, ledc_timer_t timerNum, ledc_channel_t channel)
{
    scoped_lock<recursive_mutex> lock(rm_);

    int pin_ = pin;
    ledc_timer_t ldc_timer_ = timerNum;
    ledc_channel_t ldc_channel_ = channel;
    uint32_t frequency_ = freqHz;  // Default 1kHz
    float duty_cycle_{0.0f};    // 0-100%

    initialize();
}

esp_err_t CPwmControl::setDutyCycle(float percent)
{
    scoped_lock<recursive_mutex> lock(rm_);

    // Validate
    if (percent < 0 || percent > 100) {
        ESP_LOGW(logTAG,"Value DC %f is out of range. It must be range [0...100]! DC is last state.", percent);
        return ESP_FAIL;
    }
    duty_cycle_ = percent;

    constexpr const uint32_t pwmmax = (2^RWM_DUTY_RESOLUTION);
    auto result = ledc_set_duty(LEDC_HIGH_SPEED_MODE, ldc_channel_, duty_cycle_ * pwmmax);
    if (!result) {
        ESP_LOGE(logTAG, "Error #%d failure set duty value!", result);
        return result;
    }
    
    return update();
}

esp_err_t CPwmControl::setFrequency(uint32_t freqHz)
{
    scoped_lock<recursive_mutex> lock(rm_);
    
    // Validate
    if (freqHz < 1000 || freqHz > 200000) {
        ESP_LOGW(logTAG,"Value Frequency %d is out of range. The value is not changed.", freqHz);
        return ESP_FAIL;
    };

    frequency_ = freqHz;
    return ESP_OK;
}

esp_err_t CPwmControl::update()
{
    auto result = ledc_update_duty(LEDC_HIGH_SPEED_MODE, ldc_channel_);
    if (!result) {
        ESP_LOGE(logTAG, "Error #%d failure update duty value!", result);
    }
    return result;
}
