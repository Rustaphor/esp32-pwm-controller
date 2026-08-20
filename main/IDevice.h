#ifndef IDEVICE_H
#define IDEVICE_H

enum devSysState {
    DEV_NOT_INITIALIZED = 0,
    DEV_INITIALIZED,
    DEV_FAILURE,
    DEV_BUSY
};

typedef unsigned char dev_state_reserved_t;

typedef struct {
   enum devSysState sysState:3;
   dev_state_reserved_t reserved:5;
} devState_t;

// Define error codes
#define DEVICE_OK                       0
#define DEVICE_IN_FAILURE               0x101
#define DEVICE_INIT_FALURE              0x102
#define DEVICE_NOT_INITIALIZED          0x103
#define DEVICE_IS_BUSY_NOW              0x104


// Интерфейс для любого аппаратного устройства
class IDevice {
public:
    virtual ~IDevice() = default;

    virtual devState_t getCurrentState() = 0;

    // Метод для инициализации конкретного устройства
    virtual int initialize() = 0;

    // Метод для деинициализации конкретного устройства
    virtual int deinitialize() = 0;
};

#endif // IDEVICE_H