#pragma once
#include <QString>
#include <QImage>
#include "common_types.h"

class SaveStrategy {
public:
    virtual ~SaveStrategy() = default;
    virtual bool save(const QImage& img, const QString& path, const SaveOptions& opts) = 0;
};