#pragma once

#include "DisplayOutput.h"

#include <QObject>
#include <QJsonObject>
#include <QVector>
#include <QTimer>

class QQmlEngine;

namespace ps::output {

// Owns the sequence being presented and all physical output windows.
// PresentationController mutates this state; QML binds to its properties.
class OutputManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY stateChanged)
    Q_PROPERTY(int slideCount READ slideCount NOTIFY slidesChanged)
    Q_PROPERTY(QJsonObject currentSlide READ currentSlide NOTIFY stateChanged)
    Q_PROPERTY(bool cleared READ cleared NOTIFY stateChanged)
    Q_PROPERTY(QString backgroundPath READ backgroundPath NOTIFY stateChanged)
    Q_PROPERTY(QString transitionType READ transitionType NOTIFY stateChanged)
    Q_PROPERTY(QString clockText READ clockText NOTIFY tick)
    Q_PROPERTY(int displayCount READ displayCount NOTIFY displayCountChanged)

public:
    explicit OutputManager(QObject* parent = nullptr);

    int currentIndex() const { return m_index; }
    int slideCount() const { return m_slides.size(); }
    int displayCount() const { return m_displays.size(); }
    QJsonObject currentSlide() const;
    bool cleared() const { return m_cleared; }
    QString backgroundPath() const { return m_background; }
    QString transitionType() const { return m_transition; }
    QString clockText() const { return m_clockText; }

    const QVector<QJsonObject>& slides() const { return m_slides; }

public slots:
    void setSlides(const QVector<QJsonObject>& slides);
    void goTo(int index);
    void next();
    void previous();
    void clearAll();
    void restore();
    void clearSlide();
    void setBackground(const QString& path);
    void setTransition(const QString& type);
    void attachEngine(QQmlEngine* engine);
    bool addDisplay(int screenIndex);
    void removeAllDisplays();

signals:
    void stateChanged();
    void slidesChanged();
    void tick();
    void slideChanged(const QJsonObject& slide);
    void displayCountChanged(int count);

private slots:
    void updateClock();

private:
    void pushToDisplays();

    QVector<QJsonObject> m_slides;
    QVector<DisplayOutput*> m_displays;
    QQmlEngine* m_engine = nullptr;
    int m_index = -1;
    bool m_cleared = false;
    bool m_slideHidden = false;
    QString m_background;
    QString m_transition = u"fade";
    QString m_clockText;
    QTimer m_clock;
};

} // namespace ps::output
