import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

WindowBase {
    id: chooseAvatarWindow
    width: 720
    height: 720
    maximumHeight: 720
    maximumWidth: 720
    minimumHeight: 720
    minimumWidth: 720
    backgroundColor: "#131317"
    hasMaxButton: false
    hasMinButton: false
    modality: Qt.WindowModal
    title: "Choose Avatar"
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    // 当使用系统标题栏的时候，窗口显示的时候先白屏，再显示QML的内容，体验不好，所以先不显示，加载后再显示
    opacity: 0

    property var mainController: null

    property string selectAvatarId: ""

    Item  {
        parent: contentArea
        width: parent.width-54*2
        height: parent.height
        anchors.centerIn: parent
        
        // Avatar列表
        Flickable {
            property int itemSize: 144
            property int columnCount: 4
            property int itemMargin: 6

            id: avatarListArea
            width: parent.width
            height: 468
            anchors.top: parent.top
            anchors.topMargin: 27
            contentWidth: width
            contentHeight: height
            clip: true
            boundsMovement: Flickable.StopAtBounds
            boundsBehavior: Flickable.StopAtBounds
            interactive: false

            ScrollBar.vertical: ScrollBar {
                active: true
            }

            GridView {
                property int borderRadius: 12

                id: avataList
                anchors.fill: parent
                clip: true
                cellWidth: avatarListArea.width/avatarListArea.columnCount
                cellHeight: cellWidth

                delegate: Rectangle {
                    // 1是new avatar按钮, 2是avatar图像
                    required property int type
                    required property string avatarId
                    required property string avatarImage
                    required property bool isSelect

                    id: avatarItem
                    width: avataList.cellWidth-2*avatarListArea.itemMargin
                    height: avataList.cellHeight-2*avatarListArea.itemMargin
                    color: "transparent"

                    NewAvatarButton {
                        id: newAvatarBtn
                        width: parent.width
                        height: parent.height
                        anchors.left: parent.left
                        radius: avataList.borderRadius
                        visible: avatarItem.type==1
                    }

                    AvatarItem {
                        width: parent.width
                        height: parent.height
                        avatarId: avatarItem.avatarId
                        avatarImage: avatarItem.avatarImage
                        borderRadius: avataList.borderRadius
                        isSelect: avatarItem.isSelect
                        visible: avatarItem.type==2

                        onAvatarClick: {
                            mainController.setSelectAvatar(avatarId)
                        }
                    }
                }

                model: mainController.avatarModels
            }
        }

        // 确认按钮
        ButtonBase {
            id: confirmBtn
            width: 138
            height: 72
            text: "Confirm"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 54
            borderColor: "#24F5F5F7"
            borderWidth: 2
            borderRadius: 15
            bgNormalColor: "#7861FA"

            onClicked: {
                var selAvatarId = mainController.getSelectAvatarId()
                if (selAvatarId === "") {
                    messageBox.show("Please select an avatar")
                    return
                }

                chooseAvatarWindow.close()
            }
        }

        // 取消按钮
        ButtonBase {
            width: 126
            height: 72
            text: "Cancel"
            anchors.right: confirmBtn.left
            anchors.rightMargin: 36
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 54
            borderColor: "#24F5F5F7"
            borderWidth: 2
            normalBorderWidth: 2
            borderRadius: 15

            onClicked: {
                chooseAvatarWindow.close()
            }
        }

        // 消息提示框
        MessageBox {
            id: messageBox
            anchors.centerIn: parent
            z: 1
        }
    }

    Component.onCompleted: {
        timer.start()
    }

    Timer {
        id: timer
        interval: 100 // Timer interval in milliseconds
        running: false // Start the timer immediately

        onTriggered: {
            chooseAvatarWindow.opacity = 1.0
        }
    }
}
