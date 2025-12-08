#include "libjpeg_save_strategy.h"
#include <jpeglib.h>
#include <setjmp.h>
#include <QDebug>

struct JpegErr { jpeg_error_mgr pub; jmp_buf jb; };
static void err_exit(j_common_ptr c) { JpegErr* e = (JpegErr*)c->err; longjmp(e->jb,1); }

bool LibJpegSaveStrategy::save(const QImage& img, const QString& path, const SaveOptions& opts) {
    QImage rgb = img;
    if (img.format() != QImage::Format_RGB888) rgb = img.convertToFormat(QImage::Format_RGB888);
    FILE * outfile = fopen(path.toLocal8Bit().constData(), "wb");
    if (!outfile) return false;
    jpeg_compress_struct cinfo;
    JpegErr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = err_exit;
    if (setjmp(jerr.jb)) { jpeg_destroy_compress(&cinfo); fclose(outfile); return false; }
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