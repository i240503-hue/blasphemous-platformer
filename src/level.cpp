// =============================================================================
// level.cpp - Level loading, updating and rendering
// =============================================================================

#include "level.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

// =============================================================================
// MovingPlatform
// =============================================================================

MovingPlatform::MovingPlatform(sf::Vector2f start, sf::Vector2f end, float speed)
    : position(start), startPos(start), endPos(end),
      size(static_cast<float>(TILE_SIZE * 3), static_cast<float>(TILE_SIZE)),
      movingToEnd(true)
{
    sf::Vector2f dir = end - start;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f)
        velocity = sf::Vector2f(dir.x / len * speed, dir.y / len * speed);
    else
        velocity = sf::Vector2f(0.f, 0.f);
}

void MovingPlatform::update(float dt)
{
    position += velocity * dt;

    if (movingToEnd)
    {
        bool reachedX = (velocity.x >= 0.f) ? position.x >= endPos.x
                                             : position.x <= endPos.x;
        bool reachedY = (velocity.y >= 0.f) ? position.y >= endPos.y
                                             : position.y <= endPos.y;
        if (reachedX && reachedY)
        {
            position    = endPos;
            velocity    = -velocity;
            movingToEnd = false;
        }
    }
    else
    {
        bool reachedX = (velocity.x >= 0.f) ? position.x >= startPos.x
                                             : position.x <= startPos.x;
        bool reachedY = (velocity.y >= 0.f) ? position.y >= startPos.y
                                             : position.y <= startPos.y;
        if (reachedX && reachedY)
        {
            position    = startPos;
            velocity    = -velocity;
            movingToEnd = true;
        }
    }
}

sf::FloatRect MovingPlatform::getBounds() const
{
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

void MovingPlatform::draw(sf::RenderWindow& window) const
{
    sf::RectangleShape shape(size);
    shape.setPosition(position);
    shape.setFillColor(COLOR_MOVING_PLATFORM);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(COLOR_MOVING_BORDER);
    window.draw(shape);

    // Direction indicator stripe on top
    sf::RectangleShape stripe(sf::Vector2f(size.x, 4.f));
    stripe.setPosition(position.x, position.y);
    stripe.setFillColor(sf::Color(80, 140, 80, 160));
    window.draw(stripe);
}

// =============================================================================
// Level constructor
// =============================================================================

Level::Level()
    : levelWidth(0), levelHeight(0),
      currentCheckpointIndex(-1),
      levelComplete(false), loaded(false)
{}

// =============================================================================
// load
// =============================================================================

bool Level::load(const std::string& path)
{
    tileMap.clear();
    tiles.clear();
    movingPlatforms.clear();
    enemies.clear();
    projectiles.clear();
    checkpoints.clear();
    checkpointActivated.clear();
    bgLayers.clear();
    levelComplete          = false;
    currentCheckpointIndex = -1;
    spawnPoint             = sf::Vector2f(64.f, 64.f);

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Level: cannot open " << path << "\n";
        return false;
    }

    std::vector<std::vector<std::string>> rawRows;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        std::vector<std::string> row;
        std::istringstream ss(line);
        std::string token;
        while (ss >> token) row.push_back(token);
        if (!row.empty()) rawRows.push_back(row);
    }
    file.close();

    if (rawRows.empty()) return false;

    levelHeight = static_cast<int>(rawRows.size());
    levelWidth  = 0;
    for (auto& r : rawRows)
        levelWidth = std::max(levelWidth, static_cast<int>(r.size()));

    tileMap.assign(levelHeight,
                   std::vector<TileType>(levelWidth, TileType::EMPTY));

    for (int row = 0; row < levelHeight; ++row)
    {
        for (int col = 0; col < static_cast<int>(rawRows[row].size()); ++col)
        {
            const std::string& tok = rawRows[row][col];
            float wx = static_cast<float>(col * TILE_SIZE);
            float wy = static_cast<float>(row * TILE_SIZE);

            TileType t = TileType::EMPTY;

            if      (tok == "1") t = TileType::PLATFORM;
            else if (tok == "2") t = TileType::MOVING_PLATFORM;
            else if (tok == "3") t = TileType::SPIKE;
            else if (tok == "4") t = TileType::TURRET;
            else if (tok == "5") t = TileType::KNIGHT;
            else if (tok == "C") t = TileType::CHECKPOINT;
            else if (tok == "S") { t = TileType::SPAWN; spawnPoint = sf::Vector2f(wx, wy - TILE_SIZE); }

            tileMap[row][col] = t;

            if (t == TileType::PLATFORM || t == TileType::SPIKE ||
                t == TileType::CHECKPOINT || t == TileType::SPAWN)
            {
                tiles.push_back({t, sf::Vector2f(wx, wy)});
            }
            else if (t == TileType::MOVING_PLATFORM)
            {
                sf::Vector2f start(wx, wy);
                sf::Vector2f end(wx + TILE_SIZE * 3, wy);
                movingPlatforms.emplace_back(start, end, 80.f);
            }
            else if (t == TileType::TURRET)
            {
                sf::Vector2f epos(wx, wy - 32.f);
                enemies.push_back(std::make_unique<Turret>(epos, &projectiles));
                tileMap[row][col] = TileType::EMPTY;
            }
            else if (t == TileType::KNIGHT)
            {
                sf::Vector2f epos(wx, wy - 44.f);
                enemies.push_back(std::make_unique<Knight>(epos));
                tileMap[row][col] = TileType::EMPTY;
            }
            else if (t == TileType::CHECKPOINT)
            {
                checkpoints.push_back(sf::Vector2f(wx, wy - TILE_SIZE));
                checkpointActivated.push_back(false);
            }
        }
    }

    buildBackground();
    loaded = true;
    return true;
}

