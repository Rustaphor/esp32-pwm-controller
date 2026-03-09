idf_component_register(SRCS "mean.c"
                    INCLUDE_DIRS "include")

set_source_files_properties(mean.c
    PROPERTIES COMPILE_FLAGS
    -Wno-unused-variable)
