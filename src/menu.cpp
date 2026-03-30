// =============================================================================
// menu.cpp - Menu rendering and navigation
// =============================================================================

#include "menu.h"
#include "input.h"
#include <cmath>

// =============================================================================
// Constructor
// =============================================================================

Menu::Menu()
    : currentState(MenuState::MAIN_MENU)
    , selectedItem(0)
    , visible(false)
    , fontLoaded(false)
    , selectedLevel(0)
    , m_font(nullptr)
    , m_animTimer(0.f)
{}

// =============================================================================
// init
// =============================================================================

void Menu::init(sf::Font& font)
{
    m_font     = &font;
    fontLoaded = true;
    showMainMenu();
}

// =============================================================================
// showXxx helpers
// =============================================================================

void Menu::showMainMenu()
{
    currentState = MenuState::MAIN_MENU;
    selectedItem = 0;
    menuItems    = { "Start Game", "Level Select", "Exit" };
    visible      = true;
}

void Menu::showPauseMenu()
{
    currentState = MenuState::PAUSE;
    selectedItem = 0;
    menuItems    = { "Resume", "Restart", "Main Menu" };
    visible      = true;
}

void Menu::showGameOver()
{
    currentState = MenuState::GAME_OVER;
    selectedItem = 0;
    menuItems    = { "Restart", "Main Menu" };
    visible      = true;
}

void Menu::showLevelComplete()
{
    currentState = MenuState::LEVEL_COMPLETE;
    selectedItem = 0;
    menuItems    = { "Next Level", "Main Menu" };
    visible      = true;
}

void Menu::showLevelSelect()
{
    currentState = MenuState::LEVEL_SELECT;
    selectedItem = 0;
    menuItems    = { "Level 1", "Level 2", "Level 3", "Back" };
    visible      = true;
}

// =============================================================================
// update
// =============================================================================

MenuAction Menu::update(InputManager& input)
{
    if (!visible) return MenuAction::NONE;

    m_animTimer += 1.f / 60.f;

    bool up     = input.isKeyJustPressed(sf::Keyboard::Up)   ||
                  input.isKeyJustPressed(sf::Keyboard::W);
    bool down   = input.isKeyJustPressed(sf::Keyboard::Down) ||
                  input.isKeyJustPressed(sf::Keyboard::S);
    bool select = input.isKeyJustPressed(sf::Keyboard::Enter)  ||
                  input.isKeyJustPressed(sf::Keyboard::Z)      ||
                  input.isKeyJustPressed(sf::Keyboard::Space)  ||
                  input.isKeyJustPressed(sf::Keyboard::Return);
    bool back   = input.isKeyJustPressed(sf::Keyboard::Escape);

    int count = static_cast<int>(menuItems.size());
    if (up   && count > 0) selectedItem = (selectedItem - 1 + count) % count;
    if (down && count > 0) selectedItem = (selectedItem + 1) % count;

    if (back)
    {
        if (currentState == MenuState::PAUSE)
            return MenuAction::RESUME;
        if (currentState == MenuState::LEVEL_SELECT)
        {
            showMainMenu();
            return MenuAction::NONE;
        }
    }

    if (!select) return MenuAction::NONE;

    // ---- Main menu ----
    if (currentState == MenuState::MAIN_MENU)
    {
        if (selectedItem == 0) return MenuAction::START_GAME;
        if (selectedItem == 1) { showLevelSelect(); return MenuAction::NONE; }
        if (selectedItem == 2) return MenuAction::EXIT_GAME;
    }

    // ---- Level select ----
    if (currentState == MenuState::LEVEL_SELECT)
    {
        if (selectedItem == 0) { selectedLevel = 0; return MenuAction::SELECT_LEVEL_1; }
        if (selectedItem == 1) { selectedLevel = 1; return MenuAction::SELECT_LEVEL_2; }
        if (selectedItem == 2) { selectedLevel = 2; return MenuAction::SELECT_LEVEL_3; }
        if (selectedItem == 3) { showMainMenu(); return MenuAction::NONE; }
    }

    // ---- Pause ----
    if (currentState == MenuState::PAUSE)
    {
        if (selectedItem == 0) return MenuAction::RESUME;
        if (selectedItem == 1) return MenuAction::RESTART;
        if (selectedItem == 2) return MenuAction::QUIT_TO_MENU;
    }

    // ---- Game over ----
    if (currentState == MenuState::GAME_OVER)
    {
        if (selectedItem == 0) return MenuAction::RESTART;
        if (selectedItem == 1) return MenuAction::QUIT_TO_MENU;
    }

    // ---- Level complete ----
    if (currentState == MenuState::LEVEL_COMPLETE)
    {
        if (selectedItem == 0) return MenuAction::START_GAME;  // "Next Level"
        if (selectedItem == 1) return MenuAction::QUIT_TO_MENU;
    }

    return MenuAction::NONE;
}

