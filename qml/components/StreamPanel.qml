import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: Theme.panelAlt
        border.color: Theme.border
    }

    Column {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        Row {
            spacing: 6
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Stream"; color: Theme.textDim; font: Theme.smallFont }
            Item { width: 8 }
            Rectangle {
                width: 8; height: 8; radius: 4
                color: ProSlides.streaming ? Theme.red : Theme.textFaint
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: ProSlides.streaming ? "LIVE" : "off"
                color: ProSlides.streaming ? Theme.red : Theme.textFaint
                font: Theme.smallFont
            }
            Item { width: 80 }
            Rectangle {
                width: 8; height: 8; radius: 4
                color: ProSlides.ndiActive ? Theme.accent : Theme.textFaint
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "NDI"
                color: ProSlides.ndiActive ? Theme.accent : Theme.textFaint
                font: Theme.smallFont
            }
        }

        Row {
            spacing: 6
            TextField {
                id: urlField; width: 260; height: 26
                placeholderText: "rtmp://localhost/live"
                color: Theme.text; placeholderTextColor: Theme.textFaint; font: Theme.smallFont
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            }
            Button {
                id: streamBtn; text: ProSlides.streaming ? "Stop" : "Start"; height: 26
                onClicked: ProSlides.streaming ? ProSlides.stopStreaming() : ProSlides.startStreaming(urlField.text, false)
                background: Rectangle { radius: 4; color: ProSlides.streaming ? Theme.red : Theme.accent }
                contentItem: Text { text: streamBtn.text; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: ndiBtn; text: ProSlides.ndiActive ? "NDI off" : "NDI on"; height: 26
                onClicked: ProSlides.ndiActive ? ProSlides.stopNdi() : ProSlides.startNdi("ProSlides")
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: ndiBtn.text; color: ProSlides.ndiActive ? Theme.accent : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
    }
}
