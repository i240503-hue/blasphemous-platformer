#include <iostream>
#include <vector>

struct AABB {
    float x, y; // Position
    float width, height; // Size
};

// Function to check for AABB collisions
bool checkCollision(const AABB& a, const AABB& b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

// Function to check if two entities overlap based on AABB
bool entitiesOverlap(const AABB& entity1, const AABB& entity2) {
    return checkCollision(entity1, entity2);
}

// Function to simulate physics (placeholder for future implementation)
void simulatePhysics(std::vector<AABB>& entities) {
    for (auto& entity : entities) {
        // Placeholder for physics simulation logic
        // Update entity position based on velocity, gravity, etc.
    }
}

int main() {
    // Example usage
    AABB player = {10, 10, 50, 50};
    AABB enemy = {30, 30, 50, 50};

    if (checkCollision(player, enemy)) {
        std::cout << "Collision detected!" << std::endl;
    } else {
        std::cout << "No collision." << std::endl;
    }

    return 0;
}
