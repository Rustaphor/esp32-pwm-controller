set(MAINAPP_DIR "${PROJECT_DIR}/../main")

idf_component_register(SRCS "utest-app.cpp" "test_acfan.cpp" "${MAINAPP_DIR}/CSineWaveHelper.cpp"
                    PRIV_REQUIRES unity cmock
                    INCLUDE_DIRS "." ${MAINAPP_DIR}
                    WHOLE_ARCHIVE)

set_source_files_properties(utest-app.cpp
    PROPERTIES COMPILE_FLAGS
    -Wno-unused-variable)
