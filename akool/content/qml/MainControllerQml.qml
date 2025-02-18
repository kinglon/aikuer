import QtQml 2.15
import QtQuick 2.15

QtObject {
    // avatar model
    property ListModel avatarModels: ListModel {}

    signal showQmlWindow(string name)

    function init() {
        cppMainController.hasNewAvatar.connect(addAvatars)
        cppMainController.showWindow.connect(showQmlWindow)

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

    function beginChat() {
        cppMainController.beginChat()
    }

    function quitApp() {
        cppMainController.quitApp()
    }
}
