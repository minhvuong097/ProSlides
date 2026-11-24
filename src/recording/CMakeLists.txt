add_library(ps_recording STATIC
    OutputRecorder.cpp
)

target_include_directories(ps_recording PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_recording PUBLIC
    ps_core
    Qt6::Core
    Qt6::Gui
)

if(PS_ENABLE_FFMPEG)
    target_link_libraries(ps_recording PUBLIC
        FFmpeg::avcodec
        FFmpeg::avformat
        FFmpeg::avutil
        FFmpeg::swscale
    )
endif()
