// =============================================================================
// input.cpp - InputManager implementation
// =============================================================================

#include "input.h"

// Poll every key we care about each frame.
// We only track the keys actually used by the game so the loop stays fast.
static const sf::Keyboard::Key TRACKED_KEYS[] = {
    sf::Keyboard::Left,
    sf::Keyboard::Right,
    sf::Keyboard::Up,
    sf::Keyboard::Down,
    sf::Keyboard::Z,        // Jump
    sf::Keyboard::X,        // Attack
    sf::Keyboard::Space,    // Jump (alternative)
    sf::Keyboard::A,        // Move left  (alternative)
    sf::Keyboard::D,        // Move right (alternative)
    sf::Keyboard::W,        // Jump       (alternative)
    sf::Keyboard::Enter,
    sf::Keyboard::Escape,
    sf::Keyboard::Return,
    sf::Keyboard::BackSpace,
    sf::Keyboard::F1,
    sf::Keyboard::F2,
    sf::Keyboard::F3,
    sf::Keyboard::Num1,
    sf::Keyboard::Num2,
    sf::Keyboard::Num3,
    sf::Keyboard::P,        // Pause
    sf::Keyboard::R,        // Restart
};

void InputManager::update()
{
    // Carry current frame's state into previous
    m_previous = m_current;

    // Poll the fresh state for every tracked key
    for (sf::Keyboard::Key k : TRACKED_KEYS)
    {
        m_current[static_cast<int>(k)] = sf::Keyboard::isKeyPressed(k);
    }
}

bool InputManager::getState(const std::unordered_map<int, bool>& map,
                             sf::Keyboard::Key key) const
{
    auto it = map.find(static_cast<int>(key));
    return (it != map.end()) && it->second;
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const
{
    return getState(m_current, key);
}

bool InputManager::isKeyJustPressed(sf::Keyboard::Key key) const
{
    // Currently down AND was NOT down last frame
    return getState(m_current, key) && !getState(m_previous, key);
}

bool InputManager::isKeyJustReleased(sf::Keyboard::Key key) const
{
    // Currently up AND was down last frame
    return !getState(m_current, key) && getState(m_previous, key);
}
