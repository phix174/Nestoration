#include "trianglechannel.h"

#include <QDebug>
#include "toneobject.h"

TriangleChannel::TriangleChannel()
{

}

QVector<Cycle> TriangleChannel::runs_to_cycles(QList<Run> &runs) {
    const samplesize LONGEST_RUN = 2048; // 32768 / 32
    QVector<Cycle> cycles;
    const Cycle clear_cycle = { 0, CycleShape::Irregular, -999, {} };
    for (int i=0; i < runs.size(); i += 1) {
        // TODO: This should put up to 32 runs together in one cycle if they all share the same semitone id.
        Cycle cycle = clear_cycle;
        cycle.start = runs[i].start;
        cycle.runs.append(runs[i]);
        if (runs[i].length < LONGEST_RUN) {
            samplesize period = 16 * runs[i].length;
            bool tip = (runs[i].value == 0 || runs[i].value == 15);
            if (!tip) {
                period *= 2;
            }
            cycle.shape = CycleShape::Triangle;
            cycle.semitone_id = period_to_semitone(period);
        } else {
            cycle.shape = CycleShape::None;
        }
        cycles.append(cycle);
        //qDebug() << cycle.start << cycle.shape << cycle.semitone_id << cycle.runs.count() << sum_run_lengths(cycle);
    }
    qDebug() << "Cycle count: " << cycles.size();
    return cycles;
}

QVector<ToneObject> TriangleChannel::find_tones(QVector<Cycle> &cycles) {
    QVector<ToneObject> tones;
    ToneObject tone;
    sampleoff tone_start;
    if (cycles.size() == 0) return tones;
    tone_start = cycles[0].start;
    tone.semitone_id = cycles[0].semitone_id;
    tone.shape = cycles[0].shape;
    tone.cycles.append(cycles[0]);
    for (int i = 1; i < cycles.size(); i++) {
        if (cycles[i].semitone_id != tone.semitone_id || cycles[i].shape != tone.shape) {
            tone.length = cycles[i].start - tone_start;
            if (tone.shape != CycleShape::None && tone.cycles.size() == 1) {
                tone.shape = CycleShape::Irregular;
            }
            //qDebug() << "Semitone" << tone.semitone_id << "for" << tone.length / 1789773.0 << "sec," << tone.cycles.size() << "cycles";
            tones.append(tone);
            tone_start = cycles[i].start;
            tone = ToneObject { cycles[i].semitone_id, cycles[i].shape };
        }
        tone.cycles.append(cycles[i]);
    }
    tone.length = sum_run_lengths(cycles.last());
    //qDebug() << "Semitone" << tone.semitone_id << "for" << tone.length / 1789773.0 << "sec," << tone.cycles.size() << "cycles";
    tones.append(tone);
    qDebug() << "Tone count: " << tones.size();
    return tones;
}
