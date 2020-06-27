#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVector>

#include <cmath>
#include <iostream>
#include <fstream>

#include "audiofile.h"
#include "toneobject.h"

using namespace std;

typedef uint8_t samplevalue;
typedef long sampleoff;
typedef long samplesize;

struct Cycle {
    sampleoff start;
    samplesize length;
};

const double CPU_FREQENCY = 1789773.0;
const double TWELFTH_ROOT = pow(2.0, 1.0 / 12.0);
const double A0 = 440.0 / pow(2, 4);
const double C0 = A0 * pow(TWELFTH_ROOT, -9.0);
double period_to_semitone(const samplesize &period) {
    double frequency = CPU_FREQENCY / period;
    return log(frequency / C0) / log(TWELFTH_ROOT);
}

QVector<Cycle> read_cycles() {
    QVector<Cycle> cycles;
    Cycle cycle;
    AudioFile audio_file;
    try {
        audio_file.open("/home/don/storage/code/qt/creator2/creator2/ducktales-5ch-10.wav.gz");
    } catch (int e) {
        cout << "Failed to open WAV file." << endl;
        return cycles;
    }
    char *block = new char[1789773 * 5];
    streamsize bytes_read = 0;
    sampleoff file_sample_i = 0;
    streamoff block_i = 0;
    char previous_value;

    audio_file.read_block(block, bytes_read);
    if (bytes_read == 0) {
        return cycles;
    }
    previous_value = block[block_i];
    cycle = { 0, 0 };
    file_sample_i += 1;
    block_i += 5;
    while (bytes_read) {
        while (block_i < bytes_read) {
            if (block[block_i] == -128 && previous_value != -128) {
                cycle.length = file_sample_i - cycle.start;
                cycles.push_back(cycle);
                cycle = { file_sample_i, 0 };
            }
            previous_value = block[block_i];
            file_sample_i += 1;
            block_i += 5;
        }
        block_i = 0;
        audio_file.read_block(block, bytes_read);
    }
    cycle.length = file_sample_i - cycle.start;
    cycles.push_back(cycle);
    delete[] block;
    audio_file.close();
    cout << "Cycle count: " << cycles.size() << endl;
    return cycles;
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
    if (cycles.length() == 0) return tones;
    ToneObject *tone;
    sampleoff tone_start;
    tone_start = cycles[0].start;
    tone = new ToneObject(period_to_semitone(cycles[0].length), 0);
    for (int i = 1; i < cycles.size(); i++) {
        if (cycles[i].length != cycles[i-1].length) {
            tone->setLength(cycles[i].start - tone_start);
            //cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
            tones.push_back(tone);
            tone_start = cycles[i].start;
            tone = new ToneObject(period_to_semitone(cycles[i].length), 0);
        }
    }
    //cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
    tones.push_back(tone);
    cout << "Tone count: " << tones.size() << endl;
    return tones;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    cout << "Reading cycles..." << endl;
    QVector<Cycle> cycles = read_cycles();
    cout << "Finding tones..." << endl;
    QList<QObject *> tones = find_tones(cycles);
    engine.rootContext()->setContextProperty(QStringLiteral("toneList"), QVariant::fromValue(tones));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
