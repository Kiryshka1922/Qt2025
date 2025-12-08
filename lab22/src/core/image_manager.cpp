#include "image_manager.h"
#include "image_handler_factory.h"
#include "libjpeg_save_strategy.h"

ImageManager::ImageManager(QObject *parent)
    : QObject(parent)
    , saver(std::make_unique<LibJpegSaveStrategy>())
{
}

ImageManager::~ImageManager() = default;

void ImageManager::loadFile(const QString &path) {
    currentPath.clear();
    cachedImage = QImage();
    currentIndex = -1;
    totalStages = 0;
    if (path.isEmpty()) { emit loadFailed("Empty path"); return; }
    ImageFormat fmt = ImageHandlerFactory::detectFormat(path);
    handler = ImageHandlerFactory::createHandler(fmt);
    if (!handler) { emit loadFailed("Unsupported format"); return; }
    if (!handler->isValid(path)) { emit loadFailed("Invalid file"); return; }
    int n = handler->stageCount(path);
    if (n <= 0) { emit loadFailed("No stages or failed to read"); return; }
    currentPath = path;
    totalStages = n;
    currentIndex = 0;
    cachedImage = handler->getStage(path, 0);
    emit stagesReady(totalStages);
    emit stageLoaded(currentIndex, totalStages);
}

void ImageManager::nextStage() {
    if (!handler) return;
    // Проверяем, есть ли следующий слой
    if (currentIndex + 1 >= totalStages) {
        emit lastStageReached();  // Уведомляем, что достигнут последний слой
        return;
    }
    ++currentIndex;
    cachedImage = handler->getStage(currentPath, currentIndex);
    emit stageLoaded(currentIndex, totalStages);
    // Проверяем, стал ли текущий слой последним
    if (currentIndex + 1 >= totalStages) {
        emit lastStageReached();  // Уведомляем, что достигнут последний слой
    }
}

QImage ImageManager::currentImage() const { return cachedImage; }

bool ImageManager::saveCurrent(const QString &path, const SaveOptions &opts) {
    if (cachedImage.isNull()) return false;
    bool ok = saver->save(cachedImage, path, opts);
    emit saveFinished(ok, path);
    return ok;
}