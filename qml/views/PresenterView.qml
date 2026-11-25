import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

// Audience display window. Properties are written from OutputManager via
// QQmlProperty; slide content is read from the shared controller context.
Window {
    id: outputWindow
    property var currentSlide: ({})
    property bool cleared: false
    property string backgroundPath: ""

    readonly property real scaleX: width / 1920
    readonly property real scaleY: height / 1080

    visible: true
    color: Theme.black
    flags: Qt.FramelessWindowHint | Qt.Window

    Rectangle {
        anchors.fill: parent
        color: Theme.black

        Image {
            anchors.fill: parent
            source: outputWindow.backgroundPath ? "file://" + outputWindow.backgroundPath : ""
            fillMode: Image.PreserveAspectCrop
            visible: source.toString().length > 0 && !outputWindow.cleared
            opacity: 0.35
        }

        Repeater {
            model: outputWindow.cleared ? [] : ProSlides.slideItems
            delegate: Item {
                property int itemFontSize: modelData.fontSize
                x: modelData.x * outputWindow.scaleX
                y: modelData.y * outputWindow.scaleY
                width: modelData.width * outputWindow.scaleX
                height: modelData.height * outputWindow.scaleY

                Text {
                    anchors.fill: parent
                    text: modelData.text
                    color: modelData.color
                    font.pixelSize: modelData.fontSize * outputWindow.scaleY
                    font.bold: true
                    wrapMode: Text.WordWrap
                    horizontalAlignment: modelData.align === "left" ? Text.AlignLeft
                                        : (modelData.align === "right" ? Text.AlignRight : Text.AlignHCenter)
                    verticalAlignment: Text.AlignVCenter
                }

                // Chord overlay — positioned approximately by character offset.
                Repeater {
                    model: modelData.chords || []
                    delegate: Text {
                        x: (modelData.pos || 0) * (itemFontSize * 0.55) * outputWindow.scaleX
                        y: -24 * outputWindow.scaleY
                        text: modelData.key
                        color: Theme.yellow
                        font.pixelSize: Math.max(12, 22 * outputWindow.scaleY)
                        font.bold: true
                    }
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
