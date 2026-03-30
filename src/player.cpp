// =============================================================================
// player.cpp - Player implementation
// =============================================================================

#include "player.h"
#include <cmath>
#include <algorithm>

// =============================================================================
// Constructor
// =============================================================================

Player::Player()
    : health(3)
    , maxHealth(3)
    , alive(true)
    , invincible(false)
    , invincibleTimer(0.f)
    , invincibleDuration(1.5f)
    , jumpCount(0)
    , maxJumps(2)
    , attackTimer(0.f)
    , attackDuration(0.25f)
    , attackCooldown(0.35f)
    , attackCoolTimer(0.f)
    , attackActive(false)
    , attackBox()
    , knockbackTimer(0.f)
    , knockbackDuration(0.25f)
    , facingRight(true)
    , state(PlayerState::IDLE)
    , checkpointPos(100.f, 100.f)
    , animTimer(0.f)
    , moveSpeed(190.f)
    , groundAccel(800.f)
    , groundFriction(700.f)
    , airAccel(600.f)
    , airFriction(200.f)
    , killCount(0)
    , screenShakeTimer(0.f)
{
    body.size     = sf::Vector2f(28.f, 44.f);
    body.position = checkpointPos;
}

// =============================================================================
// update
// =============================================================================

void Player::update(float dt, InputManager& input)
{
    if (!alive) return;

    animTimer += dt;

    // Tick timers
    if (invincible)
    {
        invincibleTimer -= dt;
        if (invincibleTimer <= 0.f)
        {
            invincible      = false;
            invincibleTimer = 0.f;
        }
    }

    if (knockbackTimer > 0.f) knockbackTimer -= dt;
    if (attackCoolTimer > 0.f) attackCoolTimer -= dt;

    updateMovement(dt, input);
    updateAttack(dt, input);
    updateState();

    // Update animation sets
    switch (state)
    {
        case PlayerState::IDLE:      animations.idle.update(dt);   break;
        case PlayerState::WALKING:   animations.walk.update(dt);   break;
        case PlayerState::JUMPING:   animations.jump.update(dt);   break;
        case PlayerState::FALLING:   animations.fall.update(dt);   break;
        case PlayerState::ATTACKING: animations.attack.update(dt); break;
        case PlayerState::HURT:      animations.hurt.update(dt);   break;
        case PlayerState::DEAD:      animations.death.update(dt);  break;
    }
}

// =============================================================================
// updateMovement
// =============================================================================

void Player::updateMovement(float dt, InputManager& input)
{
    // Refresh jump-from-ground availability
    if (body.onGround)
        jumpCount = 0;

    bool moveLeft  = input.isKeyPressed(sf::Keyboard::Left)  ||
                     input.isKeyPressed(sf::Keyboard::A);
    bool moveRight = input.isKeyPressed(sf::Keyboard::Right) ||
                     input.isKeyPressed(sf::Keyboard::D);
    bool jumpKey   = input.isKeyJustPressed(sf::Keyboard::Z)     ||
                     input.isKeyJustPressed(sf::Keyboard::Space)  ||
                     input.isKeyJustPressed(sf::Keyboard::Up)     ||
                     input.isKeyJustPressed(sf::Keyboard::W);

    // Horizontal movement – don't override during heavy knockback
    if (knockbackTimer <= 0.f)
    {
        float accel    = body.onGround ? groundAccel : airAccel;
        float friction = body.onGround ? groundFriction : airFriction;

        if (moveLeft)
        {
            body.velocity.x -= accel * dt;
            if (body.velocity.x < -moveSpeed) body.velocity.x = -moveSpeed;
            facingRight = false;
        }
        else if (moveRight)
        {
            body.velocity.x += accel * dt;
            if (body.velocity.x > moveSpeed) body.velocity.x = moveSpeed;
            facingRight = true;
        }
        else
        {
            // Decelerate
            float fric = friction * dt;
            if (body.velocity.x > 0.f)
            {
                body.velocity.x -= fric;
                if (body.velocity.x < 0.f) body.velocity.x = 0.f;
            }
            else if (body.velocity.x < 0.f)
            {
                body.velocity.x += fric;
                if (body.velocity.x > 0.f) body.velocity.x = 0.f;
            }
        }
    }

    // Jump
    if (jumpKey) jump();

    // Apply gravity
    applyGravity(body, dt);

    // NOTE: tile collision is resolved externally by Level::update via
    // resolveWithTiles(), which moves body.position based on velocity.
}

// =============================================================================
// resolveWithTiles  (called by Level each frame after updateMovement)
// =============================================================================

void Player::resolveWithTiles(const std::vector<sf::FloatRect>& tiles)
{
    CollisionResult cr = resolveCollision(body, tiles);
    if (cr.hit_top) body.velocity.y = 0.f;
    // onGround is already set inside resolveCollision
}

