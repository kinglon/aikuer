import QtQml 2.15
import QtQuick 2.15
import akoolqml 1.0

QtObject {
    // avatar model
    property ListModel avatarModels: ListModel {}

    signal showQmlWindow(string name)

    function init() {
        MainController.hasNewAvatar.connect(addAvatars)
        MainController.showWindow.connect(showQmlWindow)

        // 添加New Avatar按钮
        avatarModels.append({"type": 1, "avatarId": "", "avatarImage": "", "isSelect": false})

        // 添加Avatar
        var avatarString = MainController.getAvatars()
        addAvatars(avatarString)
    }

    function addAvatars(avatarJson) {
        var avatars = JSON.parse(avatarJson)
        for (var i=0; i<avatars.length; i++) {
            avatarModels.append({"type": 2, "avatarId": avatars[i]["id"], "avatarImage": avatars[i]["imagePath"], "isSelect": false})
        }

        var selAvatarId = MainController.getSelAvatarId()
        setSelectAvatar(selAvatarId)
    }

    function setSelectAvatar(avatarId) {
        for (var j=1; j<avatarModels.length; j++)
        {
            if (avatarModels[j]["avatarId"] === selAvatarId)
            {
                avatarModels[j]["isSelect"] = true
            }
            else
            {
                avatarModels[j]["isSelect"] = false
            }
        }
        MainController.setSelAvatarId(avatarId)
    }

    function beginChat() {
        MainController.beginChat()
    }

    function quitApp() {
        MainController.quitApp()
    }
}
