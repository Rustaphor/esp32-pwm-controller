#include "AacFanMotor.h"

// TODO: (Debug only) Удалить после отладки блок ниже
#include "esp_log.h"

mot_err_t AacFanMotor::run() {
    if (!this->m_pwrOut) {
        m_status = AC_MOTOR_IS_STOPPED;
    }
    return AC_MOTOR_OK;
}

mot_err_t AacFanMotor::deinitialize()
{
   
    auto result = this->hw_deinit();

    m_status = AC_MOTOR_NOT_INITIALIZED;
    return result;
}

AacFanMotor::~AacFanMotor()
{
    // TODO: Добавить назначения статуса мотора при уничтожении объекта (fail) без возможности отмены процедуры
    // TODO: Добавить процедуру остановки мотора и отключения драйвера
    this->deinitialize();
}

mot_status_t AacFanMotor::get_status()
{
    return mot_status_t();
}

// Private constructor
AacFanMotor::AacFanMotor() {

    // m_pwrOut = 0;
    // m_status = AC_MOTOR_NOT_INITIALIZED;
}
