import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    width: 100
    height: 100
    // text: "确定"
    font.pixelSize: 18
    font.weight: Font.Medium
    palette.buttonText: textNormalColor
    display: AbstractButton.TextBesideIcon
    hoverEnabled: true    

    // 各状态下的背景颜色
    property color bgNormalColor: "transparent"
    property color bgClickColor: "#24F5F5F7"
    property color bgHoverColor: "#24F5F5F7"
    property color bgDisableColor: "transparent"

    // 各状态下的字体颜色
    property color textNormalColor: "#F5F5F7"
    property color textDisableColor: "#26F5F5F7"

    // 自定义属性
    property bool isSelected: false
    property color borderColor: "#ffffff"
    property int borderWidth: 0 // 默认边框宽度
    property int disableBorderWidth: 0 // 禁用状态下边框宽度
    property int normalBorderWidth: 0 // 正常状态下边框宽度
    property int borderRadius: 15

    function updateButtonStatus() {
        if (!enabled) {
            palette.buttonText = textDisableColor
            solidBackground.color = bgDisableColor
            solidBackground.border.width = disableBorderWidth
        } else {
            palette.buttonText = textNormalColor
            if (isSelected || down) {
                solidBackground.color = bgClickColor
                solidBackground.border.width = borderWidth
            } else if (hovered) {
                solidBackground.color = bgHoverColor
                solidBackground.border.width = borderWidth
            } else {
                solidBackground.color = bgNormalColor
                solidBackground.border.width = normalBorderWidth
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

    onDownChanged: { updateButtonStatus(); }

    onEnabledChanged: { updateButtonStatus(); }

    onHoveredChanged: { updateButtonStatus(); }

    onIsSelectedChanged: { updateButtonStatus(); }
}
