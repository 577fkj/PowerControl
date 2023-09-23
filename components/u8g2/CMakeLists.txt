file(GLOB U8G2 "src/*.c")
idf_component_register(SRCS ${U8G2}
                    INCLUDE_DIRS "include")
