import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: 156
    z: 3

    Rectangle {
        anchors.fill: parent
        color: Theme.panelAlt
        border.color: Theme.border
        border.width: 1
    }

    // --- transport -------------------------------------------------------
    Row {
        id: transport
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.top: parent.top
        anchors.topMargin: 8
        spacing: 8

        Text { anchors.verticalCenter: parent.verticalCenter; text: "Timeline"; color: Theme.text; font: Theme.smallFont; font.bold: true }

        Button {
            id: playBtn; text: ProSlides.timelinePlaying ? "❚❚" : "▶"; width: 30; implicitHeight: 24
            onClicked: ProSlides.timelinePlaying ? ProSlides.pauseTimeline() : ProSlides.startTimeline()
            background: Rectangle { radius: 4; color: Theme.accent }
            contentItem: Text { text: playBtn.text; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
        Button {
            id: stopBtn; text: "■"; width: 30; implicitHeight: 24
            onClicked: ProSlides.stopTimeline()
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: stopBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
        Slider {
            id: playhead
            width: 260
            from: 0
            to: Math.max(1000, timelineModel.count > 0 ? (timelineModel.count + 1) * 1000 : 1000)
            value: ProSlides.timelinePosition
            onMoved: ProSlides.seekTimeline(value)
            background: Rectangle { height: 4; radius: 2; color: Theme.surface; anchors.verticalCenter: parent.verticalCenter }
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: (ProSlides.timelinePosition / 1000).toFixed(1) + "s"
            color: Theme.textDim
            font: Theme.smallFont
        }
        Button {
            id: addBtn; text: "+ Action"; implicitHeight: 24
            onClicked: {
                const t = (ProSlides.timelinePosition / 1000).toFixed(1);
                ProSlides.addTimelineAction("Slide " + (ProSlides.currentIndex + 1), "slide", ProSlides.timelinePosition, 1.0);
            }
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: addBtn.text; color: Theme.accent; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
    }

    ListModel { id: timelineModel }

    Connections {
        target: ProSlides
        onTimelineActionsChanged: {
            timelineModel.clear();
            const actions = ProSlides.timelineActions;
            for (let i = 0; i < actions.length; ++i) timelineModel.append(actions[i]);
        }
    }
    Component.onCompleted: {
        const actions = ProSlides.timelineActions;
        for (let i = 0; i < actions.length; ++i) timelineModel.append(actions[i]);
    }

    ListView {
        id: list
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: transport.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 10
        orientation: ListView.Horizontal
        model: timelineModel
        spacing: 6
        clip: true
        delegate: Item {
            width: 150
            height: 62
            Rectangle {
                anchors.fill: parent
                radius: 6
                color: model.type === "slide" ? Theme.purple : Theme.blue
                opacity: 0.85
            }
            Column {
                anchors.fill: parent
                anchors.margins: 8
                Text { text: model.name; color: Theme.black; font.bold: true; font: Theme.smallFont; elide: Text.ElideRight; width: parent.width }
                Text { text: (model.time / 1000).toFixed(1) + "s  ·  " + model.duration + "s"; color: Theme.black; font: Theme.smallFont }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: ProSlides.seekTimeline(model.time)
                onDoubleClicked: ProSlides.removeTimelineAction(model.id)
            }
        }
    }
}
