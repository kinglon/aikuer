import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Rectangle {
    property bool isFocus: false
    property int borderRadius: 8

    signal newAvatarClick()

    id: root
    width: 100
    height: 100
    radius: borderRadius
    color: "#312A5B"

    Image {
        width: 22
        height: 22
        source: "../res/icon1.png"
        fillMode: Image.PreserveAspectFit
        anchors.top: parent.top
        anchors.topMargin: 28
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        text: "New Avatar"
        color: "#F5F5F7"
        font.weight: Font.Medium
        font.pixelSize: 12
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignBottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 26
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        id: coverArea
        anchors.fill: parent
        border.width: 1
        border.color: isFocus? "#7861FA":"#1AF5F5F7"
        radius: borderRadius
        color: "transparent"
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true // Enable hover events

        onEntered: {
            isFocus = true
        }

        onExited: {
            isFocus = false
        }

        onClicked: {
            newAvatarClick()
            Qt.openUrlExternally("https://akool.com/apps/streaming-avatar/edit")
        }
    }
}
