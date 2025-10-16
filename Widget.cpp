#include "Widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <tiffio.h>


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
        QDir::homePath() + "/filtered_image.tif",
        "TIFF (*.tif *.tiff);;PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;Все файлы (*)"
    );

    if (fileName.isEmpty()) return;

    if (fileName.endsWith(".tif", Qt::CaseInsensitive) || fileName.endsWith(".tiff", Qt::CaseInsensitive)) {
        QDialog dialog(this);
        dialog.setWindowTitle("Параметры сохранения TIFF");
        QFormLayout form(&dialog);

        QComboBox *combo = new QComboBox(&dialog);
        combo->addItem("No compression (None)");
        combo->addItem("Deflate (zlib) — уровень 1..9");
        combo->addItem("LZMA — preset 1..9");
        combo->addItem("PackBits");
        combo->addItem("LZW");
        combo->addItem("CCITT Group 3 (моно)");
        combo->addItem("CCITT Group 4 (моно)");
        combo->addItem("JPEG — качество 0..100");
        form.addRow("Compression:", combo);

        QSpinBox *paramSpin = new QSpinBox(&dialog);
        paramSpin->setRange(0, 100);
        paramSpin->setValue(6);
        form.addRow("Параметр (если применимо):", paramSpin);

        QLabel *hint = new QLabel("Для Deflate/LZMA используйте 1..9; для JPEG 0..100. Для остальных параметр игнорируется.");
        form.addRow(hint);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);
        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) return;

        TiffCompression comp = TiffCompression::None;
        switch (combo->currentIndex()) {
            case 0: comp = TiffCompression::None; break;
            case 1: comp = TiffCompression::Deflate; break;
            case 2: comp = TiffCompression::LZMA; break;
            case 3: comp = TiffCompression::PackBits; break;
            case 4: comp = TiffCompression::LZW; break;
            case 5: comp = TiffCompression::CCITTFax3; break;
            case 6: comp = TiffCompression::CCITTFax4; break;
            case 7: comp = TiffCompression::JPEG; break;
            default: comp = TiffCompression::None; break;
        }

        int param = paramSpin->value();
        if (comp == TiffCompression::Deflate || comp == TiffCompression::LZMA) {
            if (param < 1) param = 1;
            if (param > 9) param = 9;
        } else if (comp == TiffCompression::JPEG) {
            if (param < 0) param = 0;
            if (param > 100) param = 100;
        }

        QString err;
        if (saveTiffWithLibTiff(processedImage, fileName, comp, param, err)) {
            QMessageBox::information(this, "Успех", QString("Изображение сохранено как:\n%1").arg(fileName));
        } else {
            QMessageBox::warning(this, "Ошибка", QString("Не удалось сохранить TIFF:\n%1").arg(err));
        }

        return;
    }

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

