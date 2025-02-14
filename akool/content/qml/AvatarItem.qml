import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    property string avatarId
    // property string avatarImage: "../res/avatar_demo.png"
    property string avatarImage
    property bool isSelect: false
    property bool isFocus: false
    property int borderRadius: 8

    signal avatarClick(string avatarId)

    width: 100
    height: 100

    function getBorderColor() {
        if (isSelect || isFocus) {
            return "#7861FA"
        }
        else
        {
            return "#1AF5F5F7"
        }
    }

    function onIsSelectChanged() {
        coverArea.border.color = getBorderColor()
    }

    function onIsFocusChanged() {
        coverArea.border.color = getBorderColor()
    }

    Image {
        anchors.fill: parent
        source: avatarImage
        fillMode: Image.PreserveAspectFit
    }

    Rectangle {
        id: coverArea
        anchors.fill: parent
        border.width: 1
        border.color: getBorderColor()
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
            avatarClick(avatarId)
        }
    }
}
