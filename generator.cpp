#include <QDebug>

#include "generator.h"

Generator::Generator()
{

}

qint64 Generator::readData(char *data, qint64 maxSize) {
    Q_UNUSED(maxSize);
    //qDebug() << maxSize; // This was always 16384
    qDebug() << this->pos();
    int half = 8192;
    size_t duty = half / 64;
    int cycles = half / duty;
    for (int i=0; i < cycles; i++) {
        memset(data+(i*duty), (i % 2) * 255, duty);
    }
    duty = half / 128;
    cycles = half / duty;
    for (int i=0; i < cycles; i++) {
        memset(data+half+(i*duty), (i % 2) * 255, duty);
    }
    return half * 2;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
