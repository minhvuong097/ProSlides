import QtQuick 2.15
import QtQuick.Window 2.15
import "qrc:/qml/theme"

Window {
    id: stageWindow
    property var stageState: ({})

    visible: true
    color: "#08080d"
    flags: Qt.FramelessWindowHint | Qt.Window

    Rectangle {
        anchors.fill: parent

        Column {
            anchors.fill: parent
            anchors.margins: 40
            spacing: 20

            // --- current & next -----------------------------------------
            Row {
                width: parent.width
                height: parent.height * 0.45
                spacing: 24

                Rectangle {
                    width: parent.width * 0.62 - 12
                    height: parent.height
                    radius: 12
                    color: Theme.panel
                    border.color: Theme.border

                    Column {
                        anchors.fill: parent
                        anchors.margins: 24
                        Text { text: "NOW"; color: Theme.accent; font.pixelSize: 18; font.bold: true }
                        Text {
                            anchors.topMargin: 16
                            width: parent.width
                            text: (stageState.current || "—")
                            color: Theme.text
                            font.pixelSize: 42
                            font.bold: true
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Rectangle {
                    width: parent.width * 0.38 - 12
                    height: parent.height
                    radius: 12
                    color: Theme.panelAlt
                    border.color: Theme.border

                    Column {
                        anchors.fill: parent
                        anchors.margins: 24
                        Text { text: "NEXT"; color: Theme.purple; font.pixelSize: 18; font.bold: true }
                        Text {
                            anchors.topMargin: 16
                            width: parent.width
                            text: (stageState.next || "—")
                            color: Theme.textDim
                            font.pixelSize: 30
                            wrapMode: Text.WordWrap
                            maximumLineCount: 6
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            // --- notes --------------------------------------------------
            Rectangle {
                width: parent.width
                height: parent.height * 0.38
                radius: 12
                color: Theme.panel
                border.color: Theme.border

                Column {
                    anchors.fill: parent
                    anchors.margins: 24
                    Text { text: "NOTES"; color: Theme.yellow; font.pixelSize: 18; font.bold: true }
                    Text {
                        anchors.topMargin: 12
                        width: parent.width
                        text: (stageState.notes || "")
                        color: Theme.text
                        font.pixelSize: 26
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // --- footer -------------------------------------------------
            Row {
                width: parent.width
                height: 40
                Text { text: stageState.number || ""; color: Theme.textDim; font.pixelSize: 18 }
                Item { width: parent.width - 400 }
                Text { text: "Slide " + (stageState.number || ""); color: Theme.textFaint; font.pixelSize: 16 }
                Item { width: 20 }
                Text { text: stageState.clock || ""; color: Theme.accent; font.pixelSize: 22; font.bold: true }
            }
        }
    }
}
