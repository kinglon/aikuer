import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

WindowBase {
    id: chooseLanguageWindow
    width: 370
    height: 310
    maximumHeight: 310
    maximumWidth: 370
    minimumHeight: 310
    minimumWidth: 370
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
        width: parent.width-36*2
        height: parent.height
        anchors.centerIn: parent
        
        Text {
            id: languageLabel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 16
            text: "Language"
            color: "#8CEBEBED"
            font.pixelSize: 14
            font.weight: Font.Medium
        }

        // 语言下拉框
        LanguageComboBox {
            id: languageComboBox
            width: parent.width
            height: 56
            anchors.top: languageLabel.bottom
            anchors.topMargin: 4
            model: chooseLanguageWindow.sourceLanguageModel
        }

        Text {
            id: targetLanguageLabel
            anchors.left: parent.left
            anchors.top: languageComboBox.bottom
            anchors.topMargin: 16
            text: "Target language"
            color: "#8CEBEBED"
            font.pixelSize: 14
            font.weight: Font.Medium
        }

        // 语言下拉框
        LanguageComboBox {
            id: targetLanguageComboBox
            width: parent.width
            height: 56
            anchors.top: targetLanguageLabel.bottom
            anchors.topMargin: 4
            model: chooseLanguageWindow.targetLanguageModel
        }

        // 确认按钮
        ButtonBase {
            id: confirmBtn
            width: 98
            height: 48
            text: "Translate"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 36
            borderColor: "#24F5F5F7"
            borderWidth: 1
            borderRadius: 10
            bgNormalColor: "#7861FA"
            disableBorderWidth: 1
            enabled: languageComboBox.selLanguageId != "" && targetLanguageComboBox.selLanguageId != ""

            onClicked: {
                mainController.selectTranslateLanguage(languageComboBox.selLanguageId, targetLanguageComboBox.selLanguageId)
                chooseLanguageWindow.close()
            }
        }

        // 取消按钮
        ButtonBase {
            width: 84
            height: 48
            text: "Cancel"
            anchors.right: confirmBtn.left
            anchors.rightMargin: 24
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 36
            borderColor: "#24F5F5F7"
            borderWidth: 1
            normalBorderWidth: 1
            borderRadius: 10

            onClicked: {
                chooseLanguageWindow.close()
            }
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
