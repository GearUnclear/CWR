#pragma once

#include <algorithm>
#include <cmath>

namespace Poseidon
{
// Simulation-time state. A context/stance interruption consumes held input
// until release, so returning from a menu/vehicle never resurrects a lean.
struct LeanState
{
    static constexpr float Distance = 0.28f;
    static constexpr float Rate = 5.0f;
    float amount = 0;
    bool requireRelease = false;

    void Reset()
    {
        amount = 0;
        requireRelease = true;
    }

    float Advance(float left, float right, bool eligible, float dt)
    {
        if (!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(dt) || dt < 0)
        {
            Reset();
            return amount;
        }
        // Use the same dead zone for release detection and the target pose.
        // Otherwise a noisy controller axis can lean while counting as released.
        left = left > 0.1f ? left : 0.0f;
        right = right > 0.1f ? right : 0.0f;
        const bool held = left != 0 || right != 0;
        if (!eligible)
        {
            Reset();
            // Release can occur while a menu or unsupported stance is active.
            requireRelease = held;
            return amount;
        }
        if (!held)
            requireRelease = false;
        const float target = requireRelease ? 0.0f : std::clamp(right - left, -1.0f, 1.0f);
        amount += std::clamp(target - amount, -Rate * dt, Rate * dt);
        if (std::fabs(amount) < 1e-6f)
            amount = 0;
        return amount;
    }
};
} // namespace Poseidon
