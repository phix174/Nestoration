#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

#include "audiofile.h"
#include "toneobject.h"
#include "channelmodel.h"

using namespace std;

/*
QList<QObject *> determine_range(QList<QObject *> &tones) {
    QList<QObject *> range_tones;
    ToneObject *tone;
    int max_semitone = 0;
    int min_semitone = 999;
    for (int i = 0; i < tones.size(); i++) {
        tone = (ToneObject *)tones[i];
        if (ceil(tone->semitone_id()) > max_semitone) {
            max_semitone = ceil(tone->semitone_id());
        }
        if (floor(tone->semitone_id()) < min_semitone && floor(tone->semitone_id()) >= 0) {
            min_semitone = floor(tone->semitone_id());
        }
    }
    for (int i = max_semitone; i >= min_semitone; i--) {
        tone = new ToneObject(i, 0);
        range_tones.push_back(tone);
    }
    return range_tones;
}
*/

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    //QList<QObject *> notes_range = determine_range(tones);
    AudioFile audioFile;
    qRegisterMetaType<ChannelModel*>("ChannelModel*");
    engine.rootContext()->setContextProperty(QStringLiteral("audioFile"), &audioFile);
    engine.rootContext()->setContextObject(&audioFile);
    //engine.rootContext()->setContextProperty(QStringLiteral("toneList"), QVariant::fromValue(tones));
    //engine.rootContext()->setContextProperty(QStringLiteral("notesRange"), QVariant::fromValue(notes_range));
    //engine.rootContext()->setContextProperty(QStringLiteral("highNote"), QVariant::fromValue(notes_range.front()));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
