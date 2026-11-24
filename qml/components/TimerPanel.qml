import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel

    Row {
        id: addRow
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        spacing: 6

        TextField {
            id: nameField
            width: 110; height: 28
            placeholderText: "Timer name"
            color: Theme.text; placeholderTextColor: Theme.textFaint
            font: Theme.smallFont
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
        }
        TextField {
            id: secondsField
            width: 64; height: 28
            text: "300"
            color: Theme.text; font: Theme.smallFont
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
        }
        Button {
            id: addTimer; text: "+"; width: 28; height: 28
            onClicked: {
                ProSlides.startTimer(nameField.text || "Timer", Number(secondsField.text) || 300, 30);
                nameField.text = "";
            }
            background: Rectangle { radius: 4; color: Theme.accent }
            contentItem: Text { text: "+"; color: Theme.black; font.bold: true; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
    }

    ListView {
        id: list
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: addRow.bottom; anchors.topMargin: 8
        anchors.bottom: parent.bottom
        model: ProSlides.timers
        clip: true
        spacing: 6
        delegate: Item {
            width: list.width; height: 34
            Rectangle {
                anchors.fill: parent; radius: 5
                color: model.warning ? "#3a2020" : Theme.surface
                border.color: model.warning ? model.warnColor : Theme.border
            }
            MouseArea { anchors.fill: parent; onClicked: ProSlides.toggleTimer(model.id) }
            Text {
                anchors.left: parent.left; anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: model.name
                color: model.warning ? model.warnColor : Theme.text
                font: Theme.smallFont
            }
            Text {
                anchors.right: parent.right; anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: model.text + (model.running ? "  ▶" : "  ❚❚")
                color: model.warning ? model.warnColor : Theme.accent
                font: Theme.bodyFont; font.bold: true
            }
        }
    }
}
