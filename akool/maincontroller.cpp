#include "maincontroller.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QCoreApplication>
#include "settingmanager.h"
#include "statusmanager.h"
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
    QString avatarId = query.queryItemValue("avatar_id");
    if (!SettingManager::getInstance()->m_loginToken.isEmpty() && !avatarId.isEmpty())
    {
        StatusManager::getInstance()->m_avatarId = "Jeric";
        StatusManager::getInstance()->m_avatarIdForService = avatarId;
    }
}

void MainController::run()
{
    if (SettingManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        return;
    }

    m_tlController.run();

    connect(&m_avatarController, &AvatarController::avatarDownloadCompletely, [this](const QVector<Avatar>& avatars) {
        QString jsonString = avatarListToJsonString(avatars);
        emit hasNewAvatar(jsonString);
    });
    m_avatarController.run();

    connect(&m_meetingController, &MeetingController::hasError, this, &MainController::showMessage);
    m_meetingController.run();

    m_ipcWorker.setKey(IPC_KEY);
    connect(&m_ipcWorker, &IpcWorker::ipcDataArrive, this, &MainController::onIpcDataArrive);
    m_ipcWorker.start();

    // 启动的时候，已经选择avatar，就开始聊天
    if (!StatusManager::getInstance()->m_avatarId.isEmpty())
    {
        m_meetingController.beginChat();
        emit chattingStatusChange(true);
    }
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

QString MainController::getSelAvatarId()
{
    return StatusManager::getInstance()->m_avatarId;
}

void MainController::setSelAvatarId(QString avatarId)
{
    QVector<Avatar> avatars = m_avatarController.getAvatars();
    for (const auto& avatar : avatars)
    {
        if (avatar.m_avatarId == avatarId)
        {
            StatusManager::getInstance()->m_avatarId = avatarId;
            StatusManager::getInstance()->m_avatarIdForService = avatar.m_avatarIdForService;
            break;
        }
    }
}

QImage* MainController::getPlayerImage()
{
    return m_meetingController.popImage();
}

bool MainController::switchMeetingMode(int meetingMode)
{
    StatusManager::getInstance()->m_currentMeetingMode = meetingMode;
    return true;
}

void MainController::enableCamera(bool enable)
{
    if (StatusManager::getInstance()->m_enableCamera == enable)
    {
        return;
    }

    StatusManager::getInstance()->m_enableCamera = enable;
    m_meetingController.enableCamera(enable);
}

void MainController::enableMicrophone(bool enable)
{
    if (StatusManager::getInstance()->m_enableMicrophone == enable)
    {
        return;
    }

    StatusManager::getInstance()->m_enableMicrophone = enable;
    m_meetingController.enableMicrophone(enable);
}

bool MainController::beginChat()
{
    int meetingMode = StatusManager::getInstance()->m_currentMeetingMode;
    if (meetingMode == MEETING_MODE_SA)
    {
        if (StatusManager::getInstance()->m_avatarId.isEmpty())
        {
            emit showMessage("Select an avatar first");
            return false;
        }
    }
    else if (meetingMode == MEETING_MODE_RTT)
    {
        if (StatusManager::getInstance()->m_sourceLanguageCode.isEmpty()
                || StatusManager::getInstance()->m_targetLanguageCode.isEmpty())
        {
            emit showMessage("Select target language first");
            return false;
        }
    }

    m_meetingController.beginChat();
    return true;
}

void MainController::stopChat()
{
    m_meetingController.stopChat();
    StatusManager::getInstance()->m_meetingEndTime = 0;
}

QString MainController::getDuration()
{
    if (StatusManager::getInstance()->m_meetingEndTime == 0)
    {
        return "00 : 00";
    }

    // UI定时获取，利用这个定时器检测是否已经结束
    qint64 now = GetTickCount64()/1000;
    if (now >= StatusManager::getInstance()->m_meetingEndTime)
    {
        stopChat();
        emit chattingStatusChange(false);
        StatusManager::getInstance()->m_meetingEndTime = 0;
        return "00 : 00";
    }

    qint64 remainTime = StatusManager::getInstance()->m_meetingEndTime - now;
    int minutes = int(remainTime / 60);
    int seconds = int(remainTime % 60);
    QString remainTimeString = QString::fromWCharArray(L"%1 : %2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    return remainTimeString;
}

QString MainController::getLanguageList(bool source)
{
    QVector<TranslateLanguage> languages;
    QString selLanguageId;
    if (source)
    {
        languages = m_tlController.getSourceLanguages();
        selLanguageId = StatusManager::getInstance()->m_sourceLanguageId;
    }
    else
    {
        languages = m_tlController.getTargetLanguages();
        selLanguageId = StatusManager::getInstance()->m_targetLanguageId;
    }

    QJsonArray languageArray;
    for (const auto& language : languages)
    {
        QJsonObject languageObject;
        languageObject["id"] = language.m_tlId;
        languageObject["language"] = language.m_languageName;
        languageObject["flagImagePath"] = QString("file:///") + language.m_localFlagImagePath;
        languageObject["isCurrent"] = selLanguageId==language.m_tlId;
        languageArray.append(languageObject);
    }
    QJsonDocument jsonDocument(languageArray);
    return QString::fromUtf8(jsonDocument.toJson());
}

void MainController::selectTranslateLanguage(QString sourceLanguageId, QString targetLanguageId)
{
    QVector<TranslateLanguage> languages = m_tlController.getSourceLanguages();
    for (const auto& language : languages)
    {
        if (language.m_tlId == sourceLanguageId)
        {
            StatusManager::getInstance()->m_sourceLanguageCode = language.m_languageCode;
            StatusManager::getInstance()->m_sourceLanguageId = sourceLanguageId;
            break;
        }
    }

    languages = m_tlController.getTargetLanguages();
    for (const auto& language : languages)
    {
        if (language.m_tlId == targetLanguageId)
        {
            StatusManager::getInstance()->m_targetLanguageCode = language.m_languageCode;
            StatusManager::getInstance()->m_targetLanguageId = targetLanguageId;
            break;
        }
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
