#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFont>


int main(int argc, char *argv[])
{
    qputenv("QT_FONT_DPI", "100");

    QGuiApplication app(argc, argv);

    QFont defaultFont("Arial");
    app.setFont(defaultFont);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/content/qml/MainWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
