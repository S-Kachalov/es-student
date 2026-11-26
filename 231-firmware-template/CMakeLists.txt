cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)

project(231_firmware_template)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

pico_sdk_init()

add_compile_options(-Wall)

# libs лежит вне папки проекта: второй аргумент задаёт каталог сборки
add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)
# выключатель профилирования: 0 — вызовов секундомеров в образе нет
target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

add_subdirectory(platform)
add_subdirectory(logic)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    logging
    profiling
    platform
    logic
    systime
    pico_stdlib
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})
