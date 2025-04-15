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
    // 加载默认设置，应用程序目录下的设置
	void Load();

    // 加载数据目录下的设置，是一些可变的设置
    void Load2();

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
