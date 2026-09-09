#pragma once

// Deterministic rolls for the Legend registry and the history generator.
// splitmix64 over a persisted campaign seed: RandomGenerator-independent, so a
// value never depends on how many other systems drew from GRandGen this tick,
// and identical on every compiler/platform (no size_t, no pointer, no std::hash,
// no locale, no float).  One draw KIND per channel constant; never reuse a
// channel, that is what keeps a row's rolls from moving when another row appears.
//
// Header-only on purpose: a .cpp placed directly under Game/Guerrilla/ has to be
// classified in Showcase.Abel/human/coverage.json or the human-suite contract
// test goes red, and none of this needs a translation unit.  It also lets the
// Legend name tables and the history generator share these three functions
// without either one depending on the other.

namespace Poseidon
{
namespace Guerrilla
{

// One step of splitmix64: advances the state and returns the mixed output.
inline unsigned long long SplitMix64(unsigned long long& s)
{
    s += 0x9E3779B97F4A7C15ull;
    unsigned long long z = s;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// FNV-1a 64 over the key string, salted with the campaign seed and mixed once.
// A null or empty string is legal and hashes to the salt's mix.
inline unsigned long long HashKey(const char* s, unsigned long long seed)
{
    unsigned long long h = 0xCBF29CE484222325ull ^ (seed * 0x100000001B3ull);
    for (const unsigned char* p = (const unsigned char*)s; p && *p; p++)
    {
        h ^= (unsigned long long)*p;
        h *= 0x100000001B3ull;
    }
    unsigned long long st = h;
    return SplitMix64(st);
}

// One draw off (key, channel) in [0, mod); 0 when mod <= 0.
inline unsigned Roll(unsigned long long key, unsigned channel, int mod)
{
    if (mod <= 0)
    {
        return 0;
    }
    unsigned long long st = key + (unsigned long long)channel * 0x9E3779B97F4A7C15ull;
    return (unsigned)(SplitMix64(st) % (unsigned long long)mod);
}

// One constant per DRAW KIND.  Values are frozen: a campaign's prose and names
// are persisted as resolved text, but a not-yet-created row draws off these, so
// moving a number moves the identities a running campaign has not met yet.
enum LegendChannel
{
    CH_OPEN_A = 1,
    CH_OPEN_B = 2,
    CH_OPEN_C = 3,
    CH_OPEN_D = 4,
    CH_EVENT0 = 10,
    CH_EVENT1 = 11,
    CH_EVENT2 = 12,
    CH_PLACE0 = 20,
    CH_PLACE1 = 21,
    CH_PLACE2 = 22,
    CH_LAST = 30,
    CH_FACE = 31,
    CH_BIOEVENT = 32,
    CH_BIOVAR = 33,
    // The award draws are FOUR distinct kinds, not two: which slot is filled and
    // which word fills it are separate rolls for each of the two awards.  Writing
    // them as one channel per award (the first draft's CH_AWARD1|CH_AWARD2) would
    // both collide and, spelled as C++, evaluate to a single channel.
    CH_AWARD1 = 40,
    CH_AWARD2 = 41,
    CH_AWARD1WORD = 42,
    CH_AWARD2WORD = 43,
    CH_BOSSFIRST = 50,
    CH_BOSSLAST = 51,
    CH_BOSSSLOT = 52,
    CH_BOSSWORD = 53
};

} // namespace Guerrilla
} // namespace Poseidon
