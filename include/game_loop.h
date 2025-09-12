#pragma once

#include <chrono>
#include <atomic>
#include <functional>

class GameManager;

class GameLoop {
public:
    GameLoop(GameManager* game_manager);
    ~GameLoop();
    
    // Main loop control
    void run();
    void stop();
    bool isRunning() const { return running; }
    
    // Frame rate control
    void setTargetFPS(int fps);
    int getTargetFPS() const { return targetFPS; }
    
    // Performance monitoring
    double getCurrentFPS() const { return currentFPS; }
    double getFrameTime() const { return frameTime; }
    double getUpdateTime() const { return updateTime; }
    double getRenderTime() const { return renderTime; }
    
    // Callbacks for integration
    void setUpdateCallback(std::function<void(double)> callback) { updateCallback = callback; }
    void setRenderCallback(std::function<void()> callback) { renderCallback = callback; }
    void setInputCallback(std::function<void()> callback) { inputCallback = callback; }
    
private:
    // Core loop methods
    void processInput();
    void update(double deltaTime);
    void render();
    void limitFrameRate();
    void updatePerformanceStats();
    
    // Game references
    GameManager* game_manager;
    
    // Loop control
    std::atomic<bool> running;
    int targetFPS;
    double fixedTimeStep;
    double accumulator;
    
    // Timing
    std::chrono::steady_clock::time_point previousTime;
    std::chrono::steady_clock::time_point frameStartTime;
    
    // Performance stats
    double currentFPS;
    double frameTime;
    double updateTime;
    double renderTime;
    
    // Frame counting for FPS
    int frameCount;
    double fpsAccumulator;
    
    // Callbacks
    std::function<void(double)> updateCallback;
    std::function<void()> renderCallback;
    std::function<void()> inputCallback;
};