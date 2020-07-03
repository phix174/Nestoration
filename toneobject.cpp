#include <cmath>
#include "toneobject.h"

ToneObject::ToneObject()
{
    this->semitone_id = -999;
    this->duty = 5;
    this->length = 0;
    this->cycles = {};
}

ToneObject::ToneObject(const double &semitone_id, const short int &duty)
    : semitone_id(semitone_id), duty(duty)
{
    this->length = 0;
    this->cycles = {};
}

QString ToneObject::name() const
{
    int closest = round(this->semitone_id);
    QString name_only = note_names[closest % 12];
    QString octave = QString::number(closest / 12);
    return name_only + octave;
}
