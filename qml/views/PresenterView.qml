import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

// Audience display window. Properties are written from OutputManager via
// QQmlProperty, so this view stays a pure, dumb renderer.
Window {
    id: outputWindow
    property var currentSlide: ({})
    property bool cleared: false
    property string backgroundPath: ""

    visible: true
    color: Theme.black
    flags: Qt.FramelessWindowHint | Qt.Window

    Rectangle {
        anchors.fill: parent
        color: Theme.black

        Image {
            id: background
            anchors.fill: parent
            source: outputWindow.backgroundPath ? "file://" + outputWindow.backgroundPath : ""
            fillMode: Image.PreserveAspectCrop
            visible: source.toString().length > 0 && !outputWindow.cleared
            opacity: 0.35
        }

        Column {
            anchors.centerIn: parent
            width: parent.width * 0.86
            spacing: 24
            visible: !outputWindow.cleared

            Repeater {
                model: {
                    const slide = outputWindow.currentSlide || {};
                    const items = slide.items || [];
                    let lines = [];
                    items.forEach(function(item) {
                        (item.lines || []).forEach(function(line) {
                            let text = "";
                            (line.text || []).forEach(function(part) { text += part.value || ""; });
                            if (text.length) lines.push(text);
                        });
                    });
                    return lines;
                }
                delegate: Text {
                    width: parent.width
                    text: modelData
                    color: Theme.text
                    font.pixelSize: 72
                    font.bold: true
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Text {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 24
            text: (outputWindow.currentSlide || {}).group || ""
            color: Theme.textFaint
            font.pixelSize: 16
            visible: !outputWindow.cleared
        }
    }
}
