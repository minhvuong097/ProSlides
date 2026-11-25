add_library(ps_stage STATIC
    StageManager.cpp
)

target_include_directories(ps_stage PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_stage PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)
