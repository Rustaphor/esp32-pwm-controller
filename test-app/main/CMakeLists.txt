set(MAINAPP_DIR "${PROJECT_DIR}/../main")

# Добавление аппаратно-зависимых тест-кейсов (в том числе для linux-приложений)
if(${IDF_TARGET} STREQUAL "linux")
    set(ADDREQUIRES cmock)
else()
    # set(ADDSRCS "test_console2.cpp" "${MAINAPP_DIR}/CMotorCtrl.cpp")
    # set(ADDREQUIRES console2)
endif()

# Общие тест-кейсы проверки лолики работы частей программ
idf_component_register(SRCS "CTestAacFanMotor.cpp" "utest-app.cpp" "test_acfan.cpp" "test_feature.cpp" "${MAINAPP_DIR}/AacFanMotor.cpp" ${ADDSRCS}
                    PRIV_REQUIRES unity esp_event ${ADDREQUIRES}
                    INCLUDE_DIRS "." ${MAINAPP_DIR}
                    WHOLE_ARCHIVE)
unset(ADDREQUIRES)
unset(ADDSRCS)
unset(MAINAPP_DIR)

set_source_files_properties(utest-app.cpp
    PROPERTIES COMPILE_FLAGS
    -Wno-unused-variable)
