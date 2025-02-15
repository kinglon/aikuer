#include "avatarcontroller.h"
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QFile>
#include "settingmanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
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
    if (!m_getAvatarListSuccess && !m_isGettingAvatarList)
    {
        getAvatarListFromServer();
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
        QVector<Avatar> avatars = getAvatars();
        if (!avatars.isEmpty())
        {
            emit avatarLocalLoadCompletely(avatars);
        }

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
        if (!avatarJson.contains("_id") || !avatarJson.contains("crop_arr") || avatarJson["crop_arr"].toArray().size() == 0)
        {
            continue;
        }

        Avatar avata;
        avata.m_avatarId = avatarJson["_id"].toString();
        avata.m_avatarUrl = avatarJson["crop_arr"].toArray()[0].toString();

        QString localAvatarImagePath = m_avatarPath + avata.m_avatarId;
        QFile file(localAvatarImagePath);
        if (file.exists())
        {
            avata.m_localImagePath = localAvatarImagePath;
        }

        if (avata.isValid())
        {
            m_avatars.append(avata);
        }
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
    while (true)
    {
        if (!m_avatars[m_nextAvatarIndex].isDownloaded())
        {
            break;
        }

        m_nextAvatarIndex = (m_nextAvatarIndex+1) % m_avatars.size();
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
            for (auto& avatar : m_avatars)
            {
                if (avatar.m_avatarId == avatarId)
                {
                    avatar.m_localImagePath = localAvatarImagePath;
                    emit avatarDownloadCompletely(avatar);
                    break;
                }
            }

            downloadAvatarImage();
        }
        fileDownloader->deleteLater();
    });

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
