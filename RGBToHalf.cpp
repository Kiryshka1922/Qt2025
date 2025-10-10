#include "RGBToHalf.h"

void RGBToHalf(QImage &image, double rWeight, double gWeight, double bWeight)
{
    if (image.isNull())
        return;

    int imageWidth = image.width(), imageHeight = image.height();

    QImage grayImage(imageWidth, imageHeight, QImage::Format_Grayscale8);

    for (int y = 0; y < imageHeight; ++y)
    {
        uchar *line = grayImage.scanLine(y);
        for (int x = 0; x < imageWidth; ++x)
        {
            QColor color = image.pixelColor(x, y);
            line[x] = static_cast<uchar>(rWeight * color.red() + gWeight * color.green() + bWeight * color.blue() + 0.5);
        }
    }

    image = grayImage;
}