#include "Document.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>
#include <functional>

namespace ps::crdt {

namespace {
void setNested(QJsonObject& node, const QStringList& parts, const QJsonValue& value)
{
    if (parts.isEmpty()) return;
    if (parts.size() == 1) {
        node.insert(parts.first(), value);
        return;
    }
    QJsonObject child = node.value(parts.first()).toObject();
    setNested(child, parts.mid(1), value);
    node.insert(parts.first(), child);
}
} // namespace

Document::Document(QString actorId, QObject* parent)
    : QObject(parent)
    , m_clock(actorId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                                : std::move(actorId))
{
}

// ---------------------------------------------------------------------------
// reads
// ---------------------------------------------------------------------------
QJsonValue Document::get(const QStringList& path) const
{
    const auto it = m_fields.constFind(key(path));
    if (it == m_fields.constEnd() || it->deleted) return QJsonValue::Undefined;
    return it->value;
}

const QVector<Document::SeqElement>& Document::seq(const QString& seqKey) const
{
    static const QVector<SeqElement> empty;
    const auto it = m_sequences.constFind(seqKey);
    return it == m_sequences.constEnd() ? empty : *it;
}

QVector<Document::SeqElement>& Document::seq(const QString& seqKey)
{
    return m_sequences[seqKey];
}

QVector<QJsonValue> Document::sequence(const QStringList& path) const
{
    QVector<QJsonValue> out;
    for (const auto& e : seq(key(path))) {
        if (!e.deleted) out << e.value;
    }
    return out;
}

QVector<OpId> Document::sequenceIds(const QStringList& path) const
{
    QVector<OpId> out;
    for (const auto& e : seq(key(path))) {
        if (!e.deleted) out << e.id;
    }
    return out;
}

QJsonObject Document::toJson() const
{
    QJsonObject out;
    for (auto it = m_fields.constBegin(); it != m_fields.constEnd(); ++it) {
        if (it->deleted) continue;
        setNested(out, it.key().split(u'/'), it->value);
    }
    return out;
}

// ---------------------------------------------------------------------------
// local writes
// ---------------------------------------------------------------------------
void Document::emitLocal(Operation op)
{
    apply(op);
    emit operationReady(op);
    emit changed();
}

void Document::set(const QStringList& path, QJsonValue value)
{
    Operation op;
    op.id = m_clock.tick();
    op.kind = OpKind::SetField;
    op.path = path;
    op.value = std::move(value);
    emitLocal(std::move(op));
}

void Document::erase(const QStringList& path)
{
    Operation op;
    op.id = m_clock.tick();
    op.kind = OpKind::DeleteField;
    op.path = path;
    emitLocal(std::move(op));
}

void Document::insert(const QStringList& path, int index, QJsonValue value)
{
    auto& elements = seq(key(path));
    sortSequence(key(path));
    const int count = std::count_if(elements.begin(), elements.end(),
                                    [](const SeqElement& e) { return !e.deleted; });
    const int at = std::clamp(index, 0, count);

    OpId parent;
    int seen = 0;
    for (const auto& e : elements) {
        if (e.deleted) continue;
        if (seen == at - 1) { parent = e.id; break; }
        ++seen;
    }
    if (at == 0) parent = {};

    Operation op;
    op.id = m_clock.tick();
    op.kind = OpKind::SeqInsert;
    op.path = path;
    op.value = std::move(value);
    op.parent = parent;
    emitLocal(std::move(op));
}

void Document::eraseElement(const QStringList& path, const OpId& elementId)
{
    Operation op;
    op.id = m_clock.tick();
    op.kind = OpKind::SeqDelete;
    op.path = path;
    op.target = elementId;
    emitLocal(std::move(op));
}

void Document::moveElement(const QStringList& path, const OpId& elementId, int index)
{
    if (hasAncestor(key(path), elementId, elementId)) return;

    Operation op;
    op.id = m_clock.tick();
    op.kind = OpKind::SeqMove;
    op.path = path;
    op.target = elementId;

    auto& elements = seq(key(path));
    sortSequence(key(path));
    const int count = std::count_if(elements.begin(), elements.end(),
                                    [](const SeqElement& e) { return !e.deleted; });
    const int at = std::clamp(index, 0, count);
    int seen = 0;
    for (const auto& e : elements) {
        if (e.deleted || e.id == elementId) continue;
        if (seen == at - 1) { op.parent = e.id; break; }
        ++seen;
    }
    if (at == 0) op.parent = {};
    emitLocal(std::move(op));
}

bool Document::hasAncestor(const QString& seqKey, const OpId& element,
                           const OpId& possibleAncestor) const
{
    OpId cursor = element;
    QSet<QString> visited;
    while (!cursor.actor.isEmpty()) {
        if (visited.contains(cursor.toString())) return false;
        visited.insert(cursor.toString());
        if (cursor == possibleAncestor && cursor != element) return true;
        OpId next;
        for (const auto& e : seq(seqKey)) {
            if (e.id == cursor) { next = e.parent; break; }
        }
        cursor = next;
    }
    return false;
}

// ---------------------------------------------------------------------------
// operation application (remote + local)
// ---------------------------------------------------------------------------
bool Document::apply(const Operation& op)
{
    if (op.id.actor.isEmpty() || m_seen.contains(op.id.toString())) return false;
    m_seen.insert(op.id.toString());
    m_clock.observe(op.id);
    m_ops.append(op);

    const QString k = key(op.path);
    switch (op.kind) {
    case OpKind::SetField: {
        auto& state = m_fields[k];
        if (state.winner.actor.isEmpty() || op.id < state.winner ||
            (op.id == state.winner && op.id.actor > state.winner.actor)) {
            state.winner = op.id;
            state.value = op.value;
            state.deleted = false;
        }
        break;
    }
    case OpKind::DeleteField: {
        auto& state = m_fields[k];
        if (state.winner.actor.isEmpty() || op.id < state.winner ||
            (op.id == state.winner && op.id.actor > state.winner.actor)) {
            state.winner = op.id;
            state.value = QJsonValue::Undefined;
            state.deleted = true;
        }
        break;
    }
    case OpKind::SeqInsert: {
        auto& elements = seq(k);
        SeqElement e;
        e.id = op.id;
        e.parent = op.parent;
        e.value = op.value;
        if (std::none_of(elements.begin(), elements.end(),
                         [&id = e.id](const SeqElement& x) { return x.id == id; })) {
            elements.append(std::move(e));
        }
        sortSequence(k);
        break;
    }
    case OpKind::SeqDelete: {
        for (auto& e : seq(k)) {
            if (e.id == op.target) { e.deleted = true; break; }
        }
        break;
    }
    case OpKind::SeqMove: {
        for (auto& e : seq(k)) {
            if (e.id == op.target) { e.parent = op.parent; break; }
        }
        sortSequence(k);
        break;
    }
    }
    return true;
}

bool Document::applyBatch(const QVector<Operation>& ops)
{
    bool changedAny = false;
    // Causal delivery: smallest Lamport timestamp first.
    QVector<Operation> sorted = ops;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const Operation& a, const Operation& b) { return a.id < b.id; });
    for (const auto& op : sorted) changedAny |= apply(op);
    if (changedAny) emit changed();
    return changedAny;
}

