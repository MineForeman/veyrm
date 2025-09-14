/**
 * @file frame_stats.h
 * @brief Frame rate and performance statistics tracking
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <deque>

/**
 * @class FrameStats
 * @brief Tracks and analyzes frame rate and rendering performance
 *
 * The FrameStats class monitors game performance by tracking frame rate,
 * frame times, update times, and render times. It maintains a rolling
 * history of the last 60 frames for calculating averages and detecting
 * performance trends.
 *
 * Performance metrics tracked:
 * - FPS (frames per second)
 * - Total frame time
 * - Update time (game logic)
 * - Render time (drawing)
 * - Min/max FPS over time
 *
 * @see Config::getTargetFPS()
 * @see Config::getShowFPS()
 */
class FrameStats {
public:
    /**
     * @brief Construct frame statistics tracker
     * @note Initializes with zero values and empty history
     */
    FrameStats();

    /**
     * @brief Update statistics with new frame data
     * @param fps Current frames per second
     * @param frameTime Total frame time in milliseconds
     * @param updateTime Game logic update time in milliseconds
     * @param renderTime Rendering time in milliseconds
     */
    void update(double fps, double frameTime, double updateTime, double renderTime);

    // Get current frame stats

    /** @brief Get current FPS @return Current frames per second */
    double getFPS() const { return currentFPS; }

    /** @brief Get current frame time @return Frame time in milliseconds */
    double getFrameTime() const { return currentFrameTime; }

    /** @brief Get current update time @return Update time in milliseconds */
    double getUpdateTime() const { return currentUpdateTime; }

    /** @brief Get current render time @return Render time in milliseconds */
    double getRenderTime() const { return currentRenderTime; }

    // Get averages over time

    /**
     * @brief Get average FPS over recent frames
     * @return Average FPS over last 60 frames
     */
    double getAverageFPS() const;

    /**
     * @brief Get average frame time over recent frames
     * @return Average frame time in milliseconds
     */
    double getAverageFrameTime() const;

    // Get min/max values

    /** @brief Get minimum recorded FPS @return Lowest FPS value */
    double getMinFPS() const { return minFPS; }

    /** @brief Get maximum recorded FPS @return Highest FPS value */
    double getMaxFPS() const { return maxFPS; }

    /**
     * @brief Format stats for basic display
     * @return Formatted string with FPS information
     */
    std::string format() const;

    /**
     * @brief Format detailed stats for debug display
     * @return Formatted string with all timing information
     */
    std::string formatDetailed() const;

    /**
     * @brief Reset all statistics to initial state
     * @note Clears history and resets min/max tracking
     */
    void reset();
    
private:
    // Current frame stats
    double currentFPS;          ///< Current frames per second
    double currentFrameTime;    ///< Current total frame time (ms)
    double currentUpdateTime;   ///< Current update time (ms)
    double currentRenderTime;   ///< Current render time (ms)

    // Historical data (last 60 frames)
    std::deque<double> fpsHistory;       ///< FPS history buffer
    std::deque<double> frameTimeHistory; ///< Frame time history buffer
    static const size_t maxHistory = 60; ///< Maximum history size

    // Min/max tracking
    double minFPS;              ///< Minimum recorded FPS
    double maxFPS;              ///< Maximum recorded FPS
};