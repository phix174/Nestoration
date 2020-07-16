#ifndef SQUARECHANNEL_H
#define SQUARECHANNEL_H

#include <QList>

class Cycle;
class Run;
class ToneObject;

class SquareChannel
{
public:
    SquareChannel();

    QVector<Cycle> runs_to_cycles(QList<Run> &runs);
    QVector<ToneObject> find_tones(QVector<Cycle> &cycles);
    void fix_transitional_tones(QVector<ToneObject> &tones);
    void fix_trailing_tones(QVector<ToneObject> &tones);
    void fix_leading_tones(QVector<ToneObject> &tones);

private:
    QList<Run> original_runs;
    QList<ToneObject> tones;
};

#endif // SQUARECHANNEL_H
