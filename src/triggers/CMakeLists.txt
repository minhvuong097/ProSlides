add_library(ps_triggers STATIC
    MidiInput.cpp
    TriggerManager.cpp
)

target_include_directories(ps_triggers PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_triggers PUBLIC
    ps_core
    Qt6::Core
)

if(WIN32)
    target_link_libraries(ps_triggers PUBLIC winmm)
endif()
