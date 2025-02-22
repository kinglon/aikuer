﻿#pragma once

#include <QString>
#include <QVector>

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

private:
	void Load();

public:
    bool enableDebugLog() { return m_nLogLevel==1; }

public:
    int m_nLogLevel = 2;  // info

    // 后端服务host
    QString m_host;

    // 登录token
    QString m_loginToken;
};
