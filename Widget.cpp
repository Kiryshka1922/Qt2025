#include "Widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>


Widget::Widget(QWidget *parentWidget) : QWidget(parentWidget) {
    setWindowTitle("Фильтрация изображений");
    resize(1400, 700);

    // Создаем виджеты
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
    
    QStringList filters = {"Blur", "Sharpen", "Outline", "Left Sobel", "Right Sobel", "Emboss", "Custom"};
    filtersList = new QComboBox(this);
    filtersList->addItems(filters);
    
    // Спинбокс для размера ядра
    kernelSizeSpinBox = new QSpinBox(this);
    kernelSizeSpinBox->setRange(1, 15);
    kernelSizeSpinBox->setValue(3);
    kernelSizeSpinBox->setPrefix("Kernel: ");
    kernelSizeSpinBox->setSuffix("x");
    
    // Таблица для ядра свертки
    kernelTable = new QTableWidget(this);
    kernelTable->setMinimumHeight(150);
    
    applyFilterButton->setEnabled(false);
    restoreButton->setEnabled(false);
    saveButton->setEnabled(false);

    imageInfoWidget = new ImageInfoWidget(this);
    imageInfoWidget->setMinimumWidth(300);
    imageInfoWidget->setMaximumWidth(350);

    // Подключение сигналов
    connect(loadImageButton, &QPushButton::clicked, this, &Widget::handleLoadClick);
    connect(applyFilterButton, &QPushButton::clicked, this, &Widget::handleFilterClick);
    connect(restoreButton, &QPushButton::clicked, this, &Widget::handleRestoreClick);
    connect(saveButton, &QPushButton::clicked, this, &Widget::handleSaveClick);
    connect(filtersList, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Widget::handleFilterSelectionChanged);
    connect(kernelSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Widget::handleKernelSizeChanged);

    // Создание компоновки
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
    controlLayout->addStretch();

    contentLayout->addWidget(displayBaseLabel, 2);
    contentLayout->addWidget(displayChangedLabel, 2);
    contentLayout->addWidget(imageInfoWidget, 1);
    contentLayout->addLayout(kernelLayout, 1);

    primaryLayout->addLayout(controlLayout);
    primaryLayout->addLayout(contentLayout, 1);

    // Инициализация таблицы ядра
    updateKernelTable();
    
    setLayout(primaryLayout);
}

void Widget::updateKernelTable() {
    int size = kernelSizeSpinBox->value();
    kernelTable->setRowCount(size);
    kernelTable->setColumnCount(size);
    
    // Настройка таблицы
    for (int i = 0; i < size; ++i) {
        kernelTable->setColumnWidth(i, 50);
        for (int j = 0; j < size; ++j) {
            QTableWidgetItem *item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            kernelTable->setItem(i, j, item);
        }
    }
    
    // Установка заголовков
    QStringList headers;
    for (int i = 0; i < size; ++i) {
        headers << QString::number(i);
    }
    kernelTable->setHorizontalHeaderLabels(headers);
    kernelTable->setVerticalHeaderLabels(headers);
}

void Widget::loadPresetKernel(int index) {
    double* kernel = nullptr;
    int size = 3; // Все предустановки 3x3
    
    switch(index) {
        case 0: kernel = blurKernel; break;
        case 1: kernel = sharpenKernel; break;
        case 2: kernel = outlineKernel; break;
        case 3: kernel = leftSobelKernel; break;
        case 4: kernel = rightSobelKernel; break;
        case 5: kernel = embossKernel; break;
        case 6: 
            // Custom - оставляем текущие значения
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