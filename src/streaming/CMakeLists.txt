add_library(ps_streaming STATIC
    StreamOutput.cpp
)

target_include_directories(ps_streaming PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_streaming PUBLIC
    ps_core
    Qt6::Core
    Qt6::Gui
)

if(PS_ENABLE_FFMPEG)
    target_link_libraries(ps_streaming PUBLIC
        FFmpeg::avcodec
        FFmpeg::avformat
        FFmpeg::avutil
        FFmpeg::swscale
    )
endif()
