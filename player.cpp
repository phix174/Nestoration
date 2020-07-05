#include "player.h"
#include <QAudioOutput>
#include <QDebug>

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
    this->start();
}

void Player::start() {
    this->audio->setBufferSize(882*4);
    this->audio->start(this->generator);
    qDebug() << "Buffer size:" << this->audio->bufferSize();
}

void Player::stop() {
    this->audio->stop();
}

void Player::handleStateChanged(QAudio::State new_state) {
    qDebug() << new_state;
}
