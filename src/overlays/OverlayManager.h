#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QJsonObject>

namespace ps::overlays {

// FreeShow Overlay type.
struct Overlay {
    Id id;
    QString name;
    QString color;
    QString category;
    bool isDefault = false;
    bool locked = false;
    bool placeUnderSlide = false;
    int displayDuration = 0; // ms, 0 = until cleared
    std::vector<Item> items;

    QJsonObject toJson() const;
    static Overlay fromJson(const QJsonObject& o);
};

// FreeShow SceneContent / Scene.
struct Scene {
    Id id;
    QString name;
    QString media;                 // media path
    QString style;                 // CSS style
    std::vector<Id> overlays;
    std::vector<Id> hiddenOverlays;
    QJsonObject bindings;

    QJsonObject toJson() const;
    static Scene fromJson(const QJsonObject& o);
};

// Owns overlays/scenes, tracks which overlays are active and drives the
// audience display through OutputManager.
class OverlayManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList overlays READ overlays NOTIFY overlaysChanged)
    Q_PROPERTY(QVariantList scenes READ scenes NOTIFY scenesChanged)
    Q_PROPERTY(QVariantList activeOverlays READ activeOverlays NOTIFY activeChanged)
    Q_PROPERTY(QString activeScene READ activeScene NOTIFY activeChanged)

public:
    explicit OverlayManager(QObject* parent = nullptr);

    void setDirectory(const QString& dir) { m_directory = dir; }
    QString directory() const { return m_directory; }

    QVariantList overlays() const { return m_overlayModel; }
    QVariantList scenes() const { return m_sceneModel; }
    QVariantList activeOverlays() const { return m_activeModel; }
    QString activeScene() const { return m_activeSceneName; }

    void load();

    Q_INVOKABLE void showOverlay(const QString& id);
    Q_INVOKABLE void hideOverlay(const QString& id);
    Q_INVOKABLE void toggleOverlay(const QString& id);
    Q_INVOKABLE void clearOverlays();
    Q_INVOKABLE bool startScene(const QString& id);
    Q_INVOKABLE void clearScene();

signals:
    void overlaysChanged();
    void scenesChanged();
    void activeChanged();
    void errorOccurred(const QString& error);

private:
    void rebuildOverlays();
    void rebuildScenes();
    void rebuildActive();
    const Overlay* findOverlay(const QString& id) const;
    const Scene* findScene(const QString& id) const;

    QString m_directory;
    QVector<Overlay> m_overlayList;
    QVector<Scene> m_sceneList;
    QVariantList m_overlayModel;
    QVariantList m_sceneModel;
    QVariantList m_activeModel;
    QVector<Id> m_active;
    QString m_activeSceneName;
};

} // namespace ps::overlays
