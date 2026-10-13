#pragma once

#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <algorithm>
#include <cmath>

namespace Poseidon
{
// Simulation-time state. A context/stance interruption consumes held input
// until release, so returning from a menu/vehicle never resurrects a lean.
struct LeanState
{
    // Upper-body roll at full lean (amount +-1), in radians: 35 degrees.
    static constexpr float MaxAngle = 35.0f * 3.14159265358979f / 180.0f;
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

// Rigid roll of one body part about the lean axis. `frame` holds the axis as
// its Direction() and a reference up/aside pair; `pivot` is a point on the
// axis. A positive angle tips the frame's up toward its aside (the soldier's
// right in model space). The result is a pure rotation about the pivot, so a
// transformed point keeps its distance to the pivot: the body tilts, it never
// shears or stretches.
inline Matrix4 LeanRollMatrix(Vector3Par pivot, Matrix3Par frame, float angle)
{
    const Matrix3 rotation = frame * Matrix3(MRotationZ, -angle) * frame.InverseRotation();
    Matrix4 result;
    result.SetOrientation(rotation);
    result.SetPosition(pivot - rotation * pivot);
    return result;
}

// The lean pose, built from three rigid moves so no body part stretches.
//
// The rifle stance is bladed: the torso is already hunched about 33 degrees
// toward the soldier's right in the plane across the bore, with the head out
// over the stock. Turning that whole torso 35 degrees about the waist folds it
// into a bow on the right and stands it upright on the left. Instead:
//  - the hips sway sideways: each leg turns rigidly about its own ankle, the
//    feet stay planted, and the pelvis and everything above it follow;
//  - the spine takes a small share of the roll about the lower spine;
//  - the head, shoulders, arms and weapon take the rest about the base of the
//    neck, so they end up rolled the full angle.
// Every move is a rotation about an axis parallel to the bore, or a
// translation, so the bore keeps its direction.
struct LeanRig
{
    // Share of the roll taken by the spine below the neck.
    static constexpr float SpineShare = 0.3f;
    // Sideways hip travel at full lean, metres.
    static constexpr float HipShift = 0.12f;
    // Aiming blend factors at or above this ride with the head and weapon.
    static constexpr float UpperFactor = 0.95f;

    Matrix3 frame = M3Identity; // Direction() is the roll axis (the bore)
    Vector3 pivot = VZero;      // lower spine
    Vector3 neck = VZero;       // base of the neck, the upper roll pivot
    Vector3 ankle[2] = {VZero, VZero};
    bool sway = false;     // both ankles known
    float lowerMax = 0.9f; // largest aiming factor below UpperFactor

    // Leg `side` (0 left, 1 right) turned about its ankle by the sway.
    Matrix4 Leg(float amount, int side) const
    {
        if (!sway || side < 0 || side > 1)
            return MIdentity;
        const Vector3 up = frame.DirectionUp();
        const float height = (pivot - ankle[side]) * up;
        if (height < 0.2f)
            return MIdentity;
        const float ratio = std::clamp(amount * HipShift / height, -0.5f, 0.5f);
        return LeanRollMatrix(ankle[side], frame, std::asin(ratio));
    }

    // How far the sway carries the pelvis: the mean of where the two legs
    // put the hips.
    Vector3 Shift(float amount) const
    {
        if (!sway)
            return VZero;
        return ((Leg(amount, 0).FastTransform(pivot) - pivot) + (Leg(amount, 1).FastTransform(pivot) - pivot)) * 0.5f;
    }

    // An upper-body bone with aiming blend factor `factor`.
    Matrix4 Spine(float amount, float factor) const
    {
        const float angle = amount * LeanState::MaxAngle;
        const float lower = angle * SpineShare;
        Matrix4 result = LeanRollMatrix(pivot, frame, lower * std::min(factor / lowerMax, 1.0f));
        if (factor >= UpperFactor)
        {
            const Vector3 bentNeck = result.FastTransform(neck);
            result = LeanRollMatrix(bentNeck, frame, angle - lower) * result;
        }
        Matrix4 shift = MIdentity;
        shift.SetPosition(Shift(amount));
        return shift * result;
    }
};
} // namespace Poseidon
