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
const double A0 = 440.0 / pow(2, 4);
const double C0 = A0 * pow(TWELFTH_ROOT, -9.0);

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
    return log(frequency / C0) / log(TWELFTH_ROOT);
}

class ToneObject {

public:
    explicit ToneObject();
    explicit ToneObject(const double &semitone_id, const short int &shape);

    double semitone_id;
    short int shape;
    samplesize length;
    QVector<Cycle> cycles;

    QString name() const;
    samplesize match_before(ToneObject &before, samplesize max_length = 0);
    samplesize match_after(ToneObject &after, samplesize max_length = 0);

private:
};

const QStringList note_names {
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

#endif // TONEOBJECT_H
