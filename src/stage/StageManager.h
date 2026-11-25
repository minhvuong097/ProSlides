#pragma once

#include <QObject>
#include <QVariantMap>

class QQmlEngine;
class QQuickWindow;

namespace ps::stage {

// Dedicated "stage" window for singers/technicians: current slide, next slide,
// notes and a running clock. Mirrors FreeShow's STAGE output concept.
class StageManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)

public:
    explicit StageManager(QObject* parent = nullptr);
    ~StageManager() override;

    QVariantMap state() const { return m_state; }
    bool visible() const { return m_window != nullptr; }

    void setState(const QVariantMap& state);

    Q_INVOKABLE bool show(QQmlEngine* engine, int screenIndex = 1);
    Q_INVOKABLE void close();
    Q_INVOKABLE void toggle(QQmlEngine* engine, int screenIndex = 1);

signals:
    void stateChanged();
    void visibleChanged();

private:
    void pushToWindow();

    QVariantMap m_state;
    QQuickWindow* m_window = nullptr;
};

} // namespace ps::stage
