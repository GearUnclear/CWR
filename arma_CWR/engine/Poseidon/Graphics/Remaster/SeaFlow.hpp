#pragma once

// Perf squad lane EX5-SEA, rounds 2 and 3: a sea that neither loops nor flows.
// On by default since 2026-11-13; POSEIDON_XP_SEAFLOW=0 gives the old sea.
//
// Today the sea's waves are the brightness of the sea texture's animation
// (more_anim.01 to .07), and the landscape plays that animation as a ping-pong:
// its phase runs 0 -> 1 -> 0 every 2 s (Landscape::DrawWater: fmod(time, 2),
// mirrored). The texture sits still on the world, so every wave layer the
// water program derives from it (the texture, the swell copy, and the glints
// on both) morphs forward for one second and then plays the same second
// backwards, for ever: a movement on a loop, with nothing that travels.
//
// Round 2 stopped the ping-pong and moved every layer over the world at 1.6 to
// 2.6 m/s on headings within 50 degrees of each other. The owner: "Sea looks
// like a river now". Measured on the round 2 clips, every 64 px block of the
// near bay moved the same way (coherence 0.998, 96 px/s to the right). Two
// reasons: the headings ran along each layer's own crests (the texture's
// streaks run at 15 degrees in its coordinates; the layers moved at 20, -12
// and 38 degrees, their streaks at 15, -22 and 31 after their maps), so the
// streaks slid lengthwise, as the lines on a current do; and the fine detail
// the eye follows is one layer's (the texture's own), so whatever way that
// layer goes, the whole sea seems to go.
//
// Round 3 (the default field, "sea"):
//   - every layer moves ACROSS its own crests, as waves do;
//   - every size comes as two equal trains that travel opposite ways: the
//     texture's layer and a twin of it (0.88 x its scale, turned 8 degrees,
//     so the two never line up), and the swell copy (3.7 times the size) and
//     a partner of it made the same way. Two equal wave trains running
//     against each other make crests that rise and fade where they stand and
//     drift nowhere. (The first cut gave only the fine layer a twin; its
//     swell and its ripples went one way each, and the near bay kept a slow
//     one-way flow toward the camera.)
//   - speeds follow the waves the texture draws: its slope detail has a
//     median wavelength of 0.28 m (more_anim.01 to .07), whose groups travel
//     at about 0.33 m/s in deep water; the texture's layer goes 0.3 m/s, its
//     twin 0.32, the swell pair 0.58 and 0.62 (speed grows as the square root
//     of the size). The first cut went twice as fast (0.6, crest speed) and
//     its near water changed shape twice as fast as today's: busy, not calm.
//     Round 2 went 1.6 to 2.6 m/s;
//   - the ripples (specular_dx, a few centimetres) stay where they are, as
//     today's do (round 2 and the first cut slid them one way);
//   - no dissolve by default: the 7 frames' one-way dissolve (an in-place
//     cycle walked forward by golden-ratio steps that never repeat) costs
//     0.13 to 0.32 ms of GPU and is kept as POSEIDON_XP_SEAFLOW_MORPH=0.8.
// Round 2's field is kept for comparison (POSEIDON_XP_SEAFLOW_FIELD=river).
//
// With the switch on the drawn sea also rises and falls less. The landscape
// raises and lowers the whole sea by 0.25 m on a sine (Landscape::Simulate;
// its period is the sea sound's length, about 8.5 s). Seen from near the water
// that slides the whole wave pattern toward the viewer and back again. With
// the switch on the picture keeps POSEIDON_XP_SEAFLOW_HEAVE of it (default
// 0.25: 6 cm, still enough for the waterline to lap). Only the picture: the
// simulated sea level (boats, wading, collisions, the network) is today's,
// always (Landscape::GetSeaLevelDrawn).
// Off (POSEIDON_XP_SEAFLOW=0) the landscape, the water program and its
// constants are exactly as they were.
//
//   POSEIDON_XP_SEAFLOW        0: off; unset or a number above 0: on, and the
//                              scale of the layers' speeds (unset = 1)
//   POSEIDON_XP_SEAFLOW_FIELD  unset or "sea": round 3; "river": round 2's
//                              layers, headings and speeds (a comparison,
//                              never a look)
//   POSEIDON_XP_SEAFLOW_MORPH  seconds per dissolve step; "0" none; unset: the
//                              field's default (none in both; 0.8 is the
//                              round 3 first cut's)
//   POSEIDON_XP_SEAFLOW_RHO    gradient correlation of the two frames the
//                              dissolve mixes, for its relief gain (unset: 0.85
//                              for the sea field's steps, 0.3 for the river's)
//   POSEIDON_XP_SEAFLOW_HEAVE  share of the sea level's 0.25 m wave the
//                              picture keeps, 0 to 1 (unset: 0.25 with the
//                              flow on, 1 = today's with it off). Set, it
//                              applies with the flow off too (a probe).
//   POSEIDON_XP_SEAFLOW_PROBE  "freeze": with the flow off, the stock sea with
//                              its animation held on one frame (a probe to
//                              tell the two causes apart, never a look)

