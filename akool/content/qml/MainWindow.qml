import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

WindowBase {
    id: mainWindow
    width: 834
    height: 650
    backgroundColor: "#0B0B0D"

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
                source: "../res/waiting_play.png"
                fillMode: Image.PreserveAspectCrop
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
                                chooseAvatarWindowComponent.createObject(mainWindow)
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

                    // New Avatar按钮
                    NewAvatarButton {
                        id: newAvatarBtn
                        width: height
                        height: parent.height
                        anchors.left: parent.left
                        borderRadius: avatarArea.borderRadius
                    }

                    // Avatar列表
                    GridView {
                        // 头像之间空距
                        property int space: 8

                        id: avataList
                        width: parent.width-newAvatarBtn.width
                        height: parent.height
                        anchors.right: parent.right
                        clip: true                        
                        cellWidth: height+space
                        cellHeight: height

                        delegate: Rectangle {
                            required property string avatarId
                            required property string avatarImage
                            required property bool isSelect

                            id: avatarItem
                            width: avataList.cellWidth
                            height: avataList.cellHeight
                            color: "transparent"

                            AvatarItem {
                                width: parent.width-avataList.space
                                height: parent.height
                                anchors.right: parent.right
                                avatarId: avatarId
                                avatarImage: avatarItem.avatarImage
                                borderRadius: avatarArea.borderRadius
                                isSelect: avatarItem.isSelect
                            }
                        }

                        model: ListModel {
                            ListElement {
                                avatarId: "1"
                                avatarImage: "../res/avatar_demo.png"
                                isSelect: true
                            }

                            ListElement {
                                avatarId: "2"
                                avatarImage: "../res/avatar_demo.png"
                                isSelect: false
                            }
                        }
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

    Component {
        id: chooseAvatarWindowComponent
        ChooseAvatarWindow {}
    }
}
