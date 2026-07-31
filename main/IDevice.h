#ifndef IDEVICE_H
#define IDEVICE_H

typedef enum {
    DEVICE_IN_FAILURE = 1,
    DEVICE_IS_BUSY,
    DEVICE_INITIALIZED,
    DEVICE_NOT_INITIALIZED
} devState_t;


// Define error codes
#define DEVICE_OK                       0
#define DEVICE_FAIL                     0x101
#define DEVICE_INIT_FALURE              0x102
#define DEVICE_NOT_INITIALIZED          0x103
#define DEVICE_IS_BUSY_NOW              0x104


// Интерфейс для любого аппаратного устройства
class IDevice {
public:
    virtual ~IDevice() = default;

    virtual const char* getName() = 0;

    // Метод для инициализации конкретного устройства
    virtual int initialize() = 0;

    // Метод для деинициализации конкретного устройства
    virtual int deinitialize() = 0;
};

#endif // IDEVICE_H