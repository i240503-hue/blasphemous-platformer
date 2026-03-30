#pragma once
// =============================================================================
// animation.h - Frame-counter based sprite animation system
//
// Since we render entirely with shapes (no texture atlas), the Animation
// class is used to drive logical state changes (which body-part offsets or
// colours to use when drawing with primitives).
// =============================================================================

#include <string>

// ---------------------------------------------------------------------------
// Animation - drives a single repeating or one-shot frame sequence
// ---------------------------------------------------------------------------
class Animation
{
public:
    int   frameCount;       // total number of frames
    int   currentFrame;     // 0-based index of the frame currently shown
    float frameDuration;    // seconds each frame is displayed
    float timer;            // accumulated time since last frame change
    bool  loop;             // whether to restart after the last frame
    bool  finished;         // true once a non-looping anim reaches its end

    // Default ctor – 1 frame, instant (useful as a placeholder)
    Animation();

    // Explicit ctor
    Animation(int frameCount, float frameDuration, bool loop = true);

    // Advance the animation by dt seconds; handles frame advancement
    void update(float dt);

    // Return current frame index
    int  getCurrentFrame() const;

    // Rewind to frame 0
    void reset();

    // Has the animation played through at least once?
    bool isFinished() const;

    // Return normalised playback position in [0, 1]
    float getProgress() const;
};

// ---------------------------------------------------------------------------
// AnimationSet - named collection of animations for one entity
// ---------------------------------------------------------------------------
struct AnimationSet
{
    Animation idle;
    Animation walk;
    Animation attack;
    Animation jump;
    Animation fall;
    Animation hurt;
    Animation death;

    AnimationSet();
};
