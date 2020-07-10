#include "player.h"
#include <QAudioOutput>
#include <QDebug>

#include "toneobject.h"

Player::Player(QObject *parent) : QObject(parent) {
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::Float);
    this->audio = new QAudioOutput(format);
    QObject::connect(this->audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    this->generator = new Generator;
    this->generator->open(QIODevice::ReadOnly);
}

void Player::setChannels(QList<Run> (&channel_runs)[5]) {
    this->audio->reset();
    this->generator->setChannels(channel_runs);
}

void Player::start() {
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
    qDebug() << "QAudioOutput state changed to:" << new_state;
}
