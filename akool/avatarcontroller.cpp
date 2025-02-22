﻿#include "avatarcontroller.h"
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QFile>
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
    if (SettingManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        emit runFinish();
        return;
    }

    getAvatarListFromServer();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AvatarController::onMainTimer);
    timer->start(1000);
}

QVector<Avatar> AvatarController::getAvatars()
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

void AvatarController::onMainTimer()
{
    if (!m_getAvatarListSuccess)
    {
        if (!m_isGettingAvatarList)
        {
            getAvatarListFromServer();
        }

        return;
    }

    if (!isAvatarDownloadFinish())
    {
        return;
    }

    auto timer = qobject_cast<QTimer *>(sender());
    if (timer)
    {
        timer->stop();
    }

    emit runFinish();
}

void AvatarController::getAvatarListFromServer()
{
    qInfo("get avatar list from server");

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + "/api/v6/content/avatar/list?from[]=2&from[]=3&from[]=4&size=100&type=2");
    request.setUrl(url);
    addCommonHeader(request);

    QString bearerToken = "Bearer ";
    bearerToken += SettingManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());
    m_networkAccessManager.get(request);

    m_isGettingAvatarList = true;
}

void AvatarController::onHttpResponse(QNetworkReply *reply)
{
    m_getAvatarListSuccess = handleGetAvatarListResponse(reply);
    m_isGettingAvatarList = false;

    if (m_getAvatarListSuccess)
    {
        m_nextAvatarIndex = 0;
        downloadAvatarImage();
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
        if (!avatarJson.contains("_id") || !avatarJson.contains("avatar_id") || !avatarJson.contains("thumbnailUrl"))
        {
            continue;
        }

        Avatar avata;
        avata.m_avatarId = avatarJson["_id"].toString();
        avata.m_avatarIdForService = avatarJson["avatar_id"].toString();
        avata.m_avatarUrl = avatarJson["thumbnailUrl"].toString();

        QUrl qUrl(avata.m_avatarUrl);
        QString path = qUrl.path(); // Get the path part of the URL
        QFileInfo fileInfo(path);
        if (avata.isValid())
        {            
            QString localAvatarImagePath = m_avatarPath + avata.m_avatarId;
            if (QFile::exists(localAvatarImagePath))
            {
                avata.m_localImagePath = localAvatarImagePath;
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

void AvatarController::downloadAvatarImage()
{
    if (m_avatars.empty())
    {
        return;
    }

    if (isAvatarDownloadFinish())
    {
        return;
    }

    // 获取下一个待下载索引
    for (; m_nextAvatarIndex < m_avatars.size(); m_nextAvatarIndex++)
    {
        if (!m_avatars[m_nextAvatarIndex].isDownloaded())
        {
            break;
        }
    }
    if (m_nextAvatarIndex >= m_avatars.size())
    {
        return;
    }

    Avatar avatar = m_avatars[m_nextAvatarIndex];
    QString localAvatarImagePath = m_avatarPath + avatar.m_avatarId;

    FileDownloader* fileDownloader = new FileDownloader();
    fileDownloader->setDownloadUrl(avatar.m_avatarUrl);
    fileDownloader->setSaveFilePath(localAvatarImagePath);
    QString avatarId = avatar.m_avatarId;
    connect(fileDownloader, &FileDownloader::runFinish,
            [this, avatarId, localAvatarImagePath, fileDownloader](bool ok) {
        if (ok)
        {
            qInfo("successful to download avatar for %s", avatarId.toStdString().c_str());

            for (auto& avatar : m_avatars)
            {
                if (avatar.m_avatarId == avatarId)
                {
                    avatar.m_localImagePath = localAvatarImagePath;
                    QVector<Avatar> avatars;
                    avatars.append(avatar);
                    emit avatarDownloadCompletely(avatars);
                    break;
                }
            }
        }
        else
        {
            qInfo("failed to download avatar for %s", avatarId.toStdString().c_str());
        }

        m_nextAvatarIndex++;
        downloadAvatarImage();

        fileDownloader->deleteLater();
    });

    qInfo("avatar download url: %s", avatar.m_avatarUrl.toStdString().c_str());
    if (!fileDownloader->run())
    {
        fileDownloader->deleteLater();
    }
}

bool AvatarController::isAvatarDownloadFinish()
{
    bool finish = true;
    for (auto& avatar : m_avatars)
    {
        if (!avatar.isDownloaded())
        {
            finish = false;
            break;
        }
    }

    return finish;
}
