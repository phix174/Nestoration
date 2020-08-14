#ifndef TONEOBJECT_H
#define TONEOBJECT_H

#include <cmath>

#include <QStringList>
#include <QVector>

typedef uint8_t samplevalue;
typedef long sampleoff;
typedef long samplesize;

const double CPU_FREQENCY = 1789773.0;
const double TWELFTH_ROOT = pow(2.0, 1.0 / 12.0);
const double ANEG1 = 440.0 / pow(2, 5);
const double CNEG1 = ANEG1 * pow(TWELFTH_ROOT, -9.0);

enum CycleShape {
    None,
    SquareEighth,
    SquareQuarter,
    SquareHalf,
    SquareThreeQuarters,
    Triangle,
    Irregular,
    Fixed
};

struct Run {
    sampleoff start;
    samplesize length;
    samplevalue value;
};

struct BoolRun {
    sampleoff start;
    samplesize length;
    bool on;
};

struct Cycle {
    sampleoff start;
    CycleShape shape;
    double semitone_id;
    qint16 nes_timer;
    QList<Run> runs;
};

inline samplesize sum_run_lengths(Cycle &cycle) {
    samplesize runs_total = 0;
    for (Run &run: cycle.runs) {
        runs_total += run.length;
    }
    return runs_total;
}

inline double period_to_semitone(const samplesize &period) {
    double frequency = CPU_FREQENCY / period;
    return log(frequency / CNEG1) / log(TWELFTH_ROOT);
}

inline qint16 period_to_nes_timer(const samplesize &period) {
    qint16 nes_timer = (period >> 4) - 1;
    return nes_timer;
}

class ToneObject {

public:
    explicit ToneObject();
    explicit ToneObject(const double &semitone_id, qint16 nes_timer, const short int &shape);

    double semitone_id;
    qint16 nes_timer;
    qint16 nes_timer_end;
    short int shape;
    sampleoff start;
    samplesize length;
    samplevalue volume;
    QVector<Cycle> cycles;

    QString name() const;
    double semitone_id_end() const;
    samplesize match_before(ToneObject &before, samplesize max_length = 0);
    samplesize match_after(ToneObject &after, samplesize max_length = 0);

private:
};

const QStringList note_names {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

#endif // TONEOBJECT_H
