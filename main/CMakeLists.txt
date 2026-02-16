idf_component_register(SRCS "consolexec.cpp" "esp-wifi-ctrl.c" "pwmctrl.cpp" "pwm-fanctrl.cpp"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_wifi nvs_flash console
                    INCLUDE_DIRS ".")

