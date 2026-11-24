add_library(ps_chords STATIC
    ChordEngine.cpp
)

target_include_directories(ps_chords PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_chords PUBLIC
    ps_core
    Qt6::Core
)
