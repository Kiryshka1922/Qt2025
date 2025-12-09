#pragma once
#include <QMainWindow>
#include <memory>

class QLabel;
class QPushButton;
class QLineEdit;
class QSlider;
class QComboBox;
class QCheckBox;
class ImageManager;
class UIStateObserver;
class SslImageClient;

class SslClientMainWindow : public QMainWindow {
    Q_OBJECT
public:
    SslClientMainWindow(QWidget *parent = nullptr);
    ~SslClientMainWindow();

private slots:
    void fetchImage();
    void openFile();
    void nextStage();
    void saveFile();
    void onStageLoaded(int index, int total);
    void onLoadFailed(const QString &reason);
    void onLastStageReached();
    void onImageReceived(const QImage &image);
    void onNetworkError(const QString &error);

private:
    void updateImage(const QImage &img);

    QLabel *imageLabel;
    QPushButton *fetchButton;
    QPushButton *openButton;
    QPushButton *nextButton;
    QPushButton *saveButton;
    QLineEdit *urlEdit;
    QSlider *qualitySlider;
    QComboBox *dctCombo;
    QCheckBox *progressiveCheck;

    std::unique_ptr<ImageManager> manager;
    std::unique_ptr<UIStateObserver> stateObserver;
    std::unique_ptr<SslImageClient> client;
    int totalStages = 0;
    int currentIndex = -1;
    QString tempImagePath;
    bool errorShown = false; // Флаг для предотвращения множественных алертов
};

