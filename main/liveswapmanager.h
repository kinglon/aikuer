#ifndef LIVESWAPMANAGER_H
#define LIVESWAPMANAGER_H

#include "httpclientbase.h"

class LiveSwapManager : public HttpClientBase
{
    Q_OBJECT

public:
    explicit LiveSwapManager(QObject *parent = nullptr);

public:
    static LiveSwapManager* getInstance();

    void createLiveSwap();

    void updateAvatar();

    void closeLiveSwap();

    QString getPushUrl() { return m_pushUrl; }

    QString getPullUrl() { return m_pullUrl; }

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void addBearerToken(QNetworkRequest& request);

    void processCreateLiveSwapResponse(QNetworkReply *reply);

    void processUpdateAvatarResponse(QNetworkReply *reply);

    void processCloseLiveSwapResponse(QNetworkReply *reply);

signals:
    void createLiveSwapResult(bool ok, QString errorMsg);

private:
    QString m_liveSwapId;

    QString m_pushUrl;

    QString m_pullUrl;
};

#endif // LIVESWAPMANAGER_H
