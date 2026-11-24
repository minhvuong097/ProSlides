#include "DisplayOutput.h"

#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QApplication>
#include <QScreen>

namespace ps::output {

DisplayOutput::DisplayOutput(int screenIndex, QQmlEngine* engine, QObject* parent)
    : QObject(parent), m_screenIndex(screenIndex)
{
    QQmlComponent component(engine, QUrl(u"qrc:/qml/views/PresenterView.qml"));
    if (component.isError()) {
        qWarning("DisplayOutput QML errors: %s", qPrintable(component.errorString()));
        return;
    }
    auto* obj = qobject_cast<QQuickWindow*>(component.create());
    if (!obj) return;
    m_window = obj;
    m_window->setColor(QColor(u"#000000"));
}

void DisplayOutput::showFullscreen()
{
    if (!m_window) return;
    const auto screens = QApplication::screens();
    if (m_screenIndex >= 0 && m_screenIndex < screens.size())
        m_window->setScreen(screens.at(m_screenIndex));
    m_window->showFullScreen();
}

void DisplayOutput::showWindowed()
{
    if (m_window) m_window->show();
}

void DisplayOutput::hide()
{
    if (m_window) m_window->hide();
}

} // namespace ps::output
