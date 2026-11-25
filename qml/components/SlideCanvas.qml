import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: editor

    property int selectedIndex: -1
    property real scaleFactor: stage.width / 1920

    Rectangle {
        id: stage
        width: Math.min(parent.width - 40, (parent.height - 130) * 16 / 9)
        height: width * 9 / 16
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 12
        radius: 6
        color: "#0b0b12"
        border.color: Theme.border

        Repeater {
            model: ProSlides.slideItems
            delegate: Rectangle {
                id: itemBox
                property bool selected: editor.selectedIndex === modelData.index
                x: modelData.x * editor.scaleFactor
                y: modelData.y * editor.scaleFactor
                width: modelData.width * editor.scaleFactor
                height: modelData.height * editor.scaleFactor
                color: selected ? Qt.rgba(0.24, 0.86, 0.52, 0.08) : "transparent"
                border.color: selected ? Theme.accent : (mouse.containsMouse ? Theme.blue : "transparent")
                border.width: selected ? 2 : 1

                Repeater {
                    model: modelData.chords || []
                    delegate: Text {
                        x: (modelData.pos || 0) * (itemBox.height / 8)
                        y: 2
                        text: modelData.key
                        color: Theme.yellow
                        font.pixelSize: 11
                        font.bold: true
                    }
                }

                Text {
                    anchors.fill: parent
                    text: modelData.text
                    color: modelData.color
                    font.pixelSize: modelData.fontSize * editor.scaleFactor
                    font.bold: true
                    horizontalAlignment: modelData.align === "left" ? Text.AlignLeft
                                        : (modelData.align === "right" ? Text.AlignRight : Text.AlignHCenter)
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    drag.target: itemBox
                    drag.axis: Drag.XAndYAxis
                    onClicked: editor.selectedIndex = modelData.index
                    onReleased: {
                        ProSlides.updateSlideItem(modelData.index, {
                            x: itemBox.x / editor.scaleFactor,
                            y: itemBox.y / editor.scaleFactor
                        });
                    }
                }
            }
        }
    }

    // --- toolbar -----------------------------------------------------------
    Rectangle {
        id: toolbar
        anchors.left: stage.left
        anchors.right: stage.right
        anchors.top: stage.bottom
        anchors.topMargin: 10
        height: 56
        radius: Theme.radiusSmall
        color: Theme.panelAlt
        border.color: Theme.border

        Row {
            anchors.centerIn: parent
            spacing: 12

            Text { anchors.verticalCenter: parent.verticalCenter; text: "Item " + (editor.selectedIndex + 1); color: Theme.textDim; font: Theme.smallFont }

            Button {
                id: smaller; text: "A−"; width: 34; implicitHeight: 28
                enabled: editor.selectedIndex >= 0
                onClicked: {
                    const item = ProSlides.slideItems[editor.selectedIndex];
                    if (item) ProSlides.updateSlideItem(editor.selectedIndex, { fontSize: Math.max(12, item.fontSize - 4) });
                }
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: smaller.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: bigger; text: "A+"; width: 34; implicitHeight: 28
                enabled: editor.selectedIndex >= 0
                onClicked: {
                    const item = ProSlides.slideItems[editor.selectedIndex];
                    if (item) ProSlides.updateSlideItem(editor.selectedIndex, { fontSize: Math.min(160, item.fontSize + 4) });
                }
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: bigger.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }

            Repeater {
                model: [Theme.text, Theme.accent, Theme.yellow, Theme.blue, Theme.purple]
                delegate: Rectangle {
                    width: 20; height: 20; radius: 10; color: modelData
                    border.color: editor.selectedIndex >= 0 && ProSlides.slideItems[editor.selectedIndex]
                                  && ProSlides.slideItems[editor.selectedIndex].color === modelData ? Theme.accent : "transparent"
                    border.width: 2
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (editor.selectedIndex >= 0) ProSlides.updateSlideItem(editor.selectedIndex, { color: String(modelData) });
                        }
                    }
                }
            }

            Button {
                id: alignBtn; text: "≡"; width: 34; implicitHeight: 28
                enabled: editor.selectedIndex >= 0
                onClicked: {
                    const item = ProSlides.slideItems[editor.selectedIndex];
                    if (!item) return;
                    const next = item.align === "left" ? "center" : (item.align === "center" ? "right" : "left");
                    ProSlides.updateSlideItem(editor.selectedIndex, { align: next });
                }
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: alignBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        text: "Kéo item để di chuyển · chọn màu / cỡ chữ / căn chỉnh ở thanh công cụ"
        color: Theme.textFaint
        font: Theme.smallFont
    }
}
