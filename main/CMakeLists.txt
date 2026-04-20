idf_component_register(SRCS "app-main.cpp" "consolexec.cpp" "esp-wifi-ctrl.c" "AacFanMotor.cpp" "CMotorDrive.cpp"
                    PRIV_REQUIRES esp_timer esp_driver_mcpwm esp_driver_gpio esp_wifi nvs_flash console
                    INCLUDE_DIRS ".")

