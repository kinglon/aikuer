import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

WindowBase {
    id: chooseLanguageWindow
    width: 645
    height: 465
    maximumHeight: 465
    maximumWidth: 645
    minimumHeight: 465
    minimumWidth: 645
    backgroundColor: "#15151A"
    hasMaxButton: false
    hasMinButton: false
    modality: Qt.WindowModal
    title: "Create Translation"
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    // 当使用系统标题栏的时候，窗口显示的时候先白屏，再显示QML的内容，体验不好，所以先不显示，加载后再显示
    opacity: 0

    property var mainController: null

    property var sourceLanguageModel: null
    property var targetLanguageModel: null

    Item  {
        parent: contentArea
        width: parent.width-54*2
        height: parent.height
        anchors.centerIn: parent
        
        Text {
            id: languageLabel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 24
            text: "Language"
            color: "#8CEBEBED"
            font.pixelSize: 21
            font.weight: Font.Medium
        }

        // 语言下拉框
        LanguageComboBox {
            id: languageComboBox
            width: parent.width
            height: 84
            anchors.top: languageLabel.bottom
            anchors.topMargin: 6
            model: chooseLanguageWindow.sourceLanguageModel
        }

        Text {
            id: targetLanguageLabel
            anchors.left: parent.left
            anchors.top: languageComboBox.bottom
            anchors.topMargin: 24
            text: "Target language"
            color: "#8CEBEBED"
            font.pixelSize: 21
            font.weight: Font.Medium
        }

        // 语言下拉框
        LanguageComboBox {
            id: targetLanguageComboBox
            width: parent.width
            height: 84
            anchors.top: targetLanguageLabel.bottom
            anchors.topMargin: 6
            model: chooseLanguageWindow.targetLanguageModel
        }

        // 确认按钮
        ButtonBase {
            id: confirmBtn
            width: 138
            height: 72
            text: "Translate"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 54
            borderColor: "#24F5F5F7"
            borderWidth: 2
            borderRadius: 15
            bgNormalColor: "#7861FA"
            disableBorderWidth: 2

            onClicked: {
                if (languageComboBox.selLanguageId === "") {
                    messageBox.show("Please select the input language")
                    return
                }

                if (targetLanguageComboBox.selLanguageId === "") {
                    messageBox.show("Please select the target language")
                    return
                }

                mainController.selectTranslateLanguage(languageComboBox.selLanguageId, targetLanguageComboBox.selLanguageId)
                chooseLanguageWindow.close()
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
                chooseLanguageWindow.close()
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
            chooseLanguageWindow.opacity = 1.0
        }
    }
}
