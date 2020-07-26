#include <QDebug>
#include <algorithm>
#include <cmath>

#include "generator.h"
#include "toneobject.h"

extern "C" {
    #include "soxr.h"
}

Generator::Generator(const int output_rate)
    : output_rate(output_rate)
{
    this->sample_rate_ratio = static_cast<double>(this->internal_rate) / this->output_rate;
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        this->channels[channel_i] = { QList<Run> {}, 0, 0, nullptr, false };
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
    this->soxr = soxr_create(this->internal_rate * this->resolution_multiplier, this->output_rate, 1, &error, &io_spec, &quality_spec, &runtime_spec);
}

void Generator::setChannels(QList<QList<Run>> channel_runs) {
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        this->channels[channel_i].runs = channel_runs[channel_i];
        this->channels[channel_i].runs_i = 0;
        this->channels[channel_i].runs_i_sample = 0;
        delete[] this->channels[channel_i].buffer;
        this->channels[channel_i].buffer = nullptr;
    }
    soxr_clear(this->soxr);
}

void Generator::toggle_mute(uint8_t channel_i) {
    this->channels[channel_i].muted = !this->channels[channel_i].muted;
}

qint64 Generator::render_runs(Channel &channel, qint64 output_samples_requested) {
    qint64 rendered_samples = 0;
    // TODO: I'm not sure what to do about the noninteger number of samples needed.
    qint64 internal_samples_needed = std::round(output_samples_requested * this->sample_rate_ratio);
    if (channel.buffer == nullptr) {
        qint64 buffer_size = internal_samples_needed * this->resolution_multiplier;
        channel.buffer = new samplevalue[buffer_size];
    }
    while (channel.runs_i < channel.runs.size() && rendered_samples < internal_samples_needed) {
        Run run = channel.runs.at(channel.runs_i);
        qint64 remaining_run_samples = run.length - channel.runs_i_sample;
        qint64 capped_samples = remaining_run_samples;
        if (rendered_samples + remaining_run_samples > internal_samples_needed) {
            capped_samples = internal_samples_needed - rendered_samples;
        }
        channel.runs_i_sample += capped_samples;
        samplevalue *buffer_offset = channel.buffer + static_cast<qint64>(rendered_samples * this->resolution_multiplier);
        samplevalue buffer_value = channel.muted ? 0 : run.value;
        qint64 buffer_samples = capped_samples * this->resolution_multiplier;
        memset(buffer_offset, buffer_value, buffer_samples);
        rendered_samples += capped_samples;
        if (channel.runs_i_sample >= run.length) {
            channel.runs_i += 1;
            channel.runs_i_sample = 0;
        }
    }
    return rendered_samples * resolution_multiplier;
}

void Generator::mix_channels(qint64 size) {
    for (qint64 sample_i = 0; sample_i < size; sample_i += 1) {
        float pulse_out = 0;
        samplevalue pulse_sum = channels[0].buffer[sample_i] + channels[1].buffer[sample_i];
        if (pulse_sum > 0) {
            pulse_out = 95.88 / (8128.0 / pulse_sum + 100.0);
        }
        float tnd_out = 0;
        samplevalue tnd_sum = channels[2].buffer[sample_i] + channels[3].buffer[sample_i] + channels[4].buffer[sample_i];
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
    size_t out_size = std::round(in_size / this->sample_rate_ratio / this->resolution_multiplier);
    size_t idone, odone;
    soxr_error_t error = soxr_process(this->soxr,
        this->mixed_buffer, in_size, &idone,
        out, out_size, &odone);
    if (error) {
        qDebug() << soxr_strerror(error);
    }
    return odone;
}

bool Generator::seek_sample(qint64 sample_position) {
    for (uint8_t channel_i = 0; channel_i < 5; channel_i += 1) {
        qint64 running_total = 0;
        qint64 i = 0;
        for(Run &run: this->channels[channel_i].runs) {
            if (running_total + run.length >= sample_position) {
                break;
            }
            running_total += run.length;
            i += 1;
        }
        this->channels[channel_i].runs_i = i;
        this->channels[channel_i].runs_i_sample = sample_position - running_total;
    }
    qint64 byte_position = sample_position / this->sample_rate_ratio * sizeof(float);
    this->seek(byte_position);
    return true;
}

qint64 Generator::readData(char *data, qint64 maxSize) {
    qint64 output_samples_requested = maxSize / sizeof(float);
    qint64 internal_samples_generated = 0;
    for (Channel &channel: this->channels) {
        internal_samples_generated = this->render_runs(channel, output_samples_requested);
        // TODO: Should probably check to make sure all the sizes are the same.
    }
    if (this->mixed_buffer == nullptr) {
        this->mixed_buffer = new float[internal_samples_generated];
    }
    this->mix_channels(internal_samples_generated);
    if (this->downsampled_buffer == nullptr) {
        this->downsampled_buffer = new float[output_samples_requested];
    }
    size_t bytes = sizeof(float) * this->resample_soxr(downsampled_buffer, internal_samples_generated);
    memcpy(data, downsampled_buffer, bytes);
    emit this->positionChanged(this->pos());
    return bytes;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