// =============================================================================
// updateAttack
// =============================================================================

void Player::updateAttack(float dt, InputManager& input)
{
    bool attackKey = input.isKeyJustPressed(sf::Keyboard::X) ||
                     input.isKeyJustPressed(sf::Keyboard::F1);

    if (attackKey && attackCoolTimer <= 0.f && state != PlayerState::DEAD)
    {
        attack();
    }

    if (attackTimer > 0.f)
    {
        attackTimer -= dt;
        attackActive = true;

        // Build the attack hitbox: a rectangle forward of the player
        float boxW = 36.f, boxH = 20.f;
        float boxX = facingRight
                       ? body.position.x + body.size.x
                       : body.position.x - boxW;
        float boxY = body.position.y + body.size.y * 0.3f;
        attackBox  = sf::FloatRect(boxX, boxY, boxW, boxH);
    }
    else
    {
        attackTimer  = 0.f;
        attackActive = false;
    }
}

// =============================================================================
// updateState
// =============================================================================

void Player::updateState()
{
    if (!alive) { state = PlayerState::DEAD; return; }

    if (knockbackTimer > 0.f && invincible)
    {
        state = PlayerState::HURT;
        return;
    }

    if (attackActive)
    {
        state = PlayerState::ATTACKING;
        return;
    }

    if (!body.onGround)
    {
        state = (body.velocity.y < 0.f) ? PlayerState::JUMPING
                                        : PlayerState::FALLING;
        return;
    }

    if (std::abs(body.velocity.x) > 10.f)
        state = PlayerState::WALKING;
    else
        state = PlayerState::IDLE;
}

// =============================================================================
// jump
// =============================================================================

void Player::jump()
{
    if (jumpCount < maxJumps)
    {
        body.velocity.y = JUMP_FORCE;
        jumpCount++;
        // Reset double-jump animation
        if (jumpCount == 2)
            animations.jump.reset();
    }
}

// =============================================================================
// attack
// =============================================================================

void Player::attack()
{
    attackTimer    = attackDuration;
    attackCoolTimer = attackCooldown;
    attackActive   = true;
    animations.attack.reset();
}

// =============================================================================
// takeDamage
// =============================================================================

void Player::takeDamage(int dmg, sf::Vector2f knockbackDir)
{
    if (invincible || !alive) return;

    health -= dmg;
    if (health <= 0)
    {
        health = 0;
        alive  = false;
        state  = PlayerState::DEAD;
        animations.death.reset();
        return;
    }

    // Apply knockback impulse
    sf::Vector2f kbNorm = normalize(knockbackDir);
    body.velocity.x = kbNorm.x * 280.f;
    body.velocity.y = -250.f;

    invincible      = true;
    invincibleTimer = invincibleDuration;
    knockbackTimer  = knockbackDuration;
    screenShakeTimer = 0.25f;

    animations.hurt.reset();
    state = PlayerState::HURT;
}

// =============================================================================
// heal
// =============================================================================

