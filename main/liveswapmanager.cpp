#include "liveswapmanager.h"
#include "avatamanager.h"
#include "settingmanager.h"
#include "loginmanager.h"
#include <QJsonObject>
#include <QJsonDocument>

// 创建liveswap接口的URI
#define URI_CREATE_LIVESWAP     "/api/v2/faceswap/web/liveswap/create"

// 更新avatar接口的URI
#define URI_UPDATE_AVATAR       "/api/v2/faceswap/web/liveswap/update"

// 关闭liveswap接口的URI
#define URI_CLOSE_LIVESWAP       "/api/v2/faceswap/web/liveswap/close"

LiveSwapManager::LiveSwapManager(QObject *parent)
    : HttpClientBase{parent}
{

}

LiveSwapManager* LiveSwapManager::getInstance()
{
    static LiveSwapManager* instance = new LiveSwapManager();
    return instance;
}

void LiveSwapManager::createLiveSwap()
{
    Avata avatar = AvataManager::getInstance()->getCurrentAvata();
    if (avatar.m_avataId.isEmpty())
    {
        emit createLiveSwapResult(false, QString::fromWCharArray(L"没有选择头像"));
        return;
    }

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_faceSwapHost + URI_CREATE_LIVESWAP);
    request.setUrl(url);
    addCommonHeader(request);
    addBearerToken(request);

    QJsonObject body;
    body["stream_type"] = "rtmp";
    body["source_id"] = avatar.m_avataId;
    body["source_url"] = avatar.m_cropArr;
    body["userId"] = LoginManager::getInstance()->getUserId();

    QJsonDocument jsonDocument(body);
    QByteArray jsonData = jsonDocument.toJson();
    m_networkAccessManager.post(request, jsonData);
}

void LiveSwapManager::updateAvatar()
{
    if (m_liveSwapId.isEmpty())
    {
        return;
    }

    Avata avatar = AvataManager::getInstance()->getCurrentAvata();
    if (avatar.m_avataId.isEmpty())
    {
        qCritical("failed to update avatar, since not select avatar");
        return;
    }

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_faceSwapHost + URI_UPDATE_AVATAR);
    request.setUrl(url);
    addCommonHeader(request);
    addBearerToken(request);

    QJsonObject body;
    body["_id"] = m_liveSwapId;
    body["source_id"] = avatar.m_avataId;
    body["source_url"] = avatar.m_cropArr;
    body["userId"] = LoginManager::getInstance()->getUserId();

    QJsonDocument jsonDocument(body);
    QByteArray jsonData = jsonDocument.toJson();
    m_networkAccessManager.post(request, jsonData);
}

void LiveSwapManager::closeLiveSwap()
{
    if (m_liveSwapId.isEmpty())
    {
        return;
    }

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_faceSwapHost + URI_CLOSE_LIVESWAP);
    request.setUrl(url);
    addCommonHeader(request);
    addBearerToken(request);

    QJsonObject body;
    body["_id"] = m_liveSwapId;
    body["userId"] = LoginManager::getInstance()->getUserId();

    QJsonDocument jsonDocument(body);
    QByteArray jsonData = jsonDocument.toJson();
    m_networkAccessManager.post(request, jsonData);

    m_liveSwapId = "";
    m_pushUrl = "";
    m_pullUrl = "";
}

void LiveSwapManager::onHttpResponse(QNetworkReply *reply)
{
    QString url = reply->request().url().toString();
    if (url.indexOf(URI_CREATE_LIVESWAP) >= 0)
    {
        processCreateLiveSwapResponse(reply);
    }
    else if (url.indexOf(URI_UPDATE_AVATAR) >= 0)
    {
        processUpdateAvatarResponse(reply);
    }
    else if (url.indexOf(URI_CLOSE_LIVESWAP) >= 0)
    {
        processCloseLiveSwapResponse(reply);
    }
}

void LiveSwapManager::addBearerToken(QNetworkRequest& request)
{
    QString bearerToken = "Bearer ";
    bearerToken += LoginManager::getInstance()->getToken();
    request.setRawHeader("Authorization", bearerToken.toUtf8());
}

void LiveSwapManager::processCreateLiveSwapResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to create live swap instance, error: %d", reply->error());
        emit createLiveSwapResult(false, QString::fromWCharArray(L"开启LiveSwap失败"));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        emit createLiveSwapResult(false, QString::fromWCharArray(L"开启LiveSwap失败"));
        return;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        emit createLiveSwapResult(false, QString::fromWCharArray(L"开启LiveSwap失败"));
        return;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to create liveswap instance, error is %d", code);
        emit createLiveSwapResult(false, QString::fromWCharArray(L"开启LiveSwap失败，错误码是%1").arg(code));
        return;
    }

    if (!root.contains("data") || !root["data"].toObject().contains("_id")
            || !root["data"].toObject().contains("cl_push_url")
            || !root["data"].toObject().contains("cl_pull_url"))
    {
        qCritical("failed to parse the json data, field is missing");
        emit createLiveSwapResult(false, QString::fromWCharArray(L"开启LiveSwap失败"));
        return;
    }

    QJsonObject dataJson = root["data"].toObject();
    m_liveSwapId = dataJson["_id"].toString();
    m_pushUrl = dataJson["cl_push_url"].toString();
    m_pullUrl = dataJson["cl_pull_url"].toString();
    qInfo("push url: %s", m_pushUrl.toStdString().c_str());
    qInfo("pull url: %s", m_pullUrl.toStdString().c_str());

    emit createLiveSwapResult(true, "");
}

void LiveSwapManager::processUpdateAvatarResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to update avatar, error: %d", reply->error());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        return;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        return;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to update avatar, error is %d", code);
        return;
    }

    qInfo("successful to update avatar");
}

void LiveSwapManager::processCloseLiveSwapResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to close liveswap, error: %d", reply->error());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        return;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        return;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to close liveswap, error is %d", code);
        return;
    }

    qInfo("successful to close liveswap");
}
