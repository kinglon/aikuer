import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    width: 100
    height: 100
    // text: "确定"
    font.pixelSize: 12
    font.weight: Font.Medium
    palette.buttonText: textNormalColor
    display: AbstractButton.TextBesideIcon
    hoverEnabled: true

    // 各状态下的背景颜色
    property color bgNormalColor: "#27272E"
    property color bgClickColor: "#373357"
    property color bgHoverColor: "#373357"
    property color bgDisableColor: "#27272E"

    // 各状态下的字体颜色
    property color textNormalColor: "#F5F5F7"
    property color textDisableColor: "#26F5F5F7"


    property color borderColor: "#ffffff"
    property int borderWidth: 0
    property int borderRadius: 10

    function updateBackgroundColor() {
        if (!enabled) {
            palette.buttonText = textDisableColor
            solidBackground.color = bgDisableColor            
        } else {
            palette.buttonText = textNormalColor
            if (down) {
                solidBackground.color = bgClickColor
            } else {
                if (hovered) {
                    solidBackground.color = bgHoverColor
                } else {
                    solidBackground.color = bgNormalColor
                }
            }
        }
    }

    background: Rectangle {
        id: solidBackground
        x: parent.leftInset
        y: parent.topInset
        width: parent.width-parent.leftInset-parent.rightInset
        height: parent.height-parent.topInset-parent.bottomInset
        color: bgNormalColor
        radius: borderRadius
        border.width: borderWidth
        border.color: borderColor
    }

    onDownChanged: { updateBackgroundColor(); }

    onEnabledChanged: { updateBackgroundColor(); }

    onHoveredChanged: { updateBackgroundColor(); }
}
