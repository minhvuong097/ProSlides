#include "TriggerManager.h"
#include "MidiInput.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>

namespace ps::triggers {

QJsonObject Trigger::toJson() const
{
    QJsonObject o;
    if (!id.isEmpty()) o.insert(QStringLiteral(u"id"), id);
    o.insert(QStringLiteral(u"action"), action);
    if (!data.isEmpty()) o.insert(QStringLiteral(u"data"), data);
    o.insert(QStringLiteral(u"source"), source);
    o.insert(QStringLiteral(u"channel"), channel);
    o.insert(QStringLiteral(u"note"), note);
    if (isControl) o.insert(QStringLiteral(u"control"), true);
    if (minValue != 1) o.insert(QStringLiteral(u"minValue"), minValue);
    if (!key.isEmpty()) o.insert(QStringLiteral(u"key"), key);
    return o;
}

Trigger Trigger::fromJson(const QJsonObject& o)
{
    Trigger t;
    t.id = o.value(QStringLiteral(u"id")).toString();
    t.action = o.value(QStringLiteral(u"action")).toString();
    if (const auto v = o.constFind(QStringLiteral(u"data")); v != o.constEnd() && v->isObject())
        t.data = v->toObject();
    t.source = o.value(QStringLiteral(u"source")).toString();
    t.channel = o.value(QStringLiteral(u"channel")).toInt();
    t.note = o.value(QStringLiteral(u"note")).toInt(-1);
    t.isControl = o.value(QStringLiteral(u"control")).toBool(false);
    t.minValue = o.value(QStringLiteral(u"minValue")).toInt(1);
    t.key = o.value(QStringLiteral(u"key")).toString();
    return t;
}

TriggerManager::TriggerManager(QObject* parent)
    : QObject(parent), m_midi(new MidiInput(this))
{
    connect(m_midi, &MidiInput::midiMessage,
            this, &TriggerManager::handleMidi);
    connect(m_midi, &MidiInput::errorOccurred,
            this, &TriggerManager::errorOccurred);
}

TriggerManager::~TriggerManager() = default;

bool TriggerManager::midiConnected() const
{
    return m_midi->isOpen();
}

void TriggerManager::rebuild()
{
    m_model.clear();
    for (const auto& trigger : m_triggers) {
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), trigger.id);
        m.insert(QStringLiteral(u"action"), trigger.action);
        m.insert(QStringLiteral(u"source"), trigger.source);
        m.insert(QStringLiteral(u"note"), trigger.note);
        m.insert(QStringLiteral(u"key"), trigger.key);
        m.insert(QStringLiteral(u"enabled"), true);
        m_model.append(m);
    }
    emit triggersChanged();
}

void TriggerManager::load()
{
    m_triggers.clear();
    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(file.readAll());
        const auto arr = doc.isArray() ? doc.array() : doc.object().value(QStringLiteral(u"triggers")).toArray();
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            auto trigger = Trigger::fromJson(v.toObject());
            if (trigger.id.isEmpty()) trigger.id = makeId();
            m_triggers.append(std::move(trigger));
        }
    }
    rebuild();
}

bool TriggerManager::connectMidi(int device)
{
    const bool ok = m_midi->open(device);
    emit midiChanged(ok);
    return ok;
}

void TriggerManager::disconnectMidi()
{
    m_midi->close();
    emit midiChanged(false);
}

QString TriggerManager::addMidiTrigger(const QString& action, int note,
                                       bool isControl, int channel, int minValue)
{
    Trigger t;
    t.id = makeId();
    t.action = action;
    t.source = QStringLiteral(u"midi");
    t.note = note;
    t.isControl = isControl;
    t.channel = channel;
    t.minValue = minValue;
    m_triggers.append(std::move(t));
    rebuild();
    return m_triggers.last().id;
}

QString TriggerManager::addKeyTrigger(const QString& action, const QString& key)
{
    Trigger t;
    t.id = makeId();
    t.action = action;
    t.source = QStringLiteral(u"key");
    t.key = key;
    m_triggers.append(std::move(t));
    rebuild();
    return m_triggers.last().id;
}

bool TriggerManager::removeTrigger(const QString& id)
{
    for (int i = 0; i < m_triggers.size(); ++i) {
        if (m_triggers[size_t(i)].id == id) {
            m_triggers.removeAt(i);
            rebuild();
            return true;
        }
    }
    return false;
}

void TriggerManager::dispatch(const Trigger& trigger)
{
    emit actionRequested(trigger.action, trigger.data);
}

void TriggerManager::handleMidi(int type, int channel, int data1, int data2)
{
    const bool noteOn = type == 0x90 && data2 > 0;
    const bool control = type == 0xB0;
    if (!noteOn && !control) return;

    for (const auto& trigger : m_triggers) {
        if (trigger.source != QStringLiteral(u"midi")) continue;
        if (trigger.isControl != control) continue;
        if (trigger.channel >= 0 && trigger.channel != channel) continue;
        if (trigger.note != data1) continue;
        if (data2 < trigger.minValue) continue;
        dispatch(trigger);
    }
}

void TriggerManager::triggerKey(const QString& key)
{
    for (const auto& trigger : m_triggers) {
        if (trigger.source == QStringLiteral(u"key")&& trigger.key.compare(key, Qt::CaseInsensitive) == 0)
            dispatch(trigger);
    }
}

} // namespace ps::triggers
