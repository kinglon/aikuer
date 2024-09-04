#ifndef AVATAMANAGER_H
#define AVATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include "httpclientbase.h"

class Avata
{
public:
    QString m_avataId;

    QString m_cropArr;

    // avata在本地的图片文件路径
    QString m_localImagePath;
};

class AvataManager : public HttpClientBase
{
    Q_OBJECT
public:
    explicit AvataManager(QObject *parent = nullptr);

    static AvataManager* getInstance();

    void getAvataFromServer();

    QVector<Avata>* getAvatas() { return &m_avatas; }

    bool setCurrentAvata(QString avataId);

    Avata getCurrentAvata() { return m_currentAvata; }

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

private:
    void downloadAvatarImage();

signals:
    void avataChanged();

    void getAvataResult(bool success, QString errorMessage);

private:
    // avatar图片存放路径，尾部有后划线
    QString m_avatarPath;

    QVector<Avata> m_avatas;

    Avata m_currentAvata;
};

#endif // AVATAMANAGER_H
