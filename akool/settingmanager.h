#pragma once

#include <QString>
#include <QVector>

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();

private:
	void Load();

public:
    bool enableDebugLog() { return m_nLogLevel==1; }

public:
    int m_nLogLevel = 2;  // info

    // 后端服务host
    QString m_host;

    // Real-time Translation language code
    QString m_sourceLanguageId;
    QString m_targetLanguageId;
    QString m_sourceLanguageCode;
    QString m_targetLanguageCode;

    // 标志是否开启调试回声
    bool m_debugEcho = false;
};
