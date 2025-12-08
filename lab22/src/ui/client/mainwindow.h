#pragma once
#include <QMainWindow>
#include <memory>

class QLabel;
class QPushButton;
class QSlider;
class QComboBox;
class QCheckBox;
class ImageManager;
class UIStateObserver;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void nextStage();
    void saveFile();
    void onStageLoaded(int index, int total);
    void onLoadFailed(const QString &reason);
    void onLastStageReached();

private:
    void updateImage(const QImage &img);

    QLabel *imageLabel;
    QPushButton *openButton;
    QPushButton *nextButton;
    QPushButton *saveButton;
    QSlider *qualitySlider;
    QComboBox *dctCombo;
    QCheckBox *progressiveCheck;

    std::unique_ptr<ImageManager> manager;
    std::unique_ptr<UIStateObserver> stateObserver;
    int totalStages = 0;
    int currentIndex = -1;
};
