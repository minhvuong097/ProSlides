add_library(ps_media STATIC
    VideoEngine.cpp
    MultimediaVideoEngine.cpp
    FFmpegVideoEngine.cpp
    MediaLibrary.cpp
    Thumbnailer.cpp
)

target_include_directories(ps_media PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_media PUBLIC
    ps_core
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
)

if(PS_ENABLE_FFMPEG)
    target_link_libraries(ps_media PUBLIC
        FFmpeg::avcodec
        FFmpeg::avformat
        FFmpeg::avutil
        FFmpeg::swscale
    )
    if(WIN32)
        target_link_libraries(ps_media PUBLIC d3d11 dxgi)
    endif()
endif()
