#ifndef BINARIZATIONFUNCTION_H
#define BINARIZATIONFUNCTION_H

#include <QImage>
#include <cmath>

void binarizationFunction(QImage &image, int threshold);

int algorithmOtsu(QImage &image);
int algorithmHuang(QImage &image);
int algorithmISODATA(QImage &image, int maxIterations = 100, double tolerance = 1.0);
void algorithmNiblack(QImage &image, int windowSize = 15, double k = -0.2);
#endif 
