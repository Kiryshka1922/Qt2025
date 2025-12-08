#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include "image_manager.h"
#include "ui_state_observer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , manager(std::make_unique<ImageManager>(this))
{
    QWidget *c = new QWidget;
    setCentralWidget(c);

    imageLabel = new QLabel;
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(800,600);
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(imageLabel);

    openButton = new QPushButton("Open JPEG");
    nextButton = new QPushButton(">");
    saveButton = new QPushButton("Save As...");
    nextButton->setDisabled(true);  // Явно отключаем кнопку при инициализации

    // Инициализация Observer Pattern для управления состоянием UI
    stateObserver = std::make_unique<UIStateObserver>(nextButton);

    qualitySlider = new QSlider(Qt::Horizontal);
    qualitySlider->setRange(0,100);
    qualitySlider->setValue(85);

    dctCombo = new QComboBox;
    dctCombo->addItem("JDCT_ISLOW (accurate)", 0);
    dctCombo->addItem("JDCT_IFAST (fast)", 1);
    dctCombo->addItem("JDCT_FLOAT (float)", 2);

    progressiveCheck = new QCheckBox("Progressive");

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->addWidget(openButton);
    topRow->addWidget(nextButton);
    topRow->addWidget(saveButton);
    topRow->addStretch();

    QHBoxLayout *optsRow = new QHBoxLayout;
    optsRow->addWidget(new QLabel("Quality:"));
    optsRow->addWidget(qualitySlider);
    optsRow->addWidget(new QLabel("DCT:"));
    optsRow->addWidget(dctCombo);
    optsRow->addWidget(progressiveCheck);
    optsRow->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(topRow);
    layout->addLayout(optsRow);
    layout->addWidget(scroll);

    c->setLayout(layout);

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::nextStage);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);

    connect(manager.get(), &ImageManager::stageLoaded, this, &MainWindow::onStageLoaded);
    connect(manager.get(), &ImageManager::loadFailed, this, &MainWindow::onLoadFailed);
    connect(manager.get(), &ImageManager::lastStageReached, this, &MainWindow::onLastStageReached);
}

MainWindow::~MainWindow() {}

void MainWindow::openFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open JPEG", QString(), "JPEG Files (*.jpg *.jpeg)");
    if (file.isEmpty()) return;
    // Используем Observer Pattern для обновления состояния UI
    if (stateObserver) {
        stateObserver->onLoadStarted();
    }
    manager->loadFile(file);
}

void MainWindow::nextStage() {
    manager->nextStage();
}

void MainWindow::saveFile() {
    QString file = QFileDialog::getSaveFileName(this, "Save JPEG", QString(), "JPEG Files (*.jpg *.jpeg)");
    if (file.isEmpty()) return;

    SaveOptions opts;
    opts.progressive = progressiveCheck->isChecked();
    opts.quality = qualitySlider->value();
    opts.dct_method = dctCombo->currentData().toInt();

    bool ok = manager->saveCurrent(file, opts);
    if (!ok) QMessageBox::warning(this, "Error", "Failed to save file.");
    else QMessageBox::information(this, "Saved", "File saved.");
}

void MainWindow::onStageLoaded(int index, int total) {
    totalStages = total;
    currentIndex = index;
    QImage img = manager->currentImage();
    if (!img.isNull()) updateImage(img);
    if (stateObserver) {
        stateObserver->onStageChanged(currentIndex, totalStages);
    }
}

void MainWindow::onLoadFailed(const QString &reason) {
    QMessageBox::warning(this, "Load failed", reason);
    // Используем Observer Pattern для обновления состояния UI
    if (stateObserver) {
        stateObserver->onLoadFailed();
    }
}

void MainWindow::onLastStageReached() {
    // Явное уведомление о достижении последнего слоя
    if (stateObserver) {
        stateObserver->onLastStageReached();
    }
}

void MainWindow::updateImage(const QImage &img) {
    imageLabel->setPixmap(QPixmap::fromImage(img).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
