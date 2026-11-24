pragma Singleton
import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

QtObject {
    property Component button: Component {
        Button {
            id: btn
            implicitHeight: 34
            padding: 14
            font: Theme.bodyFont
            background: Rectangle {
                radius: Theme.radiusSmall
                color: btn.hovered ? Theme.surfaceHover : Theme.surface
                border.color: Theme.border
                border.width: 1
            }
            contentItem: Text {
                text: btn.text
                color: Theme.text
                font: btn.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    property Component accentButton: Component {
        Button {
            id: btn
            implicitHeight: 34
            padding: 16
            font: Theme.bodyFont
            background: Rectangle {
                radius: Theme.radiusSmall
                color: btn.hovered ? Theme.accentDim : Theme.accent
            }
            contentItem: Text {
                text: btn.text
                color: Theme.black
                font.family: btn.font.family
                font.pixelSize: btn.font.pixelSize
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    property Component textField: Component {
        TextField {
            id: field
            implicitHeight: 34
            font: Theme.bodyFont
            color: Theme.text
            placeholderTextColor: Theme.textFaint
            background: Rectangle {
                radius: Theme.radiusSmall
                color: Theme.panelAlt
                border.color: field.activeFocus ? Theme.accent : Theme.border
                border.width: 1
            }
        }
    }
}
