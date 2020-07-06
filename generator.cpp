#include <QDebug>
#include <algorithm>

#include "generator.h"
#include "toneobject.h"

Generator::Generator()
{

}

QByteArray Generator::render_runs(qint64 maxSize) {
    QByteArray buffer;
    qint64 rendered_samples = 0;
    qint64 scaled_maxSize = maxSize * 40;
    qDebug() << "A";
    while (this->run_i < this->runs.size() && rendered_samples < scaled_maxSize) {
        qDebug() << "run_i" << this->runs.at(this->run_i).length;
        Run run = this->runs.at(this->run_i);
        qDebug() << "C";
        qint64 remaining_run_samples = run.length - this->run_i_sample;
        qint64 capped_samples = remaining_run_samples;
        if (rendered_samples + remaining_run_samples > scaled_maxSize) {
            capped_samples = scaled_maxSize - rendered_samples;
        }
        this->run_i_sample += capped_samples;
        rendered_samples += capped_samples;
        qDebug() << "D";
        buffer.append(capped_samples, run.on ? 127 : 0);
        if (this->run_i_sample >= run.length) {
            this->run_i += 1;
            this->run_i_sample = 0;
        }
        qDebug() << "E";
    }
    qDebug() << "B";
    return buffer;
}

QByteArray Generator::resample_bad(const QByteArray orig) {
    QByteArray output;
    for (qint64 i=0; i<orig.size(); i += 40) {
        output.append(orig.at(i));
    }
    return output;
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
    QByteArray buffer = this->resample_bad(this->render_runs(maxSize));
    memcpy(data, buffer.data(), maxSize);
    return maxSize;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
