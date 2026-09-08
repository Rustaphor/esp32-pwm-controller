#include "ADevice.h"


int ADevice::initialize()
{
    // Check if the device not yet initialized
    int result = checkCurrentStateOrGetError(DEV_NOT_INITIALIZED);
    if (result == DEVICE_OK) {
        result = this->hw_init();
        if (result == DEVICE_OK) {
            dev_state.sysState = DEV_INITIALIZED;
        }
    }

    return result;
}

int ADevice::deinitialize()
{
    // Check if the device is initialized
    int result = checkCurrentStateOrGetError(DEV_INITIALIZED);
    if (result == DEVICE_OK) {
        result = this->hw_deinit();
        if (result == DEVICE_OK) {
            dev_state.sysState = DEV_NOT_INITIALIZED;
        }
    }

    return result;
}

int ADevice::checkCurrentStateOrGetError(enum devSysState state){

    if (dev_state.sysState == state) return DEVICE_OK;

    int result;
    switch (dev_state.sysState) {
        case DEV_NOT_INITIALIZED:
            result = DEVICE_ERR_NOT_INITIALIZED;
            break;
        case DEV_BUSY:
            result = DEVICE_ERR_IS_BUSY_NOW;
            break;
        case DEV_FAILURE:
            result = DEVICE_ERR_IN_FAILURE_STATE;
            break;
        default:
            result = DEVICE_ERR_UNKNOW_STATE;
    }

    return result;
}
