#pragma once
// =============================================================================
// player.h - Player class with state machine, physics, combat and rendering
// =============================================================================

#include <SFML/Graphics.hpp>
#include "utils.h"
#include "physics.h"
#include "input.h"
#include "animation.h"

// ---------------------------------------------------------------------------
// Player state machine states
// ---------------------------------------------------------------------------
enum class PlayerState
{
    IDLE,
    WALKING,
    JUMPING,
    FALLING,
    ATTACKING,
    HURT,
    DEAD
};

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------
class Player
{
public:
    // Physics
    PhysicsBody body;           // position, velocity, size, onGround

    // Vitals
    int  health;
    int  maxHealth;
    bool alive;

    // Invincibility frames after taking a hit
    bool  invincible;
    float invincibleTimer;
    float invincibleDuration;

    // Jump logic
    int  jumpCount;         // 0 = grounded, 1 = first jump, 2 = double jump used
    int  maxJumps;          // 2  (single + double)

    // Attack
    float attackTimer;      // time remaining in current attack
    float attackDuration;   // total attack window in seconds
    float attackCooldown;   // minimum time between attacks
    float attackCoolTimer;  // countdown to next allowed attack
    bool  attackActive;     // true while hitbox is live
    sf::FloatRect attackBox;

    // Knockback
    float knockbackTimer;
    float knockbackDuration;

    // Direction and state
    bool        facingRight;
    PlayerState state;

    // Checkpoint
    sf::Vector2f checkpointPos;

    // Animation
    AnimationSet animations;
    float        animTimer;    // generic timer used for shape-animation

    // Horizontal movement tuning
    float moveSpeed;
    float groundAccel;
    float groundFriction;
    float airAccel;
    float airFriction;

    // Score / combat feedback
    int   killCount;
    float screenShakeTimer;

    // Constructor
    Player();

    // Core loop
    void update(float dt, InputManager& input);
    void draw(sf::RenderWindow& window) const;

    // Actions
    void jump();
    void attack();
    void takeDamage(int dmg, sf::Vector2f knockbackDir);
    void heal(int amount);

    // Respawn at stored checkpoint
    void respawn();

    // Accessors
    sf::FloatRect getBounds()        const;
    sf::FloatRect getAttackBounds()  const;
    bool          isAttacking()      const;
    bool          isAlive()          const { return alive; }

    void setCheckpoint(sf::Vector2f pos);

    // Apply tile-based collisions (called by Level each frame)
    void resolveWithTiles(const std::vector<sf::FloatRect>& tiles);

private:
    void updateMovement(float dt, InputManager& input);
    void updateAttack(float dt, InputManager& input);
    void updateState();
    void drawBody(sf::RenderWindow& window) const;
    void drawSword(sf::RenderWindow& window) const;
    void drawHealthBar(sf::RenderWindow& window) const;
    void drawDebugBounds(sf::RenderWindow& window) const;
};
