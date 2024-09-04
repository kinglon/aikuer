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
    int m_nLogLevel = 2;  // debug

    QString m_loginHost;

    QString m_faceSwapHost;
};