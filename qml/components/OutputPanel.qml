import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel

    Rectangle { anchors.fill: parent; color: Theme.panel }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // --- output preview ----------------------------------------------
        Rectangle {
            width: parent.width
            height: 220
            radius: Theme.radius
            color: Theme.black
            border.color: Theme.border

            Item {
                anchors.fill: parent
                anchors.margins: 8

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 4
                    text: "Output Preview"
                    color: Theme.textDim
                    font: Theme.smallFont
                }

                Rectangle {
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.top: parent.top; anchors.topMargin: 28
                    anchors.bottom: parent.bottom; anchors.bottomMargin: 34
                    radius: 6
                    color: "#080810"
                    border.color: Theme.surfaceHover

                    Column {
                        anchors.centerIn: parent
                        width: parent.width - 40
                        spacing: 12
                        Text {
                            width: parent.width
                            text: (ProSlides.slides[ProSlides.currentIndex] || {}).title || ""
                            color: Theme.text
                            font.pixelSize: 20
                            font.bold: true
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                            maximumLineCount: 3
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: (ProSlides.slides[ProSlides.currentIndex] || {}).group || ""
                            color: Theme.textDim
                            font: Theme.smallFont
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    text: "Audience Display · 1920×1080"
                    color: Theme.textFaint
                    font: Theme.smallFont
                }
            }
        }

        // --- confidence monitor ------------------------------------------
        Rectangle {
            width: parent.width
            height: 150
            radius: Theme.radius
            color: Theme.panelAlt
            border.color: Theme.border

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                Text { text: "Confidence Monitor"; color: Theme.textDim; font: Theme.smallFont }
                Rectangle {
                    width: parent.width; height: 74
                    radius: 6; color: Theme.black
                    Text {
                        id: monitorText
                        anchors.centerIn: parent
                        width: parent.width - 30
                        text: (ProSlides.slides[ProSlides.currentIndex] || {}).text || "Nothing selected"
                        color: Theme.yellow
                        font.pixelSize: 15
                        font.bold: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        maximumLineCount: 3
                        elide: Text.ElideRight
                    }
                }
                Row {
                    spacing: 8
                    Text { text: "C"; color: Theme.yellow; font: Qt.font({ family: Theme.smallFont.family, pixelSize: Theme.smallFont.pixelSize, bold: true }) }
                    Text { text: (ProSlides.slides[ProSlides.currentIndex] || {}).key || "—"; color: Theme.textDim; font: Theme.smallFont }
                }
            }
        }

        // --- service controls --------------------------------------------
        Rectangle {
            width: parent.width
            height: panel.height - 220 - 150 - 48
            radius: Theme.radius
            color: Theme.panelAlt
            border.color: Theme.border

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Item {
                    width: parent.width
                    height: 22
                    Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Service Controls"; color: Theme.text; font: Qt.font({ family: Theme.bodyFont.family, pixelSize: Theme.bodyFont.pixelSize, bold: true }) }
                    Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: ProSlides.clockText; color: Theme.accent; font: Qt.font({ family: Theme.bodyFont.family, pixelSize: Theme.bodyFont.pixelSize, bold: true }) }
                }

                Rectangle { width: parent.width; height: 1; color: Theme.border }

                Row {
                    spacing: 8
                    Button {
                        id: presentBtn2; text: "Start presenting"
                        onClicked: ProSlides.startPresenting()
                        background: Rectangle { radius: Theme.radiusSmall; color: Theme.accent }
                        contentItem: Text { text: presentBtn2.text; color: Theme.black; font: Qt.font({ family: Theme.smallFont.family, pixelSize: Theme.smallFont.pixelSize, bold: true }); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                    Button {
                        id: restoreBtn; text: "Restore"
                        onClicked: ProSlides.restoreOutput()
                        background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: restoreBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                }

                Row {
                    spacing: 8
                    Button {
                        id: clearBtn2; text: "Clear all"
                        onClicked: ProSlides.clearOutput()
                        background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: clearBtn2.text; color: Theme.yellow; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                    Button {
                        id: saveBtn; text: "Save"
                        onClicked: ProSlides.saveShow()
                        background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                        contentItem: Text { text: saveBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                }

                Item { height: 6 }
                Text {
                    width: parent.width
                    text: ProSlides.remoteEnabled ? ("Remote: " + ProSlides.remoteAddress) : "Remote is off — start it from the top bar to control from your phone."
                    color: ProSlides.remoteEnabled ? Theme.accent : Theme.textFaint
                    font: Theme.smallFont
                    wrapMode: Text.WordWrap
                }

                Rectangle { width: parent.width; height: 1; color: Theme.border }

                TimerPanel {
                    width: parent.width
                    height: Math.max(90, panel.height - 560)
                }

                OverlayPanel {
                    width: parent.width
                    height: 100
                }

                AudioPanel {
                    width: parent.width
                    height: 70
                }

                StreamPanel {
                    width: parent.width
                    height: 76
                }
            }
        }
    }
}
