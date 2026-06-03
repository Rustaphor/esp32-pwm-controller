idf_component_register(SRCS "consolexec.cpp" "AConsole2Cmd.cpp" "AConsole2.cpp" "CUartConsole2.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES console
                    PRIV_REQUIRES esp_driver_uart esp_driver_usb_serial_jtag)


