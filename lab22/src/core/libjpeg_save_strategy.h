#pragma once
#include "save_strategy.h"

class LibJpegSaveStrategy : public SaveStrategy {
public:
    bool save(const QImage& img, const QString& path, const SaveOptions& opts) override;
};