import QtQml 2.15
import QtQuick 2.15

QtObject {
    // avatar model
    property ListModel avatarModels: ListModel {}

    signal showQmlWindow(string name)

    signal showMessage(string message)

    function init() {
        cppMainController.hasNewAvatar.connect(addAvatars)
        cppMainController.showWindow.connect(showQmlWindow)
        cppMainController.showMessage.connect(showMessage)

        // 添加New Avatar按钮
        avatarModels.append({"type": 1, "avatarId": "", "avatarImage": "", "isSelect": false})

        // 添加Avatar
        var avatarString = cppMainController.getAvatars()
        addAvatars(avatarString)
    }

    function addAvatars(avatarJson) {
        var avatars = JSON.parse(avatarJson)
        for (var i=0; i<avatars.length; i++) {
            avatarModels.append({"type": 2, "avatarId": avatars[i]["id"], "avatarImage": avatars[i]["imagePath"], "isSelect": false})
        }

        var selAvatarId = cppMainController.getSelAvatarId()
        setSelectAvatar(selAvatarId)
    }

    function setSelectAvatar(avatarId) {
        for (var j=1; j<avatarModels.count; j++)
        {
            if (avatarModels.get(j)["avatarId"] === avatarId)
            {
                avatarModels.setProperty(j, "isSelect", true);
            }
            else
            {
                avatarModels.setProperty(j, "isSelect", false);
            }
        }
        cppMainController.setSelAvatarId(avatarId)
    }

    // 切换会议模式 rtt=1 sa=2 lfs=3
    function switchMeetingMode(meetingMode) {
        // todo by yejinlong
        return true
    }

    // 获取剩余时长
    function getDuration() {
        // todo by yejinlong
        return "00 : 00"
    }

    // 摄像头开关
    function enableCamera(enable) {
        // todo by yejinlong
    }

    // 麦克风开关
    function enableMicrophone(enable) {
        // todo by yejinlong
    }

    // 开始聊天
    function beginChat() {
        // todo by yejinlong
        return cppMainController.beginChat()
    }

    // 结束聊天
    function stopChat() {
        // todo by yejinlong
    }

    // 获取语言
    function getTranslateLanguageList(sourceLanguages, targetLanguages) {
        // todo by yejinlong
        sourceLanguages.clear()
        sourceLanguages.append({ tlId: "1", language: "Chinese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: false})
        sourceLanguages.append({ tlId: "2", language: "Japanese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: true})
        targetLanguages.clear()
        targetLanguages.append({ tlId: "3", language: "Chinese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: false})
        targetLanguages.append({ tlId: "4", language: "Japanese", flagImagePath: "qrc:/content/res/icon_camera.png", isCurrent: true})
    }

    // 选择语言
    function selectTranslateLanguage(sourceLanguageId, targetLanguageId) {
        // todo by yejinlong
    }

    // 退出程序
    function quitApp() {
        cppMainController.quitApp()
    }

    // 动态创建ListModel对象
    function createListModel(parent) {
        var model = Qt.createQmlObject(`
            import QtQuick 2.15
            ListModel {
            }
        `, parent, "dynamicModel");
        return model;
    }
}