void Document::sortSequence(const QString& seqKey) const
{
    auto& elements = m_sequences[seqKey];
    if (elements.isEmpty()) return;

    QMap<QString, QVector<int>> children;
    for (int i = 0; i < elements.size(); ++i)
        children[elements[i].parent.toString()].append(i);

    for (auto& list : children) {
        std::sort(list.begin(), list.end(), [&elements](int a, int b) {
            return elements[a].id < elements[b].id; // RGA: newer sibling first
        });
    }

    QVector<int> order;
    QSet<QString> visited;
    std::function<void(const QString&)> walk = [&](const QString& parentKey) {
        for (int idx : children.value(parentKey)) {
            if (visited.contains(elements[idx].id.toString())) continue;
            visited.insert(elements[idx].id.toString());
            order.append(idx);
            walk(elements[idx].id.toString());
        }
    };
    walk({});

    // Append any elements whose parent is missing (e.g. received out of order).
    for (int i = 0; i < elements.size(); ++i) {
        if (!visited.contains(elements[i].id.toString())) order.append(i);
    }

    QVector<SeqElement> sorted;
    sorted.reserve(elements.size());
    for (int idx : order) sorted.append(elements[idx]);
    elements = std::move(sorted);
}

// ---------------------------------------------------------------------------
// replication
// ---------------------------------------------------------------------------
QVector<Operation> Document::operationsSince(const VectorClock& since) const
{
    QVector<Operation> out;
    for (const auto& op : m_ops) {
        if (since.value(op.id.actor, 0) < op.id.time) out.append(op);
    }
    return out;
}

void Document::mergeVectorClock(const VectorClock& other)
{
    m_clock.observe(other);
}

QByteArray Document::serialize() const
{
    QJsonObject root;
    root.insert(u"actor", actorId());
    root.insert(u"clock", m_clock.value());
    QJsonObject vector;
    const auto vc = m_clock.vector();
    for (auto it = vc.constBegin(); it != vc.constEnd(); ++it)
        vector.insert(it.key(), it.value());
    root.insert(u"vector", vector);
    QJsonArray ops;
    for (const auto& op : m_ops) ops.append(op.toJson());
    root.insert(u"ops", ops);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

Document Document::deserialize(const QByteArray& data, QString actorId)
{
    Document doc(std::move(actorId));
    const auto obj = QJsonDocument::fromJson(data).object();
    VectorClock vc;
    const auto vobj = obj.value(u"vector").toObject();
    for (auto it = vobj.constBegin(); it != vobj.constEnd(); ++it)
        vc.insert(it.key(), static_cast<ClockValue>(it.value().toDouble()));
    doc.m_clock.setVector(vc);

    QVector<Operation> ops;
    const auto arr = obj.value(u"ops").toArray();
    for (const auto& v : arr) {
        if (v.isObject()) ops.append(Operation::fromJson(v.toObject()));
    }
    doc.applyBatch(ops);
    return doc;
}

} // namespace ps::crdt
