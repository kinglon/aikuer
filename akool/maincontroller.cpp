#include "maincontroller.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QCoreApplication>
#include "settingmanager.h"
#include "../Utility/DumpUtil.h"

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
}

void MainController::run()
{
    if (SettingManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        return;
    }

    connect(&m_avatarController, &AvatarController::avatarDownloadCompletely, [this](const QVector<Avatar>& avatars) {
        QString jsonString = avatarListToJsonString(avatars);
        emit hasNewAvatar(jsonString);
    });
    m_avatarController.run();

    m_ipcWorker.setKey(IPC_KEY);
    connect(&m_ipcWorker, &IpcWorker::ipcDataArrive, this, &MainController::onIpcDataArrive);
    m_ipcWorker.start();
}

QString MainController::avatarListToJsonString(const QVector<Avatar>& avatars)
{
    QJsonArray avatarArray;
    for (const auto& avatar : avatars)
    {
        QJsonObject avatarObject;
        avatarObject["id"] = avatar.m_avatarId;
        avatarObject["imagePath"] = QString("file:///") + avatar.m_localImagePath;
        avatarArray.append(avatarObject);
    }
    QJsonDocument jsonDocument(avatarArray);
    return QString::fromUtf8(jsonDocument.toJson());
}

QString MainController::getAvatars()
{
    QVector<Avatar> avatars = m_avatarController.getAvatars();
    return avatarListToJsonString(avatars);
}

QImage* MainController::getPlayerImage()
{
    return m_meetingController.popImage();
}

void MainController::beginChat()
{
    QString avatarForService;
    QVector<Avatar> avatars = m_avatarController.getAvatars();
    for (const auto& avatar : avatars)
    {
        if (avatar.m_avatarId == m_selectAvatarId)
        {
            avatarForService = avatar.m_avatarIdForService;
            break;
        }
    }
    if (avatarForService.isEmpty())
    {
        qCritical("avatar is not selected");
        return;
    }

    m_meetingController.setAvatarId(avatarForService);
    if (!m_meetingController.isRun())
    {
        m_meetingController.run();
    }
}

void MainController::quitApp()
{
    static bool first = true;
    if (!first)
    {
        return;
    }
    first = false;

    if (m_meetingController.isRun())
    {
        m_meetingController.requestStop();
    }
}

void MainController::onIpcDataArrive(QString data)
{
    // 简单处理，把主界面带到前台
    emit showWindow("main");
}
