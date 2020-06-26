#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVector>

#include <cmath>
#include <iostream>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>

#include "toneobject.h"

using namespace std;

typedef uint8_t samplevalue;
typedef long sampleoff;
typedef long samplesize;

struct Run {
    samplevalue value;
    sampleoff start;
    samplesize length;
};

struct Cycle {
    sampleoff start;
    samplesize length;
};

const double CPU_FREQENCY = 1789773.0;
const double TWELFTH_ROOT = pow(2.0, 1.0 / 12.0);
const double C0 = 440 * pow(TWELFTH_ROOT, -57.0);
double period_to_semitone(const samplesize &period) {
    double frequency = CPU_FREQENCY / period;
    return log(frequency / C0) / log(TWELFTH_ROOT);
}

QVector<Cycle> read_cycles() {
    QVector<Cycle> cycles;
    Cycle cycle;
    fstream wavfile;
    samplesize sample_count;
    char frame[5];
    char previous_value;

    wavfile.open("/home/don/storage/code/qt/creator2/creator2/ducktales-5ch-10.wav", ios_base::in);
    wavfile.seekg(0, ios_base::end);
    sample_count = (wavfile.tellg() - (streamoff)44) / 5;
    //cout << "Sample count: " << sample_count << endl;
    wavfile.seekg(44);
    wavfile.read(frame, 5);
    previous_value = frame[0];
    cycle = { 0, 0 };
    for (sampleoff i = 1; i < sample_count; i++) {
        wavfile.read(frame, 5);
        if (frame[0] == -128 && previous_value != -128) {
            cycle.length = i - cycle.start;
            cycles.push_back(cycle);
            cycle = { i, 0 };
        }
        previous_value = frame[0];
    }
    cout << "Cycle count: " << cycles.size() << endl;
    return cycles;
}

QVector<Run> read_runs() {
    QVector<Run> runs;
    fstream wavfile("/home/don/storage/code/qt/creator2/creator2/ducktales-5ch-10.wav", ios_base::in);
    wavfile.seekg(0, ios_base::end);
    streamsize filesize = wavfile.tellg();
    cout << "File size: " << filesize << endl;
    char frame[5];
    streamoff sample_start = 44;
    wavfile.seekg(sample_start);
    wavfile.read(frame, 5);
    char previous_value = frame[0];
    streamoff previous_start = sample_start;
    uint8_t calcvalue;
    for (streamoff i = sample_start + 5; i < filesize; i += 5) {
        wavfile.read(frame, 5);
        if (frame[0] != previous_value) {
            calcvalue = (uint8_t)((previous_value + 128) << 3);
            Run run({ calcvalue, (previous_start - sample_start) / 5, (i - previous_start) / 5 });
            runs.push_back(run);
            //cout << (int)run.value << " " << run.start << " " << run.length << endl;
            //cout << i << " " << (int)previous_value << " " << i-previous_start << endl;
            previous_value = frame[0];
            previous_start = i;
        }
    }
    cout << "Run count: " << runs.size() << endl;
    return runs;
}

/*
 * Pulses:
 *
 * 1/8: _-______  20 + 20 + 120
 *
 * 1/4: _--_____  20 + 40 + 100
 *
 * 1/2: _----___  20 + 80 + 60
 *
 * 3/4: -__-----  20 + 40 + 100
 *
 */

QList<QObject *> find_tones(QVector<Cycle> &cycles) {
    QList<QObject *> tones;
    ToneObject *tone;
    sampleoff tone_start;
    tone_start = cycles[0].start;
    tone = new ToneObject(period_to_semitone(cycles[0].length), 0);
    for (int i = 1; i < cycles.size(); i++) {
        if (cycles[i].length != cycles[i-1].length) {
            tone->setLength(cycles[i].start - tone_start);
            cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
            tones.push_back(tone);
            tone_start = cycles[i].start;
            tone = new ToneObject(period_to_semitone(cycles[i].length), 0);
        }
    }
    cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
    tones.push_back(tone);
    cout << "Tone count: " << tones.size() << endl;
    return tones;
}

int main(int argc, char *argv[])
{
    /*
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    cout << "Reading cycles" << endl;
    QVector<Cycle> cycles = read_cycles();
    QList<QObject *> tones = find_tones(cycles);
    */

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QVector<Cycle> cycles = read_cycles();
    QList<QObject *> tones = find_tones(cycles);
    engine.rootContext()->setContextProperty(QStringLiteral("toneList"), QVariant::fromValue(tones));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    cout << "Reading cycles" << endl;

    return app.exec();
}