bool Widget::saveTiffWithLibTiff(const QImage &imageIn, const QString &fileName,
                                 TiffCompression compression, int parameter, QString &errorString)
{
    if (imageIn.isNull()) {
        errorString = "Пустое изображение";
        return false;
    }

    QImage image = imageIn;

    if ((compression == TiffCompression::CCITTFax3 || compression == TiffCompression::CCITTFax4)
        && image.format() != QImage::Format_Mono) {
        image = image.convertToFormat(QImage::Format_Grayscale8);
        QImage mono(image.width(), image.height(), QImage::Format_Mono);
        for (int y = 0; y < image.height(); ++y) {
            const uchar *src = image.constScanLine(y);
            for (int x = 0; x < image.width(); ++x) {
                mono.setPixel(x, y, src[x] > 128 ? 1 : 0);
            }
        }
        image = mono;
    }

    int width = image.width();
    int height = image.height();

    QByteArray fn = QFile::encodeName(fileName);
    TIFF *tif = TIFFOpen(fn.constData(), "w");
    if (!tif) {
        errorString = "TIFFOpen() failed (возможно, отсутствует libtiff или нет прав на запись)";
        return false;
    }

    // Общие поля
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)height);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "Custom Qt/libtiff");

    int tiffCompression = COMPRESSION_NONE;
    switch (compression) {
        case TiffCompression::None:
            tiffCompression = COMPRESSION_NONE;
            break;
        case TiffCompression::Deflate:
            tiffCompression = COMPRESSION_DEFLATE;
            TIFFSetField(tif, TIFFTAG_ZIPQUALITY, parameter);
            break;
        case TiffCompression::LZMA:
            tiffCompression = COMPRESSION_LZMA;
            TIFFSetField(tif, TIFFTAG_LZMAPRESET, parameter);
            break;
        case TiffCompression::PackBits:
            tiffCompression = COMPRESSION_PACKBITS;
            break;
        case TiffCompression::LZW:
            tiffCompression = COMPRESSION_LZW;
            break;
        case TiffCompression::CCITTFax3:
            tiffCompression = COMPRESSION_CCITTFAX3;
            break;
        case TiffCompression::CCITTFax4:
            tiffCompression = COMPRESSION_CCITTFAX4;
            break;
        case TiffCompression::JPEG:
            tiffCompression = COMPRESSION_JPEG;
            TIFFSetField(tif, TIFFTAG_JPEGQUALITY, parameter);
            break;
        default:
            tiffCompression = COMPRESSION_NONE;
            break;
    }
    TIFFSetField(tif, TIFFTAG_COMPRESSION, tiffCompression);

    if (image.format() == QImage::Format_Mono) {
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)1);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)1);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

        tsize_t scanlineSize = TIFFScanlineSize(tif);
        std::vector<unsigned char> buffer(scanlineSize);

        for (int row = 0; row < height; ++row) {
            const uchar *src = image.constScanLine(row);

            int bytesInSrc = (width + 7) / 8;
            int bytesToCopy = std::min<int>(bytesInSrc, (int)scanlineSize);
            memcpy(buffer.data(), src, bytesToCopy);

            if (scanlineSize > bytesToCopy) memset(buffer.data() + bytesToCopy, 0, scanlineSize - bytesToCopy);

            if (TIFFWriteScanline(tif, buffer.data(), row, 0) < 0) {
                TIFFClose(tif);
                errorString = QString("TIFFWriteScanline failed on row %1").arg(row);
                return false;
            }
        }
    } else if (image.format() == QImage::Format_Grayscale8 || image.format() == QImage::Format_Indexed8) {
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)1);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

        tsize_t scanlineSize = TIFFScanlineSize(tif);
        std::vector<unsigned char> buffer(scanlineSize);

        for (int row = 0; row < height; ++row) {
            const uchar *src = image.constScanLine(row);
            memcpy(buffer.data(), src, width);
            if (TIFFWriteScanline(tif, buffer.data(), row, 0) < 0) {
                TIFFClose(tif);
                errorString = QString("TIFFWriteScanline failed on row %1").arg(row);
                return false;
            }
        }
    } else {
        QImage rgb = image.convertToFormat(QImage::Format_RGB888);

        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)3);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

        if (compression == TiffCompression::JPEG) {
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_YCBCR);
            TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
            TIFFSetField(tif, TIFFTAG_JPEGQUALITY, parameter);
        } else {
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        }

        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

        tsize_t scanlineSize = TIFFScanlineSize(tif);
        std::vector<unsigned char> buffer(scanlineSize);

        for (int row = 0; row < height; ++row) {
            const uchar *src = rgb.constScanLine(row);
            memcpy(buffer.data(), src, width * 3);
            if (TIFFWriteScanline(tif, buffer.data(), row, 0) < 0) {
                TIFFClose(tif);
                errorString = QString("TIFFWriteScanline failed on row %1").arg(row);
                return false;
            }
        }
    }

    TIFFClose(tif);
    return true;
}
