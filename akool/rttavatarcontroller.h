#ifndef RTTAVATARCONTROLLER_H
#define RTTAVATARCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include "../Utility/httpclientbase.h"

class RttAvatarController : public HttpClientBase
{
    Q_OBJECT

    enum {
        STEP_INIT = 1,
        STEP_GET_AVATAR_LIST = 2,
        STEP_DOWNLOAD_FILE = 3,
        STEP_FINISH = 4
    };

public:
    explicit RttAvatarController(QObject *parent = nullptr);

public:
    void run();

signals:
    void runFinish();

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void getAvatarListFromServer();

    bool handleGetAvatarListResponse(QNetworkReply *reply);

private slots:
    void onMainTimer();

private:
    // 当前步骤
    int m_currentStep = STEP_INIT;

    // 标志当前步骤是否正在进行
    bool m_currentStepOnGoing = false;
};

#endif // RTTAVATARCONTROLLER_H
