#ifndef TONEOBJECT_H
#define TONEOBJECT_H

#include <QObject>
#include <cmath>

//![0]
class ToneObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(double semitone_id READ semitone_id WRITE setSemitoneId NOTIFY semitoneChanged)
    Q_PROPERTY(qint64 length READ length WRITE setLength NOTIFY lengthChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
//![0]
public:
    explicit ToneObject(QObject *parent = nullptr);
    explicit ToneObject(const double &semitone_id, const qint64 &length, QObject *parent=0);

    double semitone_id() const;
    void setSemitoneId(const double &semitone_id);

    qint64 length() const;
    void setLength(const qint64 &length);

    QString name() const;

signals:
    void semitoneChanged();
    void lengthChanged();

private:
    double m_semitone_id;
    qint64 m_length;
};

const QStringList note_names({
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
});

#endif // TONEOBJECT_H
