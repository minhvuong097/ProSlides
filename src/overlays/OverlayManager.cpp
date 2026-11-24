#include "OverlayManager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>
#include <algorithm>

namespace ps::overlays {

QJsonObject Overlay::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(QStringLiteral(u"id"), id);
    o.insert(QStringLiteral(u"name"), name);
    if (!color.isEmpty()) o.insert(QStringLiteral(u"color"), color);
    if (!category.isEmpty()) o.insert(QStringLiteral(u"category"), category);
    if (isDefault) o.insert(QStringLiteral(u"isDefault"), true);
    if (locked) o.insert(QStringLiteral(u"locked"), true);
    if (placeUnderSlide) o.insert(QStringLiteral(u"placeUnderSlide"), true);
    if (displayDuration > 0) o.insert(QStringLiteral(u"displayDuration"), displayDuration);
    QJsonArray itemsArr;
    for (const auto& item : items) itemsArr.append(item.toJson());
    o.insert(QStringLiteral(u"items"), itemsArr);
    return o;
}

Overlay Overlay::fromJson(const QJsonObject& o)
{
    Overlay overlay;
    overlay.id = o.value(QStringLiteral(u"id")).toString();
    overlay.name = o.value(QStringLiteral(u"name")).toString();
    overlay.color = o.value(QStringLiteral(u"color")).toString();
    overlay.category = o.value(QStringLiteral(u"category")).toString();
    overlay.isDefault = o.value(QStringLiteral(u"isDefault")).toBool(false);
    overlay.locked = o.value(QStringLiteral(u"locked")).toBool(false);
    overlay.placeUnderSlide = o.value(QStringLiteral(u"placeUnderSlide")).toBool(false);
    overlay.displayDuration = o.value(QStringLiteral(u"displayDuration")).toInt();
    const auto arr = o.value(QStringLiteral(u"items")).toArray();
    for (const auto& v : arr) {
        if (v.isObject()) overlay.items.push_back(Item::fromJson(v.toObject()));
    }
    return overlay;
}

QJsonObject Scene::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(QStringLiteral(u"id"), id);
    o.insert(QStringLiteral(u"name"), name);
    if (!media.isEmpty()) {
        QJsonObject m;
        m.insert(QStringLiteral(u"path"), media);
        o.insert(QStringLiteral(u"media"), m);
    }
    if (!style.isEmpty()) o.insert(QStringLiteral(u"style"), style);
    if (!overlays.empty()) {
        QJsonArray arr;
        for (const auto& id : overlays) arr.append(id);
        o.insert(QStringLiteral(u"overlays"), arr);
    }
    if (!hiddenOverlays.empty()) {
        QJsonArray arr;
        for (const auto& id : hiddenOverlays) arr.append(id);
        o.insert(QStringLiteral(u"hiddenOverlays"), arr);
    }
    if (!bindings.isEmpty()) o.insert(QStringLiteral(u"bindings"), bindings);
    return o;
}

Scene Scene::fromJson(const QJsonObject& o)
{
    Scene scene;
    scene.id = o.value(QStringLiteral(u"id")).toString();
    scene.name = o.value(QStringLiteral(u"name")).toString();
    const auto media = o.value(QStringLiteral(u"media"));
    if (media.isObject()) scene.media = media.toObject().value(QStringLiteral(u"path")).toString();
    else if (media.isString()) scene.media = media.toString();
    scene.style = o.value(QStringLiteral(u"style")).toString();
    const auto oa = o.value(QStringLiteral(u"overlays")).toArray();
    for (const auto& v : oa) if (v.isString()) scene.overlays.push_back(v.toString());
    const auto ha = o.value(QStringLiteral(u"hiddenOverlays")).toArray();
    for (const auto& v : ha) if (v.isString()) scene.hiddenOverlays.push_back(v.toString());
    if (const auto b = o.constFind(QStringLiteral(u"bindings")); b != o.constEnd() && b->isObject())
        scene.bindings = b->toObject();
    return scene;
}

OverlayManager::OverlayManager(QObject* parent) : QObject(parent)
{
}

const Overlay* OverlayManager::findOverlay(const QString& id) const
{
    for (const auto& overlay : m_overlayList) {
        if (overlay.id == id) return &overlay;
    }
    return nullptr;
}

