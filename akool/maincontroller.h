#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <qqml.h>
#include "avatarcontroller.h"
#include "meetingcontroller.h"
#include "ipcworker.h"

#define IPC_KEY  "{4ED33E4A-ee3A-920A-8523-158D74420098}"

class MainController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MainController(QObject *parent = nullptr);

public:
    void run();

    // 设置启动参数（命令行参数）
    void setLaunchParam(const QString& launchParam);

    // 调用者释放image
    QImage* getPlayerImage();

public: // QML调用接口
    // 获取所有可用avatars，返回JSON串
    Q_INVOKABLE QString getAvatars();

    Q_INVOKABLE QString getSelAvatarId() { return m_selectAvatarId; }
    Q_INVOKABLE void setSelAvatarId(QString avatarId) { m_selectAvatarId = avatarId; }

    Q_INVOKABLE void beginChat();

    Q_INVOKABLE void quitApp();

signals:
    // 下载好新avatar，JSON串
    void hasNewAvatar(QString avatarJsonString);

    // 显示窗口
    void showWindow(QString name);

    // 显示消息
    void showMessage(QString message);

private slots:
    void onIpcDataArrive(QString data);

private:
    QString avatarListToJsonString(const QVector<Avatar>& avatars);

private:
    // 选择的avatar id
    QString m_selectAvatarId;

    AvatarController m_avatarController;

    MeetingController m_meetingController;

    IpcWorker m_ipcWorker;
};

#endif // MAINCONTROLLER_H
