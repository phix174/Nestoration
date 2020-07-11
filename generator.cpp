#include <QDebug>
#include <algorithm>

#include "generator.h"
#include "toneobject.h"

extern "C" {
    #include "soxr.h"
}

Generator::Generator(const int output_rate)
    : output_rate(output_rate)
{
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        this->channels[channel_i] = { QList<Run> {}, 0, 0, nullptr };
    }
    this->init_soxr();
}

Generator::~Generator() {
    delete[] downsampled_buffer;
    delete[] mixed_buffer;
    soxr_delete(this->soxr);
}

void Generator::init_soxr() {
    const soxr_datatype_t itype = static_cast<soxr_datatype_t>(0);
    const soxr_datatype_t otype = static_cast<soxr_datatype_t>(0);
    const soxr_io_spec_t io_spec = soxr_io_spec(itype, otype);
    const unsigned long q_recipe = SOXR_MQ;
    const unsigned long q_flags = 0;
    const soxr_quality_spec_t quality_spec = soxr_quality_spec(q_recipe, q_flags);
    const int use_threads = 1;
    const soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(use_threads);
    soxr_error_t error;
    this->soxr = soxr_create(1789773, this->output_rate, 1, &error, &io_spec, &quality_spec, &runtime_spec);
}

void Generator::setChannels(QList<Run> (&channel_runs)[5]) {
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        this->channels[channel_i].runs = channel_runs[channel_i];
        this->channels[channel_i].runs_i = 0;
        this->channels[channel_i].runs_i_sample = 0;
        delete[] this->channels[channel_i].buffer;
        this->channels[channel_i].buffer = nullptr;
    }
    soxr_clear(this->soxr);
}

qint64 Generator::render_runs(Channel &channel, qint64 maxSize) {
    qint64 rendered_samples = 0;
    qint64 scaled_maxSize = maxSize * 1789773.0 / this->output_rate;
    if (channel.buffer == nullptr) {
        channel.buffer = new uint8_t[scaled_maxSize];
    }
    while (channel.runs_i < channel.runs.size() && rendered_samples < scaled_maxSize) {
        Run run = channel.runs.at(channel.runs_i);
        qint64 remaining_run_samples = run.length - channel.runs_i_sample;
        qint64 capped_samples = remaining_run_samples;
        if (rendered_samples + remaining_run_samples > scaled_maxSize) {
            capped_samples = scaled_maxSize - rendered_samples;
        }
        channel.runs_i_sample += capped_samples;
        memset(channel.buffer+rendered_samples, run.value, capped_samples);
        rendered_samples += capped_samples;
        if (channel.runs_i_sample >= run.length) {
            channel.runs_i += 1;
            channel.runs_i_sample = 0;
        }
    }
    return rendered_samples;
}

void Generator::mix_channels(qint64 size) {
    for (qint64 sample_i = 0; sample_i < size; sample_i += 1) {
        float pulse_out = 0;
        uint8_t pulse_sum = channels[0].buffer[sample_i] + channels[1].buffer[sample_i];
        if (pulse_sum > 0) {
            pulse_out = 95.88 / (8128.0 / pulse_sum + 100.0);
        }
        float tnd_out = 0;
        uint8_t tnd_sum = channels[2].buffer[sample_i] + channels[3].buffer[sample_i] + channels[4].buffer[sample_i];
        if (tnd_sum > 0) {
            tnd_out = 159.79 / (1 / (
                channels[2].buffer[sample_i] / 8227.0 +
                channels[3].buffer[sample_i] / 12241.0 +
                channels[4].buffer[sample_i] / 22638.0) + 100);
        }
        this->mixed_buffer[sample_i] = pulse_out + tnd_out;
    }
}

size_t Generator::resample_soxr(float out[], size_t in_size) {
    size_t out_size = (size_t)(in_size * this->output_rate / 1789773.0 + 0.5);
    size_t idone, odone;
    soxr_error_t error = soxr_process(this->soxr,
        this->mixed_buffer, in_size, &idone,
        out, out_size, &odone);
    if (error) {
        qDebug() << soxr_strerror(error);
    }
    return odone;
}

bool Generator::seek(qint64 pos) {
    QIODevice::seek(pos);
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        qint64 running_total = 0;
        qint64 i = 0;
        for(Run &run: this->channels[channel_i].runs) {
            if (running_total + run.length >= pos) {
                break;
            }
            running_total += run.length;
            i += 1;
        }
        this->channels[channel_i].runs_i = i;
        this->channels[channel_i].runs_i_sample = pos - running_total;
    }
    return true;
}

qint64 Generator::readData(char *data, qint64 maxSize) {
    qint64 actual_size = 0;
    for (Channel &channel: this->channels) {
        actual_size = this->render_runs(channel, maxSize / sizeof(float));
        // TODO: Should probably check to make sure all the actual sizes are the same.
    }
    if (this->mixed_buffer == nullptr) {
        this->mixed_buffer = new float[actual_size];
    }
    this->mix_channels(actual_size);
    if (this->downsampled_buffer == nullptr) {
        this->downsampled_buffer = new float[(size_t)(actual_size * this->output_rate / 1789773.0 + 0.5)];
    }
    size_t bytes = sizeof(float) * this->resample_soxr(downsampled_buffer, actual_size);
    memcpy(data, downsampled_buffer, bytes);
    return bytes;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
