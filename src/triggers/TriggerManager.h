#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QJsonObject>

namespace ps::triggers {

class MidiInput;

// A trigger maps an external event (MIDI note/CC or keyboard shortcut) to a
// FreeShow-style API action. Layout mirrors FreeShow Action.triggers.
struct Trigger {
    Id id;
    QString action;             // e.g. "next_slide", "clear_all"
    QJsonObject data;
    QString source;             // "midi" | "key" | "osc"
    int channel = 0;
    int note = -1;              // MIDI note number or CC number
    bool isControl = false;
    int minValue = 1;           // velocity / CC value threshold
    QString key;                // keyboard sequence, e.g. "PgDown"

    QJsonObject toJson() const;
    static Trigger fromJson(const QJsonObject& o);
};

class TriggerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList triggers READ triggers NOTIFY triggersChanged)
    Q_PROPERTY(bool midiConnected READ midiConnected NOTIFY midiChanged)

public:
    explicit TriggerManager(QObject* parent = nullptr);
    ~TriggerManager() override;

    void setFilePath(const QString& path) { m_filePath = path; }
    QString filePath() const { return m_filePath; }

    QVariantList triggers() const { return m_model; }
    bool midiConnected() const;

    void load();
    Q_INVOKABLE bool connectMidi(int device = 0);
    Q_INVOKABLE void disconnectMidi();

    Q_INVOKABLE QString addMidiTrigger(const QString& action, int note,
                                       bool isControl = false, int channel = 0,
                                       int minValue = 1);
    Q_INVOKABLE QString addKeyTrigger(const QString& action, const QString& key);
    Q_INVOKABLE bool removeTrigger(const QString& id);

    void handleMidi(int type, int channel, int data1, int data2);
    Q_INVOKABLE void triggerKey(const QString& key);

signals:
    void triggersChanged();
    void midiChanged(bool connected);
    void actionRequested(const QString& action, const QJsonObject& data);
    void errorOccurred(const QString& error);

private:
    void rebuild();
    void dispatch(const Trigger& trigger);

    QString m_filePath;
    QVector<Trigger> m_triggers;
    QVariantList m_model;
    MidiInput* m_midi = nullptr;
};

} // namespace ps::triggers
