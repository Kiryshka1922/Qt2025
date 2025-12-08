#pragma once
#include <QObject>
#include <QImage>
#include <memory>
#include "common_types.h"
#include "save_strategy.h"

class ImageHandler;

class ImageManager : public QObject {
    Q_OBJECT
public:
    explicit ImageManager(QObject *parent = nullptr);
    ~ImageManager();
    void loadFile(const QString &path);
    void nextStage();
    QImage currentImage() const;
    bool saveCurrent(const QString &path, const SaveOptions &opts);
signals:
    void stageLoaded(int index, int total);
    void stagesReady(int total);
    void loadFailed(const QString &reason);
    void saveFinished(bool ok, const QString &path);
    void lastStageReached();  // Сигнал о достижении последнего слоя
private:
    std::unique_ptr<ImageHandler> handler;
    std::unique_ptr<SaveStrategy> saver;
    QImage cachedImage;
    int currentIndex = -1;
    int totalStages = 0;
    QString currentPath;
};