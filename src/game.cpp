// =============================================================================
// game.cpp - Main game loop and state management
// =============================================================================

#include "game.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

// =============================================================================
// Constructor
// =============================================================================

Game::Game()
    : m_state(GameState::STATE_MENU)
    , m_currentLevelIndex(0)
    , m_totalLevels(3)
    , m_deltaTime(0.f)
    , m_shakeTimer(0.f)
    , m_shakeMagnitude(8.f)
    , m_shakeOffset(0.f, 0.f)
{}

// =============================================================================
// init
// =============================================================================

bool Game::init()
{
    m_window.create(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Blasphemous Platformer",
        sf::Style::Titlebar | sf::Style::Close);
    m_window.setFramerateLimit(FPS);
    m_window.setKeyRepeatEnabled(false);

    m_gameView = sf::View(sf::FloatRect(0.f, 0.f,
                                        static_cast<float>(WINDOW_WIDTH),
                                        static_cast<float>(WINDOW_HEIGHT)));
    m_hudView  = sf::View(sf::FloatRect(0.f, 0.f,
                                        static_cast<float>(WINDOW_WIDTH),
                                        static_cast<float>(WINDOW_HEIGHT)));

    // Try common system font paths
    bool fontOk = m_font.loadFromFile(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!fontOk)
        fontOk = m_font.loadFromFile(
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    if (!fontOk)
        fontOk = m_font.loadFromFile(
            "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");

    if (fontOk)
        m_menu.init(m_font);
    else
        m_menu.showMainMenu();

    m_menu.showMainMenu();
    m_state = GameState::STATE_MENU;

    // Pre-load level 0 so it is immediately ready when the player starts
    loadLevel(0);

    return true;
}

// =============================================================================
// run
// =============================================================================

void Game::run()
{
    while (m_window.isOpen())
    {
        float rawDt = m_clock.restart().asSeconds();
        m_deltaTime = (rawDt > 0.05f) ? 0.05f : rawDt;

        handleEvents();
        update();
        render();
    }
}

// =============================================================================
// handleEvents
// =============================================================================

void Game::handleEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            m_window.close();

        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
        {
            if (m_state == GameState::STATE_PLAYING)
            {
                m_state = GameState::STATE_PAUSED;
                m_menu.showPauseMenu();
            }
        }
    }
}

// =============================================================================
// update
// =============================================================================

void Game::update()
{
    m_input.update();

    // Screen shake decay
    if (m_shakeTimer > 0.f)
    {
        m_shakeTimer -= m_deltaTime;
        float strength = m_shakeMagnitude * (m_shakeTimer / 0.3f);
        m_shakeOffset.x =
            (static_cast<float>(std::rand() % 200) - 100.f) / 100.f * strength;
        m_shakeOffset.y =
            (static_cast<float>(std::rand() % 200) - 100.f) / 100.f * strength;
    }
    else
    {
        m_shakeTimer  = 0.f;
        m_shakeOffset = sf::Vector2f(0.f, 0.f);
    }

    // Trigger shake on player damage
    if (m_state == GameState::STATE_PLAYING &&
        m_player.screenShakeTimer > 0.f)
    {
        m_shakeTimer              = m_player.screenShakeTimer;
        m_player.screenShakeTimer = 0.f;
    }

    switch (m_state)
    {
    // -----------------------------------------------------------------------
    case GameState::STATE_MENU:
    case GameState::STATE_LEVEL_SELECT:
    {
        MenuAction action = m_menu.update(m_input);
        handleMenuAction(action);
        break;
    }

    // -----------------------------------------------------------------------
    case GameState::STATE_PLAYING:
    {
        // Allow P key to pause in-game as well
        if (m_input.isKeyJustPressed(sf::Keyboard::P))
        {
            m_state = GameState::STATE_PAUSED;
            m_menu.showPauseMenu();
            break;
        }

        // 1. Update player (movement, gravity, attack timers)
        m_player.update(m_deltaTime, m_input);

        // 2. Resolve player against all solid tiles (platforms + moving platforms)
        m_player.resolveWithTiles(m_level.getAllSolidBounds());

        // 3. Update level (enemies, moving platforms, projectiles, collision checks)
        //    Level::update does NOT call player.update – only the parts above were
        //    handled in steps 1-2.
        m_level.update(m_deltaTime, m_player);

        // Check for death
        if (!m_player.isAlive())
        {
            m_state = GameState::STATE_GAME_OVER;
            m_menu.showGameOver();
        }

        // Check for level completion
        if (m_level.isComplete())
        {
            m_state = GameState::STATE_LEVEL_COMPLETE;
            m_menu.showLevelComplete();
        }

        updateCamera();
        break;
    }

    // -----------------------------------------------------------------------
    case GameState::STATE_PAUSED:
    {
        MenuAction action = m_menu.update(m_input);
        handleMenuAction(action);
        break;
    }

    // -----------------------------------------------------------------------
    case GameState::STATE_GAME_OVER:
    {
        MenuAction action = m_menu.update(m_input);
        handleMenuAction(action);
        break;
    }

    // -----------------------------------------------------------------------
    case GameState::STATE_LEVEL_COMPLETE:
    {
        MenuAction action = m_menu.update(m_input);
        handleMenuAction(action);
        break;
    }
    } // switch
}

