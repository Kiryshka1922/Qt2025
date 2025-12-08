#include "ui_state_observer.h"
#include <QPushButton>

UIStateObserver::UIStateObserver(QPushButton* nextButton)
    : nextButton_(nextButton)
{
}

void UIStateObserver::onStageChanged(int currentIndex, int totalStages) {
    if (!nextButton_) return;
    // Проверяем валидность индексов
    if (totalStages <= 0 || currentIndex < 0) {
        nextButton_->setDisabled(true);
        return;
    }
    // Отключаем кнопку, если загружен последний слой (индексация с 0)
    // Если currentIndex = totalStages - 1, значит это последний слой
    bool hasNextStage = (currentIndex + 1 < totalStages);
    if (hasNextStage) {
        nextButton_->setEnabled(true);
        nextButton_->setDisabled(false);  // Явно включаем кнопку
    } else {
        nextButton_->setDisabled(true);  // Явно отключаем кнопку
        nextButton_->setEnabled(false);  // Дополнительная гарантия отключения
    }
}

void UIStateObserver::onLoadStarted() {
    if (nextButton_) {
        nextButton_->setDisabled(true);  // Явно отключаем кнопку
    }
}

void UIStateObserver::onLoadFailed() {
    if (nextButton_) {
        nextButton_->setDisabled(true);  // Явно отключаем кнопку
    }
}

void UIStateObserver::onLastStageReached() {
    if (nextButton_) {
        nextButton_->setDisabled(true);  // Явно отключаем кнопку при достижении последнего слоя
        nextButton_->setEnabled(false);  // Дополнительная гарантия отключения
    }
}

