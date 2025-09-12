#pragma once

#include <string>
#include <deque>

class FrameStats {
public:
    FrameStats();
    
    // Update with new frame data
    void update(double fps, double frameTime, double updateTime, double renderTime);
    
    // Get current stats
    double getFPS() const { return currentFPS; }
    double getFrameTime() const { return currentFrameTime; }
    double getUpdateTime() const { return currentUpdateTime; }
    double getRenderTime() const { return currentRenderTime; }
    
    // Get averages over time
    double getAverageFPS() const;
    double getAverageFrameTime() const;
    
    // Get min/max
    double getMinFPS() const { return minFPS; }
    double getMaxFPS() const { return maxFPS; }
    
    // Format for display
    std::string format() const;
    std::string formatDetailed() const;
    
    // Reset stats
    void reset();
    
private:
    // Current frame stats
    double currentFPS;
    double currentFrameTime;
    double currentUpdateTime;
    double currentRenderTime;
    
    // Historical data (last 60 frames)
    std::deque<double> fpsHistory;
    std::deque<double> frameTimeHistory;
    static const size_t maxHistory = 60;
    
    // Min/max tracking
    double minFPS;
    double maxFPS;
};