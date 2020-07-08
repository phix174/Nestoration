#ifndef TONEOBJECT_H
#define TONEOBJECT_H

#include <QStringList>
#include <QVector>

typedef uint8_t samplevalue;
typedef long sampleoff;
typedef long samplesize;

enum CycleDuty {
    Eighth,
    Quarter,
    Half,
    ThreeQuarters,
    None,
    Irregular
};

struct Run {
    sampleoff start;
    samplesize length;
    uint8_t value;
};

struct BoolRun {
    sampleoff start;
    samplesize length;
    bool on;
};

struct Cycle {
    sampleoff start;
    CycleDuty duty;
    double semitone_id;
    QVector<samplesize> runs;
};

class ToneObject {

public:
    explicit ToneObject();
    explicit ToneObject(const double &semitone_id, const short int &duty);

    double semitone_id;
    short int duty;
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