namespace Poseidon::remaster
{

enum class SeaFlowField
{
    Sea,   // round 3: across the crests, a twin against the texture's layer
    River, // round 2: along the crests, one way (kept to compare)
};

// The switch's value from the text of the variable: 0 off, else the speed
// scale. Unset, empty, non-numeric or not above 0 is off; capped at 4.
float ParseSeaFlowXp(const char* value);
// The process's POSEIDON_XP_SEAFLOW: unset or empty is 1 (on, the shipped look,
// owner verdict 2026-11-13 "much better"); otherwise ParseSeaFlowXp, so "0" is
// off (today's sea, for the identity gates).
float ParseSeaFlowXpEnv(const char* value);
// "river" is round 2's field; anything else (unset too) round 3's.
SeaFlowField ParseSeaFlowField(const char* value);
// Seconds per dissolve step: unset, empty or non-numeric is the field's
// default; below 0.25 (so "0") is 0 (no dissolve); capped at 120.
float ParseSeaFlowMorph(const char* value, SeaFlowField field);
// 0 to 0.95; unset, empty or non-numeric is the field's default.
float ParseSeaFlowRho(const char* value, SeaFlowField field);
// 0 to 1; unset, empty or non-numeric is the default: 0.25 with the flow on,
// 1 with it off.
float ParseSeaFlowHeave(const char* value, bool flowOn);
// True for "freeze" only.
bool ParseSeaFlowFreezeProbe(const char* value);

// The defaults of the round 3 field: no dissolve (it cost 0.13 to 0.32 ms of
// GPU, r36 against r38); 0.8 s a step when POSEIDON_XP_SEAFLOW_MORPH asks.
constexpr float kSeaFlowSeaMorphSec = 0.0f;
constexpr float kSeaFlowSeaRho = 0.85f;  // frames one or two apart: 0.78 to 0.93 (more_anim.01 to .07)
constexpr float kSeaFlowRiverRho = 0.3f; // round 2's guess for frames three apart

// The process's values (read once).
float SeaFlowXpSpeed();
SeaFlowField SeaFlowXpField();
float SeaFlowXpMorph();
float SeaFlowXpRho();
inline bool SeaFlowXpOn()
{
    return SeaFlowXpSpeed() > 0.0f;
}
// The share of the sea level's wave the drawn sea keeps: 0.25 with the flow on
// (the default), 1 with it off.
float SeaFlowXpHeave();
// The freeze probe: only with the flow off.
bool SeaFlowXpFreezeProbe();

// ---- The frames ------------------------------------------------------------

// Without the dissolve the two units carry two frames that are not
// neighbours: frame 0 and this many on (3 for the Remaster's 7 frames).
int SeaFlowFrameStride(int frames);
// The dissolve's frame for step `step`: floor(step * golden ratio) mod
// frames. Each step moves one or two frames forward round the cycle, never
// back, and the run of ones and twos never repeats.
int SeaFlowMorphFrame(long long step, int frames);

struct SeaFlowFrames
{
    int bound = 0;            // the frame the landscape's faces carry (texture unit 0)
    int other = 0;            // the frame on the backend's second unit
    float otherWeight = 0.0f; // the dissolve: share of `other` in every layer
};
// The frames at `timeSec` for an animation of `frames` frames and a dissolve
// of `morphSec` seconds per step (0: frame 0 and the stride's frame, no
// dissolve). The share runs 0 -> 1 over a step, and the next step starts with
// the frame just reached as `bound` at share 0: continuous and one way.
SeaFlowFrames SeaFlowFrameBlend(double timeSec, int frames, float morphSec);
// The phase to hand PolyProperties::AnimateTexture so it binds `frame`.
float SeaFlowAnimationPhase(int frame, int frames);

// ---- The layers ------------------------------------------------------------

// The shader's clock wraps here (litWater2.y, EngineGL33_Sea.cpp); every rate
// is a whole number of 1 / kSeaFlowTimeWrap, so fract(rate * t) is continuous
// across the wrap.
constexpr float kSeaFlowTimeWrap = 4096.0f;
// The sea texture's repeat (m) and the detail texgen's repeats per repeat.
constexpr float kSeaFlowTileM = 50.0f;
constexpr float kSeaFlowRippleRepeats = 32.0f;
// The swell layer's map from the texture's coordinates (the stock program's:
// 0.27 x frequency, rotated 37 degrees), column-major as the GLSL mat2.
constexpr float kSeaFlowSwellFreq = 0.27f;
constexpr float kSeaFlowSwellTurnDeg = 37.0f;
constexpr float kSeaFlowSwellMap[4] = {0.7986f * 0.27f, 0.6018f * 0.27f, -0.6018f * 0.27f, 0.7986f * 0.27f};
constexpr float kSeaFlowSwellK = 1.2f; // the stock program's
// The direction the crests run in the textures' own coordinates (u, v), from
// their luminance's structure tensor at 2 to 8 texels: the sea texture's 15
// degrees (its streaks are 3 to 4 times longer than wide), the ripples' 13.
constexpr float kSeaFlowTexCrestDeg = 15.0f;
constexpr float kSeaFlowRippleCrestDeg = 13.0f;

// The wave height constants of the layers and the maps of the third layer and
// of the swell's partner (frequency, turn). Both fields keep the stock sea's
// slope spread on the fine pair, 0.8^2 = baseK^2 + (chopK * chopFreq)^2, and
// on the swell, 1.2^2 = swellK^2 + (swell2K * swell2Freq)^2.
struct SeaFlowShape
{
    float baseK = 0.0f;
    float chopK = 0.0f;
    float chopFreq = 0.0f;
    float chopTurnDeg = 0.0f;
    float swellK = 1.2f;        // the stock program's kSeaSwellK
    float swell2K = 0.0f;       // 0: no partner for the swell
    float swell2Freq = 1.0f;    // the partner's frequency over the swell's
    float swell2TurnDeg = 0.0f; // the partner's turn on top of the swell's 37 degrees
};
// Round 2: a third layer at 0.61 x, turned -16 degrees (at -52 the two wove
// a visible cross-hatch), with a smaller share of the relief; the swell alone.
constexpr SeaFlowShape kSeaFlowRiverShape{0.71f, 0.60f, 0.61f, -16.0f, 1.2f, 0.0f, 1.0f, 0.0f};
// Round 3: the texture's twin, 0.88 x and turned -8 degrees, with the same
// slope share as the texture's layer (0.566 each: 0.8 / sqrt 2). The swell
// has a partner the same way (0.88 x, turned -8 degrees more, 0.849 each:
// 1.2 / sqrt 2), so the big waves stand too.
constexpr SeaFlowShape kSeaFlowSeaShape{0.5657f, 0.6428f, 0.88f, -8.0f, 0.8485f, 0.9642f, 0.88f, -8.0f};
const SeaFlowShape& SeaFlowFieldShape(SeaFlowField field);
// The third layer's map, column-major as the GLSL mat2: a turn by
// chopTurnDeg of the texture's coordinates, scaled by chopFreq.
void SeaFlowChopMap(const SeaFlowShape& shape, float out[4]);
// The swell partner's map, the same way: a turn by 37 + swell2TurnDeg
// degrees, scaled by 0.27 x swell2Freq.
void SeaFlowSwell2Map(const SeaFlowShape& shape, float out[4]);

struct SeaFlowLayer
{
    float speedMps = 0.0f;   // over the world
    float headingDeg = 0.0f; // in the texture's plane (0 = +u)
};
struct SeaFlowLayers
{
    SeaFlowLayer base, swell, chop, ripple, swell2;
};

// Round 2: the four trains along their crests, one way (no swell partner).
constexpr SeaFlowLayers kSeaFlowRiver{{1.6f, 20.0f}, {2.6f, -12.0f}, {2.05f, 38.0f}, {0.45f, 5.0f}, {0.0f, 0.0f}};

// Round 3. A layer's crests run at the texture's crest angle less its map's
// turn; it travels square to them. Each size comes as a pair of equal trains
// that go opposite ways (the texture's layer and its twin; the swell and its
// partner), so every size stands and none drifts. Deep water waves travel at a
// speed that grows as the square root of their length, so a layer whose
// features are k times the texture's goes sqrt(k) times as fast. The ripples
// (a few centimetres, a single layer) stay where they are, as today's do.
//
// The base speed: the group speed of the texture's 0.28 m slope waves. The
// first cut's 0.6 m/s (their crest speed) changed the near water twice as
// fast as today's (EX5-SEA r33, the motion screen's lower region).
constexpr float kSeaFlowSeaBaseMps = 0.3f;
constexpr float kSeaFlowSeaSwellDeg = kSeaFlowTexCrestDeg - kSeaFlowSwellTurnDeg - 90.0f; // -112
constexpr SeaFlowLayers kSeaFlowSea{
    {kSeaFlowSeaBaseMps, kSeaFlowTexCrestDeg + 90.0f},                              // 105
    {kSeaFlowSeaBaseMps * 1.9245f, kSeaFlowSeaSwellDeg},                            // sqrt(1 / 0.27)
    {kSeaFlowSeaBaseMps * 1.0660f, kSeaFlowTexCrestDeg + 8.0f - 90.0f},             // sqrt(1 / 0.88); -67
    {0.0f, kSeaFlowRippleCrestDeg + 90.0f},                                         // still
    {kSeaFlowSeaBaseMps * 1.9245f * 1.0660f, kSeaFlowSeaSwellDeg + 8.0f + 180.0f}}; // 76

const SeaFlowLayers& SeaFlowFieldLayers(SeaFlowField field);

struct SeaFlowRates
{
    // Offsets per second added to each layer's texture coordinate (the
    // pattern travels the other way: along the layer's heading).
    float base[2] = {0.0f, 0.0f};
    float swell[2] = {0.0f, 0.0f};
    float chop[2] = {0.0f, 0.0f};
    float ripple[2] = {0.0f, 0.0f};
    float swell2[2] = {0.0f, 0.0f};
};
// The rates of `field` at `speedScale`, each snapped to a whole number of
// 1 / kSeaFlowTimeWrap.
SeaFlowRates ComputeSeaFlowRates(float speedScale, SeaFlowField field);

} // namespace Poseidon::remaster
