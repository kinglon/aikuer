#ifndef AVATARCONTROLLER_H
#define AVATARCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include "../Utility/httpclientbase.h"

class Avatar
{
public:
    // avatar id
    QString m_avatarId;

    // avatar id，服务端用的
    QString m_avatarIdForService;

    // avatar图片url
    QString m_avatarImageUrl;

    // avatar视频url
    QString m_avatarVideoUrl;

    // avatar在本地的图片文件路径
    QString m_localImagePath;

    // avatar在本地的视频文件路径
    QString m_localVideoPath;

public:
    bool isValid() const
    {
        if (m_avatarId.isEmpty() || m_avatarIdForService.isEmpty())
        {
            return false;
        }

        return true;
    }

    bool isDownloaded() const
    {
        return !m_localImagePath.isEmpty() && !m_localVideoPath.isEmpty();
    }

    QString getLocalImageFileName() const
    {
        return m_avatarId;
    }

    QString getLocalVideoFileName() const
    {
        return m_avatarId + "_video";
    }
};

class AvatarController : public HttpClientBase
{
    Q_OBJECT

    enum {
        STEP_INIT = 1,
        STEP_GET_AVATAR_LIST = 2,
        STEP_DOWNLOAD_FILE = 3,
        STEP_FINISH = 4
    };

public:
    explicit AvatarController(QObject *parent = nullptr);

public:
    void run();

    // 获取已下载好的avatar
    QVector<Avatar> getDownloadedAvatars();

    Avatar getAvatar(const QString& avatarId);

signals:
    void runFinish();

    void avatarDownloadCompletely(const QVector<Avatar>& avatars);

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void getAvatarListFromServer();

    bool handleGetAvatarListResponse(QNetworkReply *reply);

    void downloadAvatarFile();

private slots:
    void onMainTimer();

private:
    // avatar图片存放路径，尾部有后划线
    QString m_avatarPath;

    // avatar列表
    QVector<Avatar> m_avatars;

    // 当前步骤
    int m_currentStep = STEP_INIT;

    // 标志当前步骤是否正在进行
    bool m_currentStepOnGoing = false;
};

#endif // AVATARCONTROLLER_H
