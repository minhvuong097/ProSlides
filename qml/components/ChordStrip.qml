import QtQuick 2.15
import "qrc:/qml/theme"

Row {
    id: strip
    property string key: ""
    spacing: 4

    Repeater {
        model: strip.key.split(/\s+/).filter(function(x) { return x.length > 0; })
        delegate: Rectangle {
            width: 26 + modelData.length * 5
            height: 20
            radius: 10
            color: Theme.surfaceHover
            Text { anchors.centerIn: parent; text: modelData; color: Theme.yellow; font: Qt.font({ family: Theme.smallFont.family, pixelSize: Theme.smallFont.pixelSize, bold: true }) }
        }
    }
}
