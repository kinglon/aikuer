#include "maincontroller.h"
#include <QUrl>
#include <QUrlQuery>
#include "settingmanager.h"

MainController::MainController(QObject *parent)
    : QObject{parent}
{

}

void MainController::setLaunchParam(const QString& launchParam)
{
    // 内容：akool://streaming-avatar?token=xxx&avatar_id=xxx
    qInfo("the launch param is: %s", launchParam.toStdString().c_str());
    QUrl qUrl(launchParam);
    if (!qUrl.isValid())
    {
        qCritical("Invalid URL");
        return;
    }

    QUrlQuery query(qUrl);
    SettingManager::getInstance()->m_loginToken = query.queryItemValue("token");
    m_selectAvatarId = query.queryItemValue("avatar_id");
}

void MainController::run()
{
    if (SettingManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        return;
    }

    avatarController.run();
}
