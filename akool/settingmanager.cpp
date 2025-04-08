#include "settingmanager.h"
#include <QFile>
#include "../Utility/ImPath.h"
#include "../Utility/ImCharset.h"
#include "../Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SettingManager::SettingManager()
{
    Load();
}

SettingManager* SettingManager::getInstance()
{
    static SettingManager* pInstance = new SettingManager();
	return pInstance;
}

void SettingManager::Load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";    
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the basic configure file : %s", strConfFilePath.c_str());
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject root = jsonDocument.object();
    m_nLogLevel = root["log_level"].toInt();
    m_host = root["host"].toString();
    m_sourceLanguageId = root["sourceLanguageId"].toString();
    m_targetLanguageId = root["targetLanguageId"].toString();
    m_sourceLanguageCode = root["sourceLanguageCode"].toString();
    m_targetLanguageCode = root["targetLanguageCode"].toString();
}

void SettingManager::save()
{
    QJsonObject root;
    root["log_level"] = m_nLogLevel;
    root["host"] = m_host;
    root["sourceLanguageId"] = m_sourceLanguageId;
    root["targetLanguageId"] = m_targetLanguageId;
    root["sourceLanguageCode"] = m_sourceLanguageCode;
    root["targetLanguageCode"] = m_targetLanguageCode;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}

