/**
 * @file game_loop.h
 * @brief Main game loop with fixed timestep
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <chrono>
#include <atomic>
#include <functional>

class GameManager;

/**
 * @class GameLoop
 * @brief Manages the main game loop with frame rate control
 *
 * The GameLoop class implements a fixed timestep game loop that ensures
 * consistent game updates regardless of rendering speed. It handles input
 * processing, game updates, rendering, and performance monitoring.
 *
 * @see GameManager
 */
class GameLoop {
public:
    /**
     * @brief Construct GameLoop with game manager
     * @param game_manager Pointer to game manager
     */
    GameLoop(GameManager* game_manager);

    /// Destructor
    ~GameLoop();

    // Main loop control

    /**
     * @brief Start the main game loop
     * @note Blocks until stop() is called
     */
    void run();

    /**
     * @brief Stop the game loop
     * @note Thread-safe via atomic flag
     */
    void stop();

    /**
     * @brief Check if loop is running
     * @return true if loop is active
     */
    bool isRunning() const { return running; }
    
    // Frame rate control

    /**
     * @brief Set target frame rate
     * @param fps Target frames per second
     */
    void setTargetFPS(int fps);

    /**
     * @brief Get target frame rate
     * @return Target FPS
     */
    int getTargetFPS() const { return targetFPS; }

    // Performance monitoring

    /**
     * @brief Get current actual FPS
     * @return Measured frames per second
     */
    double getCurrentFPS() const { return currentFPS; }

    /**
     * @brief Get frame duration
     * @return Time for complete frame (ms)
     */
    double getFrameTime() const { return frameTime; }

    /**
     * @brief Get update duration
     * @return Time spent in update (ms)
     */
    double getUpdateTime() const { return updateTime; }

    /**
     * @brief Get render duration
     * @return Time spent rendering (ms)
     */
    double getRenderTime() const { return renderTime; }
    
    // Callbacks for integration

    /**
     * @brief Set update callback
     * @param callback Function called each update with delta time
     */
    void setUpdateCallback(std::function<void(double)> callback) { updateCallback = callback; }

    /**
     * @brief Set render callback
     * @param callback Function called each frame for rendering
     */
    void setRenderCallback(std::function<void()> callback) { renderCallback = callback; }

    /**
     * @brief Set input callback
     * @param callback Function called for input processing
     */
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