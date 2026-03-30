#pragma once
// =============================================================================
// level.h - Tile map, moving platforms, enemies, projectiles and level state
// =============================================================================

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "utils.h"
#include "enemy.h"
#include "player.h"

// ---------------------------------------------------------------------------
// Tile types
// ---------------------------------------------------------------------------
enum class TileType
{
    EMPTY        = 0,
    PLATFORM     = 1,
    MOVING_PLATFORM = 2,
    SPIKE        = 3,
    TURRET       = 4,
    KNIGHT       = 5,
    CHECKPOINT   = 100,
    SPAWN        = 101
};

// ---------------------------------------------------------------------------
// Tile
// ---------------------------------------------------------------------------
struct Tile
{
    TileType     type;
    sf::Vector2f position;  // world-space top-left in pixels
};

// ---------------------------------------------------------------------------
// MovingPlatform
// ---------------------------------------------------------------------------
struct MovingPlatform
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f startPos;
    sf::Vector2f endPos;
    sf::Vector2f size;
    bool         movingToEnd;

    MovingPlatform(sf::Vector2f start, sf::Vector2f end, float speed);
    void update(float dt);
    sf::FloatRect getBounds() const;
    void draw(sf::RenderWindow& window) const;
};

// ---------------------------------------------------------------------------
// Level
// ---------------------------------------------------------------------------
class Level
{
public:
    // Tile data
    std::vector<std::vector<TileType>> tileMap;  // [row][col]
    std::vector<Tile>                  tiles;
    int                                levelWidth;  // tiles
    int                                levelHeight; // tiles

    // Dynamic objects
    std::vector<MovingPlatform>        movingPlatforms;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile>            projectiles;

    // Player spawn / checkpoints
    sf::Vector2f               spawnPoint;
    std::vector<sf::Vector2f>  checkpoints;
    std::vector<bool>          checkpointActivated;
    int                        currentCheckpointIndex;

    // State
    bool levelComplete;
    bool loaded;

    // Background parallax layers
    std::vector<sf::RectangleShape> bgLayers;

    Level();

    bool load(const std::string& path);
    void update(float dt, Player& player);
    void draw(sf::RenderWindow& window);

    // Returns solid tile bounds for collision (platforms only)
    std::vector<sf::FloatRect> getStaticTileBounds() const;

    // Returns solid bounds including moving platforms
    std::vector<sf::FloatRect> getAllSolidBounds() const;

    TileType getTileAt(int tileX, int tileY) const;

    bool isComplete() const { return levelComplete; }

    void addProjectile(sf::Vector2f pos, sf::Vector2f vel);
    void reset();

private:
    void checkPlayerCollisions(Player& player);
    void buildBackground();
    void drawBackground(sf::RenderWindow& window);
    void drawTiles(sf::RenderWindow& window);
    void drawSpike(sf::RenderWindow& window, sf::Vector2f pos) const;
    void drawCheckpoint(sf::RenderWindow& window, sf::Vector2f pos,
                        bool activated) const;
};
