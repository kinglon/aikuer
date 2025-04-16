import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: comboBoxId
    width: parent.width
    height: 84
    color: "transparent"
    border.color: "#24F5F5F7"
    border.width: 2
    radius: 24

    // 下拉框数据 { id: "1", language: "Chinese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: false}
    property var model: null

    // 下拉框
    property var popup: null

    // 选择的语言ID
    property string selLanguageId: ""

    // 显示当前选中的文本
    Text {
        id: selectedText
        anchors.left: parent.left
        anchors.leftMargin: 36
        anchors.verticalCenter: parent.verticalCenter
        text: "Please select a language"
        color: "#F5F5F7"
        font.pixelSize: 24
        font.weight: Font.Medium
    }

    // 下拉箭头
    Image {
        width: 30
        height: 30
        anchors.right: parent.right
        anchors.rightMargin: 36
        anchors.verticalCenter: parent.verticalCenter
        source: "qrc:/content/res/icon_arrowdown.png"
    }

    // 点击时弹出下拉列表
    MouseArea {
        anchors.fill: parent
        onClicked: {
            var globalPos = comboBoxId.mapToGlobal(0, 0)
            if (popup == null) {
                popup = languagePopupComponent.createObject(null, {"comboBox": comboBoxId})
            }
            popup.x = globalPos.x
            popup.y = globalPos.y + comboBoxId.height + 6
            popup.visible = true;
            popup.requestActivate();
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

    Component {
        id: languagePopupComponent
        LanguagePopup {}
    }
}
