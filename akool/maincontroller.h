#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <qqml.h>
#include "avatarcontroller.h"
#include "meetingcontroller.h"
#include "translatelanguagecontroller.h"
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

    MeetingController& getMeetingController() { return m_meetingController; }
    AvatarController& getAvatarController() { return m_avatarController; }

public: // QML调用接口
    // 获取所有可用avatars，返回JSON串
    Q_INVOKABLE QString getAvatars();
    Q_INVOKABLE QString getSelAvatarId();
    Q_INVOKABLE void setSelAvatarId(QString avatarId);

    Q_INVOKABLE bool switchMeetingMode(int meetingMode);

    Q_INVOKABLE void enableCamera(bool enable);
    Q_INVOKABLE void enableMicrophone(bool enable);

    Q_INVOKABLE bool beginChat();
    Q_INVOKABLE void stopChat();

    Q_INVOKABLE QString getDuration();

    // 获取所有语言，返回JSON串
    Q_INVOKABLE QString getLanguageList(bool source);
    Q_INVOKABLE void selectTranslateLanguage(QString sourceLanguageId, QString targetLanguageId);

    Q_INVOKABLE void quitApp();

signals:
    // 下载好新avatar，JSON串
    void hasNewAvatar(QString avatarJsonString);

    // 显示窗口
    void showWindow(QString name);

    // 显示消息
    void showMessage(QString message);

    // 聊天状态变化
    void chattingStatusChange(bool isChatting);

private slots:
    void onIpcDataArrive(QString data);

private:
    QString avatarListToJsonString(const QVector<Avatar>& avatars);

private:
    AvatarController m_avatarController;

    TranslateLanguageController m_tlController;

    MeetingController m_meetingController;

    IpcWorker m_ipcWorker;
};

#endif // MAINCONTROLLER_H
