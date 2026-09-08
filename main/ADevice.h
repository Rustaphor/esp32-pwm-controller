#pragma once

enum devSysState {
    DEV_NOT_INITIALIZED = 1,
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
#define DEVICE_ERR_INIT_FALURE          0x101
#define DEVICE_ERR_NOT_INITIALIZED      0x102
#define DEVICE_ERR_IS_BUSY_NOW          0x103
#define DEVICE_ERR_IN_FAILURE_STATE     0x104
#define DEVICE_ERR_UNKNOW_STATE         0x105


// Интерфейс для любого аппаратного устройства
class ADevice {
public:
    virtual ~ADevice() = default;

    // __always_inline
    devState_t getCurrentState() { return dev_state; }

    // Первичная инициализация устройства
    int initialize();

    // Деициализация устройства
    int deinitialize();

protected:

    /**
    * @brief Аппаратно-завиисимая инициализация оборудования, например, до первого использования или после перехода в спящий режим
    * @details Метод вызывается из метода initialize()
    */
    virtual int hw_init() = 0;

    /**
     * @brief Деинициализация оборудования, например, при переходе в спящий режим
     * @details Метод вызывается из метода deinitialize()
     */
    virtual int hw_deinit() = 0;
    
    /**
     * @brief Проверяет текущее состояние устройства относительно ожидаемого и возвращает код ошибки при несоответствии
     * @param state Ожидаемое состояние устройства для проверки (значение из enum devSysState)
     * @return Код ошибки: DEVICE_OK при совпадении состояний, иначе один из кодов ошибки устройства
     */
    int checkCurrentStateOrGetError(enum devSysState state);

    devState_t dev_state = {
        .sysState{DEV_NOT_INITIALIZED},
        .reserved{0}
    };

};