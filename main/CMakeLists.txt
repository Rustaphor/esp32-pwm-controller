idf_component_register(SRCS "esp-wifi-ctrl.c" "pwmconexec.cpp" "pwmctrl.cpp" "pwm-fanctrl.cpp"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_wifi nvs_flash console
                    INCLUDE_DIRS ".")

