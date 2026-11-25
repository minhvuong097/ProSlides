#include "AudioMixer.h"

#include <QUrl>
#include <QVariantMap>

namespace ps::audio {

AudioMixer::AudioMixer(QObject* parent) : QObject(parent)
{
}

AudioMixer::~AudioMixer()
{
    clear();
}

int AudioMixer::find(const QString& id) const
{
    for (int i = 0; i < m_channels.size(); ++i) {
        if (m_channels[size_t(i)].id == id) return i;
    }
    return -1;
}

void AudioMixer::rebuild()
{
    m_model.clear();
    for (const auto& channel : m_channels) {
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), channel.id);
        m.insert(QStringLiteral(u"name"), channel.name);
        m.insert(QStringLiteral(u"volume"), channel.volume);
        m.insert(QStringLiteral(u"muted"), channel.muted);
        m.insert(QStringLiteral(u"solo"), channel.solo);
        m.insert(QStringLiteral(u"playing"), channel.player && channel.player->playbackState() == QMediaPlayer::PlayingState);
        m_model.append(m);
    }
    emit channelsChanged();
}

void AudioMixer::applyRouting()
{
    bool anySolo = false;
    for (const auto& channel : m_channels) {
        if (channel.solo) { anySolo = true; break; }
    }
    for (auto& channel : m_channels) {
        if (!channel.output) continue;
        const bool silenced = channel.muted || (anySolo && !channel.solo);
        channel.output->setMuted(silenced);
    }
    rebuild();
}

QString AudioMixer::addChannel(const QString& name, const QString& path)
{
    MixerChannel channel;
    channel.id = makeId();
    channel.name = name.isEmpty() ? path : name;
    channel.path = path;
    channel.output = new QAudioOutput(this);
    channel.output->setVolume(channel.volume);
    channel.player = new QMediaPlayer(this);
    channel.player->setAudioOutput(channel.output);
    channel.player->setSource(QUrl::fromLocalFile(path));
    connect(channel.player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioMixer::onStatus);
    m_channels.append(channel);
    applyRouting();
    return channel.id;
}

bool AudioMixer::removeChannel(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    auto channel = m_channels[size_t(i)];
    if (channel.player) channel.player->deleteLater();
    if (channel.output) channel.output->deleteLater();
    m_channels.removeAt(i);
    applyRouting();
    return true;
}

bool AudioMixer::playChannel(const QString& id)
{
    const int i = find(id);
    if (i < 0 || !m_channels[size_t(i)].player) return false;
    m_channels[size_t(i)].player->play();
    rebuild();
    return true;
}

bool AudioMixer::stopChannel(const QString& id)
{
    const int i = find(id);
    if (i < 0 || !m_channels[size_t(i)].player) return false;
    m_channels[size_t(i)].player->stop();
    rebuild();
    return true;
}

void AudioMixer::playAll()
{
    for (auto& channel : m_channels) {
        if (channel.player) channel.player->play();
    }
    rebuild();
}

void AudioMixer::stopAll()
{
    for (auto& channel : m_channels) {
        if (channel.player) channel.player->stop();
    }
    rebuild();
}

bool AudioMixer::setChannelVolume(const QString& id, double volume)
{
    const int i = find(id);
    if (i < 0) return false;
    m_channels[size_t(i)].volume = qBound(0.0, volume, 1.0);
    if (m_channels[size_t(i)].output)
        m_channels[size_t(i)].output->setVolume(m_channels[size_t(i)].volume);
    rebuild();
    return true;
}

bool AudioMixer::toggleMute(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    m_channels[size_t(i)].muted = !m_channels[size_t(i)].muted;
    applyRouting();
    return true;
}

bool AudioMixer::toggleSolo(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    m_channels[size_t(i)].solo = !m_channels[size_t(i)].solo;
    applyRouting();
    return true;
}

void AudioMixer::clear()
{
    for (auto& channel : m_channels) {
        if (channel.player) channel.player->deleteLater();
        if (channel.output) channel.output->deleteLater();
    }
    m_channels.clear();
    rebuild();
}

void AudioMixer::onStatus(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::EndOfMedia) return;
    auto* player = qobject_cast<QMediaPlayer*>(sender());
    for (auto& channel : m_channels) {
        if (channel.player != player) continue;
        if (channel.loop) {
            player->setPosition(0);
            player->play();
        } else {
            player->stop();
        }
        rebuild();
        return;
    }
}

} // namespace ps::audio
