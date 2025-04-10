import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15

WindowBase {
    id: mainWindow
    width: 805
    height: 650
    minimumWidth: 805
    minimumHeight: 650
    backgroundColor: "#0B0B0D"

    // 当使用系统标题栏的时候，窗口显示的时候先白屏，再显示QML的内容，体验不好，所以先不显示，加载后再显示
    opacity: 0

    // 当前会议模式 rtt=1 sa=2 lfs=3
    property int currentMeetingMode: 2

    ColumnLayout  {
        parent: contentArea
        anchors.fill: parent
        spacing: 0

        // 顶部按钮区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
            color: "transparent"

            // 标签页头部
            Rectangle {
                width: 488
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
                        id: rttBtn
                        width: parent.width/3
                        height: parent.height
                        text: "Real-time Translation"
                        borderRadius: height/2
                        anchors.left: parent.left
                        isSelected: mainWindow.currentMeetingMode==1
                        icon.source: "qrc:/content/res/icon_rtt.png"
                        display: AbstractButton.TextBesideIcon
                        spacing: 6

                        onClicked: {
                            mainWindow.switchMeetingMode(1)
                        }
                    }

                    ButtonBase {
                        id: streamingAvatarBtn
                        width: parent.width/3
                        height: parent.height
                        text: "Streaming Avatar"
                        borderRadius: height/2
                        anchors.left: rttBtn.right
                        isSelected: mainWindow.currentMeetingMode==2
                        icon.source: "qrc:/content/res/icon_streamavatar.png"
                        display: AbstractButton.TextBesideIcon
                        spacing: 6

                        onClicked: {
                            mainWindow.switchMeetingMode(2)
                        }
                    }

                    ButtonBase {
                        id: liveFaceSwapBtn
                        width: parent.width/3
                        height: parent.height
                        text: "Live Face Swap"
                        borderRadius: height/2
                        enabled: false
                        anchors.right: parent.right
                        isSelected: mainWindow.currentMeetingMode==3
                        icon.source: "qrc:/content/res/icon_livefaceswap.png"
                        display: AbstractButton.TextBesideIcon
                        spacing: 6

                        onClicked: {
                            mainWindow.switchMeetingMode(3)
                        }
                    }
                }
            }

            // 反馈按钮
            ButtonBase {
                id: feedBackButton
                width: 20
                height: 20
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 28
                icon.source: "qrc:/content/res/icon_feedback.png"
                display: AbstractButton.IconOnly
                borderRadius: 2
                padding: 0

                onClicked: {
                    Qt.openUrlExternally("https://akool.com")
                }
            }
        }

        // 中间视频区域
        Item {
            id: item1
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 中间图片
            Image {
                id: videoPlayer
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                cache: false

                Timer {
                    id: videoPlayerTimer
                    interval: 60 // Timer interval in milliseconds
                    running: true // Start the timer immediately
                    repeat: true // Repeat the timer indefinitely

                    onTriggered: {
                        videoPlayer.source = ""
                        videoPlayer.source = "image://memory/videoPlayer"
                    }
                }
            }

            // 左上角的剩余时长
            Rectangle {
                id: durationArea
                width: 89
                height: 48
                anchors.top: parent.top
                anchors.topMargin: 12
                anchors.left: parent.left
                anchors.leftMargin: 12
                color: "#990A0A11"
                radius: height/2

                Image {
                    id: durationIcon
                    width: 24
                    height: 24
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/content/res/icon_duration.png"
                }

                Text {
                    id: durationText
                    anchors.left: durationIcon.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: "00 : 00"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: "#F5F5F7"
                }

                Timer {
                    interval: 300 // Timer interval in milliseconds
                    running: true
                    repeat: true

                    onTriggered: {
                        durationText.text = mainController.getDuration()
                    }
                }
            }

            // 右上角的摄像头显示区域
            Item {
                id: cameraDisplayArea
                visible: rttBtn.isSelected
                width: 280
                height: 156
                anchors.top: parent.top
                anchors.topMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 12

                property int borderWidth: 2
                property int borderRadius: 15
                property color borderColor: "#59F5F5F7"

                // 摄像头禁用时显示
                Rectangle {
                    visible: !cameraBtn.cameraEnable
                    anchors.fill: parent
                    border.color: cameraDisplayArea.borderColor
                    border.width: cameraDisplayArea.borderWidth
                    radius: cameraDisplayArea.borderRadius
                    color: "#0A0A11"

                    Image {
                        width: 42
                        height: 43
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 42
                        source: "qrc:/content/res/icon_camera_disable_big.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 42
                        text: "Click allow to access camera"
                        color: "#F5F5F7"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                    }
                }

                // 摄像头画面显示区域
                Item {
                    visible: cameraBtn.cameraEnable
                    anchors.fill: parent

                    Image {
                        id: cameraImage
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        source: "qrc:/content/res/no_camera_bg.png"
                        visible: false
                        cache: false
                    }

                    Rectangle {
                        id: mask
                        anchors.fill: parent
                        radius: cameraDisplayArea.borderRadius                        
                    }

                    OpacityMask {
                        anchors.fill: parent
                        source: cameraImage
                        maskSource: mask
                    }

                    Rectangle {
                        anchors.fill: parent
                        border.width: cameraDisplayArea.borderWidth
                        border.color: cameraDisplayArea.borderColor
                        radius: cameraDisplayArea.borderRadius
                        color: "transparent"
                    }
                }

                // 更新摄像头画面定时器
                Timer {
                    id: cameraTimer
                    interval: 60 // Timer interval in milliseconds
                    running: true
                    repeat: true

                    onTriggered: {
                        if (!cameraBtn.cameraEnable) {
                            return
                        }

                        cameraImage.source = ""
                        cameraImage.source = "image://memory/cameraImage"
                    }
                }
            }


            // 工具栏
            Rectangle {
                id: toolbarArea
                width: parent.width-12*2
                height: 80
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                color: "#D00A0A11"
                radius: 10

                // 开始按钮
                ButtonBase {
                    id: startBtn
                    width: 59
                    height: 36
                    text: isChatting?"Stop":"Start"
                    anchors.centerIn: parent
                    borderRadius: 8
                    bgNormalColor: isChatting?"#DC4D48":"#7861FA"
                    bgClickColor: isChatting?"#CC3D38":"#6851E0"
                    bgHoverColor: bgClickColor

                    property bool isChatting: false

                    onClicked: {
                        if (isChatting) {
                            mainController.stopChat()
                            isChatting = false
                        } else {
                            if (!mainController.beginChat()) {
                                return
                            }
                            isChatting = true
                        }
                    }
                }

                // Streaming Avatar专有
                Item {
                    id: saToolbarPanel
                    anchors.fill: parent
                    visible: streamingAvatarBtn.isSelected

                    // 选择Avatar按钮
                    ButtonBase {
                        id: selectAvatarBtn
                        width: 72
                        height: 56
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        text: "Avatar"
                        borderRadius: 8
                        icon.source: "qrc:/content/res/icon_avatar.png"
                        display: AbstractButton.TextUnderIcon
                        spacing: 6

                        onClicked: {
                            chooseAvatarWindowComponent.createObject(mainWindow, {"mainController": mainController})
                        }
                    }
                }

                // Real-time Translation专有
                Item {
                    id: rttToolbarPanel
                    anchors.fill: parent
                    visible: rttBtn.isSelected

                    // 摄像头开关按钮
                    ButtonBase {
                        id: cameraBtn
                        visible: true
                        width: 72
                        height: 56
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        text: "Camera"
                        textNormalColor: "#DC4D48"
                        borderRadius: 8
                        icon.source: "qrc:/content/res/icon_camera_disable.png"
                        display: AbstractButton.TextUnderIcon
                        spacing: 6

                        property bool cameraEnable: false

                        onClicked: {
                            cameraEnable = !cameraEnable
                            mainController.enableCamera(cameraEnable)
                        }

                        onCameraEnableChanged: {
                            if (cameraEnable) {
                                icon.source = "qrc:/content/res/icon_camera.png"
                                textNormalColor = "#F5F5F7"
                            } else {
                                icon.source = "qrc:/content/res/icon_camera_disable.png"
                                textNormalColor = "#DC4D48"
                            }
                        }
                    }

                    // 麦克风开关按钮
                    ButtonBase {
                        id: microPhoneBtn
                        visible: true
                        width: 72
                        height: 56
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: cameraBtn.right
                        anchors.leftMargin: 12
                        text: "Mic"
                        textNormalColor: "#DC4D48"
                        borderRadius: 8
                        icon.source: "qrc:/content/res/icon_microphone_disable.png"
                        display: AbstractButton.TextUnderIcon
                        spacing: 6

                        property bool microPhoneEnable: false

                        onClicked: {
                            microPhoneEnable = !microPhoneEnable
                            mainController.enableMicrophone(microPhoneEnable)
                        }

                        onMicroPhoneEnableChanged: {
                            if (microPhoneEnable) {
                                icon.source = "qrc:/content/res/icon_microphone.png"
                                textNormalColor = "#F5F5F7"
                            } else {
                                icon.source = "qrc:/content/res/icon_microphone_disable.png"
                                textNormalColor = "#DC4D48"
                            }
                        }
                    }

                    // 选择语言按钮
                    ButtonBase {
                        id: selectLanguageBtn
                        visible: true
                        width: 120
                        height: 56
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        text: "Target Language"
                        borderRadius: 8
                        icon.source: "qrc:/content/res/icon_language.png"
                        display: AbstractButton.TextUnderIcon
                        spacing: 6

                        onClicked: {
                            var sourceLanguages = mainController.createListModel(selectLanguageBtn)
                            var targetLanguages = mainController.createListModel(selectLanguageBtn)
                            mainController.getTranslateLanguageList(sourceLanguages, targetLanguages)
                            if (sourceLanguages.count === 0 || targetLanguages === 0) {
                                mainController.showMessage()()("There is no language")
                                return
                            }

                            chooseLanguageWindowComponent.createObject(mainWindow, {mainController: mainController,
                                                                           sourceLanguageModel: sourceLanguages,
                                                                           targetLanguageModel: targetLanguages})
                        }
                    }
                }
            }

            // 消息提示框
            MessageBox {
                id: messageBox
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: toolbarArea.top
                anchors.bottomMargin: 12
            }
        }
    }

    MainControllerQml {
        id: mainController

        onShowQmlWindow: function(name) {
            if (name === "main")
            {                
                mainWindow.show()
                mainWindow.raise()
                mainWindow.requestActivate()
            }
        }

        onShowMessage: function(message) {
            messageBox.show(message)
        }

        onChattingStatusChange: function(isChatting) {
            startBtn.isChatting = isChatting
        }
    }

    Component.onCompleted: {
        mainController.init()
        mainWindow.opacity = 1.0
    }

    //可能是qmltype信息不全，有M16警告，这里屏蔽下
    //@disable-check M16
    onClosing: function(closeEvent) {
        videoPlayerTimer.stop()
        cameraTimer.stop()
        mainController.quitApp();
    }

    Component {
        id: chooseAvatarWindowComponent
        ChooseAvatarWindow {}
    }

    Component {
        id: chooseLanguageWindowComponent
        ChooseLanguageWindow {}
    }

    function switchMeetingMode(meetingMode) {
        if (startBtn.isChatting) {
            return
        }

        if (!mainController.switchMeetingMode(meetingMode)) {
            return
        }

        mainWindow.currentMeetingMode = meetingMode
    }
}
