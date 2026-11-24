import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: card
    property var slide: ({})
    property int slideIndex: 0
    property bool active: false
    signal clicked(int index)

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius
        color: card.active ? Theme.panelAlt : Theme.surface
        border.color: card.active ? Theme.accent : Theme.border
        border.width: card.active ? 2 : 1
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Row {
            width: parent.width
            spacing: 6
            Rectangle {
                width: 22; height: 20; radius: 5
                color: card.active ? Theme.accent : Theme.surfaceHover
                Text { anchors.centerIn: parent; text: card.slideIndex + 1; color: card.active ? Theme.black : Theme.textDim; font: Theme.smallFont }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: card.slide.group || "Untitled"
                color: Theme.textDim
                font: Theme.smallFont
                elide: Text.ElideRight
                width: parent.width - 30
            }
        }

        Item { width: 1; height: 4 }

        Text {
            width: parent.width
            text: card.slide.title || "(empty slide)"
            color: card.slide.title ? Theme.text : Theme.textFaint
            font: Theme.slideFont
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 4
            elide: Text.ElideRight
        }

        Item { width: 1; height: 4 }

        ChordStrip {
            visible: !!card.slide.key
            key: card.slide.key || ""
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: card.clicked(card.slideIndex)
    }
}
