#ifndef STATUSMANAGER_H
#define STATUSMANAGER_H

#include <QString>

// 会议模式，RTT同音传译、SA StreamingAvatar、LFS Live Face Swap
#define MEETING_MODE_RTT  1
#define MEETING_MODE_SA  2
#define MEETING_MODE_LFS  3


class StatusManager
{
public:
    StatusManager();

    static StatusManager* getInstance();

public:
    // 会议模式
    int m_currentMeetingMode = MEETING_MODE_SA;

    // Real-time Translation language code
    QString m_sourceLanguageCode;
    QString m_targetLanguageCode;
};

#endif // STATUSMANAGER_H
