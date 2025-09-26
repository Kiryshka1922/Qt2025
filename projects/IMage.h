#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QScrollArea>

// Функция фильтрации изображения
void filter2D(QImage &image, double *kernel, size_t kWidth, size_t kHeight);

// Класс для отображения информации об изображении
class ImageInfoWidget : public QWidget
{
    Q_OBJECT

private:
    QImage *currentImage;
    QLabel *imageLabel;
    QTextEdit *infoText;
    QScrollArea *scrollArea;

public:
    ImageInfoWidget(QWidget *parent = nullptr);
    void setImage(const QImage &image);
    void updateInfo();
    void clear();

private slots:
    void onImageClicked();
};

// Основной виджет приложения
class Widget : public QWidget
{
    Q_OBJECT

private:
    QImage originalImage;
    QImage filteredImage;
    ImageInfoWidget *infoWidget;
    QLabel *imageLabel;
    QPushButton *loadButton;
    QPushButton *filterButton;
    QPushButton *resetButton;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *imageLayout;

public:
    Widget(QWidget *parent = nullptr);
    void loadImage(const QString &filename);
    void applyFilter();
    void resetImage();

private slots:
    void onLoadClicked();
    void onFilterClicked();
    void onResetClicked();
};