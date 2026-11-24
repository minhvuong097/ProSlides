add_library(ps_output STATIC
    DisplayOutput.cpp
    OutputManager.cpp
    NdiOutput.cpp
)

target_include_directories(ps_output PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_output PUBLIC
    ps_core
    ps_recording
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

if(PS_ENABLE_NDI AND NDI_LIBRARY)
    target_link_libraries(ps_output PUBLIC ${NDI_LIBRARY})
endif()
