import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

ApplicationWindow {
    id: window
    width: 1680
    height: 1000
    minimumWidth: 1180
    minimumHeight: 720
    visible: true
    color: Theme.bg
    title: "ProSlides — " + ProSlides.activeShowName

    MainView {
        anchors.fill: parent
    }
}
