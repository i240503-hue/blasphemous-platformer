// =============================================================================
// animation.cpp - Animation implementation
// =============================================================================

#include "animation.h"

// ---------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------

Animation::Animation()
    : frameCount(1), currentFrame(0), frameDuration(0.1f),
      timer(0.f), loop(true), finished(false)
{}

Animation::Animation(int frameCount, float frameDuration, bool loop)
    : frameCount(frameCount), currentFrame(0), frameDuration(frameDuration),
      timer(0.f), loop(loop), finished(false)
{}

void Animation::update(float dt)
{
    if (finished) return;

    timer += dt;

    // Advance frames while the accumulated time exceeds one frame's duration
    while (timer >= frameDuration)
    {
        timer -= frameDuration;
        currentFrame++;

        if (currentFrame >= frameCount)
        {
            if (loop)
            {
                currentFrame = 0;   // wrap around
            }
            else
            {
                currentFrame = frameCount - 1;  // hold last frame
                finished     = true;
                timer        = 0.f;
                break;
            }
        }
    }
}

int Animation::getCurrentFrame() const
{
    return currentFrame;
}

void Animation::reset()
{
    currentFrame = 0;
    timer        = 0.f;
    finished     = false;
}

bool Animation::isFinished() const
{
    return finished;
}

float Animation::getProgress() const
{
    if (frameCount <= 1) return 1.f;
    return static_cast<float>(currentFrame) / static_cast<float>(frameCount - 1);
}

// ---------------------------------------------------------------------------
// AnimationSet - sensible defaults for a humanoid character
// ---------------------------------------------------------------------------

AnimationSet::AnimationSet()
    : idle  (4,  0.18f, true )   // 4 frames, slow breathing bob
    , walk  (6,  0.10f, true )   // 6 frames, walking cycle
    , attack(4,  0.07f, false)   // 4 frames, snappy attack; no loop
    , jump  (2,  0.12f, false)   // 2 frames, ascending; no loop
    , fall  (2,  0.12f, true )   // 2 frames, falling cycle
    , hurt  (3,  0.08f, false)   // 3 frames, stagger; no loop
    , death (6,  0.12f, false)   // 6 frames, death collapse; no loop
{}
