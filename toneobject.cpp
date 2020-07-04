#include <cmath>
#include <algorithm>
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

samplesize ToneObject::match_before(ToneObject &before, samplesize max_length) {
    samplesize matched_length {0};

    if (max_length == 0) {
        max_length = before.length;
    } else {
        max_length = std::min(before.length, max_length);
    }
    QVector<samplesize>::reverse_iterator ref_i, cand_i;
    if (this->cycles.isEmpty() or before.cycles.isEmpty()) {
        return 0;
    }
    ref_i = this->cycles.first().runs.rbegin();
    cand_i = before.cycles.last().runs.rbegin();
    while (ref_i != this->cycles.first().runs.rend() && cand_i != before.cycles.last().runs.rend()) {
        samplesize reference = *ref_i;
        samplesize candidate = *cand_i;
        samplesize new_matched = std::min(reference, candidate);
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
    QVector<samplesize>::iterator ref_i, cand_i;
    if (this->cycles.isEmpty() or after.cycles.isEmpty()) {
        return 0;
    }
    ref_i = this->cycles.first().runs.begin();
    cand_i = after.cycles.last().runs.begin();
    for (int i=0; i < 2; i += 1) {
        samplesize reference = *ref_i;
        samplesize candidate = *cand_i;
        samplesize new_matched = std::min(reference, candidate);
        matched_length += new_matched;
        if (new_matched < reference || matched_length >= max_length) {
            break;
        }
        ref_i += 1;
        cand_i += 1;
    }
    return std::min(matched_length, max_length);
}
