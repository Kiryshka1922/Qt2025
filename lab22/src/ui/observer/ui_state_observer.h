#pragma once
#include "image_state_observer.h"

class QPushButton;

class UIStateObserver : public ImageStateObserver {
public:
    explicit UIStateObserver(QPushButton* nextButton);
    void onStageChanged(int currentIndex, int totalStages) override;
    void onLoadStarted() override;
    void onLoadFailed() override;
    void onLastStageReached();  // Явное уведомление о достижении последнего слоя

private:
    QPushButton* nextButton_;
};


