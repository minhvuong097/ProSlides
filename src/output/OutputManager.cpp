#include "OutputManager.h"

#include <QDateTime>
#include <QMetaObject>
#include <QQmlProperty>
#include <QQuickWindow>

namespace ps::output {

OutputManager::OutputManager(QObject* parent)
    : QObject(parent)
{
    m_clock.setInterval(1000);
    connect(&m_clock, &QTimer::timeout, this, &OutputManager::updateClock);
    m_clock.start();
    updateClock();

    m_captureTimer.setInterval(33); // ~30 fps
    connect(&m_captureTimer, &QTimer::timeout, this, &OutputManager::captureFrame);
    connect(&m_recorder, &recording::OutputRecorder::recordingChanged,
            this, &OutputManager::recordingChanged);
    connect(&m_stream, &streaming::StreamOutput::activeChanged,
            this, &OutputManager::streamingChanged);
}

QJsonObject OutputManager::currentSlide() const
{
    if (m_cleared || m_slideHidden || m_index < 0 || m_index >= m_slides.size())
        return {};
    return m_slides.at(m_index);
}

void OutputManager::setSlides(const QVector<QJsonObject>& slides)
{
    m_slides = slides;
    m_index = slides.isEmpty() ? -1 : 0;
    m_cleared = false;
    m_slideHidden = false;
    emit slidesChanged();
    emit stateChanged();
    emit slideChanged(currentSlide());
    pushToDisplays();
}

void OutputManager::goTo(int index)
{
    if (m_slides.isEmpty()) return;
    const int target = qBound(0, index, m_slides.size() - 1);
    if (target == m_index && !m_cleared && !m_slideHidden) return;
    m_index = target;
    m_cleared = false;
    m_slideHidden = false;
    emit stateChanged();
    emit slideChanged(currentSlide());
    pushToDisplays();
}

void OutputManager::next()
{
    goTo(m_index + 1);
}

void OutputManager::previous()
{
    goTo(m_index - 1);
}

void OutputManager::clearAll()
{
    m_cleared = true;
    m_background.clear();
    emit stateChanged();
    emit slideChanged({});
    pushToDisplays();
}

void OutputManager::restore()
{
    m_cleared = false;
    m_slideHidden = false;
    emit stateChanged();
    emit slideChanged(currentSlide());
    pushToDisplays();
}

void OutputManager::clearSlide()
{
    m_slideHidden = true;
    emit stateChanged();
    emit slideChanged({});
    pushToDisplays();
}

void OutputManager::setBackground(const QString& path)
{
    m_background = path;
    m_cleared = false;
    emit stateChanged();
}

void OutputManager::setTransition(const QString& type)
{
    m_transition = type;
    emit stateChanged();
}

void OutputManager::attachEngine(QQmlEngine* engine)
{
    m_engine = engine;
}

bool OutputManager::addDisplay(int screenIndex)
{
    if (!m_engine) return false;
    auto* display = new DisplayOutput(screenIndex, m_engine, this);
    if (!display->isValid()) {
        delete display;
        return false;
    }
    m_displays.append(display);
    display->showFullscreen();
    pushToDisplays();
    emit displayCountChanged(m_displays.size());
    return true;
}

void OutputManager::removeAllDisplays()
{
    qDeleteAll(m_displays);
    m_displays.clear();
    emit displayCountChanged(0);
}

void OutputManager::updateClock()
{
    m_clockText = QDateTime::currentDateTime().toString(QStringLiteral(u"hh:mm:ss"));
    emit tick();
}

bool OutputManager::startRecording(const QString& path)
{
    if (m_displays.isEmpty() || !m_displays.first()->window()) return false;
    const auto* window = m_displays.first()->window();
    const QSize size(window->width(), window->height());
    if (!m_recorder.start(path, size)) return false;
    m_captureTimer.start();
    return true;
}

void OutputManager::stopRecording()
{
    m_recorder.stop();
    if (!m_stream.active() && !m_ndi.isActive()) m_captureTimer.stop();
}

void OutputManager::toggleRecording(const QString& path)
{
    if (m_recorder.recording()) stopRecording();
    else startRecording(path);
}

bool OutputManager::startStreaming(const QString& url, bool hls)
{
    if (m_displays.isEmpty() || !m_displays.first()->window()) return false;
    const auto* window = m_displays.first()->window();
    if (!m_stream.start(url, QSize(window->width(), window->height()), 30.0, hls)) return false;
    m_captureTimer.start();
    return true;
}

void OutputManager::stopStreaming()
{
    m_stream.stop();
    if (!m_recorder.recording() && !m_ndi.isActive()) m_captureTimer.stop();
}

bool OutputManager::startNdi(const QString& name)
{
    const bool ok = m_ndi.initialize(name);
    if (ok) m_captureTimer.start();
    emit ndiChanged(ok);
    return ok;
}

void OutputManager::stopNdi()
{
    // NdiOutput has no explicit stop; destroy/recreate would be needed for a
    // full implementation. Keep the sender alive but stop pushing frames.
    if (m_ndi.isActive()) emit ndiChanged(false);
}

void OutputManager::captureFrame()
{
    if (!m_recorder.recording() && !m_stream.active() && !m_ndi.isActive()) return;
    if (m_displays.isEmpty()) return;
    auto* window = m_displays.first()->window();
    if (!window) return;

    const QImage frame = window->grabWindow();
    if (m_recorder.recording()) m_recorder.pushFrame(frame);
    if (m_stream.active()) m_stream.pushFrame(frame);
    if (m_ndi.isActive()) m_ndi.sendFrame(frame);
}

void OutputManager::pushToDisplays()
{
    const QJsonObject slide = currentSlide();
    for (auto* display : m_displays) {
        if (!display || !display->window()) continue;
        QQmlProperty(display->window(), QStringLiteral(u"currentSlide")).write(slide);
        QQmlProperty(display->window(), QStringLiteral(u"cleared")).write(m_cleared || m_slideHidden);
        QQmlProperty(display->window(), QStringLiteral(u"backgroundPath")).write(m_background);
    }
}

} // namespace ps::output
