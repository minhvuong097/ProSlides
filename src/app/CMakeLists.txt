add_executable(proslides_app
    main.cpp
    ${PS_RESOURCES}
)

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