// =============================================================================
// update  (does NOT call player.update – that is done by Game)
// =============================================================================

void Level::update(float dt, Player& player)
{
    if (!loaded) return;

    // Update moving platforms
    for (auto& mp : movingPlatforms)
        mp.update(dt);

    // Update enemies
    std::vector<sf::FloatRect> staticTiles = getStaticTileBounds();
    sf::Vector2f playerCenter = player.body.position +
                                sf::Vector2f(player.body.size.x * 0.5f,
                                             player.body.size.y * 0.5f);
    for (auto& e : enemies)
    {
        if (e->isAlive())
            e->update(dt, playerCenter, staticTiles);
    }

    // Update projectiles
    for (auto& p : projectiles)
        p.update(dt);

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
                       [](const Projectile& p){ return !p.alive; }),
        projectiles.end());

    // Collision checks between player and world objects
    checkPlayerCollisions(player);

    // Level complete when player reaches the right edge
    if (player.body.position.x >= static_cast<float>((levelWidth - 2) * TILE_SIZE))
        levelComplete = true;
}

// =============================================================================
// checkPlayerCollisions
// =============================================================================

void Level::checkPlayerCollisions(Player& player)
{
    if (!player.alive) return;

    sf::FloatRect pb = player.getBounds();

    // --- Spikes ---
    for (auto& tile : tiles)
    {
        if (tile.type != TileType::SPIKE) continue;
        sf::FloatRect sb(tile.position.x, tile.position.y,
                         static_cast<float>(TILE_SIZE),
                         static_cast<float>(TILE_SIZE));
        if (rectIntersect(pb, sb))
        {
            player.takeDamage(3, sf::Vector2f(0.f, -1.f));
            return; // one lethal interaction per frame is enough
        }
    }

    // --- Checkpoints ---
    for (int i = 0; i < static_cast<int>(checkpoints.size()); ++i)
    {
        if (checkpointActivated[i]) continue;
        sf::FloatRect cb(checkpoints[i].x, checkpoints[i].y,
                         static_cast<float>(TILE_SIZE),
                         static_cast<float>(TILE_SIZE) * 2.f);
        if (rectIntersect(pb, cb))
        {
            checkpointActivated[i] = true;
            currentCheckpointIndex = i;
            player.setCheckpoint(checkpoints[i]);
        }
    }

    // --- Enemy projectiles ---
    for (auto& proj : projectiles)
    {
        if (!proj.alive) continue;
        if (rectIntersect(pb, proj.getBounds()))
        {
            sf::Vector2f kbDir = normalize(player.body.position - proj.position);
            player.takeDamage(1, kbDir);
            proj.alive = false;
        }
    }

    // --- Enemy melee (Knights) ---
    for (auto& e : enemies)
    {
        if (!e->isAlive() || e->type != EnemyType::KNIGHT) continue;
        Knight* k = static_cast<Knight*>(e.get());
        if (k->pendingMeleeDamage)
        {
            sf::Vector2f kbDir = normalize(player.body.position - k->position);
            player.takeDamage(k->getMeleeDamage(), kbDir);
            k->clearMeleeFlag();
        }
    }

    // --- Player attack vs enemies ---
    if (player.isAttacking())
    {
        sf::FloatRect ab = player.getAttackBounds();
        for (auto& e : enemies)
        {
            if (!e->isAlive()) continue;
            if (rectIntersect(ab, e->getBounds()))
            {
                e->takeDamage(1);
                if (!e->isAlive()) player.killCount++;
            }
        }
        // Deflect incoming projectiles with the sword
        for (auto& proj : projectiles)
        {
            if (!proj.alive) continue;
            if (rectIntersect(ab, proj.getBounds()))
                proj.alive = false;
        }
    }
}

