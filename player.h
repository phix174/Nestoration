#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QAudioOutput>

#include "generator.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);

    void start();
    void seek(qint64 pos);
    void stop();

public slots:
    void handleStateChanged(QAudio::State new_state);

signals:

private:
    QAudioOutput *audio;
    Generator *generator;
};

#endif // PLAYER_H
