// =============================================================================
// physics.cpp - Gravity application and AABB vs tilemap collision resolution
// =============================================================================

#include "physics.h"
#include "utils.h"

// ---------------------------------------------------------------------------
// PhysicsBody
// ---------------------------------------------------------------------------

PhysicsBody::PhysicsBody()
    : position(0.f, 0.f), velocity(0.f, 0.f), size(32.f, 32.f),
      onGround(false)
{}

PhysicsBody::PhysicsBody(sf::Vector2f pos, sf::Vector2f sz)
    : position(pos), velocity(0.f, 0.f), size(sz), onGround(false)
{}

sf::FloatRect PhysicsBody::getBounds() const
{
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

// ---------------------------------------------------------------------------
// applyGravity
// ---------------------------------------------------------------------------
void applyGravity(PhysicsBody& body, float dt)
{
    body.velocity.y += GRAVITY * dt;
    if (body.velocity.y > MAX_FALL_SPEED)
        body.velocity.y = MAX_FALL_SPEED;
}

// ---------------------------------------------------------------------------
// resolveCollision
//
// Strategy: move X first, resolve X overlaps, then move Y, resolve Y overlaps.
// This avoids the corner-case "sticking" that happens when both axes are
// resolved simultaneously.
// ---------------------------------------------------------------------------
CollisionResult resolveCollision(PhysicsBody& body,
                                  const std::vector<sf::FloatRect>& tiles)
{
    CollisionResult result;
    body.onGround = false;

    // --- X axis -----------------------------------------------------------
    body.position.x += body.velocity.x * (1.f / 60.f); // use fixed sub-step

    sf::FloatRect bx(body.position.x, body.position.y,
                     body.size.x,     body.size.y);

    for (const auto& tile : tiles)
    {
        sf::FloatRect overlap;
        if (!bx.intersects(tile, overlap)) continue;

        // Push out on the smaller penetration depth
        if (overlap.width < overlap.height)
        {
            // Horizontal collision
            if (body.position.x + body.size.x * 0.5f < tile.left + tile.width * 0.5f)
            {
                // Body is to the left of the tile
                body.position.x -= overlap.width;
                result.hit_right = true;
            }
            else
            {
                body.position.x += overlap.width;
                result.hit_left  = true;
            }
            body.velocity.x = 0.f;
            bx = sf::FloatRect(body.position.x, body.position.y,
                               body.size.x,     body.size.y);
        }
    }

    // --- Y axis -----------------------------------------------------------
    body.position.y += body.velocity.y * (1.f / 60.f);

    sf::FloatRect by(body.position.x, body.position.y,
                     body.size.x,     body.size.y);

    for (const auto& tile : tiles)
    {
        sf::FloatRect overlap;
        if (!by.intersects(tile, overlap)) continue;

        if (overlap.height <= overlap.width)
        {
            // Vertical collision
            if (body.position.y + body.size.y * 0.5f < tile.top + tile.height * 0.5f)
            {
                // Body is above the tile (landing)
                body.position.y -= overlap.height;
                body.onGround    = true;
                result.hit_bottom = true;
            }
            else
            {
                // Body is below (hitting ceiling)
                body.position.y  += overlap.height;
                result.hit_top    = true;
            }
            body.velocity.y = 0.f;
            by = sf::FloatRect(body.position.x, body.position.y,
                               body.size.x,     body.size.y);
        }
    }

    return result;
}
