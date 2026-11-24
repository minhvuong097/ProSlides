#pragma once

#include <QObject>
#include <QUrl>

class QQmlEngine;
class QQuickWindow;

namespace ps::output {

// A fullscreen QML window placed on a physical screen (audience display).
class DisplayOutput : public QObject
{
    Q_OBJECT
public:
    explicit DisplayOutput(int screenIndex, QQmlEngine* engine, QObject* parent = nullptr);

    bool isValid() const { return m_window != nullptr; }
    QQuickWindow* window() const { return m_window; }
    int screenIndex() const { return m_screenIndex; }

public slots:
    void showFullscreen();
    void showWindowed();
    void hide();

private:
    int m_screenIndex = 0;
    QQuickWindow* m_window = nullptr;
};

} // namespace ps::output
