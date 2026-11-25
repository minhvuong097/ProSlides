import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: 186
    z: 3

    property real pixelsPerSecond: 40
    property int totalSeconds: Math.max(20, Math.ceil((lastActionEnd() / 1000) + 5))

    function lastActionEnd() {
        let end = 0;
        const list = ProSlides.timelineActions;
        for (let i = 0; i < list.length; ++i) {
            const a = list[i];
            end = Math.max(end, a.time + a.duration * 1000);
        }
        return end;
    }

    function formatTime(ms) {
        const total = Math.floor(ms / 1000);
        const m = Math.floor(total / 60);
        const s = total % 60;
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s;
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.panelAlt
        border.color: Theme.border
        border.width: 1
    }

    // --- transport ---------------------------------------------------------
    Row {
        id: transport
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.top: parent.top
        anchors.topMargin: 6
        spacing: 8

        Text { anchors.verticalCenter: parent.verticalCenter; text: "Timeline"; color: Theme.text; font: Qt.font({ family: Theme.smallFont.family, pixelSize: Theme.smallFont.pixelSize, bold: true }) }

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
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: formatTime(ProSlides.timelinePosition)
            color: Theme.accent
            font.pixelSize: 13
            font.bold: true
        }

        Item { width: 8 }

        Repeater {
            model: [
                { label: "Slide", type: "slide", color: Theme.purple },
                { label: "Show", type: "show", color: Theme.blue },
                { label: "Audio", type: "audio", color: Theme.yellow },
                { label: "Style", type: "style", color: Theme.red }
            ]
            delegate: Button {
                id: addBtn
                text: "+" + modelData.label
                implicitHeight: 24
                onClicked: ProSlides.addTimelineAction(modelData.label, modelData.type,
                                                       ProSlides.timelinePosition, 1.0)
                background: Rectangle { radius: 4; color: Theme.surface; border.color: modelData.color }
                contentItem: Text { text: addBtn.text; color: modelData.color; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }

        Item { width: 8 }
        Button {
            id: zoomOut; text: "−"; width: 24; implicitHeight: 24
            onClicked: panel.pixelsPerSecond = Math.max(12, panel.pixelsPerSecond - 8)
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: zoomOut.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
        Button {
            id: zoomIn; text: "+"; width: 24; implicitHeight: 24
            onClicked: panel.pixelsPerSecond = Math.min(160, panel.pixelsPerSecond + 8)
            background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            contentItem: Text { text: zoomIn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        }
    }

    // --- ruler + tracks ----------------------------------------------------
    Flickable {
        id: flick
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: transport.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 8
        contentWidth: Math.max(panel.totalSeconds * panel.pixelsPerSecond, width)
        contentHeight: height
        clip: true

        Rectangle {
            id: ruler
            anchors.left: parent.left
            anchors.top: parent.top
            width: flick.contentWidth
            height: 18
            color: Theme.surface

            Repeater {
                model: panel.totalSeconds + 1
                delegate: Item {
                    x: index * panel.pixelsPerSecond
                    width: panel.pixelsPerSecond
                    height: 18
                    Rectangle { anchors.bottom: parent.bottom; width: 1; height: index % 5 === 0 ? 10 : 5; color: Theme.textFaint }
                    Text {
                        visible: index % 5 === 0
                        anchors.left: parent.left; anchors.leftMargin: 2
                        anchors.top: parent.top
                        text: index + "s"
                        color: Theme.textFaint
                        font.pixelSize: 9
                    }
                }
            }
        }

        Rectangle {
            id: track
            anchors.left: parent.left
            anchors.top: ruler.bottom
            width: flick.contentWidth
            height: 70
            color: Theme.panel

            Repeater {
                id: blocks
                model: ProSlides.timelineActions
                delegate: Rectangle {
                    id: block
                    x: modelData.time / 1000 * panel.pixelsPerSecond
                    y: 6
                    width: Math.max(48, modelData.duration * panel.pixelsPerSecond)
                    height: 34
                    radius: 5
                    color: modelData.type === "slide" ? Theme.purple
                         : (modelData.type === "audio" ? Theme.yellow
                         : (modelData.type === "style" ? Theme.red : Theme.blue))
                    opacity: 0.9

                    Text {
                        anchors.left: parent.left; anchors.leftMargin: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.name
                        color: Theme.black
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width - 16
                    }

                    MouseArea {
                        id: moveArea
                        anchors.fill: parent
                        anchors.rightMargin: 8
                        drag.target: block
                        drag.axis: Drag.XAxis
                        onClicked: ProSlides.seekTimeline(modelData.time)
                        onReleased: ProSlides.setTimelineActionTime(modelData.id,
                                                                    Math.max(0, block.x / panel.pixelsPerSecond * 1000))
                        onPressAndHold: ProSlides.removeTimelineAction(modelData.id)
                    }

                    Rectangle {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        width: 8
                        height: parent.height
                        color: Theme.black
                        opacity: 0.25
                        MouseArea {
                            anchors.fill: parent
                            property real startX: 0
                            property real startWidth: 0
                            onPressed: { startX = mouseX; startWidth = block.width; }
                            onPositionChanged: block.width = Math.max(48, startWidth + (mouseX - startX))
                            onReleased: ProSlides.setTimelineActionDuration(modelData.id,
                                                                            block.width / panel.pixelsPerSecond)
                        }
                    }
                }
            }

            // playhead
            Rectangle {
                id: playhead
                x: ProSlides.timelinePosition / 1000 * panel.pixelsPerSecond
                anchors.top: parent.top
                width: 2
                height: parent.height
                color: Theme.accent
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: 10; height: 10; radius: 5; color: Theme.accent
                }
                MouseArea {
                    anchors.left: parent.left; anchors.leftMargin: -8
                    anchors.right: parent.right; anchors.rightMargin: -8
                    anchors.top: parent.top
                    height: parent.height
                    drag.target: playhead
                    drag.axis: Drag.XAxis
                    onReleased: ProSlides.seekTimeline(Math.max(0, playhead.x / panel.pixelsPerSecond * 1000))
                }
            }
        }
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        text: "Kéo block = đổi thời điểm · kéo méo phải = đổi duration · nhấn giữ = xóa"
        color: Theme.textFaint
        font.pixelSize: 10
    }
}
