#pragma once


#include "AConsole2.h"
#include <unistd.h>

#include "esp_system.h"
#include "esp_console.h"



/*
 * Warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif



class CUartConsole2 : public AConsole2{

public:
    CUartConsole2();
    ~CUartConsole2();

protected:

    esp_err_t init_periph(void) override;
    
};
