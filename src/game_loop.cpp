#include "game_loop.h"
#include "game_state.h"
#include <thread>
#include <algorithm>
#include <iostream>

GameLoop::GameLoop(GameManager* gm) 
    : game_manager(gm),
      running(false),
      targetFPS(60),
      fixedTimeStep(1.0 / 60.0),
      accumulator(0.0),
      currentFPS(0.0),
      frameTime(0.0),
      updateTime(0.0),
      renderTime(0.0),
      frameCount(0),
      fpsAccumulator(0.0) {
}

GameLoop::~GameLoop() {
    stop();
}

void GameLoop::setTargetFPS(int fps) {
    targetFPS = fps;
    fixedTimeStep = 1.0 / static_cast<double>(fps);
}

void GameLoop::run() {
    running = true;
    previousTime = std::chrono::steady_clock::now();
    
    while (running) {
        frameStartTime = std::chrono::steady_clock::now();
        
        // Calculate delta time
        auto currentTime = std::chrono::steady_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;
        
        // Prevent spiral of death (cap at 250ms)
        deltaTime = std::min(deltaTime, 0.25);
        
        // Update FPS counter
        fpsAccumulator += deltaTime;
        frameCount++;
        
        if (fpsAccumulator >= 1.0) {
            currentFPS = frameCount / fpsAccumulator;
            frameCount = 0;
            fpsAccumulator = 0.0;
        }
        
        // Add to accumulator for fixed timestep
        accumulator += deltaTime;
        
        // Process input once per frame
        processInput();
        
        // Fixed timestep updates
        auto updateStart = std::chrono::steady_clock::now();
        while (accumulator >= fixedTimeStep) {
            update(fixedTimeStep);
            accumulator -= fixedTimeStep;
        }
        auto updateEnd = std::chrono::steady_clock::now();
        updateTime = std::chrono::duration<double>(updateEnd - updateStart).count() * 1000.0; // Convert to ms
        
        // Render at actual frame rate
        auto renderStart = std::chrono::steady_clock::now();
        render();
        auto renderEnd = std::chrono::steady_clock::now();
        renderTime = std::chrono::duration<double>(renderEnd - renderStart).count() * 1000.0; // Convert to ms
        
        // Calculate total frame time
        auto frameEnd = std::chrono::steady_clock::now();
        frameTime = std::chrono::duration<double>(frameEnd - frameStartTime).count() * 1000.0; // Convert to ms
        
        // Limit frame rate
        limitFrameRate();
    }
}

void GameLoop::stop() {
    running = false;
}

void GameLoop::processInput() {
    if (inputCallback) {
        inputCallback();
    }
}

void GameLoop::update(double deltaTime) {
    // Skip updates if paused
    if (game_manager && game_manager->getState() == GameState::PAUSED) {
        return;
    }
    
    // Call update callback if set
    if (updateCallback) {
        updateCallback(deltaTime);
    }
    
    // Update game systems
    if (game_manager) {
        // Future: Update animations, particles, etc.
        // For now, turn system handles its own timing
    }
}

void GameLoop::render() {
    if (renderCallback) {
        renderCallback();
    }
}

void GameLoop::limitFrameRate() {
    // Calculate how long this frame took
    auto frameEnd = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::duration<double>(frameEnd - frameStartTime).count();
    
    // Calculate target frame time
    double targetFrameTime = 1.0 / static_cast<double>(targetFPS);
    
    // Sleep if we're running too fast
    if (frameDuration < targetFrameTime) {
        double sleepTime = targetFrameTime - frameDuration;
        std::this_thread::sleep_for(
            std::chrono::duration<double>(sleepTime)
        );
    }
}