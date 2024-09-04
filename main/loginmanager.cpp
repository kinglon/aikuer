#include "loginmanager.h"
#include <QNetworkProxy>
#include "settingmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>

LoginManager::LoginManager() : HttpClientBase(nullptr)
{

}

LoginManager* LoginManager::getInstance()
{
    static LoginManager* instance = new LoginManager();
    return instance;
}

void LoginManager::login(QString name, QString password)
{
    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_loginHost + "/api/v1/public/login");
    request.setUrl(url);
    addCommonHeader(request);

    QJsonObject body;
    body["email"] = name;
    body["password"] = password;

    QJsonDocument jsonDocument(body);
    QByteArray jsonData = jsonDocument.toJson();
    m_networkAccessManager.post(request, jsonData);
}

void LoginManager::onHttpResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send the http request for login, error: %d", reply->error());
        emit onLoginResult(false, QString::fromWCharArray(L"登录失败"));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        emit onLoginResult(false, QString::fromWCharArray(L"登录失败"));
        return;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        emit onLoginResult(false, QString::fromWCharArray(L"登录失败"));
        return;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to login, error is %d", code);
        if (code == 1003 || code == 1004)
        {
            emit onLoginResult(false, QString::fromWCharArray(L"登录失败(用户名或密码错误)"));
        }
        else
        {
            emit onLoginResult(false, QString::fromWCharArray(L"登录失败(code=%1)").arg(code));
        }
        return;
    }

    if (!root.contains("token") || !root.contains("user") || !root["user"].toObject().contains("_id"))
    {
        qCritical("failed to parse the json data, some required field is missing");
        emit onLoginResult(false, QString::fromWCharArray(L"登录失败"));
        return;
    }

    m_token = root["token"].toString();
    m_userId = root["user"].toObject()["_id"].toString();
    emit onLoginResult(true, "");
}
