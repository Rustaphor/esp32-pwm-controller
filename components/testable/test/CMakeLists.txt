set(MAINAPP_DIR "${PROJECT_DIR}/../main")

idf_component_register(SRCS "test_mean.c" "test_acfan.cpp" "${MAINAPP_DIR}/CSineWaveHelper.cpp" 
                    INCLUDE_DIRS "." ${MAINAPP_DIR}
                    REQUIRES cmock testable)
