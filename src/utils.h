#pragma once
// =============================================================================
// utils.h - Global constants, color palette, and utility function declarations
// =============================================================================

#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

// ---------------------------------------------------------------------------
// Window / display constants
// ---------------------------------------------------------------------------
static const int WINDOW_WIDTH  = 1280;
static const int WINDOW_HEIGHT = 720;
static const int TILE_SIZE     = 32;
static const int FPS           = 60;

// ---------------------------------------------------------------------------
// Dark / gothic colour palette
// ---------------------------------------------------------------------------
static const sf::Color COLOR_BACKGROUND       (10,  10,  15);
static const sf::Color COLOR_PLATFORM         (45,  45,  55);
static const sf::Color COLOR_PLATFORM_BORDER  (75,  75,  90);
static const sf::Color COLOR_MOVING_PLATFORM  (40,  70,  45);
static const sf::Color COLOR_MOVING_BORDER    (60, 100,  65);

static const sf::Color COLOR_PLAYER_BODY      (55,  85, 130);
static const sf::Color COLOR_PLAYER_HEAD      (195, 165, 130);
static const sf::Color COLOR_PLAYER_CLOAK     (35,  55,  90);
static const sf::Color COLOR_SWORD            (210, 210, 230);
static const sf::Color COLOR_SWORD_GLOW       (150, 200, 255, 160);

static const sf::Color COLOR_ENEMY_TURRET     (120,  25,  25);
static const sf::Color COLOR_ENEMY_TURRET_ACC (160,  40,  40);
static const sf::Color COLOR_ENEMY_KNIGHT     (25,   45, 110);
static const sf::Color COLOR_ENEMY_KNIGHT_ACC (45,   75, 170);

static const sf::Color COLOR_SPIKE            (170,  25,  25);
static const sf::Color COLOR_SPIKE_TIP        (220,  60,  60);

static const sf::Color COLOR_CHECKPOINT_OFF   (110,  90,  18);
static const sf::Color COLOR_CHECKPOINT_ON    (255, 215,   0);
static const sf::Color COLOR_CHECKPOINT_GLOW  (255, 240, 100, 80);

static const sf::Color COLOR_PROJECTILE       (220,  55,  55);
static const sf::Color COLOR_PROJECTILE_TRAIL (255, 130,  50, 120);

static const sf::Color COLOR_HP_FULL          (200,  45,  45);
static const sf::Color COLOR_HP_EMPTY         (55,   18,  18);
static const sf::Color COLOR_HP_BORDER        (140,  30,  30);

static const sf::Color COLOR_HUD_BG           (0,    0,   0, 170);
static const sf::Color COLOR_TEXT             (220, 210, 180);
static const sf::Color COLOR_TEXT_TITLE       (255, 215,   0);
static const sf::Color COLOR_TEXT_DIM         (130, 120, 100);
static const sf::Color COLOR_HURT_TINT        (255,  50,  50, 100);
static const sf::Color COLOR_DEATH_OVERLAY    (0,    0,   0, 180);

// ---------------------------------------------------------------------------
// Free utility functions
// ---------------------------------------------------------------------------
bool         rectIntersect(sf::FloatRect a, sf::FloatRect b);
float        clamp(float value, float minVal, float maxVal);
float        lerp(float a, float b, float t);
float        distance(sf::Vector2f a, sf::Vector2f b);
sf::Vector2f normalize(sf::Vector2f v);
sf::Color    lerpColor(sf::Color a, sf::Color b, float t);
