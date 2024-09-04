#ifndef LOGINMANAGER_H
#define LOGINMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include "httpclientbase.h"

class LoginManager: public HttpClientBase
{
    Q_OBJECT

protected:
    LoginManager();

public:
    static LoginManager* getInstance();

    void login(QString name, QString password);

    QString getToken() { return m_token; }

    QString getUserId() { return m_userId; }

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

signals:
    void onLoginResult(bool success, QString errorMessage);

private:
    QString m_token;

    QString m_userId;
};

#endif // LOGINMANAGER_H