// =============================================================================
// draw
// =============================================================================

void Menu::draw(sf::RenderWindow& window, int playerHealth, int maxHealth,
                int currentLevel, int totalLevels)
{
    // HUD is always drawn (even while playing)
    drawHUD(window, playerHealth, maxHealth, currentLevel, totalLevels);

    if (!visible) return;

    switch (currentState)
    {
    case MenuState::MAIN_MENU:      drawMainMenu(window);                     break;
    case MenuState::PAUSE:          drawPauseMenu(window);                    break;
    case MenuState::GAME_OVER:      drawGameOver(window);                     break;
    case MenuState::LEVEL_COMPLETE: drawLevelComplete(window, currentLevel);  break;
    case MenuState::LEVEL_SELECT:   drawLevelSelect(window);                  break;
    }
}

// =============================================================================
// drawOverlay
// =============================================================================

void Menu::drawOverlay(sf::RenderWindow& window, sf::Color color)
{
    sf::RectangleShape overlay(sf::Vector2f(
        static_cast<float>(WINDOW_WIDTH),
        static_cast<float>(WINDOW_HEIGHT)));
    overlay.setFillColor(color);
    window.draw(overlay);
}

// =============================================================================
// drawMenuItem
// =============================================================================

void Menu::drawMenuItem(sf::RenderWindow& window, const std::string& text,
                        float x, float y, bool selected)
{
    float pulse   = selected ? (0.85f + 0.15f * std::sin(m_animTimer * 5.f)) : 1.f;
    sf::Color col = selected
                  ? sf::Color(static_cast<sf::Uint8>(255 * pulse),
                              static_cast<sf::Uint8>(215 * pulse),
                              0)
                  : COLOR_TEXT_DIM;
    unsigned int sz = selected ? 28u : 22u;

    if (fontLoaded && m_font)
    {
        sf::Text t;
        t.setFont(*m_font);
        t.setString(text);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        sf::FloatRect bounds = t.getLocalBounds();
        t.setPosition(x - bounds.width * 0.5f, y);
        window.draw(t);

        if (selected)
        {
            sf::Text cursor;
            cursor.setFont(*m_font);
            cursor.setString(">");
            cursor.setCharacterSize(sz);
            cursor.setFillColor(col);
            cursor.setPosition(x - bounds.width * 0.5f - 30.f, y);
            window.draw(cursor);
        }
    }
    else
    {
        drawFallbackText(window, text, x, y, col, selected ? 1.2f : 1.f);
    }
}

// =============================================================================
// drawFallbackText  (colored rectangles when no font is available)
// =============================================================================

void Menu::drawFallbackText(sf::RenderWindow& window, const std::string& text,
                             float x, float y, sf::Color color, float scale)
{
    float charW   = 10.f * scale;
    float charH   = 18.f * scale;
    float spacing = 12.f * scale;
    float totalW  = static_cast<float>(text.size()) * spacing;
    float startX  = x - totalW * 0.5f;

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == ' ') continue;
        sf::RectangleShape ch(sf::Vector2f(charW, charH));
        ch.setPosition(startX + static_cast<float>(i) * spacing, y);
        ch.setFillColor(color);
        window.draw(ch);
    }
}

// =============================================================================
// drawHUD
// =============================================================================

void Menu::drawHUD(sf::RenderWindow& window, int playerHealth, int maxHealth,
                   int currentLevel, int totalLevels)
{
    sf::RectangleShape hudBg(sf::Vector2f(220.f, 40.f));
    hudBg.setPosition(8.f, 8.f);
    hudBg.setFillColor(COLOR_HUD_BG);
    window.draw(hudBg);

    const float sqSize = 20.f;
    const float sqGap  = 4.f;
    for (int i = 0; i < maxHealth; ++i)
    {
        sf::RectangleShape sq(sf::Vector2f(sqSize, sqSize));
        sq.setPosition(14.f + static_cast<float>(i) * (sqSize + sqGap), 14.f);
        sq.setFillColor(i < playerHealth ? COLOR_HP_FULL : COLOR_HP_EMPTY);
        sq.setOutlineThickness(1.5f);
        sq.setOutlineColor(COLOR_HP_BORDER);
        window.draw(sq);
    }

    if (fontLoaded && m_font)
    {
        sf::Text lvlText;
        lvlText.setFont(*m_font);
        lvlText.setString("Level " + std::to_string(currentLevel + 1) +
                          " / " + std::to_string(totalLevels));
        lvlText.setCharacterSize(16u);
        lvlText.setFillColor(COLOR_TEXT_DIM);
        lvlText.setPosition(
            14.f + static_cast<float>(maxHealth) * (sqSize + sqGap) + 8.f,
            16.f);
        window.draw(lvlText);
    }
}

