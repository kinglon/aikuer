#include "avatarcontroller.h"
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QFile>
#include "statusmanager.h"
#include "settingmanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QFileInfo>
#include "../Utility/ImPath.h"
#include "filedownloader.h"

AvatarController::AvatarController(QObject *parent)
    : HttpClientBase{parent}
{
    std::wstring avatarPath = CImPath::GetDataPath() + L"avatar\\";
    ::CreateDirectory(avatarPath.c_str(), nullptr);
    m_avatarPath = QString::fromStdWString(avatarPath);
}

void AvatarController::run()
{
    if (StatusManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        emit runFinish();
        return;
    }

    m_currentStep = STEP_GET_AVATAR_LIST;
    getAvatarListFromServer();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AvatarController::onMainTimer);
    timer->start(1000);
}

QVector<Avatar> AvatarController::getDownloadedAvatars()
{
    QVector<Avatar> avatars;
    for (const auto& avatar : m_avatars)
    {
        if (avatar.isDownloaded())
        {
            avatars.append(avatar);
        }
    }

    return avatars;
}

Avatar AvatarController::getAvatar(const QString& avatarId)
{
    for (const auto& item : m_avatars)
    {
        if (item.m_avatarId == avatarId)
        {
            return item;
        }
    }

    return Avatar();
}

void AvatarController::onMainTimer()
{
    if (m_currentStep == STEP_FINISH)
    {
        auto timer = qobject_cast<QTimer *>(sender());
        if (timer)
        {
            timer->stop();
        }

        emit runFinish();
    }

    // 失败重试
    if (m_currentStep == STEP_GET_AVATAR_LIST && !m_currentStepOnGoing)
    {
        getAvatarListFromServer();
    }
    else if (m_currentStep == STEP_DOWNLOAD_FILE && !m_currentStepOnGoing)
    {
        downloadAvatarFile();
    }
}

void AvatarController::getAvatarListFromServer()
{
    qInfo("get avatar list from server");

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + "/api/v6/content/avatar/list?from[]=2&from[]=3&from[]=4&size=100&type=2");
    request.setUrl(url);
    addCommonHeader(request);

    QString bearerToken = "Bearer ";
    bearerToken += StatusManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());
    m_networkAccessManager.get(request);

    m_currentStepOnGoing = true;
}

void AvatarController::onHttpResponse(QNetworkReply *reply)
{
    m_currentStepOnGoing = false;
    if (!handleGetAvatarListResponse(reply))
    {
        return;
    }

    if (m_currentStep == STEP_GET_AVATAR_LIST)
    {
        m_currentStep = STEP_DOWNLOAD_FILE;
        downloadAvatarFile();
    }
}

bool AvatarController::handleGetAvatarListResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send the http request for avata, error: %d", reply->error());
        return false;
    }

    QByteArray data = reply->readAll();
    if (SettingManager::getInstance()->enableDebugLog())
    {
        qDebug("get avatar list response: %s", QString::fromUtf8(data).toStdString().c_str());
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        return false;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        return false;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to get avata, error is %d", code);
        return false;
    }

    if (!root.contains("data") || !root["data"].toObject().contains("result"))
    {
        qCritical("failed to parse the json data, result field is missing");
        return false;
    }

    m_avatars.clear();
    QJsonArray avatar_result = root["data"].toObject()["result"].toArray();
    for (auto avatar : avatar_result)
    {
        QJsonObject avatarJson = avatar.toObject();
        if (!avatarJson.contains("_id") || !avatarJson.contains("avatar_id")
                || !avatarJson.contains("thumbnailUrl") || !avatarJson.contains("url"))
        {
            continue;
        }

        Avatar avata;
        avata.m_avatarId = avatarJson["_id"].toString();
        avata.m_avatarIdForService = avatarJson["avatar_id"].toString();
        avata.m_avatarImageUrl = avatarJson["thumbnailUrl"].toString();
        avata.m_avatarVideoUrl = avatarJson["url"].toString();
        if (avata.isValid())
        {            
            QString localAvatarImagePath = m_avatarPath + avata.getLocalImageFileName();
            if (QFile::exists(localAvatarImagePath))
            {
                avata.m_localImagePath = localAvatarImagePath;
            }

            QString localAvatarVideoPath = m_avatarPath + avata.getLocalVideoFileName();
            if (QFile::exists(localAvatarVideoPath))
            {
                avata.m_localVideoPath = localAvatarVideoPath;
            }

            m_avatars.append(avata);
        }
    }    

    QVector<Avatar> localAvatars;
    for (const auto& avatar : m_avatars)
    {
        if (avatar.isDownloaded())
        {
            localAvatars.append(avatar);
        }
    }

    qInfo("avatar total count: %d, local cache count: %d", m_avatars.size(), localAvatars.size());
    if (!localAvatars.empty())
    {
        emit avatarDownloadCompletely(localAvatars);
    }

    return true;
}

void AvatarController::downloadAvatarFile()
{
    Avatar nextAvatar;
    for (const auto& avatar : m_avatars)
    {
        if (!avatar.isDownloaded())
        {
            nextAvatar = avatar;
            break;
        }
    }
    if (!nextAvatar.isValid())
    {
        m_currentStep = STEP_FINISH;
        return;
    }

    bool downloadImage = false;
    QString downloadUrl;
    QString localDownloadPath;
    if (nextAvatar.m_localImagePath.isEmpty())
    {
        downloadImage = true;
        downloadUrl = nextAvatar.m_avatarImageUrl;
        localDownloadPath = m_avatarPath + nextAvatar.getLocalImageFileName();
    }
    else
    {
        downloadImage = false;
        downloadUrl = nextAvatar.m_avatarVideoUrl;
        localDownloadPath = m_avatarPath + nextAvatar.getLocalVideoFileName();
    }

    FileDownloader* fileDownloader = new FileDownloader();
    fileDownloader->setDownloadUrl(downloadUrl);
    fileDownloader->setSaveFilePath(localDownloadPath);
    QString avatarId = nextAvatar.m_avatarId;
    connect(fileDownloader, &FileDownloader::runFinish,
            [this, avatarId, downloadImage, localDownloadPath, fileDownloader](bool ok) {
        if (ok)
        {
            if (downloadImage)
            {
                qInfo("successful to download avatar image for %s", avatarId.toStdString().c_str());
            }
            else
            {
                qInfo("successful to download avatar video for %s", avatarId.toStdString().c_str());
            }

            for (auto& avatar : m_avatars)
            {
                if (avatar.m_avatarId == avatarId)
                {
                    if (downloadImage)
                    {
                        avatar.m_localImagePath = localDownloadPath;
                    }
                    else
                    {
                        avatar.m_localVideoPath = localDownloadPath;
                    }

                    if (avatar.isDownloaded())
                    {
                        QVector<Avatar> avatars;
                        avatars.append(avatar);
                        emit avatarDownloadCompletely(avatars);
                    }

                    break;
                }
            }

            downloadAvatarFile();
        }
        else
        {
            qInfo("failed to download avatar file for %s", avatarId.toStdString().c_str());
            m_currentStepOnGoing = false;
        }

        fileDownloader->deleteLater();
    });

    qInfo("avatar download url: %s", downloadUrl.toStdString().c_str());
    if (!fileDownloader->run())
    {
        fileDownloader->deleteLater();
    }
    else
    {
        m_currentStepOnGoing = true;
    }
}
