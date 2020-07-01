#ifndef TONEOBJECT_H
#define TONEOBJECT_H

#include <QStringList>

class ToneObject {

public:
    explicit ToneObject();
    explicit ToneObject(const double &semitone_id, const qint64 &length);

    double semitone_id;
    qint64 length;

    QString name() const;

private:
};

const QStringList note_names {
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

#endif // TONEOBJECT_H
