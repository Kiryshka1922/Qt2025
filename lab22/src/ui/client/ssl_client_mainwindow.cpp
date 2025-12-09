#include "ssl_client_mainwindow.h"
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
#include <QLineEdit>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include "../../core/image_manager.h"
#include "../../ui/observer/ui_state_observer.h"
#include "../../network/client/ssl_image_client.h"
#include "../../core/common_types.h"

SslClientMainWindow::SslClientMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , manager(std::make_unique<ImageManager>(this))
    , client(std::make_unique<SslImageClient>(this))
{
    QWidget *c = new QWidget;
    setCentralWidget(c);

    imageLabel = new QLabel;
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(800,600);
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(imageLabel);

    urlEdit = new QLineEdit;
    urlEdit->setPlaceholderText("https://localhost:8443/image.jpg");
    fetchButton = new QPushButton("Fetch from SSL Server");
    openButton = new QPushButton("Open JPEG");
    nextButton = new QPushButton(">");
    saveButton = new QPushButton("Save As...");
    nextButton->setDisabled(true);

    stateObserver = std::make_unique<UIStateObserver>(nextButton);

    qualitySlider = new QSlider(Qt::Horizontal);
    qualitySlider->setRange(0,100);
    qualitySlider->setValue(85);

    dctCombo = new QComboBox;
    dctCombo->addItem("JDCT_ISLOW (accurate)", 0);
    dctCombo->addItem("JDCT_IFAST (fast)", 1);
    dctCombo->addItem("JDCT_FLOAT (float)", 2);

    progressiveCheck = new QCheckBox("Progressive");

    QHBoxLayout *urlRow = new QHBoxLayout;
    urlRow->addWidget(new QLabel("URL (HTTPS):"));
    urlRow->addWidget(urlEdit);
    urlRow->addWidget(fetchButton);

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
    layout->addLayout(urlRow);
    layout->addLayout(topRow);
    layout->addLayout(optsRow);
    layout->addWidget(scroll);

    c->setLayout(layout);

    connect(fetchButton, &QPushButton::clicked, this, &SslClientMainWindow::fetchImage);
    connect(openButton, &QPushButton::clicked, this, &SslClientMainWindow::openFile);
    connect(nextButton, &QPushButton::clicked, this, &SslClientMainWindow::nextStage);
    connect(saveButton, &QPushButton::clicked, this, &SslClientMainWindow::saveFile);

    connect(manager.get(), &ImageManager::stageLoaded, this, &SslClientMainWindow::onStageLoaded);
    connect(manager.get(), &ImageManager::loadFailed, this, &SslClientMainWindow::onLoadFailed);
    connect(manager.get(), &ImageManager::lastStageReached, this, &SslClientMainWindow::onLastStageReached);

    connect(client.get(), &SslImageClient::imageReceived, this, &SslClientMainWindow::onImageReceived);
    connect(client.get(), &SslImageClient::errorOccurred, this, &SslClientMainWindow::onNetworkError);
}

SslClientMainWindow::~SslClientMainWindow() {
    if (!tempImagePath.isEmpty()) {
        QFile::remove(tempImagePath);
    }
}

void SslClientMainWindow::fetchImage() {
    QString url = urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a URL");
        return;
    }
    
    // Сбрасываем флаг ошибки перед новым запросом
    errorShown = false;
    
    if (stateObserver) {
        stateObserver->onLoadStarted();
    }
    
    fetchButton->setEnabled(false);
    client->fetchImage(url);
}

void SslClientMainWindow::onImageReceived(const QImage &image) {
    fetchButton->setEnabled(true);
    
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Received invalid image");
        if (stateObserver) {
            stateObserver->onLoadFailed();
        }
        return;
    }

    // Сохраняем изображение во временный файл для работы с ImageManager
    QTemporaryFile tempFile;
    tempFile.setFileTemplate(QDir::temp().absoluteFilePath("lab22_ssl_XXXXXX.jpg"));
    if (!tempFile.open()) {
        QMessageBox::warning(this, "Error", "Failed to create temporary file");
        return;
    }
    
    if (!image.save(&tempFile, "JPEG")) {
        QMessageBox::warning(this, "Error", "Failed to save temporary image");
        return;
    }
    
    tempFile.close();
    tempImagePath = tempFile.fileName();
    
    // Обновляем UI сразу с полученным изображением
    updateImage(image);
    // Загружаем через ImageManager для поддержки прогрессивных JPEG
    manager->loadFile(tempImagePath);
}

void SslClientMainWindow::onNetworkError(const QString &error) {
    fetchButton->setEnabled(true);
    
    // Показываем ошибку только один раз
    if (!errorShown) {
        errorShown = true;
        QMessageBox::warning(this, "Network Error", error);
    }
    
    if (stateObserver) {
        stateObserver->onLoadFailed();
    }
}

void SslClientMainWindow::openFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open JPEG", QString(), "JPEG Files (*.jpg *.jpeg)");
    if (file.isEmpty()) return;
    if (stateObserver) {
        stateObserver->onLoadStarted();
    }
    manager->loadFile(file);
}

void SslClientMainWindow::nextStage() {
    manager->nextStage();
}

void SslClientMainWindow::saveFile() {
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

void SslClientMainWindow::onStageLoaded(int index, int total) {
    totalStages = total;
    currentIndex = index;
    QImage img = manager->currentImage();
    if (!img.isNull()) updateImage(img);
    if (stateObserver) {
        stateObserver->onStageChanged(currentIndex, totalStages);
    }
}

void SslClientMainWindow::onLoadFailed(const QString &reason) {
    QMessageBox::warning(this, "Load failed", reason);
    if (stateObserver) {
        stateObserver->onLoadFailed();
    }
}

void SslClientMainWindow::onLastStageReached() {
    if (stateObserver) {
        stateObserver->onLastStageReached();
    }
}

void SslClientMainWindow::updateImage(const QImage &img) {
    imageLabel->setPixmap(QPixmap::fromImage(img).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

