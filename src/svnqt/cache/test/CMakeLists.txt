set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h
)
build_test(sqlite)
