#include "frame_stats.h"
#include <sstream>
#include <iomanip>
#include <numeric>
#include <algorithm>

FrameStats::FrameStats() 
    : currentFPS(0.0),
      currentFrameTime(0.0),
      currentUpdateTime(0.0),
      currentRenderTime(0.0),
      minFPS(999999.0),
      maxFPS(0.0) {
}

void FrameStats::update(double fps, double frameTime, double updateTime, double renderTime) {
    currentFPS = fps;
    currentFrameTime = frameTime;
    currentUpdateTime = updateTime;
    currentRenderTime = renderTime;
    
    // Update history
    fpsHistory.push_back(fps);
    frameTimeHistory.push_back(frameTime);
    
    // Limit history size
    while (fpsHistory.size() > maxHistory) {
        fpsHistory.pop_front();
    }
    while (frameTimeHistory.size() > maxHistory) {
        frameTimeHistory.pop_front();
    }
    
    // Update min/max
    if (fps > 0) {
        minFPS = std::min(minFPS, fps);
        maxFPS = std::max(maxFPS, fps);
    }
}

double FrameStats::getAverageFPS() const {
    if (fpsHistory.empty()) return 0.0;
    
    double sum = std::accumulate(fpsHistory.begin(), fpsHistory.end(), 0.0);
    return sum / fpsHistory.size();
}

double FrameStats::getAverageFrameTime() const {
    if (frameTimeHistory.empty()) return 0.0;
    
    double sum = std::accumulate(frameTimeHistory.begin(), frameTimeHistory.end(), 0.0);
    return sum / frameTimeHistory.size();
}

std::string FrameStats::format() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "FPS: " << currentFPS;
    oss << " | Frame: " << currentFrameTime << "ms";
    return oss.str();
}

std::string FrameStats::formatDetailed() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "FPS: " << currentFPS << " (avg: " << getAverageFPS() << ")";
    oss << " | Frame: " << currentFrameTime << "ms";
    oss << " | Update: " << currentUpdateTime << "ms";
    oss << " | Render: " << currentRenderTime << "ms";
    oss << " | Min/Max FPS: " << minFPS << "/" << maxFPS;
    return oss.str();
}

void FrameStats::reset() {
    currentFPS = 0.0;
    currentFrameTime = 0.0;
    currentUpdateTime = 0.0;
    currentRenderTime = 0.0;
    minFPS = 999999.0;
    maxFPS = 0.0;
    fpsHistory.clear();
    frameTimeHistory.clear();
}