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
            ComboBox {
                id: projectBox
                width: 150
                height: 28
                anchors.verticalCenter: parent.verticalCenter
                model: ProSlides.projects
                textRole: "name"
                valueRole: "id"
                onCurrentValueChanged: {
                    if (currentValue) ProSlides.openProject(currentValue);
                }
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: projectBox.displayText; color: Theme.text; font: Theme.smallFont; verticalAlignment: Text.AlignVCenter }
            }
            Text { anchors.verticalCenter: parent.verticalCenter; text: Qt.formatDateTime(new Date(), "dddd, MMM d, yyyy"); color: Theme.textDim; font: Theme.smallFont }
        }
    }

    ListModel { id: serviceModel }

    function reload() {
        serviceModel.clear();
        const items = ProSlides.servicePlan().items || [];
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

    Component.onCompleted: reload()

    Connections {
        target: ProSlides
        function onStateChanged() { reload() }
        function onSlidesChanged() { if (serviceModel.count === 0) reload() }
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
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        if (ProSlides.activeProjectId) {
                            ProSlides.removeProjectItem(ProSlides.activeProjectId, index);
                            panel.reload();
                        }
                        return;
                    }
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
        Row {
            anchors.left: parent.left; anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            Button {
                id: newProjectBtn; text: "+ Project"; height: 24
                onClicked: {
                    const id = ProSlides.createProject("New Service");
                    if (id) ProSlides.openProject(id);
                }
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
                contentItem: Text { text: newProjectBtn.text; color: Theme.accent; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
            TextField {
                id: newItemField; width: 100; height: 24
                placeholderText: "Item name"
                color: Theme.text; placeholderTextColor: Theme.textFaint; font: Theme.smallFont
                background: Rectangle { radius: 4; color: Theme.surface; border.color: Theme.border }
            }
            Button {
                id: addItemBtn; text: "+"; width: 24; height: 24
                onClicked: {
                    if (!ProSlides.activeProjectId) {
                        ProSlides.openProject(ProSlides.createProject("New Service"));
                    }
                    if (newItemField.text.trim().length > 0) {
                        ProSlides.addProjectItem(ProSlides.activeProjectId, "", newItemField.text);
                        newItemField.text = "";
                        panel.reload();
                    }
                }
                background: Rectangle { radius: 4; color: Theme.accent }
                contentItem: Text { text: "+"; color: Theme.black; font: Theme.smallFont; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            }
        }
        Text {
            anchors.right: parent.right; anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: serviceModel.count + " items"
            color: Theme.textFaint
            font: Theme.smallFont
        }
    }
}
