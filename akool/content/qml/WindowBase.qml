import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3


Window {
    id: windowBase
    visible: true
    width: 800
    height: 600
    title: "Akool Camera"

    // 是否有自定义标题栏，不使用系统默认
    property bool useSystemTitleBar: true

    // 客户区内容（不包含标题栏、边框）
    property alias contentArea: contentArea

    // 客户区背景颜色
    property color backgroundColor: "#222222"

    // 是否有最小化按钮
    property bool hasMinButton: true

    // 是否有最大化按钮
    property bool hasMaxButton: false

    // 最小化、最大化、关闭按钮图片资源路径
    property string minBtnImage: ""
    property string maxBtnImage: ""
    property string closeBtnImage: ""

    // 是否有LOGO
    property bool hasLogo: true

    // logo图片路径
    property string logoImage: ""

    // logo宽度与高度
    property int logoWidth: 20
    property int logoHeight: logoWidth

    // 标题文本颜色
    property color titleTextColor: "white"

    // 标题文本字体大小
    property int titleTextFontSize: 16

    // 边框的宽度
    property int borderWidth: 4

    // 边框的颜色
    property color borderColor: "#222222"

    // 标题栏颜色
    property color titleBarColor: "#222222"

    // 标题栏高度
    property int titleBarHeight: 44

    Rectangle {
        id: windowArea
        anchors.fill: parent
        border.width: windowBase.borderWidth
        border.color: windowBase.borderColor

        Column {
            width: parent.width-2*windowArea.border.width
            // 高度减去底部，顶部跟标题栏在一起
            height: parent.height-windowArea.border.width
            x: windowArea.border.width
            y: 0

            // title bar
            Rectangle {
                id: titleBar
                visible: !useSystemTitleBar
                width: parent.width
                height: windowBase.titleBarHeight
                color: windowBase.titleBarColor

                MouseArea {
                    anchors.fill: parent

                    property point clickPos: Qt.point(1,1)

                    onPressed: {
                        clickPos  = Qt.point(mouse.x,mouse.y);
                    }

                    onPositionChanged: {
                        var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y);
                        windowBase.x += delta.x;
                        windowBase.y += delta.y;
                    }
                }

                // Logo
                Image {
                    id: logo
                    visible: windowBase.hasLogo
                    width: windowBase.logoWidth
                    height: windowBase.logoHeight
                    x: 6
                    y: (parent.height - height)/2
                    fillMode: Image.PreserveAspectFit
                    source: windowBase.logoImage
                }

                // Title text
                Text {
                    id: titleText
                    width: 300
                    height: parent.height
                    anchors.left: windowBase.hasLogo? logo.right : parent.left
                    leftPadding: 6
                    text: windowBase.title
                    color: windowBase.titleTextColor
                    font.pixelSize: windowBase.titleTextFontSize
                    verticalAlignment: Text.AlignVCenter
                    anchors.leftMargin: 6
                }

                // 最小化按钮
                ButtonBase {
                    id: minBtn
                    visible: windowBase.hasMinButton
                    width: height
                    height: titleBar.height
                    topInset: 4
                    bottomInset: 4
                    leftInset: 4
                    rightInset: 4
                    anchors.right: windowBase.hasMaxButton? maxBtn.left : closeBtn.left
                    anchors.rightMargin: 6
                    icon.source: windowBase.minBtnImage

                    onClicked: {
                        windowBase.visibility = Window.Minimized
                    }
                }

                // 最大化按钮
                ButtonBase {
                    id: maxBtn
                    width: height
                    height: titleBar.height
                    topInset: 4
                    bottomInset: 4
                    leftInset: 4
                    rightInset: 4
                    anchors.right: closeBtn.left
                    anchors.rightMargin: 6
                    icon.source: windowBase.maxBtnImage
                    onClicked: {
                        windowBase.visibility = Window.Maximized
                    }
                }

                // 关闭按钮
                ButtonBase {
                    id: closeBtn
                    width: height
                    height: titleBar.height
                    topInset: 4
                    bottomInset: 4
                    leftInset: 4
                    rightInset: 4
                    anchors.right: parent.right
                    anchors.rightMargin: 6
                    icon.source: windowBase.closeBtnImage
                    onClicked: {
                        windowBase.close()
                    }
                }
            }

            // Main content area
            Rectangle {
                id: contentArea
                width: parent.width
                height: parent.height - (windowBase.useSystemTitleBar? 0:titleBar.height)
                color: windowBase.backgroundColor
            }
        }
    }

    Component.onCompleted: {        
        if (!windowBase.useSystemTitleBar) {
            windowBase.flags = Qt.Window|Qt.FramelessWindowHint
            if (windowBase.hasMaxButton)
            {
                windowBase.flags |= Qt.WindowMaximizeButtonHint;
            }
            if (windowBase.hasMinButton)
            {
                windowBase.flags |= Qt.WindowMinimizeButtonHint;
            }
        }

        // 居中显示窗口在屏幕上
        windowBase.x = (Screen.desktopAvailableWidth-windowBase.width)/2
        if (windowBase.x < 0) {
            windowBase.x = 0
        }
        windowBase.y = (Screen.desktopAvailableHeight-windowBase.height)/2
        if (windowBase.y < 0) {
            windowBase.y = 0
        }
    }
}