void Player::heal(int amount)
{
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

// =============================================================================
// respawn
// =============================================================================

void Player::respawn()
{
    body.position  = checkpointPos;
    body.velocity  = sf::Vector2f(0.f, 0.f);
    health         = maxHealth;
    alive          = true;
    invincible     = false;
    invincibleTimer = 0.f;
    knockbackTimer  = 0.f;
    attackTimer     = 0.f;
    attackActive    = false;
    jumpCount       = 0;
    state           = PlayerState::IDLE;
    animations.idle.reset();
}

// =============================================================================
// setCheckpoint
// =============================================================================

void Player::setCheckpoint(sf::Vector2f pos)
{
    checkpointPos = pos;
}

// =============================================================================
// Accessors
// =============================================================================

sf::FloatRect Player::getBounds() const
{
    return body.getBounds();
}

sf::FloatRect Player::getAttackBounds() const
{
    return attackBox;
}

bool Player::isAttacking() const
{
    return attackActive;
}

// =============================================================================
// draw
// =============================================================================

void Player::draw(sf::RenderWindow& window) const
{
    drawBody(window);
    if (attackActive) drawSword(window);
}

// ---------------------------------------------------------------------------
// drawBody
// ---------------------------------------------------------------------------
void Player::drawBody(sf::RenderWindow& window) const
{
    // Invincibility blink: skip drawing on every other 0.1-second interval
    if (invincible)
    {
        int blinkPhase = static_cast<int>(invincibleTimer / 0.1f);
        if (blinkPhase % 2 == 0) return;
    }

    float bob = (state == PlayerState::WALKING)
              ? std::sin(animTimer * 10.f) * 2.f
              : 0.f;

    sf::Vector2f pos = body.position;
    float        w   = body.size.x;
    float        h   = body.size.y;

    // --- Cloak / lower body ------------------------------------------------
    sf::RectangleShape cloak(sf::Vector2f(w, h * 0.55f));
    cloak.setPosition(pos.x, pos.y + h * 0.45f + bob);
    cloak.setFillColor(COLOR_PLAYER_CLOAK);
    window.draw(cloak);

    // --- Torso -------------------------------------------------------------
    sf::RectangleShape torso(sf::Vector2f(w * 0.85f, h * 0.45f));
    torso.setPosition(pos.x + w * 0.075f, pos.y + h * 0.15f + bob);
    torso.setFillColor(COLOR_PLAYER_BODY);
    torso.setOutlineThickness(1.5f);
    torso.setOutlineColor(sf::Color(90, 120, 180));
    window.draw(torso);

    // --- Head --------------------------------------------------------------
    float headR = 9.f;
    sf::CircleShape head(headR);
    head.setOrigin(headR, headR);
    head.setPosition(pos.x + w * 0.5f, pos.y + headR + 1.f + bob);
    head.setFillColor(COLOR_PLAYER_HEAD);

    // Hurt tint
    if (invincible)
        head.setFillColor(lerpColor(COLOR_PLAYER_HEAD, sf::Color(255, 80, 80), 0.5f));

    window.draw(head);

    // --- Eyes --------------------------------------------------------------
    float eyeOffX = facingRight ? 3.f : -3.f;
    sf::CircleShape eye(2.f);
    eye.setOrigin(2.f, 2.f);
    eye.setPosition(pos.x + w * 0.5f + eyeOffX, pos.y + headR - 1.f + bob);
    eye.setFillColor(sf::Color(220, 40, 40));
    window.draw(eye);

    // --- Health bar above head ---------------------------------------------
    drawHealthBar(window);

    // --- Dead overlay ------------------------------------------------------
    if (state == PlayerState::DEAD)
    {
        sf::RectangleShape deadOverlay(sf::Vector2f(w, h));
        deadOverlay.setPosition(pos);
        deadOverlay.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(deadOverlay);
    }
}

// ---------------------------------------------------------------------------
// drawSword
// ---------------------------------------------------------------------------
void Player::drawSword(sf::RenderWindow& window) const
{
    float swordLen = 36.f;
    float swordH   = 5.f;
    float offY     = body.position.y + body.size.y * 0.35f;

    // Glow behind the blade
    sf::RectangleShape glow(sf::Vector2f(swordLen + 8.f, swordH + 8.f));
    glow.setOrigin(0.f, (swordH + 8.f) * 0.5f);
    glow.setPosition(facingRight ? body.position.x + body.size.x - 4.f
                                 : body.position.x - swordLen + 4.f,
                     offY);
    glow.setFillColor(COLOR_SWORD_GLOW);
    window.draw(glow);

    // Blade
    sf::RectangleShape blade(sf::Vector2f(swordLen, swordH));
    blade.setOrigin(0.f, swordH * 0.5f);
    blade.setPosition(facingRight ? body.position.x + body.size.x
                                  : body.position.x - swordLen,
                      offY);
    blade.setFillColor(COLOR_SWORD);
    blade.setOutlineThickness(1.f);
    blade.setOutlineColor(sf::Color(180, 180, 200));
    window.draw(blade);

    // Cross-guard
    sf::RectangleShape guard(sf::Vector2f(5.f, 16.f));
    guard.setOrigin(2.5f, 8.f);
    guard.setPosition(facingRight ? body.position.x + body.size.x
                                  : body.position.x,
                      offY);
    guard.setFillColor(sf::Color(160, 140, 100));
    window.draw(guard);
}

// ---------------------------------------------------------------------------
// drawHealthBar
// ---------------------------------------------------------------------------
void Player::drawHealthBar(sf::RenderWindow& window) const
{
    const float barW    = body.size.x;
    const float barH    = 5.f;
    float       barX    = body.position.x;
    float       barY    = body.position.y - 12.f;
    float       ratio   = static_cast<float>(health) /
                          static_cast<float>(maxHealth);

    sf::RectangleShape bg(sf::Vector2f(barW, barH));
    bg.setPosition(barX, barY);
    bg.setFillColor(COLOR_HP_EMPTY);
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(COLOR_HP_BORDER);
    window.draw(bg);

    if (ratio > 0.f)
    {
        sf::RectangleShape fg(sf::Vector2f(barW * ratio, barH));
        fg.setPosition(barX, barY);
        fg.setFillColor(COLOR_HP_FULL);
        window.draw(fg);
    }
}
