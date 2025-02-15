﻿#ifndef AVATARCONTROLLER_H
#define AVATARCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include "../Utility/httpclientbase.h"

class Avatar
{
public:
    QString m_avatarId;

    QString m_avatarUrl;

    // avatar在本地的图片文件路径
    QString m_localImagePath;

public:
    bool isValid() const
    {
        if (m_avatarId.isEmpty() || m_avatarUrl.isEmpty())
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

public:
    explicit AvatarController(QObject *parent = nullptr);

public:
    void run();

    // 获取已下载好的avatar
    QVector<Avatar> getAvatars();

signals:
    void runFinish();

    // 本地缓存加载的avatar
    void avatarLocalLoadCompletely(QVector<Avatar> avatars);

    void avatarDownloadCompletely(Avatar avatar);

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
