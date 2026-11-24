#pragma once

#include "Types.h"

#include <QObject>
#include <QJsonObject>

namespace ps {

// Owns a single Show and exposes high-level editing operations with an
// in-memory undo/redo stack (snapshots of the JSON document).
class ShowDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY historyChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY historyChanged)

public:
    explicit ShowDocument(QObject* parent = nullptr);

    const Show& show() const { return m_show; }
    Show& show() { return m_show; }
    void setShow(Show show, bool clearHistory = true);

    QString name() const { return m_show.name; }
    void setName(const QString& name);

    bool modified() const { return m_modified; }
    bool canUndo() const { return !m_undo.isEmpty(); }
    bool canRedo() const { return !m_redo.isEmpty(); }

public slots:
    bool undo();
    bool redo();
    void markSaved();

    // --- slides ---------------------------------------------------------
    Id addSlide(Slide slide, const QString& group = {});
    bool updateSlide(const Id& id, Slide slide);
    bool removeSlide(const Id& id);
    Id duplicateSlide(const Id& id);
    bool moveSlide(const Id& id, int delta);

    // MxU-style fast entry: create a lyric slide from plain lines with
    // optional chord markers (e.g. "[C]Amazing grace").
    Id addLyricSlide(const QString& group, const QString& text,
                     const QString& key = {}, const QString& style = {});

    Id addMediaSlide(const QString& group, const QString& mediaPath,
                     MediaType type = MediaType::Image);

    // --- layouts --------------------------------------------------------
    Id addLayout(const QString& name);
    bool renameLayout(const Id& id, const QString& name);
    bool removeLayout(const Id& id);
    bool setActiveLayout(const Id& id);
    bool appendToLayout(const Id& slideId, const Id& layoutId = {});

    // --- media ----------------------------------------------------------
    Id addMedia(const MediaDescriptor& media);
    bool removeMedia(const Id& id);

signals:
    void changed();
    void modifiedChanged(bool modified);
    void historyChanged();
    void slideAdded(const Id& id);
    void slideRemoved(const Id& id);
    void activeLayoutChanged(const Id& id);

private:
    void pushUndo();
    void commit(const QString& signalName = {});

    Show m_show;
    QList<QJsonObject> m_undo;
    QList<QJsonObject> m_redo;
    bool m_modified = false;
    static constexpr int kMaxHistory = 100;
};

} // namespace ps
