#include "jpeghandler.h"
#include <jpeglib.h>
#include <setjmp.h>
#include <QFile>
#include <QDebug>
#include <cstring>

void JpegBase::setupStdError(j_common_ptr cinfo, JpegErrorMgr* mgr) {
    cinfo->err = jpeg_std_error(&mgr->pub);
    mgr->pub.error_exit = [](j_common_ptr c) {
        JpegErrorMgr* me = reinterpret_cast<JpegErrorMgr*>(c->err);
        char buffer[JMSG_LENGTH_MAX];
        (*c->err->format_message)(c, buffer);
        qDebug() << "libjpeg error:" << buffer;
        longjmp(me->setjmp_buffer, 1);
    };
}

bool JpegHandler::isValid(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray h = f.peek(4);
    f.close();
    return h.size() >= 3 && h[0] == char(0xFF) && h[1] == char(0xD8) && h[2] == char(0xFF);
}

int JpegHandler::stageCount(const QString &path) {
    FILE * infile = fopen(path.toLocal8Bit().constData(), "rb");
    if (!infile) return 0;
    jpeg_decompress_struct cinfo;
    JpegErrorMgr jerr;
    setupStdError((j_common_ptr)&cinfo, &jerr);
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.buffered_image = TRUE;
    jpeg_start_decompress(&cinfo);
    int stages = 0;
    const int MAX_PASSES = 128;
    while (jpeg_consume_input(&cinfo) == JPEG_SUSPENDED) break;
    for (int pass = 1; pass <= MAX_PASSES; ++pass) {
        if (!jpeg_start_output(&cinfo, pass)) break;
        stages = pass;
        jpeg_finish_output(&cinfo);
        if (!jpeg_has_multiple_scans(&cinfo)) break;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    // Если stages = 0, значит это непрогрессивное изображение с одним слоем
    return (stages > 0) ? stages : 1;
}

QImage JpegHandler::getStage(const QString &path, int index) {
    QImage img;
    if (index < 0) return img;
    FILE * infile = fopen(path.toLocal8Bit().constData(), "rb");
    if (!infile) return img;
    jpeg_decompress_struct cinfo;
    JpegErrorMgr jerr;
    setupStdError((j_common_ptr)&cinfo, &jerr);
    if (setjmp(jerr.setjmp_buffer)) { jpeg_destroy_decompress(&cinfo); fclose(infile); return img; }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.buffered_image = TRUE;
    jpeg_start_decompress(&cinfo);
    while (jpeg_consume_input(&cinfo) == JPEG_SUSPENDED) break;
    if (!jpeg_start_output(&cinfo, index+1)) {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return img;
    }
    int w = cinfo.output_width;
    int h = cinfo.output_height;
    int comps = cinfo.output_components;
    img = QImage(w, h, QImage::Format_RGB888);
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, w * comps, 1);
    int row = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        JDIMENSION lines_read = jpeg_read_scanlines(&cinfo, buffer, 1);
        if (lines_read == 0) break;
        unsigned char *src = buffer[0];
        uchar *dest = img.scanLine(row);
        if (comps == 3) {
            memcpy(dest, src, w * 3);
        } else {
            for (int x = 0; x < w; ++x) {
                uchar v = src[x];
                dest[3*x] = v;
                dest[3*x+1] = v;
                dest[3*x+2] = v;
            }
        }
        ++row;
    }
    jpeg_finish_output(&cinfo);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return img;
}

