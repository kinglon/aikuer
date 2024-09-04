#include "avatamanager.h"
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QFile>
#include "settingmanager.h"
#include "loginmanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "Utility/ImPath.h"
#include "filedownloader.h"

AvataManager::AvataManager(QObject *parent)
    : HttpClientBase{parent}
{
    std::wstring avatarPath = CImPath::GetDataPath() + L"avatar\\";
    ::CreateDirectory(avatarPath.c_str(), nullptr);
    m_avatarPath = QString::fromStdWString(avatarPath);
}

AvataManager* AvataManager::getInstance()
{
    static AvataManager* instance = new AvataManager();
    return instance;
}

void AvataManager::getAvataFromServer()
{
    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_faceSwapHost + "/api/v2/faceswap/demo/list?userId="
             + LoginManager::getInstance()->getUserId());
    request.setUrl(url);
    addCommonHeader(request);

    QString bearerToken = "Bearer ";
    bearerToken += LoginManager::getInstance()->getToken();
    request.setRawHeader("Authorization", bearerToken.toUtf8());
    m_networkAccessManager.get(request);
}

bool AvataManager::setCurrentAvata(QString avataId)
{
    for (auto& avata : m_avatas)
    {
        if (avataId == avata.m_avataId)
        {
            m_currentAvata = avata;
            return true;
        }
    }

    return false;
}


void AvataManager::onHttpResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send the http request for avata, error: %d", reply->error());
        emit getAvataResult(false, QString::fromWCharArray(L"获取头像列表失败"));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        emit getAvataResult(false, QString::fromWCharArray(L"获取头像列表失败"));
        return;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        emit getAvataResult(false, QString::fromWCharArray(L"获取头像列表失败"));
        return;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to get avata, error is %d", code);
        emit getAvataResult(false, QString::fromWCharArray(L"获取头像列表失败，错误码是：%1").arg(code));
        return;
    }

    if (!root.contains("data") || !root["data"].toObject().contains("avatar_result"))
    {
        qCritical("failed to parse the json data, avatar_result field is missing");
        emit getAvataResult(false, QString::fromWCharArray(L"获取头像列表失败，错误码是：%1").arg(code));
        return;
    }

    m_avatas.clear();
    QJsonArray avatar_result = root["data"].toObject()["avatar_result"].toArray();
    for (auto avatar : avatar_result)
    {
        QJsonObject avatarJson = avatar.toObject();
        if (!avatarJson.contains("_id") || !avatarJson.contains("crop_arr") || avatarJson["crop_arr"].toArray().size() == 0)
        {
            continue;
        }

        Avata avata;
        avata.m_avataId = avatarJson["_id"].toString();
        avata.m_cropArr = avatarJson["crop_arr"].toArray()[0].toString();

        QString localAvatarImagePath = m_avatarPath + avata.m_avataId;
        QFile file(localAvatarImagePath);
        if (file.exists())
        {
            avata.m_localImagePath = localAvatarImagePath;
        }
        m_avatas.append(avata);
    }

    downloadAvatarImage();

    emit avataChanged();
}


void AvataManager::downloadAvatarImage()
{
    for (auto& avatar : m_avatas)
    {
        QString localAvatarImagePath = m_avatarPath + avatar.m_avataId;
        QFile file(localAvatarImagePath);
        if (file.exists())
        {
            continue;
        }

        FileDownloader* fileDownloader = new FileDownloader();
        fileDownloader->setDownloadUrl(avatar.m_cropArr);
        fileDownloader->setSaveFilePath(localAvatarImagePath);
        QString avatarId = avatar.m_avataId;
        connect(fileDownloader, &FileDownloader::runFinish,
                [this, avatarId, localAvatarImagePath, fileDownloader](bool ok) {
            if (ok)
            {
                for (auto& avatar : m_avatas)
                {
                    if (avatar.m_avataId == avatarId)
                    {
                        avatar.m_localImagePath = localAvatarImagePath;
                        break;
                    }
                }
                emit avataChanged();

                downloadAvatarImage();
            }
            fileDownloader->deleteLater();
        });
        if (!fileDownloader->run())
        {
            fileDownloader->deleteLater();
        }

        break;
    }
}
