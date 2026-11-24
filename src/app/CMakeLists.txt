set(PS_APP_SOURCES main.cpp ${PS_RESOURCES})
if(WIN32)
    list(APPEND PS_APP_SOURCES ${CMAKE_SOURCE_DIR}/appicon.rc)
endif()

add_executable(proslides_app ${PS_APP_SOURCES})

target_link_libraries(proslides_app PRIVATE
    ps_presentation
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
)

set_target_properties(proslides_app PROPERTIES OUTPUT_NAME ProSlides)
