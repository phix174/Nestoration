#ifndef TRIANGLECHANNEL_H
#define TRIANGLECHANNEL_H

#include <QList>

struct Run;
struct Cycle;
class ToneObject;

class TriangleChannel
{
public:
    TriangleChannel();

    QVector<Cycle> runs_to_cycles(QList<Run> &runs);
    QVector<ToneObject> find_tones(QVector<Cycle> &cycles);

private:
    QList<Run> original_runs;
    QList<ToneObject> tones;
};

#endif // TRIANGLECHANNEL_H