const Scene* OverlayManager::findScene(const QString& id) const
{
    for (const auto& scene : m_sceneList) {
        if (scene.id == id) return &scene;
    }
    return nullptr;
}

void OverlayManager::rebuildOverlays()
{
    m_overlayModel.clear();
    for (const auto& overlay : m_overlayList) {
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), overlay.id);
        m.insert(QStringLiteral(u"name"), overlay.name);
        m.insert(QStringLiteral(u"color"), overlay.color.isEmpty() ? QStringLiteral(u"#4aa8ff"): overlay.color);
        m.insert(QStringLiteral(u"category"), overlay.category);
        m.insert(QStringLiteral(u"active"), m_active.contains(overlay.id));
        m_overlayModel.append(m);
    }
    emit overlaysChanged();
}

void OverlayManager::rebuildScenes()
{
    m_sceneModel.clear();
    for (const auto& scene : m_sceneList) {
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), scene.id);
        m.insert(QStringLiteral(u"name"), scene.name);
        m.insert(QStringLiteral(u"media"), scene.media);
        m_sceneModel.append(m);
    }
    emit scenesChanged();
}

void OverlayManager::rebuildActive()
{
    m_activeModel.clear();
    for (const auto& id : m_active) {
        if (const auto* overlay = findOverlay(id)) {
            QVariantList items;
            for (const auto& item : overlay->items) items.append(item.toJson());
            QVariantMap m;
            m.insert(QStringLiteral(u"id"), overlay->id);
            m.insert(QStringLiteral(u"name"), overlay->name);
            m.insert(QStringLiteral(u"items"), items);
            m_activeModel.append(m);
        }
    }
    emit activeChanged();
}

void OverlayManager::load()
{
    m_overlayList.clear();
    m_sceneList.clear();

    QFile overlaysFile(QDir(m_directory).filePath(QStringLiteral(u"overlays.json")));
    if (overlaysFile.exists() && overlaysFile.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(overlaysFile.readAll());
        const auto arr = doc.isArray() ? doc.array() : doc.object().value(QStringLiteral(u"overlays")).toArray();
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            auto overlay = Overlay::fromJson(v.toObject());
            if (overlay.id.isEmpty()) overlay.id = makeId();
            m_overlayList.append(std::move(overlay));
        }
    }

    QFile scenesFile(QDir(m_directory).filePath(QStringLiteral(u"scenes.json")));
    if (scenesFile.exists() && scenesFile.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(scenesFile.readAll());
        const auto arr = doc.isArray() ? doc.array() : doc.object().value(QStringLiteral(u"scenes")).toArray();
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            auto scene = Scene::fromJson(v.toObject());
            if (scene.id.isEmpty()) scene.id = makeId();
            m_sceneList.append(std::move(scene));
        }
    }

    rebuildOverlays();
    rebuildScenes();
    rebuildActive();
}

void OverlayManager::showOverlay(const QString& id)
{
    if (!findOverlay(id) || m_active.contains(id)) return;
    m_active.append(id);
    rebuildOverlays();
    rebuildActive();
}

void OverlayManager::hideOverlay(const QString& id)
{
    if (!m_active.contains(id)) return;
    m_active.removeAll(id);
    rebuildOverlays();
    rebuildActive();
}

void OverlayManager::toggleOverlay(const QString& id)
{
    if (m_active.contains(id)) hideOverlay(id);
    else showOverlay(id);
}

void OverlayManager::clearOverlays()
{
    m_active.clear();
    rebuildOverlays();
    rebuildActive();
}

bool OverlayManager::startScene(const QString& id)
{
    const Scene* scene = findScene(id);
    if (!scene) return false;
    m_active.clear();
    for (const auto& overlay : scene->overlays) {
        if (std::find(scene->hiddenOverlays.begin(), scene->hiddenOverlays.end(), overlay) == scene->hiddenOverlays.end())
            m_active.append(overlay);
    }
    m_activeSceneName = scene->name;
    rebuildOverlays();
    rebuildActive();
    return true;
}

void OverlayManager::clearScene()
{
    m_active.clear();
    m_activeSceneName.clear();
    rebuildOverlays();
    rebuildActive();
}

} // namespace ps::overlays
