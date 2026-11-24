#include "MidiInput.h"

#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif

namespace ps::triggers {

#if defined(Q_OS_WIN)
namespace {
thread_local MidiInput* t_currentInput = nullptr;

void CALLBACK midiCallback(HMIDIIN, UINT msg, DWORD_PTR, DWORD_PTR param1, DWORD_PTR)
{
    Q_UNUSED(msg);
    auto* self = t_currentInput;
    if (!self || (param1 & 0xF0) < 0x80) return;
    const int status = int(param1 & 0xFF);
    const int type = status & 0xF0;
    const int channel = status & 0x0F;
    const int data1 = int((param1 >> 8) & 0x7F);
    const int data2 = int((param1 >> 16) & 0x7F);
    emit self->midiMessage(type, channel, data1, data2);
}
} // namespace
#endif

MidiInput::MidiInput(QObject* parent) : QObject(parent)
{
}

MidiInput::~MidiInput()
{
    close();
}

bool MidiInput::open(int device)
{
#if defined(Q_OS_WIN)
    if (m_handle) return true;
    t_currentInput = this;
    HMIDIIN handle = nullptr;
    const MMRESULT result = midiInOpen(&handle, UINT(device),
                                       reinterpret_cast<DWORD_PTR>(&midiCallback), 0,
                                       CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR) {
        char text[128];
        midiInGetErrorTextA(result, text, sizeof(text));
        emit errorOccurred(QString::fromLatin1(text));
        return false;
    }
    m_handle = handle;
    midiInStart(handle);
    return true;
#else
    Q_UNUSED(device);
    return false;
#endif
}

void MidiInput::close()
{
#if defined(Q_OS_WIN)
    if (!m_handle) return;
    auto* handle = reinterpret_cast<HMIDIIN>(m_handle);
    midiInStop(handle);
    midiInClose(handle);
    m_handle = nullptr;
#endif
}

bool MidiInput::isOpen() const
{
    return m_handle != nullptr;
}

} // namespace ps::triggers
