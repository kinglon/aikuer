import QtQml 2.15
import QtQuick 2.15

QtObject {
    // avatar model
    property ListModel avatarModels: ListModel {}

    signal showQmlWindow(string name)

    signal showMessage(string message)

    signal chattingStatusChange(bool isChatting)

    function init() {
        cppMainController.hasNewAvatar.connect(addAvatars)
        cppMainController.showWindow.connect(showQmlWindow)
        cppMainController.showMessage.connect(showMessage)
        cppMainController.chattingStatusChange.connect(chattingStatusChange)

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

    function getSelectAvatarId() {
        return cppMainController.getSelAvatarId()
    }

    // 切换会议模式 rtt=1 sa=2 lfs=3
    function switchMeetingMode(meetingMode) {
        return cppMainController.switchMeetingMode(meetingMode)
    }

    // 获取剩余时长
    function getDuration() {
        return cppMainController.getDuration()
    }

    // 摄像头开关
    function enableCamera(enable) {
        cppMainController.enableCamera(enable)
    }

    // 麦克风开关
    function enableMicrophone(enable) {
        cppMainController.enableMicrophone(enable)
    }

    // 开始聊天
    function beginChat() {
        return cppMainController.beginChat()
    }

    // 结束聊天
    function stopChat() {
        cppMainController.stopChat()
    }

    // 获取语言
    function getTranslateLanguageList(sourceLanguages, targetLanguages) {
        sourceLanguages.clear()
        var languageJsonString = cppMainController.getLanguageList(true);
        var languages = JSON.parse(languageJsonString)
        for (var i=0; i<languages.length; i++) {
            sourceLanguages.append(languages[i])
        }

        targetLanguages.clear()
        languageJsonString = cppMainController.getLanguageList(false);
        languages = JSON.parse(languageJsonString)
        for (i=0; i<languages.length; i++) {
            targetLanguages.append(languages[i])
        }
    }

    // 选择语言
    function selectTranslateLanguage(sourceLanguageId, targetLanguageId) {
        cppMainController.selectTranslateLanguage(sourceLanguageId, targetLanguageId)
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
