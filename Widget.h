#pragma once


#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QImageReader>
#include <QApplication>
#include <ImageInfoWidget.h>
#include <RGBToHalf.h>
#include <binarizationFunction.h>
#include <QDialogButtonBox>

void filter2D(QImage &sourceImage, double *filterMatrix, size_t matrixWidth, size_t matrixHeight);

enum class TiffCompression {
    None,
    Deflate,   // параметр 1..9 (zip quality)
    LZMA,      // параметр 1..9 (preset)
    PackBits,
    LZW,
    CCITTFax3, // Group 3 - монохром
    CCITTFax4, // Group 4 - монохром
    JPEG       // параметр 0..100 (jpeg quality)
};

class Widget : public QWidget
{
    Q_OBJECT
private:
    QImage          sourceImage;
    QImage          processedImage;
    QLabel          *displayBaseLabel;
    QLabel          *displayChangedLabel;
    QPushButton     *loadImageButton;
    QPushButton     *applyFilterButton;
    QPushButton     *restoreButton;
    QPushButton     *saveButton;
    QVBoxLayout     *primaryLayout;
    QHBoxLayout     *controlLayout;
    QHBoxLayout     *contentLayout;
    ImageInfoWidget *imageInfoWidget;
    QComboBox       *filtersList;
    QSpinBox        *kernelSizeSpinBox;
    QTableWidget    *kernelTable;

    QPushButton *convertToGray601Button;
    QPushButton *convertToGray709Button;
    QPushButton *otsuBinarizationButton;
    QPushButton *huangBinarizationButton;
    QPushButton *niblackBinarizationButton;
    QPushButton *isodataBinarizationButton;
        
    double blurKernel[9] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
    double sharpenKernel[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    double outlineKernel[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    double leftSobelKernel[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
    double rightSobelKernel[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    double embossKernel[9] = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
    
    bool saveTiffWithLibTiff(const QImage &image, const QString &fileName,
                         TiffCompression compression, int parameter, QString &errorString);
public:
    Widget(QWidget *parentWidget = nullptr);
    void loadImageFile(const QString &filePath);
    void executeFilter();
    void restoreOriginal();
    void updateKernelTable();
    void loadPresetKernel(int index);
    double* getCurrentKernel();

private slots:
    void handleLoadClick();
    void handleFilterClick();
    void handleRestoreClick();
    void handleSaveClick();
    void handleKernelSizeChanged(int size);
    void handleFilterSelectionChanged(int index);
    void handleConvertToGray601();
    void handleConvertToGray709();
    void handleOtsuBinarization();
    void handleHuangBinarization();
    void handleNiblackBinarization();
    void handleISODATABinarization();
    void updateProcessedImage();

};