#pragma once

class ImageStateObserver {
public:
    virtual ~ImageStateObserver() = default;
    virtual void onStageChanged(int currentIndex, int totalStages) = 0;
    virtual void onLoadStarted() = 0;
    virtual void onLoadFailed() = 0;
};


