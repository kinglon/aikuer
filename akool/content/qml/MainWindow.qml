import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

WindowBase {
    id: mainWindow
    width: 805
    height: 650
    backgroundColor: "#0B0B0D"

    // 当使用系统标题栏的时候，窗口显示的时候先白屏，再显示QML的内容，体验不好，所以先不显示，加载后再显示
    opacity: 0

    ColumnLayout  {
        parent: contentArea
        anchors.fill: parent
        spacing: 0

        // 顶部按钮区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
            color: "transparent"

            Rectangle {
                width: 277
                height: 48
                color: "#0B0B0D"
                border.color: "#1AF5F5F7"
                border.width: 1
                radius: 24
                anchors.centerIn: parent

                Rectangle {
                    width: parent.width-16
                    height: parent.height-16
                    anchors.centerIn: parent
                    color: "transparent"

                    ButtonBase {
                        id: streamingAvatarBtn
                        width: parent.width/2
                        height: parent.height
                        text: "Streaming Avatar"
                        borderRadius: height/2
                        anchors.left: parent.left
                    }

                    ButtonBase {
                        id: liveFaceSwapBtn
                        width: parent.width/2
                        height: parent.height
                        text: "Live Face Swap"
                        borderRadius: height/2
                        enabled: false
                        anchors.right: parent.right
                    }
                }
            }
        }

        // 中间图片显示区域
        Item {
            id: item1
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                id: videoPlayer
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop

                Timer {
                    interval: 60 // Timer interval in milliseconds
                    running: true // Start the timer immediately
                    repeat: true // Repeat the timer indefinitely

                    onTriggered: {
                        videoPlayer.source = ""
                        videoPlayer.source = "image://memory/videoPlayer"
                    }
                }
            }

            // Let's chat 按钮
            Rectangle {
                id: letChatArea
                width: 584
                height: 68
                color: "#990A0A11"
                radius: 12
                border.width: 1
                border.color: "#1AF5F5F7"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                anchors.horizontalCenter: parent.horizontalCenter
                visible: false

                ButtonBase {
                    width: 129
                    height: 48
                    anchors.centerIn: parent
                    borderRadius: 12
                    text: "Let's chat"
                    bgNormalColor: "#7861FA"
                    bgHoverColor: bgClickColor
                    bgClickColor: "#6349F5"
                    textNormalColor: "#F5F5F7"

                    onClicked: {
                        letChatArea.visible = false
                        mainController.beginChat()
                    }
                }
            }
        }

        // avatar选择区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 153
            color: "transparent"

            Rectangle {
                width: parent.width-80
                height: parent.height-32
                anchors.centerIn: parent
                color: "transparent"

                // view all按钮
                Rectangle {
                    width: parent.width
                    height: 27
                    color: "transparent"

                    Text {
                        color: "#F5F5F7"
                        anchors.fill: parent
                        text: "Choose Avatar"
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignTop
                    }

                    Text {                        
                        width: 70
                        height: parent.height
                        anchors.right: parent.right
                        text: "View All"
                        color: "#6349F5"
                        horizontalAlignment: Text.AlignRight
                        font.pointSize: 10
                        verticalAlignment: Text.AlignTop

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                var chooseAvatarWindow = chooseAvatarWindowComponent.createObject(mainWindow, {"mainController": mainController})
                                chooseAvatarWindow.confirmClick.connect(function() {
                                    letChatArea.visible = true
                                })
                            }
                        }
                    }
                }

                // avatar列表
                Rectangle {
                    property int borderRadius: 8

                    id: avatarArea
                    width: parent.width
                    height: 94
                    anchors.bottom: parent.bottom
                    color: "transparent"

                    // Avatar列表
                    GridView {
                        // 头像之间空距
                        property int space: 8

                        id: avataList
                        anchors.fill: parent
                        clip: true                        
                        cellWidth: height+space
                        cellHeight: height

                        delegate: Rectangle {
                            // 1是new avatar按钮, 2是avatar图像
                            required property int type
                            required property string avatarId
                            required property string avatarImage
                            required property bool isSelect

                            id: avatarItem
                            width: avataList.cellWidth
                            height: avataList.cellHeight
                            color: "transparent"

                            // New Avatar按钮
                            NewAvatarButton {
                                id: newAvatarBtn
                                width: height
                                height: parent.height
                                anchors.left: parent.left
                                borderRadius: avatarArea.borderRadius
                                visible: avatarItem.type==1
                            }

                            AvatarItem {
                                width: height
                                height: parent.height
                                anchors.left: parent.left
                                avatarId: avatarItem.avatarId
                                avatarImage: avatarItem.avatarImage
                                borderRadius: avatarArea.borderRadius
                                isSelect: avatarItem.isSelect
                                visible: avatarItem.type==2

                                onAvatarClick: {
                                    mainController.setSelectAvatar(avatarId)
                                    letChatArea.visible = true
                                }
                            }
                        }

                        model: mainController.avatarModels
                    }
                }
            }
        }

        // 底部操作区域
        Row {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
        }
    }

    MainControllerQml {
        id: mainController

        onShowQmlWindow: function(name) {
            if (name === "main")
            {                
                mainWindow.raise();
            }
        }
    }

    Component.onCompleted: {
        mainController.init()
        mainWindow.opacity = 1.0
    }

    //可能是qmltype信息不全，有M16警告，这里屏蔽下
    //@disable-check M16
    onClosing: function(closeEvent) {
        closeEvent.accepted = false
        mainController.quitApp();
    }

    Component {
        id: chooseAvatarWindowComponent
        ChooseAvatarWindow {}
    }
}
