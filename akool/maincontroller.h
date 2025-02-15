#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <qqml.h>
#include "avatarcontroller.h"

class MainController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit MainController(QObject *parent = nullptr);

public:
    void run();

    // 设置启动参数（命令行参数）
    void setLaunchParam(const QString& launchParam);

signals:

private:
    // 选择的avatar id
    QString m_selectAvatarId;

    AvatarController avatarController;
};

#endif // MAINCONTROLLER_H
