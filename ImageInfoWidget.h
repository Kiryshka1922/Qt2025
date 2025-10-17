#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QImage>
class ImageInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(QWidget *parent = nullptr);
    void updateImageInfo(const QImage &image);
    void clearInfo();
private:
    void setupUI();
    
    QLabel *m_filenameLabel;
    QLabel *m_sizeLabel;
    QLabel *m_resolutionLabel;
    QLabel *m_formatLabel;
    QLabel *m_depthLabel;
    
    QFormLayout *m_formLayout;
};
#endif