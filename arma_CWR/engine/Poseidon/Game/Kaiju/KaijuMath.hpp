#pragma once

// Pure math for the giant-creature support (Game/Kaiju). No engine types, so
// the unit tests can pin every curve without a world.

#include <cmath>

namespace Poseidon::Kaiju
{

inline float Clamp01(float t)
{
    return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
}

inline float Smooth01(float t)
{
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// ---------------------------------------------------------------- far draw

// Part of the window depth range left to the far pass. The main scene keeps
// [0, kFarDepthSplit], a far-drawn object gets [kFarDepthSplit, 1], so it is
// always behind everything the main scene writes and keeps full precision
// inside its own slice.
constexpr float kFarDepthSplit = 0.95f;

// Lowest fog visibility (1 = unfogged, 0 = fog colour) a far-drawn object is
// allowed to reach at a distance. Inside the fog range the stock fog is
// usually above the floor and nothing changes; past it the floor is what is
// left of the object: a silhouette that thins toward farDist and is gone
// there, so nothing pops at the limit.
inline float FarFogFloor(float dist, float fogEnd, float farDist, float contrast)
{
    if (farDist <= 0.0f || contrast <= 0.0f || dist >= farDist)
    {
        return 0.0f;
    }
    contrast = Clamp01(contrast);
    // aerial perspective: the silhouette loses a little over half its
    // contrast between the fog end and the far limit
    float span = farDist - fogEnd;
    float t = span > 1.0f ? Clamp01((dist - fogEnd) / span) : 1.0f;
    float haze = 1.0f - 0.55f * t;
    // fade out over the last 15 percent
    float fade = 1.0f - Smooth01((dist - 0.85f * farDist) / (0.15f * farDist));
    return contrast * haze * fade;
}

// The fog floor at night. By day a far-drawn unit is a silhouette against a
// bright sky and loses contrast with distance; at night its own light is what
// carries (a fire shows from further off than a hill does), so the floor is
// raised toward nightFloor in step with the night (night: 0 day .. 1 night).
// The fade over the last 15 percent of the far limit is kept, so nothing pops.
inline float FarNightFloor(float dayFloor, float dist, float farDist, float nightFloor, float night)
{
    if (nightFloor <= 0.0f || farDist <= 0.0f || dist >= farDist)
    {
        return dayFloor;
    }
    night = Clamp01(night);
    float fade = 1.0f - Smooth01((dist - 0.85f * farDist) / (0.15f * farDist));
    float lit = Clamp01(nightFloor) * fade;
    return lit > dayFloor ? dayFloor + (lit - dayFloor) * night : dayFloor;
}

// True when the object reaches the camera's far plane and has to go through
// the far pass (own projection) instead of the normal object pass.
inline bool FarNeedsOwnPass(float planarZ, float radius, float clipFar)
{
    return planarZ + radius > clipFar * 0.98f;
}

struct ClipBracket
{
    float zNear;
    float zFar;
};

// Near and far planes that just hold the object, for the far pass.
inline ClipBracket FarClipBracket(float planarZ, float radius)
{
    ClipBracket b;
    float pad = radius * 1.5f + 5.0f;
    b.zNear = planarZ - pad;
    if (b.zNear < 5.0f)
    {
        b.zNear = 5.0f;
    }
    b.zFar = planarZ + pad;
    if (b.zFar < b.zNear + 10.0f)
    {
        b.zFar = b.zNear + 10.0f;
    }
    return b;
}

struct HazeBand
{
    float z;    // planar distance of the band from the camera
    float midY; // height of the top of the solid part, relative to the camera
    float topY; // height where the band has thinned to nothing
};

// Ground haze in front of a unit drawn by the far pass. Past the view distance
// the terrain under it is not drawn, so without this it would stand in the
// air above the horizon. The band is the horizon band again, nearer and
// lower: solid to his shins, thinning to nothing a little over half the body,
// so the unit rises out of the haze. It grows from nothing at the far plane
// to full size a tenth of a view distance further out: the land under him
// stops being drawn at the far plane, so the band has to be there almost at
// once or his feet hang above the last drawn ridge (seen by day from 2.3 km
// at a view distance of 2 km, when the band grew over a third of a view
// distance). Nothing pops where the far pass takes over: the band starts at
// nothing. feetRelY is the height of the feet relative to the camera.
inline HazeBand FarHazeBand(float planarZ, float radius, float feetRelY, float height, float dist, float clipFar)
{
    HazeBand b;
    b.z = planarZ - radius;
    if (b.z < 10.0f)
    {
        b.z = 10.0f;
    }
    const float scale = planarZ > 1.0f ? b.z / planarZ : 1.0f;
    const float rise = clipFar > 1.0f ? Smooth01((dist - clipFar) / (0.10f * clipFar)) : 1.0f;
    b.midY = (feetRelY + height * (0.17f * rise - 0.05f)) * scale;
    b.topY = (feetRelY + height * 0.55f * rise) * scale;
    return b;
}

// ------------------------------------------------------------------- glow

// The glow trait (0..2) split into the emissive scale of the shining faces
// (0 dark .. 1 the painted colour) and the white-hot share added on top of the
// texture above 1 (0..1).
inline void GlowSplit(float glow, float& scale, float& heat)
{
    if (glow < 0.0f)
    {
        glow = 0.0f;
    }
    scale = glow < 1.0f ? glow : 1.0f;
    heat = Clamp01(glow - 1.0f);
}

// ------------------------------------------------------------ planted blend

// Share of a walk's step the body takes in one frame while the walk blends in
// from a stand, when the standing foot is to stay planted. The pose is
// (1 - f) * stand + f * walk(t), and in the walk clip the planted foot recedes
// from the body in step with t, so the foot stays where it is only when the
// body has advanced by step * t * f. The stock rule advances it by
// step * (t1 - t0) * f0 a frame, which leaves the foot dragged back by
// step * integral(t df). t0, t1: clip phase before and after the frame;
// f0, f1: blend factor before and after.
inline float PlantedBlendShare(float t0, float t1, float f0, float f1)
{
    return t1 * f1 - t0 * f0;
}

// ------------------------------------------------------------ camera shake

// How hard one tremor is felt: strength at the source out to fullRadius,
// falling to nothing at zeroRadius, with a very short attack and a decay that
// is gone when the duration is over.
inline float TremorGain(float strength, float dist, float fullRadius, float zeroRadius, float age, float duration)
{
    if (strength <= 0.0f || duration <= 0.0f || age < 0.0f || age >= duration)
    {
        return 0.0f;
    }
    float space = 1.0f;
    if (dist > fullRadius)
    {
        if (zeroRadius <= fullRadius || dist >= zeroRadius)
        {
            return 0.0f;
        }
        float t = (dist - fullRadius) / (zeroRadius - fullRadius);
        space = (1.0f - t) * (1.0f - t);
    }
    const float attack = 0.03f;
    float rise = age < attack ? age / attack : 1.0f;
    float t = age / duration;
    float decay = (1.0f - t) * (1.0f - t);
    return strength * space * rise * decay;
}

struct ShakeOffset
{
    float right = 0, up = 0, forward = 0; // metres, camera space
    float pitch = 0, yaw = 0, roll = 0;   // radians
};

// The largest total gain the camera takes; several tremors add up to this.
constexpr float kShakeMaxGain = 3.0f;

// Camera offset for a total gain at time t (seconds). A sum of sines at
// unrelated rates, so it never visibly repeats and needs no state.
inline ShakeOffset ShakeAt(float t, float gain, float freq = 7.0f)
{
    ShakeOffset o;
    if (gain <= 0.0f)
    {
        return o;
    }
    if (gain > kShakeMaxGain)
    {
        gain = kShakeMaxGain;
    }
    const float w = 6.2831853f * freq * t;
    auto wave = [w](float a, float b, float pa, float pb)
    { return 0.62f * std::sin(w * a + pa) + 0.38f * std::sin(w * b + pb); };
    o.up = 0.035f * gain * wave(1.00f, 2.37f, 0.0f, 1.3f);
    o.right = 0.020f * gain * wave(0.83f, 1.91f, 2.1f, 0.4f);
    o.forward = 0.010f * gain * wave(1.21f, 2.83f, 4.0f, 2.2f);
    o.pitch = 0.0110f * gain * wave(0.91f, 2.11f, 0.7f, 3.1f);
    o.roll = 0.0085f * gain * wave(1.13f, 1.79f, 5.2f, 1.9f);
    o.yaw = 0.0045f * gain * wave(0.77f, 2.53f, 3.3f, 0.2f);
    return o;
}

// ------------------------------------------------------------------ steer

// Signed turn (radians, -pi..pi) from a heading to a wanted heading, both
// measured clockwise from north as atan2(x, z).
inline float HeadingError(float heading, float wanted)
{
    const float pi = 3.14159265f;
    float d = std::fmod(wanted - heading, 2.0f * pi);
    if (d > pi)
    {
        d -= 2.0f * pi;
    }
    if (d < -pi)
    {
        d += 2.0f * pi;
    }
    return d;
}

// ---------------------------------------------------------------- damage

// Total damage after a hit when a ceiling is set (ceiling < 0 = none).
inline float ClampDamage(float total, float ceiling)
{
    if (ceiling < 0.0f)
    {
        return total;
    }
    return total > ceiling ? ceiling : total;
}

} // namespace Poseidon::Kaiju
