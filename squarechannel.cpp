#include "squarechannel.h"

#include <QDebug>
#include "toneobject.h"

SquareChannel::SquareChannel()
{

}

QVector<Cycle> SquareChannel::runs_to_cycles(QList<Run> &runs) {
    const samplesize SEVEN_EIGHTHS_OFF = 28672; // 32768 * 7/8
    QVector<Cycle> cycles;
    const Cycle clear_cycle = { 0, CycleDuty::Irregular, -999, {} };
    for (int i=0; i < runs.size(); i += 1) {
        Cycle cycle = clear_cycle;
        cycle.start = runs[i].start;
        if (runs[i].value > 0) {
            samplesize on_length = 0;
            while (i < runs.size() && runs[i].value > 0) {
                on_length += runs[i].length;
                cycle.runs.append(runs[i]);
                i += 1;
            }
            int next_zero = i;
            bool on_then_off = next_zero < runs.size();
            samplesize cycle_length = on_length;
            if (on_then_off) {
                 cycle_length += runs[next_zero].length;
                 bool is_normal_size = (144 <= cycle_length && cycle_length <= 32768 && (cycle_length & 15) == 0);
                 if (is_normal_size) {
                     if (on_length * 8 == cycle_length) {
                         cycle.duty = CycleDuty::Eighth;
                     } else if (on_length * 4 == cycle_length) {
                         cycle.duty = CycleDuty::Quarter;
                     } else if (on_length * 2 == cycle_length) {
                         cycle.duty = CycleDuty::Half;
                     } else if (on_length * 4 == cycle_length * 3) {
                         cycle.duty = CycleDuty::ThreeQuarters;
                     }
                 }
                 bool rest_follows = runs[next_zero].length > SEVEN_EIGHTHS_OFF;
                 if (rest_follows) {
                     i -= 1;
                 } else {
                     cycle.semitone_id = period_to_semitone(cycle_length);
                     cycle.runs.append(runs[next_zero]);
                 }
            }
        } else {
            cycle.runs.append(runs[i]);
            if (runs[i].length > SEVEN_EIGHTHS_OFF) {
                cycle.duty = CycleDuty::None;
            }
        }
        cycles.append(cycle);
        //qDebug() << cycle.start << cycle.duty << cycle.semitone_id << cycle.runs.count() << sum_run_lengths(cycle);
    }
    qDebug() << "Cycle count: " << cycles.size();
    return cycles;
}

QVector<ToneObject> SquareChannel::find_tones(QVector<Cycle> &cycles) {
    QVector<ToneObject> tones;
    ToneObject tone;
    sampleoff tone_start;
    if (cycles.size() == 0) return tones;
    tone_start = cycles[0].start;
    tone.semitone_id = cycles[0].semitone_id;
    tone.duty = cycles[0].duty;
    tone.cycles.append(cycles[0]);
    for (int i = 1; i < cycles.size(); i++) {
        if (cycles[i].semitone_id != tone.semitone_id || cycles[i].duty != tone.duty) {
            tone.length = cycles[i].start - tone_start;
            if (tone.duty != CycleDuty::None && tone.cycles.size() == 1) {
                tone.duty = CycleDuty::Irregular;
            }
            //qDebug() << "Semitone" << tone.semitone_id << "for" << tone.length / 1789773.0 << "sec," << tone.cycles.size() << "cycles";
            tones.append(tone);
            tone_start = cycles[i].start;
            tone = ToneObject { cycles[i].semitone_id, cycles[i].duty };
        }
        tone.cycles.append(cycles[i]);
    }
    tone.length = sum_run_lengths(cycles.last());
    if (tone.duty != CycleDuty::None && tone.cycles.size() == 1) {
        tone.duty = CycleDuty::Irregular;
    }
    //qDebug() << "Semitone" << tone.semitone_id << "for" << tone.length / 1789773.0 << "sec," << tone.cycles.size() << "cycles";
    tones.append(tone);
    qDebug() << "Tone count: " << tones.size();
    return tones;
}

void SquareChannel::fix_transitional_tones(QVector<ToneObject> &tones) {
    double a, b, c;
    double midpoint;
    samplesize left_size;
    ToneObject left, right;
    for (int i = 1; (i+1) < tones.size(); ) {
        if (tones[i-1].duty >= 4 || tones[i].duty < 4 || tones[i+1].duty >= 4) {
            // Skip tones with standard duty cycles
            // and skip tones with nonstandard neighbors.
             i += 1;
            continue;
        }
        a = tones[i-1].semitone_id;
        b = tones[i].semitone_id;
        c = tones[i+1].semitone_id;
        if (a < b && b < c) {
            midpoint = (c - b) / (c - a);
        } else if (a > b && b > c) {
            midpoint = (b - c) / (a - c);
        } else {
            i += 1;
            continue;
        }
        left_size = sum_run_lengths(tones[i-1].cycles.last()) * midpoint;
        //qDebug() << "Dividing tone" << i << "into" << left_size << "and" << tones[i].length - left_size;
        left.semitone_id = tones[i-1].semitone_id;
        left.duty = CycleDuty::Fixed;
        left.length = left_size;
        // TODO: Add cycles to left tone
        right.semitone_id = tones[i+1].semitone_id;
        right.duty = CycleDuty::Fixed;
        right.length = tones[i].length - left_size;
        // TODO: Add cycles to right tone
        tones.removeAt(i);
        tones.insert(i, left);
        i += 1;
        tones.insert(i, right);
        i += 1;
    }
}

void SquareChannel::fix_trailing_tones(QVector<ToneObject> &tones) {
    ToneObject left, right;
    for (int i = 1; i < tones.size(); ) {
        if (tones[i-1].duty >= 4 || tones[i].duty < 4 || tones[i].duty == CycleDuty::Fixed) {
            // Skip tones with standard duty cycles
            // and skip tones with nonstandard neighbors.
            i += 1;
            continue;
        }
        samplesize left_size = tones[i-1].match_after(tones[i]);
        left.semitone_id = tones[i-1].semitone_id;
        left.duty = tones[i-1].duty;
        left.length = left_size;
        right.semitone_id = -999;
        right.duty = CycleDuty::None;
        right.length = tones[i].length - left_size;
        tones.removeAt(i);
        if (left.length) {
            tones.insert(i, left);
            i += 1;
        }
        if (right.length) {
            tones.insert(i, right);
            i += 1;
        }
    }
}

void SquareChannel::fix_leading_tones(QVector<ToneObject> &tones) {
    ToneObject left, right;
    for (int i = 0; (i+1) < tones.size(); ) {
        if (tones[i].duty != CycleDuty::Irregular || tones[i+1].duty >= 4) {
            // Skip tones with standard duty cycles
            // and skip tones with nonstandard neighbors.
            i += 1;
            continue;
        }
        samplesize right_size = tones[i+1].match_before(tones[i]);
        left.semitone_id = tones[i+1].semitone_id;
        left.duty = CycleDuty::Irregular;
        left.length = tones[i].length - right_size;
        right.semitone_id = tones[i+1].semitone_id;
        right.duty = CycleDuty::Fixed;
        right.length = right_size;
        tones.removeAt(i);
        if (left.length) {
            tones.insert(i, left);
            i += 1;
        }
        if (right.length) {
            tones.insert(i, right);
            i += 1;
        }
    }
}
