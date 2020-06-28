#include <cmath>
#include "toneobject.h"

ToneObject::ToneObject(QObject *parent)
    : QObject(parent)
{
}

ToneObject::ToneObject(const double &semitone_id, const qint64 &length, QObject *parent)
    : QObject(parent), m_semitone_id(semitone_id), m_length(length)
{
}

double ToneObject::semitone_id() const
{
    return m_semitone_id;
}

void ToneObject::setSemitoneId(const double &semitone_id)
{
    if (semitone_id != m_semitone_id) {
        m_semitone_id = semitone_id;
    }
}

qint64 ToneObject::length() const
{
    return m_length;
}

void ToneObject::setLength(const qint64 &length)
{
    if (length != m_length) {
        m_length = length;
    }
}

QString ToneObject::name() const
{
    int closest = round(m_semitone_id);
    QString name_only = note_names[closest % 12];
    QString octave = QString::number(closest / 12);
    return name_only + octave;
}