// =============================================================================
// getStaticTileBounds / getAllSolidBounds
// =============================================================================

std::vector<sf::FloatRect> Level::getStaticTileBounds() const
{
    std::vector<sf::FloatRect> bounds;
    bounds.reserve(tiles.size());
    for (const auto& t : tiles)
    {
        if (t.type == TileType::PLATFORM)
        {
            bounds.emplace_back(t.position.x, t.position.y,
                                static_cast<float>(TILE_SIZE),
                                static_cast<float>(TILE_SIZE));
        }
    }
    return bounds;
}

std::vector<sf::FloatRect> Level::getAllSolidBounds() const
{
    std::vector<sf::FloatRect> bounds = getStaticTileBounds();
    for (const auto& mp : movingPlatforms)
        bounds.push_back(mp.getBounds());
    return bounds;
}

TileType Level::getTileAt(int tx, int ty) const
{
    if (ty < 0 || ty >= levelHeight || tx < 0 || tx >= levelWidth)
        return TileType::EMPTY;
    return tileMap[ty][tx];
}

void Level::addProjectile(sf::Vector2f pos, sf::Vector2f vel)
{
    projectiles.emplace_back(pos, vel);
}

// =============================================================================
// reset
// =============================================================================

void Level::reset()
{
    levelComplete          = false;
    currentCheckpointIndex = -1;
    std::fill(checkpointActivated.begin(), checkpointActivated.end(), false);
    projectiles.clear();
}

// =============================================================================
// buildBackground
// =============================================================================

void Level::buildBackground()
{
    bgLayers.clear();
    float levelPxW = static_cast<float>(levelWidth  * TILE_SIZE);
    float levelPxH = static_cast<float>(levelHeight * TILE_SIZE);

    // Far layer: very dark near-black fill
    sf::RectangleShape far(sf::Vector2f(levelPxW, levelPxH));
    far.setFillColor(sf::Color(8, 8, 12));
    bgLayers.push_back(far);

    // Mid layer: subtle dark vertical pillars every 8 tiles
    for (int x = 0; x < levelWidth; x += 8)
    {
        float px = static_cast<float>(x * TILE_SIZE);
        sf::RectangleShape pillar(sf::Vector2f(4.f, levelPxH));
        pillar.setPosition(px, 0.f);
        pillar.setFillColor(sf::Color(18, 16, 22));
        bgLayers.push_back(pillar);
    }

    // Near layer: faint horizontal bands in the upper portion
    for (int y = 0; y < 10; ++y)
    {
        float py = static_cast<float>(y * TILE_SIZE);
        sf::RectangleShape band(sf::Vector2f(levelPxW, 2.f));
        band.setPosition(0.f, py + 16.f);
        band.setFillColor(sf::Color(25, 20, 30, 60));
        bgLayers.push_back(band);
    }
}

