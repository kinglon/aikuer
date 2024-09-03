#ifndef LOGINMANAGER_H
#define LOGINMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

class LoginManager: public QObject
{
    Q_OBJECT

protected:
    LoginManager();

public:
    static LoginManager* getInstance();

    void login(QString name, QString password);

    QString getToken() { return m_token; }

    QString getUserId() { return m_userId; }

private:
    void addCommonHeader(QNetworkRequest& request);

signals:
    void onLoginResult(bool success, QString errorMessage);

private slots:
    void onHttpFinished(QNetworkReply *reply);

private:
    static QNetworkAccessManager *m_networkAccessManager;

    QString m_token;

    QString m_userId;
};

#endif // LOGINMANAGER_H
