import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel

    Row {
        id: title
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        spacing: 8
        Text { anchors.verticalCenter: parent.verticalCenter; text: "Overlays / Scenes"; color: Theme.textDim; font: Theme.smallFont }
        Item { width: parent.width - 250 }
        Button {
            id: clearBtn; text: "Clear"; height: 24
            onClicked: { ProSlides.clearOverlays(); ProSlides.clearScene(); }
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: clearBtn.text; color: Theme.yellow; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
    }

    ListView {
        id: list
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: title.bottom; anchors.topMargin: 6
        anchors.bottom: parent.bottom
        model: ProSlides.overlays
        orientation: ListView.Horizontal
        spacing: 6
        clip: true
        delegate: Item {
            width: 96; height: 30
            Rectangle {
                anchors.fill: parent; radius: 6
                color: model.active ? model.color : Theme.surface
                border.color: model.active ? Theme.accent : Theme.border
            }
            MouseArea { anchors.fill: parent; onClicked: ProSlides.toggleOverlay(model.id) }
            Text {
                anchors.centerIn: parent
                text: model.name
                color: model.active ? Theme.black : Theme.text
                font: Theme.smallFont
                elide: Text.ElideRight
                width: parent.width - 12
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    ListView {
        id: sceneList
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: list.bottom; anchors.topMargin: 4
        anchors.bottom: parent.bottom
        model: ProSlides.scenes
        orientation: ListView.Horizontal
        spacing: 6
        clip: true
        delegate: Item {
            width: 96; height: 26
            Rectangle { anchors.fill: parent; radius: 5; color: Theme.panelAlt; border.color: Theme.border }
            MouseArea { anchors.fill: parent; onClicked: ProSlides.startScene(model.id) }
            Text {
                anchors.centerIn: parent
                text: "Scene: " + model.name
                color: Theme.purple
                font: Theme.smallFont
                elide: Text.ElideRight
                width: parent.width - 10
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
