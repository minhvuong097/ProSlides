import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Popup {
    id: searchPopup
    width: 620
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

    onAboutToShow: {
        searchField.text = "";
        searchField.forceActiveFocus();
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        TextField {
            id: searchField
            width: parent.width
            height: 40
            placeholderText: "Tìm slide, kinh thánh, ghi chú…  (FTS5)"
            color: Theme.text
            placeholderTextColor: Theme.textFaint
            font: Theme.bodyFont
            onTextChanged: {
                if (text.trim().length > 0) ProSlides.search(text);
            }
            Keys.onEscapePressed: searchPopup.close()
            background: Rectangle {
                radius: Theme.radiusSmall
                color: Theme.panelAlt
                border.color: searchField.activeFocus ? Theme.accent : Theme.border
            }
        }

        Text {
            text: "Kết quả · " + ProSlides.searchResults.length
            color: Theme.textDim
            font: Theme.smallFont
        }

        ListView {
            id: resultsView
            width: parent.width
            height: parent.height - 90
            model: ProSlides.searchResults
            clip: true
            delegate: Item {
                width: resultsView.width
                height: 62
                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusSmall
                    color: area.containsMouse ? Theme.surface : Theme.panelAlt
                }
                MouseArea {
                    id: area
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (ProSlides.openSearchResult(index)) searchPopup.close();
                    }
                }
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3
                    Row {
                        spacing: 8
                        Text { text: modelData.group || modelData.showName; color: Theme.accent; font: Theme.smallFont }
                        Text { text: "·  " + modelData.showName; color: Theme.textFaint; font: Theme.smallFont }
                    }
                    Text {
                        width: parent.width
                        text: modelData.snippet || modelData.title
                        color: Theme.text
                        font: Theme.smallFont
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }
                }
            }
        }
    }
}
