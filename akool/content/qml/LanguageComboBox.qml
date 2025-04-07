import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: comboBoxId
    width: parent.width
    height: 56
    color: "transparent"
    border.color: "#24F5F5F7"
    border.width: 1
    radius: 16

    // 下拉框数据 { id: "1", language: "Chinese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: false}
    property var model: null

    // 选择的语言ID
    property string selLanguageId: ""

    // 显示当前选中的文本
    Text {
        id: selectedText
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        text: "Please select a language"
        color: "#F5F5F7"
        font.pixelSize: 16
        font.weight: Font.Medium
    }

    // 下拉箭头
    Image {
        width: 20
        height: 20
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        source: "qrc:/content/res/icon_arrowdown.png"
    }

    // 点击时弹出下拉列表
    MouseArea {
        anchors.fill: parent
        onClicked: {
            popup.open()
        }
    }

    // 自定义下拉弹窗
    Popup {
        id: popup
        y: parent.height
        width: parent.width
        padding: 6
        height: Math.min(7, listView.count) * contentItemHeight + 2*padding

        property int contentItemHeight: 41

        background: Rectangle {
            color: "#1A1A1F"
            border.color: "#24F5F5F7"
            border.width: 1
            radius: 12
        }

        contentItem: ListView {
            id: listView            
            model: comboBoxId.model
            clip: true

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
                            var item = comboBoxId.model.get(i)
                            if (item.tlId !== tlId) {
                                listView.model.setProperty(i, "isCurrent", false)
                            } else
                            {
                                listView.model.setProperty(i, "isCurrent", true)
                                selectedText.text = item.language
                                comboBoxId.selLanguageId = item.tlId
                            }
                        }

                        popup.close()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        for (var i=0; i<comboBoxId.model.count; i++) {
            var item = comboBoxId.model.get(i)
            if (item.isCurrent) {
                selectedText.text = item.language
                comboBoxId.selLanguageId = item.tlId
                break
            }
        }
    }
}
