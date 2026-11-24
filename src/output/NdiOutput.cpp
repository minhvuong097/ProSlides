#include "NdiOutput.h"

#if defined(PS_ENABLE_NDI)
#include <Processing.NDI.Lib.h>
#endif

namespace ps::output {

NdiOutput::NdiOutput(QObject* parent) : QObject(parent)
{
#if defined(PS_ENABLE_NDI)
    NDIlib_initialize();
#endif
}

NdiOutput::~NdiOutput()
{
#if defined(PS_ENABLE_NDI)
    if (m_sender) NDIlib_send_destroy(reinterpret_cast<NDIlib_send_instance_t>(m_sender));
    NDIlib_destroy();
#endif
}

bool NdiOutput::initialize(const QString& name)
{
#if defined(PS_ENABLE_NDI)
    if (m_sender) return true;
    NDIlib_send_create_t desc{};
    desc.p_ndi_name = name.toUtf8().constData();
    auto* sender = NDIlib_send_create(&desc);
    if (!sender) return false;
    m_sender = sender;
    return true;
#else
    Q_UNUSED(name);
    return false;
#endif
}

bool NdiOutput::isActive() const
{
    return m_sender != nullptr;
}

void NdiOutput::sendFrame(const QImage& frame)
{
#if defined(PS_ENABLE_NDI)
    if (!m_sender || frame.isNull()) return;
    const QImage rgb = frame.convertToFormat(QImage::Format_RGBX8888);
    NDIlib_video_frame_v2_t video{};
    video.xres = quint16(rgb.width());
    video.yres = quint16(rgb.height());
    video.FourCC = NDIlib_FourCC_type_RGBX;
    video.frame_rate_N = 60000;
    video.frame_rate_D = 1001;
    video.p_data = reinterpret_cast<const uint8_t*>(rgb.constBits());
    video.line_stride_in_bytes = quint32(rgb.bytesPerLine());
    NDIlib_send_send_video_v2(reinterpret_cast<NDIlib_send_instance_t>(m_sender), &video);
#else
    Q_UNUSED(frame);
#endif
}

} // namespace ps::output
