// =============================================================================
// enemy.cpp - Turret, Knight and Projectile implementations
// =============================================================================

#include "enemy.h"
#include <cmath>
#include <algorithm>

// =============================================================================
// Projectile
// =============================================================================

Projectile::Projectile(sf::Vector2f pos, sf::Vector2f vel)
    : position(pos), velocity(vel), radius(6.f),
      alive(true), lifeTimer(0.f), maxLifeTime(4.f)
{}

void Projectile::update(float dt)
{
    if (!alive) return;
    position  += velocity * dt;
    lifeTimer += dt;
    if (lifeTimer >= maxLifeTime) alive = false;
}

void Projectile::draw(sf::RenderWindow& window) const
{
    if (!alive) return;

    // Glowing core
    sf::CircleShape core(radius);
    core.setOrigin(radius, radius);
    core.setPosition(position);
    core.setFillColor(COLOR_PROJECTILE);

    // Outer glow halo
    sf::CircleShape glow(radius + 4.f);
    glow.setOrigin(radius + 4.f, radius + 4.f);
    glow.setPosition(position);
    glow.setFillColor(COLOR_PROJECTILE_TRAIL);

    window.draw(glow);
    window.draw(core);
}

sf::FloatRect Projectile::getBounds() const
{
    return sf::FloatRect(position.x - radius, position.y - radius,
                         radius * 2.f,        radius * 2.f);
}

// =============================================================================
// Enemy base
// =============================================================================

Enemy::Enemy(EnemyType t, sf::Vector2f pos, sf::Vector2f sz, int hp)
    : position(pos), velocity(0.f, 0.f), size(sz),
      health(hp), maxHealth(hp), alive(true), facingRight(true), type(t)
{}

