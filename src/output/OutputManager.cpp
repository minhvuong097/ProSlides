#include "OutputManager.h"

#include <QDateTime>
#include <QMetaObject>
#include <QQmlProperty>

namespace ps::output {

OutputManager::OutputManager(QObject* parent)
    : QObject(parent)
{
    m_clock.setInterval(1000);
    connect(&m_clock, &QTimer::timeout, this, &OutputManager::updateClock);
    m_clock.start();
    updateClock();
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
    m_clockText = QDateTime::currentDateTime().toString(u"hh:mm:ss");
    emit tick();
}

void OutputManager::pushToDisplays()
{
    const QJsonObject slide = currentSlide();
    for (auto* display : m_displays) {
        if (!display || !display->window()) continue;
        QQmlProperty(display->window(), u"currentSlide").write(slide);
        QQmlProperty(display->window(), u"cleared").write(m_cleared || m_slideHidden);
        QQmlProperty(display->window(), u"backgroundPath").write(m_background);
    }
}

} // namespace ps::output
