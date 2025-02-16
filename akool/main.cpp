#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>
#include "../Utility/LogUtil.h"
#include "../Utility/DumpUtil.h"
#include "../Utility/ImPath.h"
#include "settingmanager.h"
#include "ipcworker.h"
#include "maincontroller.h"
#include "memoryimageprovider.h"

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

    qputenv("QT_FONT_DPI", "100");

    QGuiApplication app(argc, argv);

    QFont defaultFont("Arial");
    app.setFont(defaultFont);

    MemoryImageProvider memoryImageProvider;

    QQmlApplicationEngine engine;
    engine.addImageProvider("memory", &memoryImageProvider);
    const QUrl url(QStringLiteral("qrc:/content/qml/MainWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    MainController controller;    
    if (argc < 2)
    {
        qCritical("miss the launch param");
    }
    else
    {
        controller.setLaunchParam(argv[1]);
        controller.run();
    }

    memoryImageProvider.setMainController(&controller);

    return app.exec();
}
