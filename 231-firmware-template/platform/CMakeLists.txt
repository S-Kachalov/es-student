add_library(platform STATIC
    src/platform.c
)

target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(platform PRIVATE
    pico_stdlib
)
