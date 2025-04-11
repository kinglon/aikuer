import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Window {
    id: popup
    flags: Qt.Popup | Qt.FramelessWindowHint
    color: "transparent"
    width: comboBox.width
    height: Math.min(7, listView.count) * contentItemHeight
    visible: false

    property int contentItemHeight: 41

    property var comboBox: null

    Rectangle {
        anchors.fill: parent
        color: "#1A1A1F"
        border.color: "#24F5F5F7"
        border.width: 1
        radius: 12

        ListView {
            id: listView
            width: parent.width - 12
            height: parent.height - 12
            anchors.centerIn: parent
            model: comboBox.model

            delegate: Rectangle {
                id: listItem
                width: listView.width
                height: popup.contentItemHeight
                color: (mouseArea.containsMouse||isCurrent)? "#27272E" : "transparent"
                radius: 8
                border.width: (mouseArea.containsMouse||isCurrent)? 1 : 0
                border.color: color

                Image {
                    id: flagImageId
                    width: 16
                    height: 12
                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.verticalCenter: parent.verticalCenter
                    source: flagImagePath
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    anchors.left: flagImageId.right
                    anchors.leftMargin: 3
                    anchors.verticalCenter: parent.verticalCenter
                    text: language
                    color: (mouseArea.containsMouse||isCurrent)?"#F5F5F7":"#8CEBEBED"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        for (var i=0; i<listView.model.count; i++) {
                            var item = comboBox.model.get(i)
                            if (item.tlId !== tlId) {
                                listView.model.setProperty(i, "isCurrent", false)
                            } else
                            {
                                listView.model.setProperty(i, "isCurrent", true)
                                selectedText.text = item.language
                                comboBox.selLanguageId = item.tlId
                            }
                        }

                        popup.close()
                    }
                }
            }
        }
    }

    onActiveChanged: {
        if (!active) {
            popup.visible = false;
        }
    }
}
