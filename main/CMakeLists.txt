idf_component_register(SRCS "app-main.cpp" "consolexec.cpp" "esp-wifi-ctrl.c" "pwmctrl.cpp" "AacFanMotor.cpp" "CMotorDrive.cpp"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_wifi nvs_flash console esp_gdbstub
                    INCLUDE_DIRS ".")

