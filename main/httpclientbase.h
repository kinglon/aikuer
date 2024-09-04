#ifndef HTTPCLIENTBASE_H
#define HTTPCLIENTBASE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

class HttpClientBase : public QObject
{
    Q_OBJECT
public:
    explicit HttpClientBase(QObject *parent = nullptr);

public:
    void setTransferTimeout(int timeOutMs) { m_networkAccessManager.setTransferTimeout(timeOutMs); }

protected:
    virtual void onHttpResponse(QNetworkReply *reply) = 0;

    void addCommonHeader(QNetworkRequest& request);

signals:

private slots:
    void onHttpFinished(QNetworkReply *reply);

protected:
    QNetworkAccessManager m_networkAccessManager;
};

#endif // HTTPCLIENTBASE_H
