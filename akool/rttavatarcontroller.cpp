#include "rttavatarcontroller.h"
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

RttAvatarController::RttAvatarController(QObject *parent)
    : HttpClientBase{parent}
{

}

void RttAvatarController::run()
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
    connect(timer, &QTimer::timeout, this, &RttAvatarController::onMainTimer);
    timer->start(1000);
}

void RttAvatarController::onMainTimer()
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
}

void RttAvatarController::getAvatarListFromServer()
{
    qInfo("get rtt avatar list from server");

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + "/api/v6/content/avatar/list?from[]=2&from[]=3&from[]=4&size=100&type=4");
    request.setUrl(url);
    addCommonHeader(request);

    QString bearerToken = "Bearer ";
    bearerToken += StatusManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());
    m_networkAccessManager.get(request);

    m_currentStepOnGoing = true;
}

void RttAvatarController::onHttpResponse(QNetworkReply *reply)
{
    m_currentStepOnGoing = false;
    if (!handleGetAvatarListResponse(reply))
    {
        return;
    }

    m_currentStep = STEP_FINISH;
}

bool RttAvatarController::handleGetAvatarListResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send the http request for rtt avatar, error: %d", reply->error());
        return false;
    }

    QByteArray data = reply->readAll();
    if (SettingManager::getInstance()->enableDebugLog())
    {
        qDebug("get rtt avatar list response: %s", QString::fromUtf8(data).toStdString().c_str());
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
        qCritical("failed to get rtt avatar, error is %d", code);
        return false;
    }

    if (!root.contains("data") || !root["data"].toObject().contains("result"))
    {
        qCritical("failed to parse the json data, result field is missing");
        return false;
    }

    QJsonArray avatar_result = root["data"].toObject()["result"].toArray();
    for (auto avatar : avatar_result)
    {
        QJsonObject avatarJson = avatar.toObject();
        if (!avatarJson.contains("avatar_id"))
        {
            continue;
        }

        StatusManager::getInstance()->m_rttAvatarId = avatarJson["avatar_id"].toString();
        qInfo("rtt avatar is %s", StatusManager::getInstance()->m_rttAvatarId.toStdString().c_str());
        break;
    }

    return true;
}
