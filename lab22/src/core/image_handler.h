#pragma once
#include <QImage>
#include <QString>
#include <vector>
#include "common_types.h"

class ImageHandler {
public:
    virtual ~ImageHandler() = default;
    virtual bool isValid(const QString &path) = 0;
    virtual int stageCount(const QString &path) = 0;
    virtual QImage getStage(const QString &path, int index) = 0;
    virtual bool save(const QImage &img, const QString &path, const SaveOptions &opts) = 0;
};