sf::FloatRect Enemy::getBounds() const
{
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

void Enemy::takeDamage(int dmg)
{
    if (!alive) return;
    health -= dmg;
    if (health <= 0)
    {
        health = 0;
        alive  = false;
    }
}

void Enemy::drawHealthBar(sf::RenderWindow& window) const
{
    if (!alive) return;

    const float barW  = size.x;
    const float barH  = 5.f;
    const float barX  = position.x;
    const float barY  = position.y - 10.f;
    float       ratio = static_cast<float>(health) / static_cast<float>(maxHealth);

    sf::RectangleShape bg(sf::Vector2f(barW, barH));
    bg.setPosition(barX, barY);
    bg.setFillColor(COLOR_HP_EMPTY);
    window.draw(bg);

    if (ratio > 0.f)
    {
        sf::RectangleShape fg(sf::Vector2f(barW * ratio, barH));
        fg.setPosition(barX, barY);
        fg.setFillColor(COLOR_HP_FULL);
        window.draw(fg);
    }
}

// =============================================================================
// Turret
// =============================================================================

Turret::Turret(sf::Vector2f pos, std::vector<Projectile>* projectiles)
    : Enemy(EnemyType::TURRET, pos, sf::Vector2f(36.f, 32.f), 3)
    , shootCooldown(3.f)
    , shootTimer(1.5f)   // stagger initial shot slightly
    , range(400.f)
    , hasLineOfSight(false)
    , projectileList(projectiles)
    , barrelAngle(0.f)
{}

void Turret::update(float dt, sf::Vector2f playerPos,
                    std::vector<sf::FloatRect>& /*tiles*/)
{
    if (!alive) return;

    float dist = distance(position + size * 0.5f, playerPos);
    hasLineOfSight = (dist <= range);

    if (hasLineOfSight)
    {
        // Track angle toward player
        sf::Vector2f diff = playerPos - (position + size * 0.5f);
        barrelAngle = std::atan2(diff.y, diff.x) * 180.f / 3.14159f;

        shootTimer -= dt;
        if (shootTimer <= 0.f)
        {
            shoot(playerPos);
            shootTimer = shootCooldown;
        }
    }
}

void Turret::shoot(sf::Vector2f playerPos)
{
    if (!projectileList) return;

    sf::Vector2f origin = position + size * 0.5f;
    sf::Vector2f dir    = normalize(playerPos - origin);
    const float  speed  = 220.f;

    projectileList->emplace_back(origin, dir * speed);
}

void Turret::draw(sf::RenderWindow& window) const
{
    if (!alive) return;

    // --- Base body ---------------------------------------------------------
    sf::RectangleShape body(size);
    body.setPosition(position);
    body.setFillColor(COLOR_ENEMY_TURRET);
    body.setOutlineThickness(2.f);
    body.setOutlineColor(COLOR_ENEMY_TURRET_ACC);
    window.draw(body);

    // --- Cannon barrel (a rotated rectangle) -------------------------------
    sf::Vector2f center = position + size * 0.5f;

    sf::RectangleShape barrel(sf::Vector2f(20.f, 7.f));
    barrel.setOrigin(0.f, 3.5f);
    barrel.setPosition(center);
    barrel.setFillColor(COLOR_ENEMY_TURRET_ACC);
    barrel.setRotation(barrelAngle);
    window.draw(barrel);

    // --- Pivot circle ------------------------------------------------------
    sf::CircleShape pivot(6.f);
    pivot.setOrigin(6.f, 6.f);
    pivot.setPosition(center);
    pivot.setFillColor(sf::Color(80, 15, 15));
    pivot.setOutlineThickness(2.f);
    pivot.setOutlineColor(COLOR_ENEMY_TURRET_ACC);
    window.draw(pivot);

    // --- Alert indicator when targeting ------------------------------------
    if (hasLineOfSight)
    {
        sf::CircleShape alert(5.f);
        alert.setOrigin(5.f, 5.f);
        alert.setPosition(position.x + size.x * 0.5f, position.y - 14.f);
        alert.setFillColor(sf::Color(255, 80, 0, 200));
        window.draw(alert);
    }

    drawHealthBar(window);
}

// =============================================================================
// Knight
// =============================================================================

Knight::Knight(sf::Vector2f pos)
    : Enemy(EnemyType::KNIGHT, pos, sf::Vector2f(28.f, 44.f), 5)
    , patrolRange(150.f)
    , patrolStartX(pos.x)
    , chaseRange(200.f)
    , meleeRange(42.f)
    , meleeCooldown(1.0f)
    , meleeTimer(0.f)
    , moveSpeed(70.f)
    , chaseSpeed(120.f)
    , isChasing(false)
    , pendingMeleeDamage(false)
    , onGround(false)
    , animTimer(0.f)
{
    body.position = pos;
    body.size     = size;
    velocity.x    = moveSpeed;   // start moving right
}

void Knight::update(float dt, sf::Vector2f playerPos,
                    std::vector<sf::FloatRect>& tiles)
{
    if (!alive) return;

    animTimer += dt;

    float dist   = distance(position + size * 0.5f, playerPos);
    isChasing    = (dist < chaseRange);

    // Decrease melee cooldown
    if (meleeTimer > 0.f) meleeTimer -= dt;

    if (dist <= meleeRange)
    {
        // Melee attack
        velocity.x = 0.f;
        if (meleeTimer <= 0.f)
        {
            pendingMeleeDamage = true;
            meleeTimer         = meleeCooldown;
        }
    }
    else if (isChasing)
    {
        chase(dt, playerPos);
    }
    else
    {
        patrol(dt);
    }

    // Face in movement direction
    if (velocity.x > 0.f) facingRight = true;
    if (velocity.x < 0.f) facingRight = false;

    // Apply gravity
    body.velocity = velocity;
    applyGravity(body, dt);
    velocity.y = body.velocity.y;

    // Resolve against tiles
    body.velocity = velocity;
    CollisionResult cr = resolveCollision(body, tiles);
    position = body.position;
    onGround = body.onGround;

    if (cr.hit_left || cr.hit_right)
    {
        // Hit a wall while patrolling – reverse direction
        velocity.x = -velocity.x;
        patrolStartX = position.x;
    }
    if (cr.hit_top || cr.hit_bottom)
        velocity.y = body.velocity.y;
}

void Knight::patrol(float dt)
{
    // Oscillate within [patrolStartX - patrolRange, patrolStartX + patrolRange]
    float leftBound  = patrolStartX - patrolRange;
    float rightBound = patrolStartX + patrolRange;

    if (position.x <= leftBound)
    {
        velocity.x =  moveSpeed;
    }
    else if (position.x + size.x >= rightBound)
    {
        velocity.x = -moveSpeed;
    }
    // keep the same horizontal velocity otherwise (set in ctor)
    (void)dt;
}

void Knight::chase(float dt, sf::Vector2f playerPos)
{
    (void)dt;
    float dx = playerPos.x - (position.x + size.x * 0.5f);
    velocity.x = (dx > 0.f) ? chaseSpeed : -chaseSpeed;
}

void Knight::draw(sf::RenderWindow& window) const
{
    if (!alive) return;

    // Subtle walking bob driven by animTimer
    float bob = std::sin(animTimer * 8.f) * 1.5f;

    // --- Legs (two small rectangles) ----------------------------------------
    float legW  = 10.f, legH = 14.f;
    float legY  = position.y + 30.f + bob;

    sf::RectangleShape legL(sf::Vector2f(legW, legH));
    legL.setPosition(position.x + 2.f, legY);
    legL.setFillColor(COLOR_ENEMY_KNIGHT);
    window.draw(legL);

    sf::RectangleShape legR(sf::Vector2f(legW, legH));
    legR.setPosition(position.x + size.x - legW - 2.f, legY);
    legR.setFillColor(COLOR_ENEMY_KNIGHT);
    window.draw(legR);

    // --- Body ---------------------------------------------------------------
    sf::RectangleShape torso(sf::Vector2f(size.x, 24.f));
    torso.setPosition(position.x, position.y + 12.f + bob);
    torso.setFillColor(COLOR_ENEMY_KNIGHT);
    torso.setOutlineThickness(1.5f);
    torso.setOutlineColor(COLOR_ENEMY_KNIGHT_ACC);
    window.draw(torso);

    // --- Head ---------------------------------------------------------------
    float headR = 9.f;
    sf::CircleShape head(headR);
    head.setOrigin(headR, headR);
    head.setPosition(position.x + size.x * 0.5f, position.y + 8.f + bob);
    head.setFillColor(COLOR_PLAYER_HEAD);
    head.setOutlineThickness(1.5f);
    head.setOutlineColor(COLOR_ENEMY_KNIGHT_ACC);
    window.draw(head);

    // --- Sword (shown on facing side) ----------------------------------------
    float swordOffX = facingRight ? (size.x + 2.f) : (-18.f);
    sf::RectangleShape sword(sf::Vector2f(16.f, 4.f));
    sword.setPosition(position.x + swordOffX, position.y + 18.f + bob);
    sword.setFillColor(COLOR_SWORD);
    window.draw(sword);

    // --- Melee flash --------------------------------------------------------
    if (pendingMeleeDamage)
    {
        sf::RectangleShape flash(sf::Vector2f(40.f, 44.f));
        flash.setPosition(facingRight ? position.x + size.x : position.x - 40.f,
                          position.y);
        flash.setFillColor(sf::Color(255, 200, 50, 100));
        window.draw(flash);
    }

    drawHealthBar(window);
}
