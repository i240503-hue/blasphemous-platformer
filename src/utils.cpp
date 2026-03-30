// =============================================================================
// utils.cpp - Implementations of utility functions
// =============================================================================

#include "utils.h"

// Returns true if two rectangles overlap (thin wrapper around SFML's method)
bool rectIntersect(sf::FloatRect a, sf::FloatRect b)
{
    return a.intersects(b);
}

// Clamps value into [minVal, maxVal]
float clamp(float value, float minVal, float maxVal)
{
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// Linear interpolation between a and b by factor t in [0,1]
float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// Euclidean distance between two 2-D points
float distance(sf::Vector2f a, sf::Vector2f b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Normalise a 2-D vector (returns zero vector for zero-length input)
sf::Vector2f normalize(sf::Vector2f v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len < 0.0001f) return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(v.x / len, v.y / len);
}

// Per-channel linear interpolation between two SFML colours
sf::Color lerpColor(sf::Color a, sf::Color b, float t)
{
    return sf::Color(
        static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
        static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
        static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
        static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
    );
}
