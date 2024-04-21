set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

# Find all source and header directories
file(GLOB_RECURSE C_DIRS "src/*.c")
file(GLOB_RECURSE H_DIRS LIST_DIRECTORIES true "src/*.h")
foreach(H_DIR ${H_DIRS})
    get_filename_component(INCLUDE_DIR ${H_DIR} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${INCLUDE_DIR})
endforeach()

# Add source and header directories to the component
idf_component_register(
    SRCS ${C_DIRS}
    INCLUDE_DIRS "../components/mlib" ${INCLUDE_DIRS}
)