import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id: messageBox
    color: "#DC4D48"
    radius: 8
    width: getFixedWidth() + messageContent.width
    height: 44
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
        width: 20
        height: 20
        anchors.left: parent.left
        anchors.leftMargin: 13
        source: "qrc:/content/res/icon_toast_info.png"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: messageContent
        color: "#F5F5F7"
        height: parent.height
        anchors.left: messageIcon.right
        anchors.leftMargin: 6
        text: ""
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
        font.weight: Font.Medium
    }

    Rectangle {
        id: messageDivider
        color: "#33F5F5F7"
        width: 1
        height: 20
        anchors.left: messageContent.right
        anchors.leftMargin: 13
        anchors.verticalCenter: parent.verticalCenter
    }

    Image {
        id: messageCloseButton
        anchors.verticalCenter: parent.verticalCenter
        width: 12
        height: 12
        anchors.left: messageDivider.right
        anchors.leftMargin: 13
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
