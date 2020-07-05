#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>

class Generator : public QIODevice
{

public:
    Generator();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
};

#endif // GENERATOR_H