// =============================================================================
// draw
// =============================================================================

void Level::draw(sf::RenderWindow& window)
{
    if (!loaded) return;
    drawBackground(window);
    drawTiles(window);

    for (auto& mp : movingPlatforms) mp.draw(window);
    for (auto& e  : enemies)         e->draw(window);
    for (auto& p  : projectiles)     p.draw(window);
}

void Level::drawBackground(sf::RenderWindow& window)
{
    for (auto& layer : bgLayers)
        window.draw(layer);
}

void Level::drawTiles(sf::RenderWindow& window)
{
    sf::RectangleShape platformShape(sf::Vector2f(
        static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)));

    for (const auto& tile : tiles)
    {
        switch (tile.type)
        {
        case TileType::PLATFORM:
            platformShape.setPosition(tile.position);
            platformShape.setFillColor(COLOR_PLATFORM);
            platformShape.setOutlineThickness(1.5f);
            platformShape.setOutlineColor(COLOR_PLATFORM_BORDER);
            window.draw(platformShape);
            break;

        case TileType::SPIKE:
            drawSpike(window, tile.position);
            break;

        case TileType::CHECKPOINT:
        {
            int idx = -1;
            for (int i = 0; i < static_cast<int>(checkpoints.size()); ++i)
            {
                if (std::abs(checkpoints[i].x - tile.position.x) < 1.f &&
                    std::abs(checkpoints[i].y - (tile.position.y - TILE_SIZE)) < 1.f)
                { idx = i; break; }
            }
            bool activated = (idx >= 0 &&
                              idx < static_cast<int>(checkpointActivated.size()))
                           ? checkpointActivated[idx] : false;
            drawCheckpoint(window, tile.position, activated);
            break;
        }

        default: break;
        }
    }
}

void Level::drawSpike(sf::RenderWindow& window, sf::Vector2f pos) const
{
    for (int s = 0; s < 2; ++s)
    {
        float offX = static_cast<float>(s) * (TILE_SIZE / 2.f);
        sf::ConvexShape spike(3);
        spike.setPoint(0, sf::Vector2f(offX + TILE_SIZE * 0.25f,
                                       static_cast<float>(TILE_SIZE)));
        spike.setPoint(1, sf::Vector2f(offX + TILE_SIZE * 0.5f,  0.f));
        spike.setPoint(2, sf::Vector2f(offX + TILE_SIZE * 0.75f,
                                       static_cast<float>(TILE_SIZE)));
        spike.setPosition(pos);
        spike.setFillColor(COLOR_SPIKE);
        spike.setOutlineThickness(1.f);
        spike.setOutlineColor(COLOR_SPIKE_TIP);
        window.draw(spike);
    }
}

void Level::drawCheckpoint(sf::RenderWindow& window, sf::Vector2f pos,
                            bool activated) const
{
    float w = static_cast<float>(TILE_SIZE);
    float h = static_cast<float>(TILE_SIZE) * 2.f;

    // Base pillar
    sf::RectangleShape pillar(sf::Vector2f(w * 0.3f, h));
    pillar.setPosition(pos.x + w * 0.35f, pos.y - h + TILE_SIZE);
    pillar.setFillColor(activated ? COLOR_CHECKPOINT_ON : COLOR_CHECKPOINT_OFF);
    window.draw(pillar);

    // Orb on top
    float orbR = 8.f;
    sf::CircleShape orb(orbR);
    orb.setOrigin(orbR, orbR);
    orb.setPosition(pos.x + w * 0.5f, pos.y - h + TILE_SIZE + 4.f);
    orb.setFillColor(activated ? COLOR_CHECKPOINT_ON : COLOR_CHECKPOINT_OFF);
    if (activated)
    {
        orb.setOutlineThickness(3.f);
        orb.setOutlineColor(COLOR_CHECKPOINT_GLOW);
    }
    window.draw(orb);
}
