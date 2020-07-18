#include "trianglechannel.h"

#include <QDebug>
#include "toneobject.h"

TriangleChannel::TriangleChannel()
{

}

QVector<Cycle> TriangleChannel::runs_to_cycles(QList<Run> &runs) {
    const samplesize LONGEST_CYCLE = 32768;
    QVector<Cycle> cycles;
    if (runs.size() == 0) return cycles;
    const Cycle clear_cycle = { 0, CycleShape::Irregular, -999, {} };
    Cycle cycle = clear_cycle;
    cycle.start = runs[0].start;
    cycle.runs.append(runs[0]);
    bool tip = (runs[0].value == 0 || runs[0].value == 15);
    samplesize period = (tip ? 16 : 32) * runs[0].length;
    if (period <= LONGEST_CYCLE) {
        cycle.shape = CycleShape::Triangle;
        cycle.semitone_id = period_to_semitone(period);
    } else {
        cycle.shape = CycleShape::None;
    }
    bool rising = true; // TODO: This assumption might cause a small problem?
    bool prev_tip = tip;
    samplesize prev_period = period;
    bool prev_rising = rising;
    for (int i=1; i < runs.size(); i += 1) {
        tip = (runs[i].value == 0 || runs[i].value == 15);
        period = (tip ? 16 : 32) * runs[i].length;
        rising = (runs[i].value > runs[i-1].value);
        bool changed_direction = (rising != prev_rising && !prev_tip);
        bool changed_period = period != prev_period;
        bool completed_cycle = cycle.runs.size() == 30;
        if (changed_direction || changed_period || completed_cycle) {
            //qDebug() << cycle.start << cycle.shape << cycle.semitone_id << cycle.runs.count() << sum_run_lengths(cycle);
            cycles.append(cycle);
            cycle = clear_cycle;
            cycle.start = runs[i].start;
        }
        cycle.runs.append(runs[i]);
        if (period <= LONGEST_CYCLE) {
            cycle.shape = CycleShape::Triangle;
            cycle.semitone_id = period_to_semitone(period);
        } else {
            cycle.shape = CycleShape::None;
        }
        prev_tip = tip;
        prev_period = period;
        prev_rising = rising;
    }
    //qDebug() << cycle.start << cycle.shape << cycle.semitone_id << cycle.runs.count() << sum_run_lengths(cycle);
    cycles.append(cycle);
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
