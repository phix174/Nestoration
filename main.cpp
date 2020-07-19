#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

#include "audiofile.h"
#include "player.h"
#include "channelmodel.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    AudioFile audioFile;
    Player player;
    QObject::connect(&audioFile, SIGNAL(channelRunsChanged(QList<QList<Run>>)),
                     &player, SLOT(setChannels(QList<QList<Run>>)));
    qRegisterMetaType<ChannelModel*>("ChannelModel*");
    engine.rootContext()->setContextProperty("audiofile", &audioFile);
    engine.rootContext()->setContextProperty("player", &player);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
