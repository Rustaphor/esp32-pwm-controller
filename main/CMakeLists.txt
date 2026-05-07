idf_component_register(SRCS "app-main.cpp" "consolexec.cpp" "esp-wifi-ctrl.c" "AacFanMotor.cpp" "CMotorDrive.cpp"
                    PRIV_REQUIRES esp_timer esp_driver_mcpwm esp_driver_gpio esp_pm esp_wifi nvs_flash console
                    INCLUDE_DIRS ".")

# Добавление пути к приватному файлу 'mcpwm_private.h' и его зависимости (компонент) 'esp_pm'
idf_component_get_property(mcpwmdir esp_driver_mcpwm COMPONENT_DIR)                    
idf_build_set_property(INCLUDE_DIRECTORIES ${mcpwmdir}/src APPEND)

