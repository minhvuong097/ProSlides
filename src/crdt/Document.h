#pragma once

#include "LamportClock.h"
#include "Operation.h"

#include <QObject>
#include <QJsonObject>
#include <QSet>
#include <QVector>

namespace ps::crdt {

// Automerge-inspired replicated document:
//   * field writes are last-writer-wins registers ordered by (Lamport, actor)
//   * sequences use an RGA-style insert/delete/move algorithm
//   * every operation is retained in a log so peers can sync by vector clock
//
// The document stores an arbitrary JSON tree, which maps 1:1 onto ProSlides'
// Show/Slide/Item model via `path` arguments.
class Document : public QObject
{
    Q_OBJECT

public:
    explicit Document(QString actorId = {}, QObject* parent = nullptr);

    QString actorId() const { return m_clock.actor(); }
    VectorClock vectorClock() const { return m_clock.vector(); }
    int operationCount() const { return m_ops.size(); }

    // --- reads -----------------------------------------------------------
    QJsonValue get(const QStringList& path) const;
    QVector<QJsonValue> sequence(const QStringList& path) const;
    QVector<OpId> sequenceIds(const QStringList& path) const;
    QJsonObject toJson() const;

    // --- local writes (generate + apply operations) ----------------------
    void set(const QStringList& path, QJsonValue value);
    void erase(const QStringList& path);
    void insert(const QStringList& path, int index, QJsonValue value);
    void eraseElement(const QStringList& path, const OpId& elementId);
    void moveElement(const QStringList& path, const OpId& elementId, int index);

    // --- replication -----------------------------------------------------
    bool apply(const Operation& op);               // idempotent
    bool applyBatch(const QVector<Operation>& ops);
    QVector<Operation> operationsSince(const VectorClock& since) const;
    void mergeVectorClock(const VectorClock& other);

    QByteArray serialize() const;
    static Document deserialize(const QByteArray& data, QString actorId = {});

signals:
    void operationReady(const Operation& op);      // broadcast this op
    void changed();

private:
    struct FieldState {
        OpId winner;
        QJsonValue value;
        bool deleted = false;
    };
    struct SeqElement {
        OpId id;
        OpId parent;
        QJsonValue value;
        bool deleted = false;
    };

    QString key(const QStringList& path) const { return path.join(u'/'); }
    void emitLocal(Operation op);
    void sortSequence(const QString& seqKey) const;
    QVector<SeqElement>& seq(const QString& seqKey);
    const QVector<SeqElement>& seq(const QString& seqKey) const;
    bool hasAncestor(const QString& seqKey, const OpId& element, const OpId& possibleAncestor) const;

    mutable QMap<QString, QVector<SeqElement>> m_sequences;
    QMap<QString, FieldState> m_fields;
    QVector<Operation> m_ops;
    QSet<QString> m_seen;
    LamportClock m_clock;
};

} // namespace ps::crdt
