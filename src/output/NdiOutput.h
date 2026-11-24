#pragma once

#include <QObject>
#include <QImage>

namespace ps::output {

// Optional NDI sender. Enable with -DPS_ENABLE_NDI=ON and the NDI SDK.
// When disabled this is a no-op so presentation code stays backend-agnostic.
class NdiOutput : public QObject
{
    Q_OBJECT
public:
    explicit NdiOutput(QObject* parent = nullptr);
    ~NdiOutput() override;

    bool initialize(const QString& name);
    bool isActive() const;
    void sendFrame(const QImage& frame);

private:
    void* m_sender = nullptr;
};

} // namespace ps::output
