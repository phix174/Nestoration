#include "player.h"
#include <QAudioOutput>
#include <QDebug>

#include "toneobject.h"

Player::Player(QObject *parent) : QObject(parent) {
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    QAudioFormat format;
    format.setSampleRate(1789773);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::Float);
    QAudioFormat nearest = device.nearestFormat(format);
    qDebug() << device.deviceName() << "Nearest sample rate:" << nearest.sampleRate();
    this->audio = new QAudioOutput(nearest);
    QObject::connect(this->audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    this->generator = new Generator { nearest.sampleRate() };
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

void Player::play_pause() {
    bool playing = this->audio->state() == QAudio::ActiveState;
    bool paused = this->audio->state() == QAudio::SuspendedState;
    if (playing) {
        qDebug() << "Suspending";
        this->audio->suspend();
    } else if (paused) {
        qDebug() << "Resuming";
        this->audio->resume();
    } else {
        qDebug() << "Starting";
        this->audio->start(this->generator);
    }
}

void Player::stop() {
    this->audio->stop();
}

void Player::handleStateChanged(QAudio::State new_state) {
    qDebug() << "QAudioOutput state changed to:" << new_state;
}
