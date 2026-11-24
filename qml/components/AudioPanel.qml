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
            width: parent.width

            Button {
                id: playBtn; text: ProSlides.audioPlaying ? "❚❚" : "▶"; width: 30; height: 24
                onClicked: ProSlides.audioPlaying ? ProSlides.pauseAudio() : ProSlides.playAudio()
                background: Rectangle { radius: 4; color: Theme.accent }
                contentItem: Text { text: playBtn.text; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: nextBtn; text: "▶|"; width: 30; height: 24
                onClicked: ProSlides.nextAudio()
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: nextBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: ProSlides.audioTrack || "No track"
                color: Theme.text
                font: Theme.smallFont
                elide: Text.ElideRight
                width: parent.width - 150
            }
        }

        Row {
            spacing: 6
            width: parent.width
            Slider {
                id: volume
                width: parent.width - 70
                from: 0; to: 1; value: 0.8
                onMoved: ProSlides.setAudioVolume(value)
                background: Rectangle { height: 4; radius: 2; color: Theme.surface; anchors.verticalCenter: parent.verticalCenter }
            }
            Button {
                id: muteBtn; text: "M"; width: 26; height: 22
                onClicked: ProSlides.setAudioMuted(!muteBtn.checked)
                checkable: true
                background: Rectangle { radius: 4; color: muteBtn.checked ? Theme.red : Theme.surface; border.color: Theme.border }
                contentItem: Text { text: "M"; color: muteBtn.checked ? Theme.black : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
    }
}
