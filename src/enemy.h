#pragma once
// =============================================================================
// enemy.h - Enemy base class, Turret, Knight and Projectile definitions
// =============================================================================

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "utils.h"
#include "physics.h"

// ---------------------------------------------------------------------------
// Enemy type tag
// ---------------------------------------------------------------------------
enum class EnemyType { TURRET, KNIGHT };

// ---------------------------------------------------------------------------
// Projectile - fired by Turrets, collides with the player
// ---------------------------------------------------------------------------
struct Projectile
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float        radius;
    bool         alive;
    float        lifeTimer;     // seconds since spawned
    float        maxLifeTime;   // despawn after this

    Projectile(sf::Vector2f pos, sf::Vector2f vel);

    void          update(float dt);
    void          draw(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
};

// ---------------------------------------------------------------------------
// Enemy - abstract base
// ---------------------------------------------------------------------------
class Enemy
{
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f size;
    int          health;
    int          maxHealth;
    bool         alive;
    bool         facingRight;
    EnemyType    type;

    Enemy(EnemyType type, sf::Vector2f pos, sf::Vector2f sz, int health);
    virtual ~Enemy() = default;

    virtual void update(float dt, sf::Vector2f playerPos,
                        std::vector<sf::FloatRect>& tiles) = 0;
    virtual void draw(sf::RenderWindow& window) const = 0;

    sf::FloatRect getBounds()    const;
    void          takeDamage(int dmg);
    bool          isAlive()      const { return alive; }

    // Draws a small health bar above the enemy
    void drawHealthBar(sf::RenderWindow& window) const;
};

// ---------------------------------------------------------------------------
// Turret - static, shoots projectiles at the player
// ---------------------------------------------------------------------------
class Turret : public Enemy
{
public:
    float shootCooldown;   // seconds between shots
    float shootTimer;      // countdown to next shot
    float range;           // maximum engagement distance in pixels
    bool  hasLineOfSight;  // updated each frame

    // pointer to the level's projectile list so we can inject bullets
    std::vector<Projectile>* projectileList;

    Turret(sf::Vector2f pos, std::vector<Projectile>* projectiles);

    void update(float dt, sf::Vector2f playerPos,
                std::vector<sf::FloatRect>& tiles) override;
    void draw(sf::RenderWindow& window) const override;

private:
    void shoot(sf::Vector2f playerPos);
    float barrelAngle; // angle in degrees toward last-seen player
};

// ---------------------------------------------------------------------------
// Knight - patrols, chases player, deals melee damage
// ---------------------------------------------------------------------------
class Knight : public Enemy
{
public:
    float patrolRange;   // half-width of patrol in pixels from startX
    float patrolStartX;  // x-position where the knight was spawned
    float chaseRange;    // distance at which knight starts chasing
    float meleeRange;    // distance at which knight deals melee damage
    float meleeCooldown; // seconds between melee hits
    float meleeTimer;    // countdown to next melee hit
    float moveSpeed;     // pixels per second while patrolling
    float chaseSpeed;    // pixels per second while chasing
    bool  isChasing;
    bool  pendingMeleeDamage; // set true when melee connects, cleared by Level
    bool  onGround;
    PhysicsBody body;

    Knight(sf::Vector2f pos);

    void update(float dt, sf::Vector2f playerPos,
                std::vector<sf::FloatRect>& tiles) override;
    void draw(sf::RenderWindow& window) const override;

    // Called by Level after processing melee damage
    void clearMeleeFlag() { pendingMeleeDamage = false; }
    int  getMeleeDamage() const { return 1; }

private:
    void patrol(float dt);
    void chase(float dt, sf::Vector2f playerPos);
    float animTimer;  // drives the bobbing walk animation
};