// =============================================================================
// render
// =============================================================================

void Game::render()
{
    m_window.clear(COLOR_BACKGROUND);

    // Game world (camera-relative view)
    if (m_state == GameState::STATE_PLAYING  ||
        m_state == GameState::STATE_PAUSED   ||
        m_state == GameState::STATE_GAME_OVER ||
        m_state == GameState::STATE_LEVEL_COMPLETE)
    {
        sf::View shakeView = m_gameView;
        shakeView.setCenter(m_gameView.getCenter() + m_shakeOffset);
        m_window.setView(shakeView);

        m_level.draw(m_window);
        m_player.draw(m_window);
    }

    // HUD and menus (fixed screen-space view)
    m_window.setView(m_hudView);
    m_menu.draw(m_window,
                m_player.health,
                m_player.maxHealth,
                m_currentLevelIndex,
                m_totalLevels);

    m_window.display();
}

// =============================================================================
// loadLevel
// =============================================================================

bool Game::loadLevel(int index)
{
    m_currentLevelIndex = index;
    std::string path    = getLevelPath(index);

    if (!m_level.load(path))
    {
        std::cerr << "Game: failed to load level " << path << "\n";
        return false;
    }

    // Place player at spawn point
    m_player.body.position   = m_level.spawnPoint;
    m_player.checkpointPos   = m_level.spawnPoint;
    m_player.body.velocity   = sf::Vector2f(0.f, 0.f);
    m_player.alive           = true;
    m_player.health          = m_player.maxHealth;
    m_player.invincible      = false;
    m_player.invincibleTimer = 0.f;
    m_player.jumpCount       = 0;

    // Snap camera immediately (no lag on level start)
    float halfW   = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float halfH   = static_cast<float>(WINDOW_HEIGHT) * 0.5f;
    float camX    = m_player.body.position.x + m_player.body.size.x * 0.5f;
    float camY    = m_player.body.position.y + m_player.body.size.y * 0.5f;
    float maxCamX = static_cast<float>(m_level.levelWidth  * TILE_SIZE) - halfW;
    float maxCamY = static_cast<float>(m_level.levelHeight * TILE_SIZE) - halfH;
    camX = clamp(camX, halfW, maxCamX);
    camY = clamp(camY, halfH, maxCamY);
    m_gameView.setCenter(camX, camY);

    return true;
}

// =============================================================================
// updateCamera
// =============================================================================

void Game::updateCamera()
{
    float halfW = static_cast<float>(WINDOW_WIDTH)  * 0.5f;
    float halfH = static_cast<float>(WINDOW_HEIGHT) * 0.5f;

    // Target: player center with slight upward look-ahead
    float targetX = m_player.body.position.x + m_player.body.size.x * 0.5f;
    float targetY = m_player.body.position.y + m_player.body.size.y * 0.5f - 40.f;

    // Smooth lerp follow
    float curX = m_gameView.getCenter().x;
    float curY = m_gameView.getCenter().y;
    float newX = lerp(curX, targetX, 0.10f);
    float newY = lerp(curY, targetY, 0.10f);

    // Clamp to level bounds
    float maxCamX = static_cast<float>(m_level.levelWidth  * TILE_SIZE) - halfW;
    float maxCamY = static_cast<float>(m_level.levelHeight * TILE_SIZE) - halfH;
    newX = clamp(newX, halfW, maxCamX);
    newY = clamp(newY, halfH, maxCamY);

    m_gameView.setCenter(newX, newY);
}

// =============================================================================
// handleMenuAction
// =============================================================================

void Game::handleMenuAction(MenuAction action)
{
    switch (action)
    {
    case MenuAction::START_GAME:
        loadLevel(m_currentLevelIndex);
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::SELECT_LEVEL_1:
        loadLevel(0);
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::SELECT_LEVEL_2:
        loadLevel(1);
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::SELECT_LEVEL_3:
        loadLevel(2);
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::RESUME:
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::RESTART:
        loadLevel(m_currentLevelIndex);
        m_state = GameState::STATE_PLAYING;
        m_menu.setVisible(false);
        break;

    case MenuAction::QUIT_TO_MENU:
        m_state = GameState::STATE_MENU;
        m_menu.showMainMenu();
        break;

    case MenuAction::EXIT_GAME:
        m_window.close();
        break;

    case MenuAction::NONE:
    default:
        break;
    }
}

// =============================================================================
// getLevelPath
// =============================================================================

std::string Game::getLevelPath(int index) const
{
    return "assets/levels/level" + std::to_string(index + 1) + ".txt";
}
