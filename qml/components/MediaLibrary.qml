import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: library
    clip: true

    Rectangle { anchors.fill: parent; color: Theme.panelAlt }

    Rectangle {
        id: header
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 44
        color: Theme.panel
        Row {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Media"; color: Theme.text; font: Qt.font({ family: Theme.bodyFont.family, pixelSize: Theme.bodyFont.pixelSize, bold: true }) }
            TextField {
                id: searchField
                width: Math.max(120, parent.width - 70)
                height: 28
                placeholderText: "Search…"
                color: Theme.text
                placeholderTextColor: Theme.textFaint
                font: Theme.smallFont
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
            }
        }
    }

    ListView {
        id: list
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: header.bottom; anchors.bottom: footer.top
        anchors.margins: 8
        model: ProSlides.mediaFiles
        clip: true
        delegate: Item {
            width: list.width; height: 40
            Rectangle {
                anchors.fill: parent; radius: Theme.radiusSmall
                color: area.containsMouse ? Theme.surface : "transparent"
            }
            MouseArea {
                id: area; anchors.fill: parent; hoverEnabled: true
                onClicked: ProSlides.addMediaSlide("", modelData.path)
            }
            Rectangle {
                anchors.left: parent.left; anchors.leftMargin: 8; anchors.verticalCenter: parent.verticalCenter
                width: 26; height: 26; radius: 5
                color: modelData.kind === "video" ? Theme.purple : Theme.blue
                Text { anchors.centerIn: parent; text: modelData.kind === "video" ? "▶" : "▣"; color: Theme.black; font.pixelSize: 12 }
            }
            Column {
                anchors.left: parent.left; anchors.leftMargin: 42
                anchors.verticalCenter: parent.verticalCenter
                Text { text: modelData.name; color: Theme.text; font: Theme.smallFont }
                Text { text: modelData.kind; color: Theme.textFaint; font.pixelSize: 10 }
            }
        }
    }

    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        height: 38; color: Theme.panel
        Text {
            anchors.left: parent.left; anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: "+  New folder"
            color: Theme.accent; font: Theme.smallFont
        }
        Text {
            anchors.right: parent.right; anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: ProSlides.mediaFiles.length + " files"
            color: Theme.textFaint; font: Theme.smallFont
        }
    }
}
