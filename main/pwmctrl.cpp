#include "pwmctrl.h"
#include "esp_log.h"

#define PWM_MAX_DUTY (2^LEDC_TIMER_8_BIT)


constexpr const char* logTAG = __FILE_NAME__;

constexpr uint32_t pow2(ledc_timer_bit_t exp) {
    uint32_t pow2 = 2;
    for (uint8_t i = 1; i < exp; i++){ pow2 *= 2; }
    return pow2;
}


CPwmControl::CPwmControl(int pin, ledc_timer_t timerNum, ledc_channel_t channel)
{
    pin_ = pin;
    ldc_timer_ = timerNum;
    ldc_channel_ = channel;
}

esp_err_t CPwmControl::initialize(uint32_t freqHz)
{
    scoped_lock<recursive_mutex> lock(rm_);

    // Validate
    if (freqHz < 1000 || freqHz > 200000) {
        ESP_LOGE(logTAG,"Value Frequency %d is out of range. The value is not changed.", freqHz);
        return ESP_FAIL;
    };

    enabled_ = false;
    frequency_ = freqHz;  // Default 1kHz

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

    return ESP_OK;
}

esp_err_t CPwmControl::setDutyCycle(float percent)
{
    scoped_lock<recursive_mutex> lock(rm_);

     // Validate
    if (percent < 0 || percent > 100) {
        ESP_LOGE(logTAG,"Value DC %f is out of range. It must be range [0...100] or [0.0...1.0]! DC is last state.", percent);
        return ESP_FAIL;
    }

    if (percent > 1.0) percent = percent / 100;

    constexpr const uint32_t pwmmax = pow2(RWM_DUTY_RESOLUTION);
    return setSysDC(percent * pwmmax);
}


esp_err_t CPwmControl::enable()
{
    scoped_lock<recursive_mutex> lock(rm_);

    enabled_ = true;
    ESP_ERROR_CHECK_WITHOUT_ABORT(setSysDC(dc_));
    return update();
}

esp_err_t CPwmControl::disable()
{
    scoped_lock<recursive_mutex> lock(rm_);

    ESP_ERROR_CHECK_WITHOUT_ABORT(setSysDC(0));
    auto result = update();
    enabled_ = false;
    return result;
}

float CPwmControl::getDutyCycle()
{
    constexpr const uint32_t pwmmax = pow2(RWM_DUTY_RESOLUTION);
    //  float result = (float) dc_ / (float) pwmmax;
    return (float) dc_ / (float) pwmmax;
}

pair<uint32_t, uint32_t> CPwmControl::getSysDC()
{
    constexpr const uint32_t pwmmax = pow2(RWM_DUTY_RESOLUTION);
    return pair<uint32_t, uint32_t>({dc_,pwmmax});
}

esp_err_t CPwmControl::update()
{
    scoped_lock<recursive_mutex> lock(rm_);

    if (!enabled_) return ESP_OK;

    auto result = ledc_update_duty(LEDC_HIGH_SPEED_MODE, ldc_channel_);
    if (result) {
        ESP_LOGE(logTAG, "Error #%d failure update duty value!", result);
    }
    return result;
}

esp_err_t CPwmControl::setSysDC(uint32_t dc)
{
    dc_ = dc;
    auto result = ledc_set_duty(LEDC_HIGH_SPEED_MODE, ldc_channel_, dc_);
    if (result) {
        ESP_LOGE(logTAG, "Error #%d failure set duty value!", result);
        return result;
    }
    return result;
}
