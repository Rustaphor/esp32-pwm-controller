idf_component_register(SRCS "CSineBuffHelper.cpp" "CFanMotor.cpp" "CHwManager.cpp" "CMotCtrlCmd.cpp" "app-main.cpp" "esp-wifi-ctrl.c"
                    PRIV_REQUIRES esp_timer esp_driver_mcpwm esp_driver_gpio esp_pm esp_wifi nvs_flash console2
                    INCLUDE_DIRS ".")

# Добавление пути к приватному файлу 'mcpwm_private.h' и его зависимости (компонент) 'esp_pm'
idf_component_get_property(mcpwmdir esp_driver_mcpwm COMPONENT_DIR)                    
idf_build_set_property(INCLUDE_DIRECTORIES ${mcpwmdir}/src APPEND)


