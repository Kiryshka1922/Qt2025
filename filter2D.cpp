#include <QImage>
#include <QWidget>
void filter2D(QImage &sourceImage, double *filterMatrix, size_t matrixWidth, size_t matrixHeight) {
    if (sourceImage.isNull() || !filterMatrix || matrixWidth == 0 || matrixHeight == 0) {
        return;
    }
    // Конвертируем в формат ARGB32 для гарантированной работы с указателями
    if (sourceImage.format() != QImage::Format_ARGB32 && 
        sourceImage.format() != QImage::Format_RGB32) {
        sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32);
    }
    // Определяем тип фильтра
    double kernelSum = 0;
    bool hasNegativeValues = false;
    bool hasPositiveValues = false;
    
    for (size_t i = 0; i < matrixWidth * matrixHeight; ++i) {
        kernelSum += filterMatrix[i];
        if (filterMatrix[i] < 0) hasNegativeValues = true;
        if (filterMatrix[i] > 0) hasPositiveValues = true;
    }
    bool isEdgeDetection = (kernelSum == 0) && hasNegativeValues && hasPositiveValues;
    QImage resultImage(sourceImage.size(), QImage::Format_ARGB32);
    resultImage.fill(Qt::black); // Заполняем черным цветом
    
    int imageWidth = sourceImage.width();
    int imageHeight = sourceImage.height();
    
    int halfMatrixWidth = matrixWidth / 2;
    int halfMatrixHeight = matrixHeight / 2;
    if (isEdgeDetection) {
        // Для обнаружения границ - ЧЕРНО-БЕЛОЕ изображение
        
        // Находим min/max для нормализации
        double minValue = 0, maxValue = 0;
        bool firstPixel = true;
        // Первый проход: находим min/max на основе яркости
        for (int row = 0; row < imageHeight; ++row) {
            uchar *sourceLine = sourceImage.scanLine(row);
            
            for (int col = 0; col < imageWidth; ++col) {
                double sum = 0;
                double totalWeight = 0;
                for (int matrixRow = 0; matrixRow < static_cast<int>(matrixHeight); ++matrixRow) {
                    for (int matrixCol = 0; matrixCol < static_cast<int>(matrixWidth); ++matrixCol) {
                        int pixelCol = col + matrixCol - halfMatrixWidth;
                        int pixelRow = row + matrixRow - halfMatrixHeight;
                        if (pixelCol >= 0 && pixelCol < imageWidth && pixelRow >= 0 && pixelRow < imageHeight) {
                            uchar *sourcePixel = sourceImage.scanLine(pixelRow) + pixelCol * 4;
                            // Вычисляем яркость пикселя по формуле luminance
                            double luminance = 0.299 * sourcePixel[2] + 0.587 * sourcePixel[1] + 0.114 * sourcePixel[0];
                            double currentWeight = filterMatrix[matrixRow * matrixWidth + matrixCol];
                            sum += luminance * currentWeight;
                            totalWeight += currentWeight;
                        }
                    }
                }
                if (totalWeight != 0) {
                    sum /= totalWeight;
                }
                if (firstPixel) {
                    minValue = maxValue = sum;
                    firstPixel = false;
                } else {
                    if (sum < minValue) minValue = sum;
                    if (sum > maxValue) maxValue = sum;
                }
            }
        }
        // Второй проход: применяем фильтр и создаем ЧЕРНО-БЕЛОЕ изображение
        double range = maxValue - minValue;
        if (range == 0) range = 1;
        for (int row = 0; row < imageHeight; ++row) {
            uchar *sourceLine = sourceImage.scanLine(row);
            uchar *resultLine = resultImage.scanLine(row);
            
            for (int col = 0; col < imageWidth; ++col) {
                double sum = 0;
                double totalWeight = 0;
                for (int matrixRow = 0; matrixRow < static_cast<int>(matrixHeight); ++matrixRow) {
                    for (int matrixCol = 0; matrixCol < static_cast<int>(matrixWidth); ++matrixCol) {
                        int pixelCol = col + matrixCol - halfMatrixWidth;
                        int pixelRow = row + matrixRow - halfMatrixHeight;
                        if (pixelCol >= 0 && pixelCol < imageWidth && pixelRow >= 0 && pixelRow < imageHeight) {
                            uchar *sourcePixel = sourceImage.scanLine(pixelRow) + pixelCol * 4;
                            // Вычисляем яркость пикселя
                            double luminance = 0.299 * sourcePixel[2] + 0.587 * sourcePixel[1] + 0.114 * sourcePixel[0];
                            double currentWeight = filterMatrix[matrixRow * matrixWidth + matrixCol];
                            sum += luminance * currentWeight;
                            totalWeight += currentWeight;
                        }
                    }
                }
                if (totalWeight != 0) {
                    sum /= totalWeight;
                }
                // Нормализуем к 0-255 и создаем ЧЕРНО-БЕЛЫЙ пиксель
                double normalized = 255 * (sum - minValue) / range;
                normalized = qBound(0.0, normalized, 255.0);
                
                // Для настоящего ЧЕРНО-БЕЛОГО - используем только 0 и 255
                uchar bwValue;
                if (normalized > 128) {
                    bwValue = 255; // БЕЛЫЙ
                } else {
                    bwValue = 0;   // ЧЕРНЫЙ
                }
                // Записываем результат через указатели
                uchar *resultPixel = resultLine + col * 4;
                resultPixel[0] = bwValue; // Blue
                resultPixel[1] = bwValue; // Green
                resultPixel[2] = bwValue; // Red
                resultPixel[3] = 255;     // Alpha
            }
        }
    } else {
        // Для остальных фильтров - цветная обработка
        for (int row = 0; row < imageHeight; ++row) {
            uchar *sourceLine = sourceImage.scanLine(row);
            uchar *resultLine = resultImage.scanLine(row);
            
            for (int col = 0; col < imageWidth; ++col) {
                double redSum = 0, greenSum = 0, blueSum = 0, alphaSum = 0;
                double totalWeight = 0;
                for (int matrixRow = 0; matrixRow < static_cast<int>(matrixHeight); ++matrixRow) {
                    for (int matrixCol = 0; matrixCol < static_cast<int>(matrixWidth); ++matrixCol) {
                        int pixelCol = col + matrixCol - halfMatrixWidth;
                        int pixelRow = row + matrixRow - halfMatrixHeight;
                        if (pixelCol >= 0 && pixelCol < imageWidth && pixelRow >= 0 && pixelRow < imageHeight) {
                            uchar *sourcePixel = sourceImage.scanLine(pixelRow) + pixelCol * 4;
                            double currentWeight = filterMatrix[matrixRow * matrixWidth + matrixCol];
                            
                            blueSum += sourcePixel[0] * currentWeight;   // Blue
                            greenSum += sourcePixel[1] * currentWeight;  // Green
                            redSum += sourcePixel[2] * currentWeight;    // Red
                            alphaSum += sourcePixel[3] * currentWeight;  // Alpha
                            totalWeight += currentWeight;
                        }
                    }
                }
                if (totalWeight != 0) {
                    redSum /= totalWeight;
                    greenSum /= totalWeight;
                    blueSum /= totalWeight;
                    alphaSum /= totalWeight;
                }
                uchar resultRed = qBound(0, static_cast<int>(redSum), 255);
                uchar resultGreen = qBound(0, static_cast<int>(greenSum), 255);
                uchar resultBlue = qBound(0, static_cast<int>(blueSum), 255);
                uchar resultAlpha = qBound(0, static_cast<int>(alphaSum), 255);
                uchar *resultPixel = resultLine + col * 4;
                resultPixel[0] = resultBlue;
                resultPixel[1] = resultGreen;
                resultPixel[2] = resultRed;
                resultPixel[3] = resultAlpha;
            }
        }
    }
    sourceImage = resultImage;
}

