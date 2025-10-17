#include "ImageInfoWidget.h"
#include <QLocale>
ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    clearInfo();
}
void ImageInfoWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("Информация об изображении");
    mainLayout->addWidget(titleLabel);
    
    m_formLayout = new QFormLayout();
    m_formLayout->setSpacing(5);
    m_formLayout->setLabelAlignment(Qt::AlignRight);
    
    m_filenameLabel = new QLabel();
    m_sizeLabel = new QLabel();
    m_resolutionLabel = new QLabel();
    
    m_formatLabel = new QLabel();
    m_depthLabel = new QLabel();

    m_formLayout->addRow("Файл:", m_filenameLabel);
    m_formLayout->addRow("Размер:", m_sizeLabel);
    m_formLayout->addRow("Разрешение:", m_resolutionLabel);
    m_formLayout->addRow("Формат:", m_formatLabel);
    m_formLayout->addRow("Глубина:", m_depthLabel);
    
    mainLayout->addLayout(m_formLayout);
    mainLayout->addStretch();
    
    setMinimumWidth(260);
}
void ImageInfoWidget::updateImageInfo(const QImage &image)
{
    if (image.isNull()) {
        clearInfo();
        return;
    }
    
    QLocale locale(QLocale::Russian);
    
    m_filenameLabel->setText("-");
    m_sizeLabel->setText(QString("%1 × %2 пикселей")
                        .arg(locale.toString(image.width()))
                        .arg(locale.toString(image.height())));
    
    m_resolutionLabel->setText(QString("%1 DPI × %2 DPI")
                              .arg(locale.toString(image.dotsPerMeterX() * 0.0254))
                              .arg(locale.toString(image.dotsPerMeterY() * 0.0254)));

    QString fmt;
    switch (image.format()) {
        case QImage::Format_Mono: fmt = "Mono (1bpp)"; break;
        case QImage::Format_Indexed8: fmt = "Indexed8"; break;
        case QImage::Format_Grayscale8: fmt = "Grayscale8"; break;
        case QImage::Format_RGB32: fmt = "RGB32"; break;
        case QImage::Format_ARGB32: fmt = "ARGB32"; break;
        case QImage::Format_RGB888: fmt = "RGB888"; break;
        default: fmt = QString("%1").arg((int)image.format()); break;
    }
    m_formatLabel->setText(fmt);
    m_depthLabel->setText(QString::number(image.depth()) + " bpp");
}
void ImageInfoWidget::clearInfo()
{
    m_filenameLabel->setText("-");
    m_sizeLabel->setText("-");
    m_resolutionLabel->setText("-");
    m_formatLabel->setText("-");
    m_depthLabel->setText("-");
}