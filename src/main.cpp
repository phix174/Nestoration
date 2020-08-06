#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QDebug>

#include "audiofile.h"
#include "nsfaudiofile.h"
#include "player.h"
#include "channelmodel.h"

using namespace std;

QAudioFormat get_audio_output_format() {
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    QAudioFormat format;
    format.setSampleRate(1789773);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    QAudioFormat nearest = device.nearestFormat(format);
    qInfo() << device.deviceName() << "Nearest sample rate:" << nearest.sampleRate();
    return nearest;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QSurfaceFormat surface_format;
    surface_format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(surface_format);
    QAudioFormat audio_format = get_audio_output_format();
    AudioFile audioFile;
    NsfAudioFile nsf { audio_format.sampleRate() };
    Player player { audio_format };
    QObject::connect(&nsf, SIGNAL(channelRunsChanged(QList<QList<Run>>)),
                     &player, SLOT(setChannels(QList<QList<Run>>)));
    QObject::connect(&nsf, SIGNAL(GmeBufferChanged(const QByteArray)),
                     &player, SLOT(setGmeBuffer(const QByteArray)));
    qRegisterMetaType<ChannelModel*>("ChannelModel*");
    engine.rootContext()->setContextProperty("audiofile", &nsf);
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
