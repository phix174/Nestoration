#include "player.h"
#include <QAudioOutput>
#include <QDebug>

#include "toneobject.h"

Player::Player(QObject *parent) : QObject(parent) {
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::UnSignedInt);
    this->audio = new QAudioOutput(format);
    QObject::connect(this->audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    this->generator = new Generator;
    this->generator->open(QIODevice::ReadOnly);
}

void Player::setRuns(QVector<Run> &runs) {
    this->generator->runs = runs;
    this->generator->run_i = 0;
    this->generator->run_i_sample = 0;
}

void Player::start() {
    this->audio->setBufferSize(882*4);
    this->audio->start(this->generator);
    qDebug() << "Buffer size:" << this->audio->bufferSize();
}

void Player::seek(qint64 pos) {
    this->generator->seek(pos);
}

void Player::stop() {
    this->audio->stop();
}

void Player::handleStateChanged(QAudio::State new_state) {
    qDebug() << new_state;
}
