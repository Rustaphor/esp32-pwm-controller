idf_component_register(SRCS "pwmconhelp.cpp" "pwmctrl.cpp" "pwm-fanctrl.cpp"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio
                    INCLUDE_DIRS ".")

