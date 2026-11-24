add_library(ps_providers STATIC
    PlanningCenterProvider.cpp
)

target_include_directories(ps_providers PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_providers PUBLIC
    ps_core
    ps_persistence
    Qt6::Core
    Qt6::Network
)
