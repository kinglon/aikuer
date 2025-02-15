#include "mainwindow.h"

#include <QApplication>

#include "Utility/LogUtil.h"
#include "Utility/DumpUtil.h"
#include "Utility/ImPath.h"
#include "settingmanager.h"
#include "loginwindow.h"
#include "meetingwindow.h"
#include "ipcworker.h"

#define IPC_KEY  "{4ED33E4A-ee3A-920A-8523-158D74420098}"

CLogUtil* g_dllLog = nullptr;

QtMessageHandler originalHandler = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (g_dllLog)
    {
        ELogLevel logLevel = ELogLevel::LOG_LEVEL_ERROR;
        if (type == QtMsgType::QtDebugMsg)
        {
            logLevel = ELogLevel::LOG_LEVEL_DEBUG;
        }
        else if (type == QtMsgType::QtInfoMsg || type == QtMsgType::QtWarningMsg)
        {
            logLevel = ELogLevel::LOG_LEVEL_INFO;
        }

        QString newMsg = msg;
        newMsg.remove(QChar('%'));
        g_dllLog->Log(context.file? context.file: "", context.line, logLevel, newMsg.toStdWString().c_str());
    }

    if (originalHandler)
    {
        (*originalHandler)(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    // 单实例
    const wchar_t* mutexName = L"{4ED33E4A-D83A-920A-8523-158D74420098}";
    HANDLE mutexHandle = CreateMutexW(nullptr, TRUE, mutexName);
    if (mutexHandle == nullptr)
    {
        return 0;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (argc > 1)
        {
            g_dllLog = CLogUtil::GetLog(L"main2");
            qInstallMessageHandler(logToFile);
            IpcWorker::sendData(IPC_KEY, argv[1]);
        }
        return 0;
    }

    g_dllLog = CLogUtil::GetLog(L"main");

    // 初始化崩溃转储机制
    CDumpUtil::SetDumpFilePath(CImPath::GetDumpPath().c_str());
    CDumpUtil::Enable(true);

    // 设置日志级别
    int nLogLevel = SettingManager::getInstance()->m_nLogLevel;
    g_dllLog->SetLogLevel((ELogLevel)nLogLevel);
    originalHandler = qInstallMessageHandler(logToFile);

    QApplication a(argc, argv);

//    LoginWindow loginWindow;
//    loginWindow.show();
//    if (loginWindow.exec() != QDialog::Accepted)
//    {
//        return 0;
//    }

//    MainWindow w;
//    w.show();

    IpcWorker ipcWorker;
    ipcWorker.setKey(IPC_KEY);
    ipcWorker.start();

    MeetingWindow meetingWindow;
    meetingWindow.show();

    return a.exec();
}
