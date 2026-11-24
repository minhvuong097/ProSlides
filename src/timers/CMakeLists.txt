add_library(ps_timers STATIC
    TimerManager.cpp
)

target_include_directories(ps_timers PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_timers PUBLIC
    ps_core
    Qt6::Core
)
