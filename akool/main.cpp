#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>
#include <QQmlContext>
#include "../Utility/LogUtil.h"
#include "../Utility/DumpUtil.h"
#include "../Utility/ImPath.h"
#include "settingmanager.h"
#include "ipcworker.h"
#include "maincontroller.h"
#include "memoryimageprovider.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>

extern "C" {
#include <libavutil/log.h>
}

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

// ffmpeg log callback function
void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    if (level > AV_LOG_ERROR)
    {
        return;
    }

    if (strstr(fmt, "frame dropped"))
    {
        return;
    }

    // Define the maximum log message length
    constexpr int MAX_LOG_LENGTH = 1024;
    char log_message[MAX_LOG_LENGTH];

    // Format the log message
    static int print_prefix = 1;
    av_log_format_line(ptr, level, fmt, vl, log_message, MAX_LOG_LENGTH, &print_prefix);
    qDebug(log_message);
}

bool isOpenGLSupported()
{
    QOpenGLContext ctx;
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        qInfo("Hardware OpenGL context creation failed");
        return false;
    }

    qInfo("OpenGL version: %d.%d", fmt.majorVersion(), fmt.minorVersion());

    QSurfaceFormat createdFormat = ctx.format();
    if (createdFormat.majorVersion() < 2)
    {
        qInfo("OpenGL version too low");
        return false;
    }

    return true;
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

    // 日志
    g_dllLog = CLogUtil::GetLog(L"main");
    int nLogLevel = SettingManager::getInstance()->m_nLogLevel;
    g_dllLog->SetLogLevel((ELogLevel)nLogLevel);
    originalHandler = qInstallMessageHandler(logToFile);
    av_log_set_callback(ffmpeg_log_callback);

    // 初始化崩溃转储机制
    CDumpUtil::SetDumpFilePath(CImPath::GetDumpPath().c_str());
    CDumpUtil::Enable(true);

    qputenv("QT_FONT_DPI", "100");

    QGuiApplication app(argc, argv);

    // 如果不支持opengl硬件加速，就使用opengl软件渲染
    if (!isOpenGLSupported())
    {
        qInfo() << "use software opengl";
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
    }
    qInfo("Actual render backend: %s", QQuickWindow::sceneGraphBackend().toStdString().c_str());

    QFont defaultFont("Arial");
    app.setFont(defaultFont);

    MainController* controller = new MainController();
    MemoryImageProvider memoryImageProvider;
    memoryImageProvider.setMainController(controller);

    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    engine->addImageProvider("memory", &memoryImageProvider);
    engine->rootContext()->setContextProperty("cppMainController", controller);
    const QUrl url(QStringLiteral("qrc:/content/qml/MainWindow.qml"));
    QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine->load(url);

    if (argc < 2)
    {
        qCritical("miss the launch param");
    }
    else
    {
        controller->setLaunchParam(argv[1]);
        controller->run();
    }

    return app.exec();
}
