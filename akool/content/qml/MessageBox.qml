import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id: messageBox
    color: "#DC4D48"
    radius: 12
    width: getFixedWidth() + messageContent.width
    height: 66
    visible: false

    function getFixedWidth() {
        return messageIcon.anchors.leftMargin + messageIcon.width
                + messageContent.anchors.leftMargin
                + messageDivider.anchors.leftMargin + messageDivider.width
                + messageCloseButton.anchors.leftMargin*2 + messageCloseButton.width
    }

    function show(message) {
        messageContent.text = message
        messageBox.visible = true
    }

    Image {
        id: messageIcon
        anchors.verticalCenter: parent.verticalCenter
        width: 30
        height: 30
        anchors.left: parent.left
        anchors.leftMargin: 19
        source: "qrc:/content/res/icon_toast_info.png"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: messageContent
        color: "#F5F5F7"
        height: parent.height
        anchors.left: messageIcon.right
        anchors.leftMargin: 9
        text: ""
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 21
        font.weight: Font.Medium
    }

    Rectangle {
        id: messageDivider
        color: "#33F5F5F7"
        width: 2
        height: 30
        anchors.left: messageContent.right
        anchors.leftMargin: 19
        anchors.verticalCenter: parent.verticalCenter
    }

    Image {
        id: messageCloseButton
        anchors.verticalCenter: parent.verticalCenter
        width: 30
        height: 30
        anchors.left: messageDivider.right
        anchors.leftMargin: 19
        source: "../res/close_button.png"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent

            onClicked: {
                messageContent.text = ""
                messageBox.visible = false
            }
        }
    }
}
