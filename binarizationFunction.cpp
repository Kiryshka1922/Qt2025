#include "binarizationFunction.h"

#include <QVector>

void binarizationFunction(QImage &image, int threshold)
{
    if (image.isNull())
        return;

    if (image.format() == QImage::Format_Grayscale8)
        image = image.convertToFormat(QImage::Format_Grayscale8);

    int imageWidth = image.width(), imageHeight = image.height();
    QImage tmpImage(imageWidth, imageHeight, QImage::Format_Mono);

    for (int y = 0; y < imageHeight; ++y)
    {
        uchar *line = image.scanLine(y);
        for (int x = 0; x < imageWidth; ++x)
        {
            tmpImage.setPixel(x, y, (line[x] <= threshold) ? 0 : 1);
        }
    }
    image = tmpImage;
}

int algorithmOtsu(QImage &image)
{
    if (image.format() != QImage::Format_Grayscale8)
    {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    int imageWidth = image.width(), imageHeight = image.height();
    int totalPixels = imageWidth * imageHeight;

    QVector<int> histogram(256, 0);
    for (int y = 0; y < imageHeight; y++)
    {
        const uchar *line = image.constScanLine(y);
        for (int x = 0; x < imageWidth; x++)
        {
            histogram[line[x]]++;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < 256; i++)
    {
        sum += i * histogram[i];
    }

    double sumB = 0.0;
    int wB = 0;
    int wF = 0;
    double maxVariance = 0.0;
    int threshold = 0;

    for (int t = 0; t < 256; t++)
    {
        wB += histogram[t];
        if (wB == 0)
            continue;

        wF = totalPixels - wB;
        if (wF == 0)
            break;

        sumB += t * histogram[t];

        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;

        double variance = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        if (variance > maxVariance)
        {
            maxVariance = variance;
            threshold = t;
        }
    }

    return threshold;
}

int algorithmHuang(QImage &image)
{
    if (image.format() != QImage::Format_Grayscale8)
    {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    int imageWidth = image.width(), imageHeight = image.height();
    int totalPixels = imageWidth * imageHeight;

    QVector<double> histogram(256, 0.0);
    for (int y = 0; y < imageHeight; y++)
    {
        const uchar *line = image.constScanLine(y);
        for (int x = 0; x < imageWidth; x++)
        {
            histogram[line[x]] += 1.0;
        }
    }

    for (int i = 0; i < 256; i++)
    {
        histogram[i] /= totalPixels;
    }

    QVector<double> P(256, 0.0);
    QVector<double> S(256, 0.0);

    P[0] = histogram[0];
    S[0] = 0;

    for (int i = 1; i < 256; i++)
    {
        P[i] = P[i - 1] + histogram[i];
        S[i] = S[i - 1] + i * histogram[i];
    }

    QVector<double> mu(256, 0.0);
    int optimalThreshold = 0;
    double minFuzziness = std::numeric_limits<double>::max();

    for (int t = 0; t < 256; t++)
    {
        if (P[t] == 0.0 || P[t] == 1.0)
        {
            continue;
        }

        double mu0 = (P[t] > 0) ? S[t] / P[t] : 0;
        double mu1 = (1.0 - P[t] > 0) ? (S[255] - S[t]) / (1.0 - P[t]) : 0;

        double fuzziness = 0.0;

        for (int i = 0; i <= t; i++)
        {
            if (histogram[i] > 0)
            {
                double membership = 1.0 / (1.0 + std::abs(i - mu0));
                double entropy = -membership * std::log(membership) -
                                 (1.0 - membership) * std::log(1.0 - membership);
                fuzziness += histogram[i] * entropy;
            }
        }

        for (int i = t + 1; i < 256; i++)
        {
            if (histogram[i] > 0)
            {
                double membership = 1.0 / (1.0 + std::abs(i - mu1));
                double entropy = -membership * std::log(membership) -
                                 (1.0 - membership) * std::log(1.0 - membership);
                fuzziness += histogram[i] * entropy;
            }
        }

        if (fuzziness < minFuzziness)
        {
            minFuzziness = fuzziness;
            optimalThreshold = t;
        }
    }

    return optimalThreshold;
}

int algorithmISODATA(QImage &image, int maxIterations, double tolerance)
{
    if (image.isNull())
        return 128;

    if (image.format() != QImage::Format_Grayscale8)
    {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    int width = image.width();
    int height = image.height();

    QVector<int> histogram(256, 0);
    for (int y = 0; y < height; y++)
    {
        const uchar *scanLine = image.constScanLine(y);
        for (int x = 0; x < width; x++)
        {
            histogram[scanLine[x]]++;
        }
    }

    double threshold = 128.0;
    double newThreshold = threshold;

    for (int iteration = 0; iteration < maxIterations; iteration++)
    {
        double sumBackground = 0.0, sumForeground = 0.0;
        int countBackground = 0, countForeground = 0;

        for (int i = 0; i < 256; i++)
        {
            if (i <= threshold)
            {
                sumBackground += i * histogram[i];
                countBackground += histogram[i];
            }
            else
            {
                sumForeground += i * histogram[i];
                countForeground += histogram[i];
            }
        }

        if (countBackground == 0 || countForeground == 0)
        {
            break;
        }

        double meanBackground = sumBackground / countBackground;
        double meanForeground = sumForeground / countForeground;

        newThreshold = (meanBackground + meanForeground) / 2.0;

        if (std::abs(newThreshold - threshold) <= tolerance)
        {
            break;
        }

        threshold = newThreshold;
    }

    return static_cast<int>(std::round(newThreshold));
}

void algorithmNiblack(QImage &image, int windowSize, double k)
{
    if (image.isNull())
        return;

    if (image.format() != QImage::Format_Grayscale8)
    {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    int imageWidth = image.width();
    int imageHeight = image.height();
    int halfWindow = windowSize / 2;

    QImage tmpImage(imageWidth, imageHeight, QImage::Format_Mono);

    QVector<QVector<long>> integral(imageWidth, QVector<long>(imageHeight, 0));
    QVector<QVector<long>> integralSquare(imageWidth, QVector<long>(imageHeight, 0));

    for (int y = 0; y < imageHeight; y++)
    {
        const uchar *line = image.constScanLine(y);
        for (int x = 0; x < imageWidth; x++)
        {
            int pixel = line[x];
            integral[x][y] = pixel;
            integralSquare[x][y] = pixel * pixel;

            if (x > 0)
            {
                integral[x][y] += integral[x - 1][y];
                integralSquare[x][y] += integralSquare[x - 1][y];
            }
            if (y > 0)
            {
                integral[x][y] += integral[x][y - 1];
                integralSquare[x][y] += integralSquare[x][y - 1];
            }
            if (x > 0 && y > 0)
            {
                integral[x][y] -= integral[x - 1][y - 1];
                integralSquare[x][y] -= integralSquare[x - 1][y - 1];
            }
        }
    }

    for (int y = 0; y < imageHeight; y++)
    {
        uchar *dst = tmpImage.scanLine(y);
        const uchar *src = image.constScanLine(y);

        for (int x = 0; x < imageWidth; x++)
        {
            int x1 = std::max(0, x - halfWindow);
            int y1 = std::max(0, y - halfWindow);
            int x2 = std::min(imageWidth - 1, x + halfWindow);
            int y2 = std::min(imageHeight - 1, y + halfWindow); 

            int area = (x2 - x1 + 1) * (y2 - y1 + 1);

            long sum = integral[x2][y2];
            long sumSquare = integralSquare[x2][y2];

            if (x1 > 0)
            {
                sum -= integral[x1 - 1][y2];
                sumSquare -= integralSquare[x1 - 1][y2];
            }
            if (y1 > 0)
            {
                sum -= integral[x2][y1 - 1];
                sumSquare -= integralSquare[x2][y1 - 1];
            }
            if (x1 > 0 && y1 > 0)
            {
                sum += integral[x1 - 1][y1 - 1];
                sumSquare += integralSquare[x1 - 1][y1 - 1];
            }

            double mean = static_cast<double>(sum) / area;
            double variance = (static_cast<double>(sumSquare) / area) - (mean * mean);
            double stdDev = std::sqrt(std::max(0.0, variance));

            double threshold = mean + k * stdDev;

            if (src[x] >= threshold)
            {
                dst[x / 8] |= (1 << (7 - (x % 8)));
            }
            else
            {
                dst[x / 8] &= ~(1 << (7 - (x % 8)));
            }
        }
    }

    image = tmpImage;
}