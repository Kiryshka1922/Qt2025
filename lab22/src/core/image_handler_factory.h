#pragma once
#include <memory>
#include <QString>
#include "image_handler.h"

enum class ImageFormat { Unknown, JPEG };

class ImageHandlerFactory {
public:
    static std::unique_ptr<ImageHandler> createHandler(ImageFormat fmt);
    static ImageFormat detectFormat(const QString &path);
};