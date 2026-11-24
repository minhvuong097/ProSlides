pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bg:            "#0d0d0f"
    readonly property color panel:         "#151518"
    readonly property color panelAlt:      "#1b1b20"
    readonly property color surface:       "#232329"
    readonly property color surfaceHover:  "#2c2c34"
    readonly property color border:        "#2e2e35"
    readonly property color text:          "#ececf1"
    readonly property color textDim:       "#9a9aa6"
    readonly property color textFaint:     "#62626d"
    readonly property color accent:        "#3ddc84"
    readonly property color accentDim:     "#2aa362"
    readonly property color blue:          "#4aa8ff"
    readonly property color purple:        "#b083ff"
    readonly property color red:           "#ff6b6b"
    readonly property color yellow:        "#ffcc66"
    readonly property color black:         "#000000"

    readonly property int radius: 10
    readonly property int radiusSmall: 6
    readonly property int spacing: 12

    readonly property font bodyFont: Qt.font({ family: "Segoe UI", pixelSize: 14 })
    readonly property font smallFont: Qt.font({ family: "Segoe UI", pixelSize: 12 })
    readonly property font titleFont: Qt.font({ family: "Segoe UI", pixelSize: 22, weight: Font.DemiBold })
    readonly property font slideFont: Qt.font({ family: "Segoe UI", pixelSize: 18, weight: Font.DemiBold })
}
