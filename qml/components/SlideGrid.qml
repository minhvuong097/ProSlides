import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: grid
    anchors.fill: parent
    property bool editMode: false

    Rectangle { anchors.fill: parent; color: Theme.bg }

    // --- header ----------------------------------------------------------
    Rectangle {
        id: header
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 108
        color: Theme.panel

        Column {
            anchors.left: parent.left; anchors.leftMargin: 18
            anchors.right: parent.right; anchors.rightMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Row {
                spacing: 10
                Text { text: "Slides"; color: Theme.text; font: Theme.titleFont }
                Text { anchors.verticalCenter: parent.verticalCenter; text: ProSlides.slideCount + " slides"; color: Theme.textDim; font: Theme.smallFont }
            }

            Row {
                id: quickAdd
                spacing: 8
                width: parent.width

                TextField {
                    id: lyricInput
                    width: Math.max(360, quickAdd.width - 300)
                    placeholderText: "Paste lyrics or type a slide title…  (use [C] for chords)"
                    color: Theme.text
                    placeholderTextColor: Theme.textFaint
                    font: Theme.bodyFont
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.panelAlt; border.color: Theme.border }
                    Keys.onReturnPressed: addSlide()
                }

                TextField {
                    id: keyInput
                    width: 70
                    placeholderText: "Key"
                    color: Theme.text
                    placeholderTextColor: Theme.textFaint
                    font: Theme.bodyFont
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.panelAlt; border.color: Theme.border }
                }

                Button {
                    id: addBtn
                    text: "Add slide"
                    onClicked: addSlide()
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.accent }
                    contentItem: Text { text: addBtn.text; color: Theme.black; font: Qt.font({ family: Theme.smallFont.family, pixelSize: Theme.smallFont.pixelSize, bold: true }); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }

                Button {
                    id: downBtn; text: "K−"; implicitHeight: 34
                    onClicked: ProSlides.transposeShow(-1)
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                    contentItem: Text { text: downBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Button {
                    id: editorBtn; text: grid.editMode ? "Grid" : "Editor"; implicitHeight: 34
                    onClicked: grid.editMode = !grid.editMode
                    background: Rectangle { radius: Theme.radiusSmall; color: grid.editMode ? Theme.accent : Theme.surface; border.color: Theme.border }
                    contentItem: Text { text: editorBtn.text; color: grid.editMode ? Theme.black : Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }

                Button {
                    id: upBtn; text: "K+"; implicitHeight: 34
                    onClicked: ProSlides.transposeShow(1)
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                    contentItem: Text { text: upBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Key " + (ProSlides.detectKey() || "—")
                    color: Theme.yellow
                    font: Theme.smallFont
                }
            }
        }
    }

    function addSlide() {
        if (lyricInput.text.trim().length === 0) return;
        ProSlides.addLyricSlide("", lyricInput.text, keyInput.text);
        lyricInput.text = "";
        keyInput.text = "";
        lyricInput.forceActiveFocus();
    }

    // --- slide grid ------------------------------------------------------
    SlideCanvas {
        id: canvas
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: header.bottom; anchors.bottom: footer.top
        visible: grid.editMode
    }

    GridView {
        id: view
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: header.bottom; anchors.bottom: footer.top
        anchors.margins: 18
        cellWidth: 250
        cellHeight: 200
        model: ProSlides.slides
        clip: true
        visible: !grid.editMode

        delegate: Item {
            width: view.cellWidth - 14
            height: view.cellHeight - 14
            SlideCard {
                anchors.fill: parent
                slide: modelData
                slideIndex: index
                active: index === ProSlides.currentIndex
                onClicked: ProSlides.goToSlide(index)
            }
        }
    }

    // --- footer / playback -----------------------------------------------
    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        height: 56
        color: Theme.panelAlt

        Row {
            anchors.left: parent.left; anchors.leftMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Button {
                id: prevBtn; text: "◀"; width: 36; implicitHeight: 32
                onClicked: ProSlides.previousSlide()
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: prevBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: nextBtn; text: "▶"; width: 36; implicitHeight: 32
                onClicked: ProSlides.nextSlide()
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: nextBtn.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Slide " + (ProSlides.currentIndex + 1) + " / " + ProSlides.slideCount; color: Theme.textDim; font: Theme.smallFont }
        }

        Text {
            anchors.right: parent.right; anchors.rightMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            text: "Space → next   ·   F5 → present   ·   Ctrl+S → save"
            color: Theme.textFaint
            font: Theme.smallFont
        }
    }
}
