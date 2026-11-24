import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"
import "qrc:/qml/components"

Item {
    id: root
    anchors.fill: parent

    TopBar {
        id: topBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 56
        z: 2
    }

    Item {
        id: leftColumn
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 330

        ServicePanel {
            id: servicePanel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height * 0.52
        }

        Rectangle { anchors.top: servicePanel.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 1; color: Theme.border }

        MediaLibrary {
            id: mediaLibrary
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: servicePanel.bottom
            anchors.bottom: scripturePanel.top
        }

        ScripturePanel {
            id: scripturePanel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }

    Rectangle {
        id: centerSeparator
        anchors.left: leftColumn.right
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.border
    }

    TimelinePanel {
        id: timelinePanel
        anchors.left: centerSeparator.right
        anchors.right: rightColumn.left
        anchors.bottom: parent.bottom
    }

    SlideGrid {
        id: slideGrid
        anchors.left: centerSeparator.right
        anchors.right: rightColumn.left
        anchors.top: topBar.bottom
        anchors.bottom: timelinePanel.top
    }

    Rectangle {
        id: rightSeparator
        anchors.right: rightColumn.left
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.border
    }

    OutputPanel {
        id: outputPanel
        anchors.right: parent.right
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        width: 390
    }

    // Keyboard shortcuts — MxU-style fast operation
    Shortcut { sequence: "Right"; onActivated: ProSlides.nextSlide() }
    Shortcut { sequence: "Left"; onActivated: ProSlides.previousSlide() }
    Shortcut { sequence: "Space"; onActivated: ProSlides.nextSlide() }
    Shortcut { sequence: "F5"; onActivated: ProSlides.startPresenting() }
    Shortcut { sequence: "Ctrl+S"; onActivated: ProSlides.saveShow() }
    Shortcut { sequence: "Esc"; onActivated: ProSlides.stopPresenting() }
}
