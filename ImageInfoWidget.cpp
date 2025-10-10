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
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    m_formLayout = new QFormLayout();
    m_formLayout->setSpacing(5);
    m_formLayout->setLabelAlignment(Qt::AlignRight);
    
    m_filenameLabel = new QLabel();
    m_sizeLabel = new QLabel();
    m_resolutionLabel = new QLabel();
    
    QString valueStyle = "QLabel { background-color: #f0f0f0; padding: 2px 5px; border: 1px solid #ccc; }";
    m_filenameLabel->setStyleSheet(valueStyle);
    m_sizeLabel->setStyleSheet(valueStyle);
    m_resolutionLabel->setStyleSheet(valueStyle);
    
    m_filenameLabel->setMinimumWidth(150);
    m_sizeLabel->setMinimumWidth(150);
    m_resolutionLabel->setMinimumWidth(150);
    
    m_formLayout->addRow("Файл:", m_filenameLabel);
    m_formLayout->addRow("Размер:", m_sizeLabel);
    m_formLayout->addRow("Разрешение:", m_resolutionLabel);
    
    mainLayout->addLayout(m_formLayout);
    mainLayout->addStretch();
    
    setStyleSheet("QWidget { background-color: white; border: 1px solid #ccc; padding: 10px; }");
    setMinimumWidth(300);
}
void ImageInfoWidget::updateImageInfo(const QImage &image)
{
    if (image.isNull()) {
        clearInfo();
        return;
    }
    
    QLocale locale(QLocale::Russian);
    
    m_filenameLabel->setText("Не сохранен");
    m_sizeLabel->setText(QString("%1 × %2 пикселей")
                        .arg(locale.toString(image.width()))
                        .arg(locale.toString(image.height())));
    
    m_resolutionLabel->setText(QString("%1 DPI × %2 DPI")
                              .arg(locale.toString(image.dotsPerMeterX() * 0.0254))
                              .arg(locale.toString(image.dotsPerMeterY() * 0.0254)));
}
void ImageInfoWidget::clearInfo()
{
    m_filenameLabel->setText("-");
    m_sizeLabel->setText("-");
    m_resolutionLabel->setText("-");
}