add_library(logic STATIC
    # исходники api, led-task, button-task и pi-task
    api/api.c
    api/api-callbacks.c
    button-task/button-task.c
    led-task/led-task.c
    pi-task/pi-task.c
)

# путь к заголовкам — папка logic
target_include_directories(logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(logic PUBLIC
    stdio-text-protocol
)

target_link_libraries(logic PRIVATE
    device
    logging
    profiling
    systime
    platform
)

