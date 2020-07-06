#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>
#include <QVector>

class Run;

class Generator : public QIODevice
{

public:
    Generator();

    QByteArray render_runs(qint64 maxSize);
    QByteArray resample_bad(const QByteArray orig);

    bool seek(qint64 pos) override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    QVector<Run> runs;

    qint64 run_i;
    qint64 run_i_sample;
};

#endif // GENERATOR_H
