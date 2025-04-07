#include "translatelanguagecontroller.h"
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

TranslateLanguageController::TranslateLanguageController(QObject *parent)
    : HttpClientBase{parent}
{
    std::wstring flagPath = CImPath::GetDataPath() + L"flag\\";
    ::CreateDirectory(flagPath.c_str(), nullptr);
    m_flagImagePath = QString::fromStdWString(flagPath);
}

void TranslateLanguageController::run()
{
    if (SettingManager::getInstance()->m_loginToken.isEmpty())
    {
        qCritical("not login");
        emit runFinish();
        return;
    }

    m_currentStep = STEP_GET_SOURCE_LANG_LIST;
    getLanguageListFromServer("3");

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TranslateLanguageController::onMainTimer);
    timer->start(1000);
}

void TranslateLanguageController::onMainTimer()
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

    // 获取语言列表失败，重试
    if (m_currentStep == STEP_GET_SOURCE_LANG_LIST && !m_currentStepOnGoing)
    {
        getLanguageListFromServer("3");
    }
    else if (m_currentStep == STEP_GET_TARGET_LANG_LIST && !m_currentStepOnGoing)
    {
        getLanguageListFromServer("4");
    }
    else if (m_currentStep == STEP_DOWNLOAD_FLAG && !m_currentStepOnGoing)
    {
        downloadFlagImage();
    }
}

void TranslateLanguageController::getLanguageListFromServer(QString from)
{
    qInfo("get language list, from=%s", from.toStdString().c_str());

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + "/api/v7/content/language/list?from=" + from);
    request.setUrl(url);
    addCommonHeader(request);

    QString bearerToken = "Bearer ";
    bearerToken += SettingManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());
    m_networkAccessManager.get(request);

    m_currentStepOnGoing = true;
}

void TranslateLanguageController::onHttpResponse(QNetworkReply *reply)
{
    m_currentStepOnGoing = false;
    bool success = handleGetLanguageListResponse(reply);
    if (!success)
    {
        return;
    }

    if (m_currentStep == STEP_GET_SOURCE_LANG_LIST)
    {
        m_currentStep = STEP_GET_TARGET_LANG_LIST;
        getLanguageListFromServer("4");
    }
    else if (m_currentStep == STEP_GET_TARGET_LANG_LIST)
    {
        m_currentStep = STEP_DOWNLOAD_FLAG;
        downloadFlagImage();
    }
}

bool TranslateLanguageController::handleGetLanguageListResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send the http request for language, error: %d", reply->error());
        return false;
    }

    QByteArray data = reply->readAll();
    if (SettingManager::getInstance()->enableDebugLog())
    {
        qDebug("get language list response: %s", QString::fromUtf8(data).toStdString().c_str());
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
        qCritical("failed to get languanges, error is %d", code);
        return false;
    }

    if (!root.contains("data"))
    {
        qCritical("failed to parse the json data, data field is missing");
        return false;
    }

    QJsonArray languageResult = root["data"].toArray();
    for (auto language : languageResult)
    {
        QJsonObject languageJson = language.toObject();
        if (!languageJson.contains("_id")
                || !languageJson.contains("lang_name")
                || !languageJson.contains("lang_code")
                || !languageJson.contains("url"))
        {
            continue;
        }

        TranslateLanguage translateLanguage;
        translateLanguage.m_tlId = languageJson["_id"].toString();
        translateLanguage.m_languageName = languageJson["lang_name"].toString();
        translateLanguage.m_languageCode = languageJson["lang_code"].toString();
        translateLanguage.m_flagUrl = languageJson["url"].toString();
        translateLanguage.m_voiceId = languageJson["voice_id"].toString();
        if (translateLanguage.isValid())
        {            
            QString localFlagImagePath = m_flagImagePath + translateLanguage.m_tlId;
            if (QFile::exists(localFlagImagePath))
            {
                translateLanguage.m_localFlagImagePath = localFlagImagePath;
            }

            if (m_currentStep == STEP_GET_SOURCE_LANG_LIST)
            {
                m_sourceLanguages.append(translateLanguage);
            }
            else {
                m_targetLanguages.append(translateLanguage);
            }
        }
    }

    return true;
}

void TranslateLanguageController::downloadFlagImage()
{
    TranslateLanguage nextLanguage;
    QVector<TranslateLanguage> languages = m_sourceLanguages;
    languages.append(m_targetLanguages);
    for (int i=0; i<languages.size(); i++)
    {
        if (!languages[i].isDownloaded())
        {
            nextLanguage = languages[i];
            break;
        }
    }

    if (!nextLanguage.isValid())
    {
        m_currentStep = STEP_FINISH;
        return;
    }

    QString localFlagImagePath = m_flagImagePath + nextLanguage.m_tlId;
    FileDownloader* fileDownloader = new FileDownloader();
    fileDownloader->setDownloadUrl(nextLanguage.m_flagUrl);
    fileDownloader->setSaveFilePath(localFlagImagePath);
    QString langId = nextLanguage.m_tlId;
    connect(fileDownloader, &FileDownloader::runFinish,
            [this, langId, localFlagImagePath, fileDownloader](bool ok) {
        if (ok)
        {
            qInfo("successful to download flag for %s", langId.toStdString().c_str());

            bool find = false;
            for (auto& language : m_sourceLanguages)
            {
                if (language.m_tlId == langId)
                {
                    language.m_localFlagImagePath = localFlagImagePath;
                    find = true;
                    break;
                }
            }

            if (!find)
            {
                for (auto& language : m_targetLanguages)
                {
                    if (language.m_tlId == langId)
                    {
                        language.m_localFlagImagePath = localFlagImagePath;
                        find = true;
                        break;
                    }
                }
            }

            if (find)
            {
                downloadFlagImage();
            }
        }
        else
        {
            qInfo("failed to download flag for %s", langId.toStdString().c_str());
            m_currentStepOnGoing = false;
        }

        fileDownloader->deleteLater();
    });

    qInfo("flag download url: %s", nextLanguage.m_flagUrl.toStdString().c_str());
    if (!fileDownloader->run())
    {
        fileDownloader->deleteLater();
    }
    else
    {
        m_currentStepOnGoing = true;
    }
}
