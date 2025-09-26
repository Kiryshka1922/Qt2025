#include "projects/IMage.h"
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

// Реализация ImageInfoWidget
ImageInfoWidget::ImageInfoWidget(QWidget *parent) : QWidget(parent), currentImage(nullptr) {
    setFixedSize(300, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    imageLabel = new QLabel("Нет изображения");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    imageLabel->setFixedHeight(150);
    
    infoText = new QTextEdit();
    infoText->setReadOnly(true);
    infoText->setMaximumHeight(200);
    
    layout->addWidget(imageLabel);
    layout->addWidget(new QLabel("Информация об изображении:"));
    layout->addWidget(infoText);
}

void ImageInfoWidget::setImage(const QImage &image) {
    currentImage = new QImage(image);
    updateInfo();
}

void ImageInfoWidget::updateInfo() {
    if (!currentImage || currentImage->isNull()) {
        imageLabel->setText("Нет изображения");
        infoText->clear();
        return;
    }

    // Отображение миниатюры
    QPixmap pixmap = QPixmap::fromImage(*currentImage);
    if (pixmap.width() > 280 || pixmap.height() > 130) {
        pixmap = pixmap.scaled(280, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    imageLabel->setPixmap(pixmap);

    // Информация об изображении
    QString info;
    info += QString("Размер: %1 x %2 пикселей\n").arg(currentImage->width()).arg(currentImage->height());
    info += QString("Формат: %1\n").arg(currentImage->format() == QImage::Format_RGB32 ? "RGB32" : 
                                       currentImage->format() == QImage::Format_ARGB32 ? "ARGB32" : "Другой");
    info += QString("Глубина цвета: %1 бит\n").arg(currentImage->bitPlaneCount());
    info += QString("Размер в байтах: %1\n").arg(currentImage->sizeInBytes());
    
    // Статистика по цветам
    int totalPixels = currentImage->width() * currentImage->height();
    info += QString("Всего пикселей: %1\n").arg(totalPixels);
    
    infoText->setPlainText(info);
}

void ImageInfoWidget::clear() {
    if (currentImage) {
        delete currentImage;
        currentImage = nullptr;
    }
    imageLabel->setText("Нет изображения");
    infoText->clear();
}

void ImageInfoWidget::onImageClicked() {
    // Можно добавить функционал при клике на изображение
}

// Реализация основного виджета
Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Фильтрация изображений");
    resize(1200, 600);

    // Создание элементов интерфейса
    infoWidget = new ImageInfoWidget(this);
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
    imageLayout = new QHBoxLayout();

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(filterButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();

    imageLayout->addWidget(imageLabel);
    imageLayout->addWidget(infoWidget);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(imageLayout);
}

void Widget::loadImage(const QString &filename) {
    if (originalImage.load(filename)) {
        filteredImage = originalImage.copy();
        imageLabel->setPixmap(QPixmap::fromImage(originalImage).scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        infoWidget->setImage(originalImage);
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
        1.0/9, 1.0/9, 1.0/9,
        1.0/9, 1.0/9, 1.0/9,
        1.0/9, 1.0/9, 1.0/9
    };

    filteredImage = originalImage.copy();
    filter2D(filteredImage, blurKernel, 3, 3);

    imageLabel->setPixmap(QPixmap::fromImage(filteredImage).scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    infoWidget->setImage(filteredImage);
}

void Widget::resetImage() {
    if (originalImage.isNull()) return;
    
    filteredImage = originalImage.copy();
    imageLabel->setPixmap(QPixmap::fromImage(originalImage).scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    infoWidget->setImage(originalImage);
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
