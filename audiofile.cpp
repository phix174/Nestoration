#include <iostream>

#include <QDebug>

#include <archive.h>
#include <archive_entry.h>

#include "audiofile.h"
#include "channelmodel.h"
#include "toneobject.h"


AudioFile::AudioFile(QObject *parent)
    : QObject(parent)
{
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
        std::cout << archive_error_string(m_archive) << std::endl;
        throw 1;
    }
    if (archive_read_next_header(m_archive, &entry) == ARCHIVE_OK) {
        archive_read_data(m_archive, &header, 44);
        if (header.audio_format != 1) throw 3;
        if (header.num_channels != 5) throw 3;
        if (header.sample_rate != 1789773) throw 3;
        if (header.bits_per_sample != 8) throw 3;
    }
}

void AudioFile::read_block(char block[], std::streamsize &bytes_read) {
    bytes_read = archive_read_data(m_archive, block, 1789773 * 5);
}

void AudioFile::openClicked()
{
    try {
        this->open("/home/don/storage/code/qt/creator2/creator2/ducktales-5ch-10.wav.gz");
    } catch (int e) {
        qDebug() << "Failed to open WAV file." << endl;
        return;
    }
    qDebug() << "Reading cycles..." << endl;
    QVector<Cycle> cycles = this->read_cycles();
    qDebug() << "Finding tones..." << endl;
    this->channel0 = new ChannelModel { this->find_tones(cycles) };
    emit modelChanged(this->channel0);
    std::cout << "Clicked!" << std::endl;
}

double period_to_semitone(const samplesize &period) {
    double frequency = CPU_FREQENCY / period;
    return log(frequency / C0) / log(TWELFTH_ROOT);
}

QVector<Cycle> AudioFile::read_cycles() {
    QVector<Cycle> cycles;
    Cycle cycle;
    char *block = new char[1789773 * 5];
    std::streamsize bytes_read = 0;
    sampleoff file_sample_i = 0;
    std::streamoff block_i = 0;
    char previous_value;

    this->read_block(block, bytes_read);
    if (bytes_read == 0) {
        return cycles;
    }
    previous_value = block[block_i];
    cycle = { 0, 0 };
    file_sample_i += 1;
    block_i += 5;
    while (bytes_read) {
        while (block_i < bytes_read) {
            if (block[block_i] == -128 && previous_value != -128) {
                cycle.length = file_sample_i - cycle.start;
                cycles.push_back(cycle);
                cycle = { file_sample_i, 0 };
            }
            previous_value = block[block_i];
            file_sample_i += 1;
            block_i += 5;
        }
        block_i = 0;
        this->read_block(block, bytes_read);
    }
    cycle.length = file_sample_i - cycle.start;
    cycles.push_back(cycle);
    delete[] block;
    this->close();
    qDebug() << "Cycle count: " << cycles.size() << endl;
    return cycles;
}

/*
 * Pulses:
 *
 * 1/8: _-______  20 + 20 + 120
 *
 * 1/4: _--_____  20 + 40 + 100
 *
 * 1/2: _----___  20 + 80 + 60
 *
 * 3/4: -__-----  20 + 40 + 100
 *
 */

QVector<ToneObject> AudioFile::find_tones(QVector<Cycle> &cycles) {
    QVector<ToneObject> tones;
    if (cycles.length() == 0) return tones;
    sampleoff tone_start;
    tone_start = cycles[0].start;
    ToneObject tone { period_to_semitone(cycles[0].length), 0 };
    for (int i = 1; i < cycles.size(); i++) {
        if (cycles[i].length != cycles[i-1].length) {
            tone.length = cycles[i].start - tone_start;
            //cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
            tones.push_back(tone);
            tone_start = cycles[i].start;
            tone = ToneObject { period_to_semitone(cycles[i].length), 0 };
        }
    }
    //cout << "Semitone " << tone->semitone_id() << " for " << tone->length() / 1789773.0 << " sec" << endl;
    tones.push_back(tone);
    qDebug() << "Tone count: " << tones.size() << endl;
    return tones;
}

void AudioFile::close() {
    int result;
    result = archive_read_free(m_archive);
    if (result != ARCHIVE_OK)
        throw 2;
}
