add_library(ps_timeline STATIC
    TimelineEngine.cpp
)

target_include_directories(ps_timeline PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_timeline PUBLIC
    ps_core
    Qt6::Core
)
