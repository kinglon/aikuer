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
        return !m_localImagePath.isEmpty();
    }
};

class AvatarController : public HttpClientBase
{
    Q_OBJECT

    enum {
        STEP_INIT = 1,
        STEP_GET_AVATAR_LIST = 2,
    };

public:
    explicit AvatarController(QObject *parent = nullptr);

public:
    void run();

    // 获取已下载好的avatar
    QVector<Avatar> getAvatars();

signals:
    void runFinish();

    void avatarDownloadCompletely(const QVector<Avatar>& avatars);

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void getAvatarListFromServer();

    bool handleGetAvatarListResponse(QNetworkReply *reply);

    void downloadAvatarImage();

    // 是否所有的avatar都下载了
    bool isAvatarDownloadFinish();

private slots:
    void onMainTimer();

private:
    // avatar图片存放路径，尾部有后划线
    QString m_avatarPath;

    QVector<Avatar> m_avatars;

    // 获取avatar列表的状态
    bool m_isGettingAvatarList = false;
    bool m_getAvatarListSuccess = false;

    // 下一个下载的索引
    int m_nextAvatarIndex = 0;
};

#endif // AVATARCONTROLLER_H
