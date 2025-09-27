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
// Функция фильтрации изображения
void filter2D(QImage &image, double *kernel, size_t kWidth, size_t kHeight);

// Основной виджет приложения
class Widget : public QWidget
{
    Q_OBJECT

private:
    QImage originalImage;
    QImage filteredImage;
    QLabel *imageLabel;
    QPushButton *loadButton;
    QPushButton *filterButton;
    QPushButton *resetButton;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;

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