#pragma once
#include <QImage>
#include <QString>
#include <vector>
#include "image_handler.h"
#include <jpeglib.h>
#include <setjmp.h>

class JpegBase {
protected:
    struct JpegErrorMgr {
        jpeg_error_mgr pub;
        jmp_buf setjmp_buffer;
    };
    static void setupStdError(j_common_ptr cinfo, JpegErrorMgr* mgr);
};

class JpegHandler : public ImageHandler, public JpegBase {
public:
    JpegHandler() = default;
    ~JpegHandler() = default;
    bool isValid(const QString &path) override;
    int stageCount(const QString &path) override;
    QImage getStage(const QString &path, int index) override;
    bool save(const QImage &img, const QString &path, const SaveOptions &opts) override;
    std::vector<QImage> loadAllStages(const QString &path);
};