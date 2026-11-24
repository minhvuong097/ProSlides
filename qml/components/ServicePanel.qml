import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/qml/theme"

Item {
    id: panel
    anchors.fill: parent
    clip: true

    Rectangle { anchors.fill: parent; color: Theme.panel }

    Rectangle {
        id: header
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 46
        color: Theme.panelAlt
        Row {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 12
            spacing: 10
            Text { anchors.verticalCenter: parent.verticalCenter; text: "Service Flow"; color: Theme.text; font: Qt.font({ family: Theme.bodyFont.family, pixelSize: Theme.bodyFont.pixelSize, bold: true }) }
            Item { width: 1 }
            Text { anchors.verticalCenter: parent.verticalCenter; text: Qt.formatDateTime(new Date(), "dddd, MMM d, yyyy"); color: Theme.textDim; font: Theme.smallFont }
        }
    }

    ListModel { id: serviceModel }

    Component.onCompleted: {
        const plan = ProSlides.servicePlan();
        const items = plan.items || [];
        if (items.length === 0) {
            for (let i = 0; i < ProSlides.slideCount; ++i) {
                const slide = ProSlides.slides[i];
                serviceModel.append({ name: slide.group || ("Slide " + (i + 1)), section: "WORSHIP SET", color: Theme.purple });
            }
        } else {
            items.forEach(function(item) {
                serviceModel.append({ name: item.name, section: "SERVICE", color: Theme.accent });
            });
        }
    }

    Connections {
        target: ProSlides
        function onSlidesChanged() {
            if (serviceModel.count > 0) return;
            for (let i = 0; i < ProSlides.slideCount; ++i) {
                const slide = ProSlides.slides[i];
                serviceModel.append({ name: slide.group || ("Slide " + (i + 1)), section: "SLIDES", color: Theme.purple });
            }
        }
    }

    ListView {
        id: list
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.margins: 8
        model: serviceModel
        clip: true
        delegate: Item {
            width: list.width
            height: 34
            Rectangle {
                anchors.fill: parent
                radius: Theme.radiusSmall
                color: mouse.containsMouse ? Theme.surface : "transparent"
            }
            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    for (let i = 0; i < ProSlides.slides.length; ++i) {
                        if (ProSlides.slides[i].group === model.name) {
                            ProSlides.goToSlide(i);
                            break;
                        }
                    }
                }
            }
            Rectangle { anchors.left: parent.left; anchors.leftMargin: 10; anchors.verticalCenter: parent.verticalCenter; width: 3; height: 16; radius: 2; color: model.color }
            Text {
                anchors.left: parent.left; anchors.leftMargin: 22
                anchors.right: parent.right; anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: model.name
                color: Theme.text
                font: Theme.bodyFont
                elide: Text.ElideRight
            }
        }
    }

    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        height: 40
        color: Theme.panelAlt
        Text {
            anchors.left: parent.left; anchors.leftMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            text: "+  Add item"
            color: Theme.accent
            font: Theme.smallFont
        }
        Text {
            anchors.right: parent.right; anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            text: serviceModel.count + " items"
            color: Theme.textFaint
            font: Theme.smallFont
        }
    }
}
