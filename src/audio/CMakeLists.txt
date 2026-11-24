add_library(ps_audio STATIC
    AudioPlayer.cpp
)

target_include_directories(ps_audio PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_audio PUBLIC
    ps_core
    Qt6::Core
    Qt6::Multimedia
)