// =============================================================================
// drawMainMenu
// =============================================================================

void Menu::drawMainMenu(sf::RenderWindow& window)
{
    drawOverlay(window, sf::Color(0, 0, 0, 200));

    float cx = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float cy = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    if (fontLoaded && m_font)
    {
        sf::Text title;
        title.setFont(*m_font);
        title.setString("BLASPHEMOUS PLATFORMER");
        title.setCharacterSize(48u);
        title.setFillColor(COLOR_TEXT_TITLE);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(cx - tb.width * 0.5f, cy - 180.f);
        window.draw(title);

        sf::Text subtitle;
        subtitle.setFont(*m_font);
        subtitle.setString("A Dark Action Platformer");
        subtitle.setCharacterSize(20u);
        subtitle.setFillColor(COLOR_TEXT_DIM);
        sf::FloatRect sb = subtitle.getLocalBounds();
        subtitle.setPosition(cx - sb.width * 0.5f, cy - 125.f);
        window.draw(subtitle);
    }
    else
    {
        drawFallbackText(window, "BLASPHEMOUS PLATFORMER",
                         cx, cy - 180.f, COLOR_TEXT_TITLE, 1.5f);
    }

    sf::RectangleShape line(sf::Vector2f(400.f, 2.f));
    line.setPosition(cx - 200.f, cy - 100.f);
    line.setFillColor(COLOR_TEXT_TITLE);
    window.draw(line);

    for (int i = 0; i < static_cast<int>(menuItems.size()); ++i)
        drawMenuItem(window, menuItems[i], cx, cy - 40.f + static_cast<float>(i) * 50.f,
                     i == selectedItem);

    if (fontLoaded && m_font)
    {
        sf::Text hint;
        hint.setFont(*m_font);
        hint.setString(
            "Arrow Keys: Navigate   Z/Enter: Select   X: Attack   Space/Z: Jump");
        hint.setCharacterSize(14u);
        hint.setFillColor(COLOR_TEXT_DIM);
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setPosition(cx - hb.width * 0.5f,
                         static_cast<float>(WINDOW_HEIGHT) - 40.f);
        window.draw(hint);
    }
}

// =============================================================================
// drawPauseMenu
// =============================================================================

void Menu::drawPauseMenu(sf::RenderWindow& window)
{
    drawOverlay(window, sf::Color(0, 0, 0, 150));

    float cx = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float cy = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    if (fontLoaded && m_font)
    {
        sf::Text title;
        title.setFont(*m_font);
        title.setString("PAUSED");
        title.setCharacterSize(52u);
        title.setFillColor(COLOR_TEXT);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(cx - tb.width * 0.5f, cy - 140.f);
        window.draw(title);
    }
    else
    {
        drawFallbackText(window, "PAUSED", cx, cy - 140.f, COLOR_TEXT, 1.5f);
    }

    for (int i = 0; i < static_cast<int>(menuItems.size()); ++i)
        drawMenuItem(window, menuItems[i], cx, cy - 40.f + static_cast<float>(i) * 50.f,
                     i == selectedItem);
}

// =============================================================================
// drawGameOver
// =============================================================================

void Menu::drawGameOver(sf::RenderWindow& window)
{
    drawOverlay(window, sf::Color(0, 0, 0, 210));

    float cx = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float cy = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    float pulse = 0.7f + 0.3f * std::sin(m_animTimer * 2.f);
    sf::Color titleColor(
        static_cast<sf::Uint8>(200.f * pulse),
        static_cast<sf::Uint8>(20.f  * pulse),
        static_cast<sf::Uint8>(20.f  * pulse));

    if (fontLoaded && m_font)
    {
        sf::Text title;
        title.setFont(*m_font);
        title.setString("YOU HAVE DIED");
        title.setCharacterSize(60u);
        title.setFillColor(titleColor);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(cx - tb.width * 0.5f, cy - 160.f);
        window.draw(title);
    }
    else
    {
        drawFallbackText(window, "YOU HAVE DIED", cx, cy - 160.f, titleColor, 1.8f);
    }

    sf::RectangleShape divider(sf::Vector2f(340.f, 3.f));
    divider.setPosition(cx - 170.f, cy - 80.f);
    divider.setFillColor(titleColor);
    window.draw(divider);

    for (int i = 0; i < static_cast<int>(menuItems.size()); ++i)
        drawMenuItem(window, menuItems[i], cx, cy - 30.f + static_cast<float>(i) * 50.f,
                     i == selectedItem);
}

