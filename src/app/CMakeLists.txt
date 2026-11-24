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

set_target_properties(proslides_app PROPERTIES
    OUTPUT_NAME ProSlides
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET proslides_app POST_BUILD
            COMMAND "${WINDEPLOYQT}" --no-translations --no-system-d3d-compiler
                    --compiler-runtime "$<TARGET_FILE:proslides_app>"
            COMMENT "Deploying Qt runtime..."
        )
    endif()
endif()
