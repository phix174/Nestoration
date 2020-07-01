#include <cmath>
#include "toneobject.h"

ToneObject::ToneObject()
{
    this->semitone_id = 0;
    this->length = 0;
}

ToneObject::ToneObject(const double &semitone_id, const qint64 &length)
    : semitone_id(semitone_id), length(length)
{
}

QString ToneObject::name() const
{
    int closest = round(this->semitone_id);
    QString name_only = note_names[closest % 12];
    QString octave = QString::number(closest / 12);
    return name_only + octave;
}
