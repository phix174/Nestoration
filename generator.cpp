#include <QDebug>
#include <algorithm>

#include "generator.h"
#include "toneobject.h"

extern "C" {
    #include "soxr.h"
}

Generator::Generator()
{

}

qint64 Generator::render_runs(QByteArray *buffer, qint64 maxSize) {
    qint64 rendered_samples = 0;
    qint64 scaled_maxSize = maxSize * 1789773.0 / 44100.0;
    while (this->run_i < this->runs.size() && rendered_samples < scaled_maxSize) {
        Run run = this->runs.at(this->run_i);
        qint64 remaining_run_samples = run.length - this->run_i_sample;
        qint64 capped_samples = remaining_run_samples;
        if (rendered_samples + remaining_run_samples > scaled_maxSize) {
            capped_samples = scaled_maxSize - rendered_samples;
        }
        this->run_i_sample += capped_samples;
        rendered_samples += capped_samples;
        buffer->append(capped_samples, run.on ? 127 : 0);
        if (this->run_i_sample >= run.length) {
            this->run_i += 1;
            this->run_i_sample = 0;
        }
    }
    return rendered_samples;
}

void Generator::convert_buffer(const QByteArray in, float out[]) {
    for (int i=0; i < in.size(); i++) {
        out[i] = static_cast<float>(in.at(i) / 127.0);
    }
}

size_t Generator::resample_soxr(float in[], float out[], size_t in_size) {
    size_t out_size = (size_t)(in_size * 44100.0 / 1789773.0 + 0.5);
    size_t idone, odone;
    const soxr_datatype_t itype = static_cast<soxr_datatype_t>(0);
    const soxr_datatype_t otype = static_cast<soxr_datatype_t>(0);
    soxr_io_spec_t io_spec = soxr_io_spec(itype, otype);
    const unsigned long q_recipe = SOXR_MQ;
    const unsigned long q_flags = 0;
    soxr_quality_spec_t quality_spec = soxr_quality_spec(q_recipe, q_flags);
    const int use_threads = 1;
    soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(use_threads);
    soxr_error_t error = soxr_oneshot(1789773, 44100, 1,
        in, in_size, &idone,
        out, out_size, &odone,
        &io_spec, &quality_spec, &runtime_spec);
    if (error) {
        qDebug() << soxr_strerror(error);
    }
    return odone;
}

bool Generator::seek(qint64 pos) {
    QIODevice::seek(pos);
    qint64 running_total = 0;
    qint64 i = 0;
    for(Run &run: this->runs) {
        if (running_total + run.length >= pos) {
            break;
        }
        running_total += run.length;
        i += 1;
    }
    if (running_total < pos) {
        // Tried to seek past end of data.
        return false;
    }
    this->run_i = i;
    this->run_i_sample = pos - running_total;
    return true;
}

qint64 Generator::readData(char *data, qint64 maxSize) {
    qDebug() << "run_i" << this->run_i << "run_i_sample" << this->run_i_sample;
    QByteArray buffer;
    qint64 actual_size = this->render_runs(&buffer, maxSize/ sizeof(float));
    float *buffer2 = new float[actual_size];
    this->convert_buffer(buffer, buffer2);
    float *buffer3 = new float[(size_t)(actual_size * 44100.0 / 1789773.0 + 0.5)];
    size_t bytes = sizeof(float) * this->resample_soxr(buffer2, buffer3, actual_size);
    memcpy(data, buffer3, bytes);
    delete[] buffer2;
    delete[] buffer3;
    return bytes;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
