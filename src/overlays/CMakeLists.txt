add_library(ps_overlays STATIC
    OverlayManager.cpp
)

target_include_directories(ps_overlays PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_overlays PUBLIC
    ps_core
    Qt6::Core
)
