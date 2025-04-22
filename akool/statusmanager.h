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
    // 登录token
    QString m_loginToken;

    // 会议模式
    int m_currentMeetingMode = MEETING_MODE_SA;

    // 摄像头开关
    bool m_enableCamera = false;

    // 麦克风开关
    bool m_enableMicrophone = false;    

    // 选择avatar
    QString m_avatarId;
    QString m_avatarIdForService;

    // RTT使用的默认avatar
    QString m_rttAvatarId = "live-translate-placeholder";

    // 会议结束时间, GetTickCount64时间戳，单位秒
    qint64 m_meetingEndTime = 0;
};

#endif // STATUSMANAGER_H
