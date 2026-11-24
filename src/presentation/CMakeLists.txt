add_library(ps_presentation STATIC
    PresentationController.cpp
)

target_include_directories(ps_presentation PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_presentation PUBLIC
    ps_core
    ps_persistence
    ps_crdt
    ps_remote
    ps_media
    ps_output
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)
