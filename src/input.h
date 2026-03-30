#pragma once
// =============================================================================
// input.h - Per-frame keyboard state management
// =============================================================================

#include <SFML/Window.hpp>
#include <unordered_map>

// ---------------------------------------------------------------------------
// InputManager
//
// Call update() once per frame AFTER processing sf::Event so that the
// "just pressed / just released" edge-detection is correct.
// ---------------------------------------------------------------------------
class InputManager
{
public:
    // Must be called once per game-loop iteration
    void update();

    // True while the key is held down this frame
    bool isKeyPressed(sf::Keyboard::Key key) const;

    // True only on the first frame the key transitions down
    bool isKeyJustPressed(sf::Keyboard::Key key) const;

    // True only on the first frame the key transitions up
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

private:
    std::unordered_map<int, bool> m_current;   // state this frame
    std::unordered_map<int, bool> m_previous;  // state last frame

    bool getState(const std::unordered_map<int, bool>& map,
                  sf::Keyboard::Key key) const;
};
