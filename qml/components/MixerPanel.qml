import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Popup {
    id: mixerPopup
    width: 520
    height: 420
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: Theme.panel
        radius: Theme.radius
        border.color: Theme.border
    }

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Row {
            width: parent.width
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Audio Mixer"; color: Theme.text; font: Qt.font({ family: Theme.bodyFont.family, pixelSize: Theme.bodyFont.pixelSize, bold: true }) }
            Item { width: parent.width - 200 }
            Button {
                id: closeBtn; text: "✕"; width: 28; height: 26
                onClicked: mixerPopup.close()
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: closeBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }

        Rectangle { width: parent.width; height: 1; color: Theme.border }

        ListView {
            id: channelsView
            width: parent.width
            height: parent.height - 140
            model: ProSlides.mixerChannels
            clip: true
            delegate: Item {
                width: channelsView.width
                height: 56

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: Theme.panelAlt
                    border.color: Theme.border
                }

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Button {
                        id: playBtn; text: modelData.playing ? "❚❚" : "▶"; width: 28; height: 26
                        onClicked: modelData.playing ? ProSlides.stopMixerChannel(modelData.id) : ProSlides.playMixerChannel(modelData.id)
                        background: Rectangle { radius: 4; color: Theme.accent }
                        contentItem: Text { text: playBtn.text; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                    Column {
                        width: 130
                        Text { text: modelData.name; color: Theme.text; font: Theme.smallFont; elide: Text.ElideRight; width: 130 }
                        Slider {
                            width: 120; height: 18
                            from: 0; to: 1; value: modelData.volume
                            onMoved: ProSlides.setMixerVolume(modelData.id, value)
                            background: Rectangle { height: 3; radius: 2; color: Theme.surface; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }
                    Button {
                        id: muteBtn; text: "M"; width: 26; height: 26
                        onClicked: ProSlides.toggleMixerMute(modelData.id)
                        background: Rectangle { radius: 4; color: modelData.muted ? Theme.red : Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: muteBtn.text; color: modelData.muted ? Theme.black : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                    Button {
                        id: soloBtn; text: "S"; width: 26; height: 26
                        onClicked: ProSlides.toggleMixerSolo(modelData.id)
                        background: Rectangle { radius: 4; color: modelData.solo ? Theme.yellow : Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: soloBtn.text; color: modelData.solo ? Theme.black : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                    Button {
                        id: removeBtn; text: "✕"; width: 26; height: 26
                        onClicked: ProSlides.removeMixerChannel(modelData.id)
                        background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: removeBtn.text; color: Theme.red; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: 6
            TextField {
                id: channelName; width: 130; height: 28
                placeholderText: "Tên kênh"
                color: Theme.text; placeholderTextColor: Theme.textFaint; font: Theme.smallFont
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            }
            TextField {
                id: channelPath; width: 240; height: 28
                placeholderText: "Đường dẫn file audio…"
                color: Theme.text; placeholderTextColor: Theme.textFaint; font: Theme.smallFont
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            }
            Button {
                id: addBtn; text: "+"; width: 28; height: 28
                onClicked: {
                    if (channelPath.text.trim().length > 0) {
                        ProSlides.addMixerChannel(channelName.text, channelPath.text);
                        channelName.text = ""; channelPath.text = "";
                    }
                }
                background: Rectangle { radius: 4; color: Theme.accent }
                contentItem: Text { text: addBtn.text; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
    }
}
