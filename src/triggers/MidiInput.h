#pragma once

#include <QObject>

namespace ps::triggers {

// Thin Windows Multimedia MIDI input wrapper. No-op on other platforms.
class MidiInput : public QObject
{
    Q_OBJECT
public:
    explicit MidiInput(QObject* parent = nullptr);
    ~MidiInput() override;

    bool open(int device = 0);
    void close();
    bool isOpen() const;

signals:
    void midiMessage(int type, int channel, int data1, int data2);
    void errorOccurred(const QString& error);

private:
    void* m_handle = nullptr;
};

} // namespace ps::triggers
