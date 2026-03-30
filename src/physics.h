#pragma once
// =============================================================================
// physics.h - Physics constants, body definition, gravity and tile collision
// =============================================================================

#include <SFML/Graphics.hpp>
#include <vector>

// ---------------------------------------------------------------------------
// Physics tuning constants
// ---------------------------------------------------------------------------
static const float GRAVITY       =  800.0f;  // px / s²  downward acceleration
static const float MAX_FALL_SPEED=  600.0f;  // px / s   terminal velocity
static const float JUMP_FORCE    = -480.0f;  // px / s   initial upward impulse

// ---------------------------------------------------------------------------
// CollisionResult - which sides were blocked after resolving one frame
// ---------------------------------------------------------------------------
struct CollisionResult
{
    bool hit_left   = false;
    bool hit_right  = false;
    bool hit_top    = false;
    bool hit_bottom = false;

    // Convenience: was any face hit?
    bool any() const { return hit_left || hit_right || hit_top || hit_bottom; }
};

// ---------------------------------------------------------------------------
// PhysicsBody - everything needed to simulate one axis-aligned rectangle
// ---------------------------------------------------------------------------
struct PhysicsBody
{
    sf::Vector2f position;   // top-left corner in world space
    sf::Vector2f velocity;   // pixels per second
    sf::Vector2f size;       // width x height in pixels
    bool         onGround;   // true when standing on solid ground

    PhysicsBody();
    PhysicsBody(sf::Vector2f pos, sf::Vector2f size);

    // Axis-aligned bounding box at current position
    sf::FloatRect getBounds() const;
};

// ---------------------------------------------------------------------------
// Free physics functions
// ---------------------------------------------------------------------------

// Accelerate body downward, capped at MAX_FALL_SPEED
void applyGravity(PhysicsBody& body, float dt);

// Push body out of any overlapping tiles and fill in CollisionResult.
// The body's position and velocity are modified; onGround is updated.
CollisionResult resolveCollision(PhysicsBody& body,
                                  const std::vector<sf::FloatRect>& tiles);
