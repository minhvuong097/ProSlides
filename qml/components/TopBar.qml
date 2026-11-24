import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: bar
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: Theme.panel
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacing
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacing

        Rectangle {
            width: 26; height: 26; radius: 7
            color: Theme.accent
            Text { anchors.centerIn: parent; text: "P"; color: Theme.black; font.bold: true; font.pixelSize: 15 }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            Text { text: "ProSlides"; color: Theme.text; font: Theme.bodyFont; font.bold: true }
            Text {
                text: ProSlides.activeShowName + (ProSlides.modified ? "  •" : "")
                color: Theme.textDim; font: Theme.smallFont
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacing
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Button {
            id: remoteBtn
            text: ProSlides.remoteEnabled ? "Remote ●" : "Remote"
            implicitHeight: 32
            onClicked: ProSlides.remoteEnabled ? ProSlides.stopRemote() : ProSlides.startRemote("")
            background: Rectangle {
                radius: Theme.radiusSmall
                color: ProSlides.remoteEnabled ? Theme.accentDim : Theme.surface
                border.color: Theme.border
            }
            contentItem: Text { text: remoteBtn.text; color: ProSlides.remoteEnabled ? Theme.black : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            ToolTip.visible: hovered
            ToolTip.text: ProSlides.remoteAddress || "Start phone remote on port 5510"
        }

        Button {
            id: clearBtn
            text: "Clear"
            implicitHeight: 32
            onClicked: ProSlides.clearOutput()
            background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: clearBtn.text; color: Theme.yellow; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }

        Button {
            id: presentBtn
            text: "Present  F5"
            implicitHeight: 32
            onClicked: ProSlides.startPresenting()
            background: Rectangle { radius: Theme.radiusSmall; color: Theme.accent }
            contentItem: Text { text: presentBtn.text; color: Theme.black; font: Theme.smallFont; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
    }
}
