#include "StageManager.h"

#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QQmlProperty>
#include <QGuiApplication>
#include <QScreen>

namespace ps::stage {

StageManager::StageManager(QObject* parent) : QObject(parent)
{
}

StageManager::~StageManager()
{
    close();
}

void StageManager::setState(const QVariantMap& state)
{
    m_state = state;
    emit stateChanged();
    pushToWindow();
}

void StageManager::pushToWindow()
{
    if (!m_window) return;
    QQmlProperty(m_window, QStringLiteral(u"stageState")).write(m_state);
}

bool StageManager::show(QQmlEngine* engine, int screenIndex)
{
    if (!engine) return false;
    if (m_window) {
        m_window->showFullScreen();
        return true;
    }

    QQmlComponent component(engine, QUrl(QStringLiteral(u"qrc:/qml/views/StageView.qml")));
    if (component.isError()) {
        qWarning("StageView QML errors: %s", qPrintable(component.errorString()));
        return false;
    }
    auto* window = qobject_cast<QQuickWindow*>(component.create());
    if (!window) return false;

    const auto screens = QGuiApplication::screens();
    if (screenIndex >= 0 && screenIndex < screens.size())
        window->setScreen(screens.at(screenIndex));
    window->setColor(QColor(QStringLiteral(u"#08080d")));
    QQmlProperty(window, QStringLiteral(u"stageState")).write(m_state);
    window->showFullScreen();
    m_window = window;
    emit visibleChanged();
    return true;
}

void StageManager::close()
{
    if (!m_window) return;
    m_window->close();
    delete m_window;
    m_window = nullptr;
    emit visibleChanged();
}

void StageManager::toggle(QQmlEngine* engine, int screenIndex)
{
    if (m_window) close();
    else show(engine, screenIndex);
}

} // namespace ps::stage
