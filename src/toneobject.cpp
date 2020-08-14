#include <cmath>
#include <algorithm>
#include "toneobject.h"

ToneObject::ToneObject()
{
    this->semitone_id = -999;
    this->nes_timer = -1;
    this->nes_timer_end = -1;
    this->shape = CycleShape::Irregular;
    this->start = 0;
    this->length = 0;
    this->volume = 0;
    this->cycles = {};
}

ToneObject::ToneObject(const double &semitone_id, qint16 nes_timer, const short int &shape)
    : semitone_id(semitone_id), nes_timer(nes_timer), nes_timer_end(nes_timer), shape(shape)
{
    this->start = 0;
    this->length = 0;
    this->volume = 0;
    this->cycles = {};
}

double ToneObject::semitone_id_end() const {
    samplesize period = 16 * (this->nes_timer_end + 1);
    if (period == 0) return this->semitone_id;
    return period_to_semitone(period);
}

QString ToneObject::name() const {
    if (this->semitone_id < 0) {
        return "Silence";
    }
    int closest = round(this->semitone_id);
    QString name_only = note_names[closest % 12];
    QString octave = QString::number(closest / 12 - 1);
    int cents = 100 * (this->semitone_id - closest);
    QString cents_plus = cents > 0 ? "+" : "";
    return name_only + octave + cents_plus + QString::number(cents) + "¢";
}

samplesize ToneObject::match_before(ToneObject &before, samplesize max_length) {
    samplesize matched_length {0};

    if (max_length == 0) {
        max_length = before.length;
    } else {
        max_length = std::min(before.length, max_length);
    }
    QList<Run>::reverse_iterator ref_i, cand_i;
    if (this->cycles.isEmpty() or before.cycles.isEmpty()) {
        return 0;
    }
    ref_i = this->cycles.first().runs.rbegin();
    cand_i = before.cycles.last().runs.rbegin();
    while (ref_i != this->cycles.first().runs.rend() && cand_i != before.cycles.last().runs.rend()) {
        samplesize reference = ref_i->length;
        samplesize candidate = cand_i->length;
        samplesize new_matched = 0;
        if (ref_i->value == cand_i->value) {
            new_matched = std::min(reference, candidate);
        }
        matched_length += new_matched;
        if (new_matched < reference || matched_length >= max_length) {
            break;
        }
        ref_i += 1;
        cand_i += 1;
    }
    return std::min(matched_length, max_length);
}

samplesize ToneObject::match_after(ToneObject &after, samplesize max_length) {
    samplesize matched_length {0};

    if (max_length == 0) {
        max_length = after.length;
    } else {
        max_length = std::min(after.length, max_length);
    }
    QList<Run>::iterator ref_i, cand_i;
    if (this->cycles.isEmpty() or after.cycles.isEmpty()) {
        return 0;
    }
    ref_i = this->cycles.last().runs.begin();
    cand_i = after.cycles.first().runs.begin();
    while (ref_i != this->cycles.last().runs.end() && cand_i != after.cycles.first().runs.end()) {
        samplesize reference = ref_i->length;
        samplesize candidate = cand_i->length;
        samplesize new_matched = 0;
        if (ref_i->value == cand_i->value) {
            new_matched = std::min(reference, candidate);
        }
        matched_length += new_matched;
        if (new_matched < reference || matched_length >= max_length) {
            break;
        }
        ref_i += 1;
        cand_i += 1;
    }
    return std::min(matched_length, max_length);
}
