#pragma once

// Experimental post-processing switches (graphics-experimental, lane xp-post).
// Every switch is an environment variable, read once at the first post frame,
// off unless set to a non-zero value, so one executable can be compared with
// a switch on and off and the default frame stays the tip's:
//   - POSEIDON_XP_CAS: the TAA sharpen becomes a contrast-adaptive sharpen on
//     brightness only (psPostXpCas): low-contrast detail (far grass, terrain,
//     roofs) is sharpened up to twice the stock sharpen, high-contrast edges
//     (crowns against the sky) less. Its strength is the taaSharpen knob.
//   - POSEIDON_XP_CAS_FAR=<gain>: with POSEIDON_XP_CAS, the strength on the far
//     land (from 150 m, full from 1.5 km) times the gain (1 to 3).
//   - POSEIDON_XP_BLOOM=<tail>: the bloom chain's downsample becomes a 13-tap
//     filter at every level, with a Karis average on the first (one bright
//     texel cannot flare the glow, less flicker from glints), and two more
//     levels extend the glow's 1/r^2 tail (the value is the weight of those two
//     levels; 1 = the same weight as the others).
// (A low sun's glare, survey S7-4, is lane xp-atmosphere's POSEIDON_XP_SUNGLARE.)
// The values are clamped to sane ranges; a value that does not parse as a
// number counts as 1.

#include <cstdlib>

namespace Poseidon::gl33xp
{

struct PostXp
{
    bool cas = false;
    float casFar = 1.0f;    // the far land's multiple of the CAS strength
    float bloomTail = 0.0f; // 0 = the stock bloom chain
};

inline float XpEnvValue(const char* name, float lo, float hi)
{
    const char* v = std::getenv(name);
    if (!v || !v[0])
        return 0.0f;
    char* end = nullptr;
    const float f = std::strtof(v, &end);
    const float x = end == v ? 1.0f : f;
    if (!(x > 0.0f))
        return 0.0f;
    return x < lo ? lo : (x > hi ? hi : x);
}

inline const PostXp& PostXpSwitches()
{
    static const PostXp xp = []
    {
        PostXp s;
        s.cas = XpEnvValue("POSEIDON_XP_CAS", 0.0f, 1.0f) > 0.0f;
        const float far = XpEnvValue("POSEIDON_XP_CAS_FAR", 1.0f, 3.0f);
        s.casFar = far > 0.0f ? far : 1.0f;
        s.bloomTail = XpEnvValue("POSEIDON_XP_BLOOM", 0.05f, 2.0f);
        return s;
    }();
    return xp;
}

} // namespace Poseidon::gl33xp
