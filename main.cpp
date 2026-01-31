#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>
#include <QQmlContext>
#include "include/datamanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Set the application icon
    QGuiApplication::setWindowIcon(QIcon(":/app.ico"));

    QQuickStyle::setStyle("Material");

    qRegisterMetaType<DataPoint>("DataPoint");
    int dataPointResult = qmlRegisterUncreatableType<DataPoint>("TLMAnalyzer", 1, 0, "DataPoint",
                                         QStringLiteral("DataPoint should not be created in QML"));
    Q_ASSERT(dataPointResult >= 0);

    QQmlApplicationEngine engine;

    // Create and expose a single DataManager instance to QML
    auto dm = new DataManager(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral("DataManager"), dm);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [](const QUrl &url) {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    const QUrl urlMain(QStringLiteral("qrc:/qml/MainUI.qml"));

    engine.load(urlMain);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QGuiApplication::exec();
}