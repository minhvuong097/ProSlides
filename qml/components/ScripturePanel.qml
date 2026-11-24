import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: 210

    Rectangle { anchors.fill: parent; color: Theme.panel }

    Rectangle {
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 34; color: Theme.panelAlt
        Row {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Scripture"; color: Theme.text; font: Theme.smallFont; font.bold: true }
            ComboBox {
                id: bibleBox
                width: 130; height: 26
                model: ProSlides.bibleNames
                onCurrentTextChanged: ProSlides.setActiveBible(currentText)
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: bibleBox.displayText; color: Theme.text; font: Theme.smallFont; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: prevVerse; text: "◀"; width: 26; height: 24
                onClicked: ProSlides.scripturePrevious()
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: prevVerse.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            Button {
                id: nextVerse; text: "▶"; width: 26; height: 24
                onClicked: ProSlides.scriptureNext()
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: nextVerse.text; color: Theme.text; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
    }

    TextField {
        id: searchField
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: parent.top; anchors.topMargin: 40
        anchors.leftMargin: 10; anchors.rightMargin: 10
        height: 28
        placeholderText: "Search or type reference (e.g. John 3:16)"
        color: Theme.text
        placeholderTextColor: Theme.textFaint
        font: Theme.smallFont
        onAccepted: doSearch()
        background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
    }

    ListModel { id: resultsModel }
    function doSearch() {
        resultsModel.clear();
        const results = ProSlides.scriptureSearch(searchField.text);
        for (let i = 0; i < results.length; ++i) resultsModel.append(results[i]);
    }
    Component.onCompleted: doSearch()

    ListView {
        id: results
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: searchField.bottom; anchors.topMargin: 6
        anchors.bottom: parent.bottom; anchors.bottomMargin: 6
        anchors.leftMargin: 10; anchors.rightMargin: 10
        model: resultsModel
        clip: true
        delegate: Item {
            width: results.width; height: 38
            Rectangle {
                anchors.fill: parent; radius: 4
                color: area.containsMouse ? Theme.surface : "transparent"
            }
            MouseArea {
                id: area; anchors.fill: parent; hoverEnabled: true
                onClicked: ProSlides.openScripture(model.reference)
            }
            Column {
                anchors.left: parent.left; anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                Text { text: model.reference; color: Theme.yellow; font: Theme.smallFont; font.bold: true }
                Text { text: model.text; color: Theme.textDim; font.pixelSize: 10; elide: Text.ElideRight; width: results.width - 20 }
            }
        }
    }
}
