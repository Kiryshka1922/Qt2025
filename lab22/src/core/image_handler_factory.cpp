#include "image_handler_factory.h"
#include "jpeghandler.h"
#include <QFile>

ImageFormat ImageHandlerFactory::detectFormat(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return ImageFormat::Unknown;
    QByteArray h = f.peek(4);
    f.close();
    if (h.size() >= 3 && h[0] == char(0xFF) && h[1] == char(0xD8) && h[2] == char(0xFF)) return ImageFormat::JPEG;
    return ImageFormat::Unknown;
}

std::unique_ptr<ImageHandler> ImageHandlerFactory::createHandler(ImageFormat fmt) {
    if (fmt == ImageFormat::JPEG) return std::make_unique<JpegHandler>();
    return nullptr;
}