add_library(ps_core STATIC
    Types.cpp
    ShowFile.cpp
    ShowDocument.cpp
)

target_include_directories(ps_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_core PUBLIC
    Qt6::Core
    Qt6::Gui
)