std::vector<QImage> JpegHandler::loadAllStages(const QString &path) {
    std::vector<QImage> stages;
    FILE * infile = fopen(path.toLocal8Bit().constData(), "rb");
    if (!infile) return stages;
    jpeg_decompress_struct cinfo;
    JpegErrorMgr jerr;
    setupStdError((j_common_ptr)&cinfo, &jerr);
    if (setjmp(jerr.setjmp_buffer)) { jpeg_destroy_decompress(&cinfo); fclose(infile); return stages; }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.buffered_image = TRUE;
    jpeg_start_decompress(&cinfo);
    const int MAX_PASSES = 128;
    while (jpeg_consume_input(&cinfo) == JPEG_SUSPENDED) break;
    for (int pass = 1; pass <= MAX_PASSES; ++pass) {
        if (!jpeg_start_output(&cinfo, pass)) break;
        int w = cinfo.output_width;
        int h = cinfo.output_height;
        int comps = cinfo.output_components;
        QImage img(w, h, QImage::Format_RGB888);
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, w * comps, 1);
        int row = 0;
        while (cinfo.output_scanline < cinfo.output_height) {
            JDIMENSION lines_read = jpeg_read_scanlines(&cinfo, buffer, 1);
            if (lines_read == 0) break;
            unsigned char *src = buffer[0];
            uchar *dest = img.scanLine(row);
            if (comps == 3) {
                memcpy(dest, src, w * 3);
            } else {
                for (int x = 0; x < w; ++x) {
                    uchar v = src[x];
                    dest[3*x] = v;
                    dest[3*x+1] = v;
                    dest[3*x+2] = v;
                }
            }
            ++row;
        }
        jpeg_finish_output(&cinfo);
        stages.push_back(img);
        if (!jpeg_has_multiple_scans(&cinfo)) break;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    if (stages.empty()) {
        FILE* in2 = fopen(path.toLocal8Bit().constData(), "rb");
        if (!in2) return stages;
        jpeg_decompress_struct c2;
        JpegErrorMgr jerr2;
        setupStdError((j_common_ptr)&c2, &jerr2);
        if (setjmp(jerr2.setjmp_buffer)) { jpeg_destroy_decompress(&c2); fclose(in2); return stages; }
        jpeg_create_decompress(&c2);
        jpeg_stdio_src(&c2, in2);
        jpeg_read_header(&c2, TRUE);
        jpeg_start_decompress(&c2);
        int w = c2.output_width, h = c2.output_height, comps = c2.output_components;
        QImage img(w, h, QImage::Format_RGB888);
        JSAMPARRAY buffer = (*c2.mem->alloc_sarray)((j_common_ptr)&c2, JPOOL_IMAGE, w * comps, 1);
        int row = 0;
        while (c2.output_scanline < c2.output_height) {
            JDIMENSION lines_read = jpeg_read_scanlines(&c2, buffer, 1);
            if (lines_read == 0) break;
            unsigned char* src = buffer[0];
            uchar* dest = img.scanLine(row);
            if (comps == 3) {
                memcpy(dest, src, w*3);
            } else {
                for (int x = 0; x < w; ++x) {
                    uchar v = src[x];
                    dest[3*x] = v;
                    dest[3*x+1] = v;
                    dest[3*x+2] = v;
                }
            }
            ++row;
        }
        jpeg_finish_decompress(&c2);
        jpeg_destroy_decompress(&c2);
        fclose(in2);
        stages.push_back(img);
    }
    return stages;
}

bool JpegHandler::save(const QImage &img, const QString &path, const SaveOptions &opts) {
    QImage rgb = img;
    if (img.format() != QImage::Format_RGB888) rgb = img.convertToFormat(QImage::Format_RGB888);
    FILE * outfile = fopen(path.toLocal8Bit().constData(), "wb");
    if (!outfile) return false;
    jpeg_compress_struct cinfo;
    JpegErrorMgr jerr;
    setupStdError((j_common_ptr)&cinfo, &jerr);
    if (setjmp(jerr.setjmp_buffer)) { jpeg_destroy_compress(&cinfo); fclose(outfile); return false; }
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = rgb.width();
    cinfo.image_height = rgb.height();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    switch (opts.dct_method) {
    case 0: cinfo.dct_method = JDCT_ISLOW; break;
    case 1: cinfo.dct_method = JDCT_IFAST; break;
    case 2: cinfo.dct_method = JDCT_FLOAT; break;
    default: cinfo.dct_method = JDCT_ISLOW; break;
    }
    jpeg_set_quality(&cinfo, opts.quality, TRUE);
    if (opts.progressive) jpeg_simple_progression(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = (JSAMPROW)rgb.scanLine(cinfo.next_scanline);
        JDIMENSION written = jpeg_write_scanlines(&cinfo, row_pointer, 1);
        if (written != 1) {
            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);
            fclose(outfile);
            return false;
        }
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    return true;
}
