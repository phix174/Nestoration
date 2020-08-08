#include "player.h"
#include <QAudioOutput>
#include <QDebug>

#include "toneobject.h"

Player::Player(QAudioFormat out_format, QObject *parent) : QObject(parent) {
    this->out_format = out_format;
    this->byte_usec_ratio = out_format.sampleRate() * out_format.bytesPerFrame() / 1000000.0;
    this->audio = new QAudioOutput(out_format);
    this->audio->setNotifyInterval(16);
    this->audio->setBufferSize(125000 * this->byte_usec_ratio);
    QObject::connect(this->audio, SIGNAL(notify()), this, SLOT(handleNotify()));
    QObject::connect(this->audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    this->generator = new Generator { out_format.sampleRate() };
    QObject::connect(this->generator, SIGNAL(positionChanged(qint64)), this, SLOT(handlePositionChanged(qint64)));
    this->generator->open(QIODevice::ReadOnly);
    this->gme_buffer = new QBuffer;
}

Player::~Player() {
    delete this->audio;
    delete this->generator;
    delete this->gme_buffer;
}

void Player::setChannels(QList<QList<Run>> channel_runs) {
    this->audio->reset();
    this->generator->setChannels(channel_runs);
}

void Player::setGmeBuffer(const QByteArray gme_array) {
    this->audio->reset();
    this->position = 0;
    this->bytes_played = 0;
    this->seek_offset = 0;
    emit this->playerPositionChanged(this->position);
    this->gme_buffer->close();
    this->gme_buffer->setData(gme_array);
    this->gme_buffer->open(QIODevice::ReadOnly);
}

void Player::start() {
    this->audio->start(this->gme_buffer);
    qDebug() << "Buffer size:" << this->audio->bufferSize();
}

void Player::seek(qint64 sample_position) {
    qreal sample_ratio = this->out_format.sampleRate() / 1789773.0; /* TODO: Don't hard-code 1789773. */
    qint64 byte_position = std::round(sample_position * sample_ratio) * this->out_format.bytesPerFrame();
    this->seek_offset = byte_position - this->bytes_played;
    this->gme_buffer->seek(byte_position);
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
        this->start();
    }
}

void Player::toggle_mute(qint8 channel_i) {
    this->generator->toggle_mute(channel_i);
}

void Player::stop() {
    this->audio->stop();
}

void Player::handleNotify() {
    qint64 bytes_processed = this->audio->processedUSecs() * this->byte_usec_ratio;
    qint64 bytes_buffered = this->audio->bufferSize() - this->audio->bytesFree();
    this->bytes_played = bytes_processed - bytes_buffered;
    this->position = this->bytes_played + this->seek_offset;
    emit this->playerPositionChanged(this->position);
}

void Player::handleStateChanged(QAudio::State new_state) {
    qDebug() << "QAudioOutput state changed to:" << new_state;
    qDebug() << "Error:" << this->audio->error();
}

void Player::handlePositionChanged(qint64 position) {
    this->position = position;
    emit this->playerPositionChanged(position);
}