// =============================================================================
// drawLevelComplete
// =============================================================================

void Menu::drawLevelComplete(sf::RenderWindow& window, int currentLevel)
{
    drawOverlay(window, sf::Color(0, 0, 0, 180));

    float cx = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float cy = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    if (fontLoaded && m_font)
    {
        sf::Text title;
        title.setFont(*m_font);
        title.setString("LEVEL COMPLETE");
        title.setCharacterSize(54u);
        title.setFillColor(COLOR_TEXT_TITLE);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(cx - tb.width * 0.5f, cy - 160.f);
        window.draw(title);

        sf::Text sub;
        sub.setFont(*m_font);
        sub.setString("Level " + std::to_string(currentLevel + 1) + " cleared!");
        sub.setCharacterSize(22u);
        sub.setFillColor(COLOR_TEXT);
        sf::FloatRect sb = sub.getLocalBounds();
        sub.setPosition(cx - sb.width * 0.5f, cy - 100.f);
        window.draw(sub);
    }

    // Celebratory orbiting dots
    for (int i = 0; i < 12; ++i)
    {
        float angle = m_animTimer * 1.5f + static_cast<float>(i) * (6.28318f / 12.f);
        float r     = 80.f + 20.f * std::sin(m_animTimer * 3.f + static_cast<float>(i));
        sf::CircleShape dot(5.f);
        dot.setOrigin(5.f, 5.f);
        dot.setPosition(cx + r * std::cos(angle),
                        cy - 130.f + r * std::sin(angle));
        dot.setFillColor(sf::Color(
            static_cast<sf::Uint8>(200 + 55 * std::sin(m_animTimer + static_cast<float>(i))),
            static_cast<sf::Uint8>(180 + 35 * std::cos(m_animTimer * 2.f + static_cast<float>(i))),
            0));
        window.draw(dot);
    }

    for (int i = 0; i < static_cast<int>(menuItems.size()); ++i)
        drawMenuItem(window, menuItems[i], cx, cy + static_cast<float>(i) * 50.f,
                     i == selectedItem);
}

// =============================================================================
// drawLevelSelect
// =============================================================================

void Menu::drawLevelSelect(sf::RenderWindow& window)
{
    drawOverlay(window, sf::Color(0, 0, 0, 195));

    float cx = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float cy = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    if (fontLoaded && m_font)
    {
        sf::Text title;
        title.setFont(*m_font);
        title.setString("SELECT LEVEL");
        title.setCharacterSize(46u);
        title.setFillColor(COLOR_TEXT_TITLE);
        title.setStyle(sf::Text::Bold);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(cx - tb.width * 0.5f, cy - 180.f);
        window.draw(title);
    }

    static const char* levelDescs[] = {
        "Tutorial - Learn the basics",
        "Cathedral - Harder foes",
        "The Abyss  - Survive or perish"
    };

    for (int i = 0; i < 3; ++i)
    {
        float panelX = cx - 240.f + static_cast<float>(i) * 160.f;
        float panelY = cy - 90.f;
        bool  sel    = (i == selectedItem);

        sf::RectangleShape panel(sf::Vector2f(140.f, 100.f));
        panel.setPosition(panelX, panelY);
        panel.setFillColor(sel ? sf::Color(50, 40, 10) : sf::Color(20, 18, 24));
        panel.setOutlineThickness(sel ? 3.f : 1.5f);
        panel.setOutlineColor(sel ? COLOR_TEXT_TITLE : COLOR_TEXT_DIM);
        window.draw(panel);

        if (fontLoaded && m_font)
        {
            sf::Text num;
            num.setFont(*m_font);
            num.setString(std::to_string(i + 1));
            num.setCharacterSize(sel ? 42u : 34u);
            num.setFillColor(sel ? COLOR_TEXT_TITLE : COLOR_TEXT_DIM);
            sf::FloatRect nb = num.getLocalBounds();
            num.setPosition(panelX + 70.f - nb.width * 0.5f, panelY + 12.f);
            window.draw(num);

            sf::Text desc;
            desc.setFont(*m_font);
            desc.setString(levelDescs[i]);
            desc.setCharacterSize(11u);
            desc.setFillColor(COLOR_TEXT_DIM);
            sf::FloatRect db = desc.getLocalBounds();
            desc.setPosition(panelX + 70.f - db.width * 0.5f, panelY + 70.f);
            window.draw(desc);
        }
    }

    // Back button
    drawMenuItem(window, "Back", cx, cy + 60.f, selectedItem == 3);
}
