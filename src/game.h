#pragma once
// =============================================================================
// game.h - Main game class tying all systems together
// =============================================================================

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <string>
#include "utils.h"
#include "input.h"
#include "player.h"
#include "level.h"
#include "menu.h"

// ---------------------------------------------------------------------------
// Top-level game state
// ---------------------------------------------------------------------------
enum class GameState
{
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_LEVEL_COMPLETE,
    STATE_LEVEL_SELECT
};

// ---------------------------------------------------------------------------
// Game
// ---------------------------------------------------------------------------
class Game
{
public:
    Game();

    bool init();
    void run();

private:
    // SFML
    sf::RenderWindow m_window;
    sf::Font         m_font;
    sf::Clock        m_clock;
    sf::View         m_gameView;   // scrolls with camera
    sf::View         m_hudView;    // fixed, covers whole window

    // Subsystems
    InputManager   m_input;
    Player         m_player;
    Level          m_level;
    Menu           m_menu;

    // State
    GameState m_state;
    int       m_currentLevelIndex;
    int       m_totalLevels;
    float     m_deltaTime;

    // Screen shake
    float        m_shakeTimer;
    float        m_shakeMagnitude;
    sf::Vector2f m_shakeOffset;

    // Main loop methods
    void handleEvents();
    void update();
    void render();

    // Helpers
    bool loadLevel(int index);
    void updateCamera();
    void handleMenuAction(MenuAction action);

    std::string getLevelPath(int index) const;
};
