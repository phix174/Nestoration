#include <QFileDialog>
#include <QDebug>

#include <archive.h>
#include <archive_entry.h>

#include "audiofile.h"
#include "channelmodel.h"
#include "toneobject.h"
#include "player.h"


AudioFile::AudioFile(QObject *parent)
    : QObject(parent), lowest_tone(8), highest_tone(8+88)
{
    this->channel0 = new ChannelModel;
    this->channel1 = new ChannelModel;
    this->player = new Player;
}

void AudioFile::open(const char *file_name)
{
    struct archive_entry *entry;
    int result;
    WAVheader header;
    m_archive = archive_read_new();
    archive_read_support_filter_gzip(m_archive);
    archive_read_support_format_raw(m_archive);
    result = archive_read_open_filename(m_archive, file_name, 1048576);
    if (result != ARCHIVE_OK) {
        qDebug() << archive_error_string(m_archive);
        throw 1;
    }
    if (archive_read_next_header(m_archive, &entry) == ARCHIVE_OK) {
        archive_read_data(m_archive, &header, 44);
        if (header.audio_format != 1) throw 3;
        if (header.num_channels != 5) throw 3;
        if (header.sample_rate != 1789773) throw 3;
        if (header.bits_per_sample != 8) throw 3;
    }
    this->is_open = true;
}

void AudioFile::read_block(char block[], std::streamsize &bytes_read) {
    bytes_read = archive_read_data(m_archive, block, 1789773 * 5);
}

void AudioFile::openClicked()
{
    player->stop();
    QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a gzipped 5-channel WAV file", QString(), "gzipped WAV (*.wav.gz)");
    if (file_name == "") {
        return;
    }
    if (this->is_open) {
        this->close();
    }
    try {
        this->open(qPrintable(file_name));
    } catch (int e) {
        qDebug() << "Failed to open WAV file.";
        return;
    }
    qDebug() << "Reading runs...";
    this->read_runs();
    qDebug() << "Converting runs to cycles...";
    this->highest_tone = -999;
    this->lowest_tone = 999;
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        QVector<Cycle> cycles = this->runs_to_cycles(this->channel_runs[channel_i]);
        QVector<ToneObject> tones { this->find_tones(cycles) };
        fix_transitional_tones(tones);
        fix_trailing_tones(tones);
        fix_leading_tones(tones);
        if (channel_i == 0) {
            this->channel0->set_tones(tones);
            emit this->channel0Changed(this->channel0);
            this->determine_range(tones);
        } else if (channel_i == 1) {
            this->channel1->set_tones(tones);
            emit this->channel1Changed(this->channel1);
            this->determine_range(tones);
            break;
        }
    }
    emit this->lowestToneChanged(this->lowest_tone);
    emit this->highestToneChanged(this->highest_tone);
    this->player->setChannels(this->channel_runs);
}

void AudioFile::playerSeek(qint64 position) {
    this->player->seek(position);
}

void AudioFile::playPause() {
    this->player->play_pause();
}

double period_to_semitone(const samplesize &period) {
    double frequency = CPU_FREQENCY / period;
    return log(frequency / C0) / log(TWELFTH_ROOT);
}

void AudioFile::read_runs() {
    const uint8_t CHANNELS = 5;
    Run run[CHANNELS];
    uint8_t *block = new uint8_t[1789773 * 5];
    std::streamsize bytes_read = 0;
    sampleoff file_sample_i = 0;
    std::streamoff block_offset = 0;
    uint8_t previous_value[CHANNELS];
    uint8_t new_value[CHANNELS];

    this->read_block(reinterpret_cast<char*>(block), bytes_read);
    if (bytes_read == 0) {
        return;
    }
    for (int channel_i = 0; channel_i < CHANNELS; channel_i += 1) {
        this->channel_runs[channel_i].clear();
        new_value[channel_i] = block[block_offset + channel_i];
        uint8_t raw_value = new_value[channel_i] - 128;
        if (channel_i < 4) {
            raw_value = raw_value >> 3;
        }
        run[channel_i] = { file_sample_i, 0, raw_value };
        previous_value[channel_i] = new_value[channel_i];
    }
    file_sample_i += 1;
    block_offset += 5;
    while (bytes_read) {
        while (block_offset < bytes_read) {
            for (int channel_i = 0; channel_i < CHANNELS; channel_i += 1) {
                new_value[channel_i] = block[block_offset + channel_i];
                if (new_value[channel_i] != previous_value[channel_i]) {
                    run[channel_i].length = file_sample_i - run[channel_i].start;
                    this->channel_runs[channel_i].append(run[channel_i]);
                    uint8_t raw_value = new_value[channel_i] - 128;
                    if (channel_i < 4) {
                        raw_value = raw_value >> 3;
                    }
                    run[channel_i] = { file_sample_i, 0, raw_value };
                    previous_value[channel_i] = new_value[channel_i];
                }
            }
            file_sample_i += 1;
            block_offset += 5;
        }
        block_offset = 0;
        this->read_block(reinterpret_cast<char*>(block), bytes_read);
    }
    for (int channel_i = 0; channel_i < CHANNELS; channel_i += 1) {
        run[channel_i].length = file_sample_i - run[channel_i].start;
        this->channel_runs[channel_i].append(run[channel_i]);
        qDebug() << "Channel" << channel_i << "run count:" << this->channel_runs[channel_i].size();
    }
    delete[] block;
    this->close();
}

QVector<Cycle> AudioFile::runs_to_cycles(QList<Run> &runs) {
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

QVector<ToneObject> AudioFile::find_tones(QVector<Cycle> &cycles) {
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

void AudioFile::fix_transitional_tones(QVector<ToneObject> &tones) {
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

void AudioFile::fix_trailing_tones(QVector<ToneObject> &tones) {
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

void AudioFile::fix_leading_tones(QVector<ToneObject> &tones) {
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

void AudioFile::determine_range(QVector<ToneObject> &tones) {
    for (ToneObject &tone: tones) {
        if (tone.duty == CycleDuty::Irregular || tone.semitone_id < 0) {
            continue;
        }
        if (ceil(tone.semitone_id) > this->highest_tone) {
            this->highest_tone = ceil(tone.semitone_id);
        }
        if (floor(tone.semitone_id) < this->lowest_tone) {
            this->lowest_tone = floor(tone.semitone_id);
        }
    }
}

void AudioFile::close() {
    int result;
    result = archive_read_free(m_archive);
    if (result != ARCHIVE_OK)
        throw 2;
    this->is_open = false;
}
