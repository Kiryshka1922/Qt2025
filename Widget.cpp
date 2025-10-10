#include "Widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
Widget::Widget(QWidget *parentWidget) : QWidget(parentWidget) {
    setWindowTitle("Фильтрация изображений");
    resize(1400, 700);
    displayBaseLabel = new QLabel("Начальное изображение");
    displayBaseLabel->setAlignment(Qt::AlignCenter);
    displayBaseLabel->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
    displayBaseLabel->setMinimumSize(400, 300);
    
    displayChangedLabel = new QLabel("Измененное изображение");
    displayChangedLabel->setAlignment(Qt::AlignCenter);
    displayChangedLabel->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
    displayChangedLabel->setMinimumSize(400, 300);
    loadImageButton = new QPushButton("Загрузить изображение");
    applyFilterButton = new QPushButton("Применить фильтр");
    restoreButton = new QPushButton("Сбросить");
    saveButton = new QPushButton("Сохранить");
    
    convertToGray601Button = new QPushButton("BT.601 Grayscale");
    convertToGray709Button = new QPushButton("BT.709 Grayscale");
    otsuBinarizationButton = new QPushButton("Otsu Binarization");
    huangBinarizationButton = new QPushButton("Huang Binarization");
    niblackBinarizationButton = new QPushButton("Niblack Binarization");
    isodataBinarizationButton = new QPushButton("ISODATA Binarization");

    convertToGray601Button->setEnabled(false);
    convertToGray709Button->setEnabled(false);
    otsuBinarizationButton->setEnabled(false);
    huangBinarizationButton->setEnabled(false);
    niblackBinarizationButton->setEnabled(false);
    isodataBinarizationButton->setEnabled(false);

    QStringList filters = {"Blur", "Sharpen", "Outline", "Left Sobel", "Right Sobel", "Emboss", "Custom"};
    filtersList = new QComboBox(this);
    filtersList->addItems(filters);
    
    kernelSizeSpinBox = new QSpinBox(this);
    kernelSizeSpinBox->setRange(1, 15);
    kernelSizeSpinBox->setValue(3);
    kernelSizeSpinBox->setPrefix("Kernel: ");
    kernelSizeSpinBox->setSuffix("x");
    
    kernelTable = new QTableWidget(this);
    kernelTable->setMinimumHeight(150);
    
    applyFilterButton->setEnabled(false);
    restoreButton->setEnabled(false);
    saveButton->setEnabled(false);
    imageInfoWidget = new ImageInfoWidget(this);
    imageInfoWidget->setMinimumWidth(300);
    imageInfoWidget->setMaximumWidth(350);

    connect(loadImageButton, &QPushButton::clicked, this, &Widget::handleLoadClick);
    connect(applyFilterButton, &QPushButton::clicked, this, &Widget::handleFilterClick);
    connect(restoreButton, &QPushButton::clicked, this, &Widget::handleRestoreClick);
    connect(saveButton, &QPushButton::clicked, this, &Widget::handleSaveClick);
    connect(filtersList, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Widget::handleFilterSelectionChanged);
    connect(kernelSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Widget::handleKernelSizeChanged);
    connect(convertToGray601Button, &QPushButton::clicked, this, &Widget::handleConvertToGray601);
    connect(convertToGray709Button, &QPushButton::clicked, this, &Widget::handleConvertToGray709);
    connect(otsuBinarizationButton, &QPushButton::clicked, this, &Widget::handleOtsuBinarization);
    connect(huangBinarizationButton, &QPushButton::clicked, this, &Widget::handleHuangBinarization);
    connect(niblackBinarizationButton, &QPushButton::clicked, this, &Widget::handleNiblackBinarization);
    connect(isodataBinarizationButton, &QPushButton::clicked, this, &Widget::handleISODATABinarization);

    primaryLayout = new QVBoxLayout(this);
    controlLayout = new QHBoxLayout();
    contentLayout = new QHBoxLayout();
    
    QVBoxLayout *kernelLayout = new QVBoxLayout();
    QHBoxLayout *kernelControlLayout = new QHBoxLayout();
    
    kernelControlLayout->addWidget(filtersList);
    kernelControlLayout->addWidget(kernelSizeSpinBox);
    kernelControlLayout->addStretch();
    
    kernelLayout->addLayout(kernelControlLayout);
    kernelLayout->addWidget(kernelTable);
    controlLayout->addWidget(loadImageButton);
    controlLayout->addWidget(applyFilterButton);
    controlLayout->addWidget(restoreButton);
    controlLayout->addWidget(saveButton);
    controlLayout->addWidget(convertToGray601Button);
    controlLayout->addWidget(convertToGray709Button);
    controlLayout->addWidget(otsuBinarizationButton);
    controlLayout->addWidget(huangBinarizationButton);
    controlLayout->addWidget(niblackBinarizationButton);
    controlLayout->addWidget(isodataBinarizationButton);
    controlLayout->addStretch();
    contentLayout->addWidget(displayBaseLabel, 2);
    contentLayout->addWidget(displayChangedLabel, 2);
    contentLayout->addWidget(imageInfoWidget, 1);
    contentLayout->addLayout(kernelLayout, 1);
    primaryLayout->addLayout(controlLayout);
    primaryLayout->addLayout(contentLayout, 1);

    updateKernelTable();
    
    setLayout(primaryLayout);
}
void Widget::updateKernelTable() {
    int size = kernelSizeSpinBox->value();
    kernelTable->setRowCount(size);
    kernelTable->setColumnCount(size);
    
    for (int i = 0; i < size; ++i) {
        kernelTable->setColumnWidth(i, 50);
        for (int j = 0; j < size; ++j) {
            QTableWidgetItem *item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            kernelTable->setItem(i, j, item);
        }
    }
    
    QStringList headers;
    for (int i = 0; i < size; ++i) {
        headers << QString::number(i);
    }
    kernelTable->setHorizontalHeaderLabels(headers);
    kernelTable->setVerticalHeaderLabels(headers);
}
void Widget::loadPresetKernel(int index) {
    double* kernel = nullptr;
    int size = 3;
    
    switch(index) {
        case 0: kernel = blurKernel; break;
        case 1: kernel = sharpenKernel; break;
        case 2: kernel = outlineKernel; break;
        case 3: kernel = leftSobelKernel; break;
        case 4: kernel = rightSobelKernel; break;
        case 5: kernel = embossKernel; break;
        case 6: 
            return;
    }
    
    if (kernel) {
        kernelSizeSpinBox->setValue(size);
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                QTableWidgetItem *item = kernelTable->item(i, j);
                if (item) {
                    item->setText(QString::number(kernel[i * size + j], 'f', 3));
                }
            }
        }
    }
}
double* Widget::getCurrentKernel() {
    int size = kernelSizeSpinBox->value();
    double* kernel = new double[size * size];
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            QTableWidgetItem *item = kernelTable->item(i, j);
            if (item) {
                kernel[i * size + j] = item->text().toDouble();
            } else {
                kernel[i * size + j] = 0;
            }
        }
    }
    
    return kernel;
}
void Widget::handleSaveClick() {
    if (processedImage.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Нет изображения для сохранения");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить изображение",
        QDir::homePath() + "/filtered_image.png",
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;Все файлы (*)"
    );
    
    if (!fileName.isEmpty()) {
        QString format = "PNG";
        if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
            format = "JPEG";
        } else if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
            format = "BMP";
        }
        
        if (processedImage.save(fileName, format.toUtf8())) {
            QMessageBox::information(this, "Успех", 
                QString("Изображение сохранено как:\n%1").arg(fileName));
        } else {
            QMessageBox::warning(this, "Ошибка", 
                QString("Не удалось сохранить изображение:\n%1").arg(fileName));
        }
    }
}
void Widget::handleKernelSizeChanged(int size) {
    updateKernelTable();
}
void Widget::handleFilterSelectionChanged(int index) {
    loadPresetKernel(index);
}
void Widget::executeFilter() {
    if (sourceImage.isNull()) return;
    
    int kernelSize = kernelSizeSpinBox->value();
    double* kernel = getCurrentKernel();
    
    processedImage = sourceImage.copy();
    filter2D(processedImage, kernel, kernelSize, kernelSize);
    
    displayChangedLabel->setPixmap(QPixmap::fromImage(processedImage).scaled(
        displayChangedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    imageInfoWidget->updateImageInfo(processedImage);
    
    delete[] kernel;
}
void Widget::loadImageFile(const QString &filePath) {
    if (sourceImage.load(filePath)) {
        processedImage = sourceImage.copy();
        
        QPixmap pixmap = QPixmap::fromImage(sourceImage);
        displayBaseLabel->setPixmap(pixmap.scaled(
            displayBaseLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        
        imageInfoWidget->updateImageInfo(sourceImage);
        
        applyFilterButton->setEnabled(true);
        restoreButton->setEnabled(true);
        saveButton->setEnabled(true);
        convertToGray601Button->setEnabled(true);
        convertToGray709Button->setEnabled(true);

        bool isGrayscale = (sourceImage.format() == QImage::Format_Grayscale8 || 
                           sourceImage.format() == QImage::Format_Grayscale16 ||
                           sourceImage.format() == QImage::Format_Indexed8);
        
        otsuBinarizationButton->setEnabled(isGrayscale);
        huangBinarizationButton->setEnabled(isGrayscale);
        niblackBinarizationButton->setEnabled(isGrayscale);
        isodataBinarizationButton->setEnabled(isGrayscale);

        setWindowTitle(QString("Фильтрация изображений - %1").arg(QFileInfo(filePath).fileName()));
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение");
        imageInfoWidget->clearInfo();
    }
}
void Widget::restoreOriginal() {
    if (sourceImage.isNull()) return;
    
    processedImage = sourceImage.copy();
    
    displayChangedLabel->setPixmap(QPixmap::fromImage(sourceImage).scaled(
        displayChangedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    imageInfoWidget->updateImageInfo(sourceImage);
}
void Widget::handleLoadClick() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите изображение", 
                                                   "", "Изображения (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!filePath.isEmpty()) {
        loadImageFile(filePath);
    }
}
void Widget::handleFilterClick() {
    executeFilter();
}
void Widget::handleRestoreClick() {
    restoreOriginal();
}

void Widget::handleConvertToGray601() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    RGBToHalf(processedImage, 0.299, 0.587, 0.114); 
    
    updateProcessedImage();
}

void Widget::handleConvertToGray709() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    RGBToHalf(processedImage, 0.2126, 0.7152, 0.0722); 
    
    updateProcessedImage();
}

void Widget::handleOtsuBinarization() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    int threshold = algorithmOtsu(processedImage);
    binarizationFunction(processedImage, threshold);
    
    updateProcessedImage();
}

void Widget::handleHuangBinarization() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    int threshold = algorithmHuang(processedImage);
    binarizationFunction(processedImage, threshold);
    
    updateProcessedImage();
}

void Widget::handleNiblackBinarization() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    algorithmNiblack(processedImage, 15, -0.2); 
    
    updateProcessedImage();
}

void Widget::handleISODATABinarization() {
    if (processedImage.isNull()) return;
    
    processedImage = processedImage.copy();
    int threshold = algorithmISODATA(processedImage, 100, 0.1); 
    binarizationFunction(processedImage, threshold);
    
    updateProcessedImage();
}

void Widget::updateProcessedImage() {
    displayChangedLabel->setPixmap(QPixmap::fromImage(processedImage).scaled(
        displayChangedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    imageInfoWidget->updateImageInfo(processedImage);

    bool isGrayscale = (processedImage.format() == QImage::Format_Grayscale8 || 
                       processedImage.format() == QImage::Format_Grayscale16 ||
                       processedImage.format() == QImage::Format_Indexed8);
    
    otsuBinarizationButton->setEnabled(isGrayscale);
    huangBinarizationButton->setEnabled(isGrayscale);
    niblackBinarizationButton->setEnabled(isGrayscale);
    isodataBinarizationButton->setEnabled(isGrayscale);
}