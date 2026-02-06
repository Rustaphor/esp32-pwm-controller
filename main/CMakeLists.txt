idf_component_register(SRCS "wifi-ctrl.cpp" "pwmconhelp.cpp" "pwmctrl.cpp" "pwm-fanctrl.cpp"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_wifi nvs_flash
                    INCLUDE_DIRS ".")

