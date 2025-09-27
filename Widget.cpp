#include "IMage.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

// Реализация функции фильтрации
void filter2D(QImage &image, double *kernel, size_t kWidth, size_t kHeight) {
    if (image.isNull() || !kernel || kWidth == 0 || kHeight == 0) {
        return;
    }

    QImage result = image.copy();
    int width = image.width();
    int height = image.height();
    
    int halfWidth = kWidth / 2;
    int halfHeight = kHeight / 2;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double r = 0, g = 0, b = 0, a = 0;
            double weightSum = 0;

            for (int ky = 0; ky < static_cast<int>(kHeight); ++ky) {
                for (int kx = 0; kx < static_cast<int>(kWidth); ++kx) {
                    int px = x + kx - halfWidth;
                    int py = y + ky - halfHeight;

                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        QRgb pixel = image.pixel(px, py);
                        double weight = kernel[ky * kWidth + kx];
                        
                        r += qRed(pixel) * weight;
                        g += qGreen(pixel) * weight;
                        b += qBlue(pixel) * weight;
                        a += qAlpha(pixel) * weight;
                        weightSum += weight;
                    }
                }
            }

            if (weightSum != 0) {
                r /= weightSum;
                g /= weightSum;
                b /= weightSum;
                a /= weightSum;
            }

            // Делаем значение каждого цвета в корректных переделах пределах
            r = qBound(0.0, r, 255.0);
            g = qBound(0.0, g, 255.0);
            b = qBound(0.0, b, 255.0);
            a = qBound(0.0, a, 255.0);

            result.setPixel(x, y, qRgba(static_cast<int>(r), static_cast<int>(g), 
                                      static_cast<int>(b), static_cast<int>(a)));
        }
    }

    image = result;
}

// Реализация основного виджета
Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Фильтрация изображений");
    resize(1200, 600);

    // Создание элементов интерфейса
    imageLabel = new QLabel("Загрузите изображение");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
    imageLabel->setMinimumSize(400, 300);

    loadButton = new QPushButton("Загрузить изображение");
    filterButton = new QPushButton("Применить фильтр");
    resetButton = new QPushButton("Сбросить");
    
    filterButton->setEnabled(false);
    resetButton->setEnabled(false);

    // Подключение сигналов
    connect(loadButton, &QPushButton::clicked, this, &Widget::onLoadClicked);
    connect(filterButton, &QPushButton::clicked, this, &Widget::onFilterClicked);
    connect(resetButton, &QPushButton::clicked, this, &Widget::onResetClicked);

    // Создание компоновки
    mainLayout = new QVBoxLayout(this);
    buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(filterButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(imageLabel);
}

void Widget::loadImage(const QString &filename) {
    if (originalImage.load(filename)) {
        filteredImage = originalImage.copy();
        imageLabel->setPixmap(QPixmap::fromImage(originalImage).scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        filterButton->setEnabled(true);
        resetButton->setEnabled(true);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение");
    }
}

void Widget::applyFilter() {
    if (originalImage.isNull()) return;

    // Создание простого фильтра размытия (3x3)
    double blurKernel[9] = {
        0.111, 0.111, 0.111,
        0.111, 0.111, 0.111,
        0.111, 0.111, 0.111
    };

    filteredImage = originalImage.copy();
    filter2D(filteredImage, blurKernel, 3, 3);

    imageLabel->setPixmap(QPixmap::fromImage(filteredImage).scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::resetImage() {
    if (originalImage.isNull()) return;
    
    filteredImage = originalImage.copy();
    imageLabel->setPixmap(QPixmap::fromImage(originalImage).scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::onLoadClicked() {
    QString filename = QFileDialog::getOpenFileName(this, "Выберите изображение", 
                                                   "", "Изображения (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!filename.isEmpty()) {
        loadImage(filename);
    }
}

void Widget::onFilterClicked() {
    applyFilter();
}

void Widget::onResetClicked() {
    resetImage();
}
