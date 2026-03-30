#pragma once
// =============================================================================
// menu.h - Menu system (main menu, pause, game over, level complete, level select)
// =============================================================================

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "utils.h"

// ---------------------------------------------------------------------------
// Menu states
// ---------------------------------------------------------------------------
enum class MenuState
{
    MAIN_MENU,
    LEVEL_SELECT,
    PAUSE,
    GAME_OVER,
    LEVEL_COMPLETE
};

// ---------------------------------------------------------------------------
// Actions returned by Menu::update()
// ---------------------------------------------------------------------------
enum class MenuAction
{
    NONE,
    START_GAME,
    SELECT_LEVEL_1,
    SELECT_LEVEL_2,
    SELECT_LEVEL_3,
    RESUME,
    RESTART,
    QUIT_TO_MENU,
    EXIT_GAME
};

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------
class Menu
{
public:
    MenuState          currentState;
    int                selectedItem;
    std::vector<std::string> menuItems;
    bool               visible;
    bool               fontLoaded;
    int                selectedLevel;

    Menu();

    void       init(sf::Font& font);
    MenuAction update(class InputManager& input);
    void       draw(sf::RenderWindow& window, int playerHealth,
                    int maxHealth, int currentLevel, int totalLevels);

    void showMainMenu();
    void showPauseMenu();
    void showGameOver();
    void showLevelComplete();
    void showLevelSelect();

    int  getSelectedLevel() const { return selectedLevel; }
    void setVisible(bool v)       { visible = v; }
    bool isVisible()        const { return visible; }

private:
    sf::Font*          m_font;
    float              m_animTimer;

    void drawMainMenu   (sf::RenderWindow& window);
    void drawPauseMenu  (sf::RenderWindow& window);
    void drawGameOver   (sf::RenderWindow& window);
    void drawLevelComplete(sf::RenderWindow& window, int currentLevel);
    void drawLevelSelect(sf::RenderWindow& window);
    void drawHUD        (sf::RenderWindow& window, int playerHealth,
                         int maxHealth, int currentLevel, int totalLevels);

    void drawFallbackText(sf::RenderWindow& window, const std::string& text,
                          float x, float y, sf::Color color, float scale = 1.f);

    void drawMenuItem(sf::RenderWindow& window, const std::string& text,
                      float x, float y, bool selected);

    void drawOverlay(sf::RenderWindow& window, sf::Color color);
};
