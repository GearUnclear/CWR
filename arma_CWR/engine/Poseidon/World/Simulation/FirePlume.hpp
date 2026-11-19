#pragma once

#include <algorithm>
#include <cmath>

// The fire's smoke plume (the realism audit's fix 2, cause C4): the law a plume
// puff (Smokes.hpp PlumePuff) follows from its source as the wind carries it,
// how thick it is there, and how it is lit. Pure: no engine state, so the unit
// tests pin it. The fire's world glue (FireSpreadWorld.cpp FireWorld::EmitPlume)
// picks the sources and emits the puffs; each puff then follows the law on its
// own for minutes, carried by the wind at its height.
//
// Rise (consult eighth 1, Briggs's bent-over plume): the centreline stands
// H = 1.6 F^(1/3) x^(2/3) / U over the smoke's source, x the distance the smoke
// has travelled downwind, F the buoyancy flux (8.8 m4/s3 per MW of convective
// heat, about 0.6 of the heat released: kPlumeBuoyancyPerMW a MW released), U
// the wind at plume height (kPlumeWindShare of the 10 m wind), until the
// distance of final rise (3.5 x*: x* = 14 F^(5/8) under F 55, else 34 F^(2/5));
// past it the plume runs level and only spreads. The scene's 120 m grass head
// (about 250 MW in an 11 m/s wind) stands 31 m up at 100 m, 145 m at 1 km and
// 230 m at 2 km, its final rise about 2.1 km out: from the source the axis
// rises 17 degrees at 100 m, 8 at 1 km and 6.5 at 2 km.
//
// Shape: the visible thickness is kPlumeThickness of the rise, centred on it;
// the width is the fire's own plus that plus the wind's spreading
// (kPlumeSpreadX of x). For the first kPlumeHugPerWind metres per m/s of wind
// (100 to 300 m in a strong one) the plume's underside lies on the ground, so
// the eye just downwind of a head stands in its smoke; past it the underside
// lifts to kPlumeUnderShare of the rise (40 to 70 m up at 1 km, about 100 m at
// 2 km). A torching crown's smoke leaves from its flame's top and does not hug.
namespace Poseidon::firespread
{
constexpr float kPlumeBuoyancyPerMW = 5.3f; // F (m4/s3) a MW released
constexpr float kPlumeWindShare = 1.1f;     // the wind at plume height over the 10 m wind
constexpr float kPlumeMinWindMps = 1.5f;    // the bent-over law's least wind
constexpr float kPlumeThickness = 1.2f;     // the visible thickness over the rise
constexpr float kPlumeUnderShare = 0.4f;    // the underside's height over the rise, past the hug
constexpr float kPlumeSpreadX = 0.04f;      // the wind's own sideways spread a metre downwind (both sides)
constexpr float kPlumeLevelThicken = 0.02f; // thickening a metre past final rise
constexpr float kPlumeHugPerWind = 30.0f;   // metres of ground hug a m/s of 10 m wind ...
constexpr float kPlumeHugMinM = 40.0f, kPlumeHugMaxM = 330.0f; // ... within these
constexpr float kPlumeLeaveM = 3.0f;                           // the top's height over the flames at the source

struct PlumeLaw
{
    float riseK = 0.0f;  // H = riseK x^(2/3) (m^(1/3))
    float finalX = 0.0f; // the distance of final rise (m)
    float baseM = 0.0f;  // where the smoke leaves the flames, over its source's ground (m)
    float hugM = 0.0f;   // how far its underside stays on the ground (m; 0: never)
};

inline float PlumeSmooth(float e0, float e1, float v)
{
    if (e1 <= e0)
        return v < e0 ? 0.0f : 1.0f;
    const float t = std::clamp((v - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// The law of a source releasing heatMW (all of it, not only the convective
// share) in a 10 m wind, its smoke leaving baseM over the ground; `hugs` for a
// surface fire's smoke (a torching crown's does not).
inline PlumeLaw MakePlumeLaw(float heatMW, float wind10, float baseM, bool hugs)
{
    PlumeLaw l;
    const float F = kPlumeBuoyancyPerMW * std::max(heatMW, 0.01f);
    const float U = kPlumeWindShare * std::max(wind10, kPlumeMinWindMps);
    l.riseK = 1.6f * std::cbrt(F) / U;
    const float xs = F < 55.0f ? 14.0f * std::pow(F, 0.625f) : 34.0f * std::pow(F, 0.4f);
    l.finalX = 3.5f * xs;
    l.baseM = std::max(baseM, 0.0f);
    l.hugM = hugs ? std::clamp(kPlumeHugPerWind * wind10, kPlumeHugMinM, kPlumeHugMaxM) : 0.0f;
    return l;
}

// The centreline's rise over the smoke's start x metres downwind (m).
inline float PlumeRiseM(const PlumeLaw& l, float x)
{
    const float xe = std::clamp(x, 0.0f, l.finalX);
    return l.riseK * std::pow(xe, 2.0f / 3.0f);
}
// Its thickening past final rise (m).
inline float PlumeLevelM(const PlumeLaw& l, float x)
{
    return kPlumeLevelThicken * std::max(x - l.finalX, 0.0f);
}
// The plume's top and underside over the source's ground x metres downwind (m).
inline float PlumeTopM(const PlumeLaw& l, float x)
{
    const float h = PlumeRiseM(l, x);
    return l.baseM + kPlumeLeaveM + h * (1.0f + 0.5f * kPlumeThickness) + 0.5f * PlumeLevelM(l, x);
}
inline float PlumeBottomM(const PlumeLaw& l, float x)
{
    const float h = PlumeRiseM(l, x);
    const float lifted = 0.5f * l.baseM + kPlumeUnderShare * h - 0.5f * PlumeLevelM(l, x);
    const float lift = l.hugM > 0.0f ? PlumeSmooth(0.3f * l.hugM, l.hugM, x) : 1.0f;
    return std::max(lifted * lift, 0.0f);
}
// Its centreline over the source's ground (m).
inline float PlumeCentreM(const PlumeLaw& l, float x)
{
    return 0.5f * (PlumeTopM(l, x) + PlumeBottomM(l, x));
}
// How far it has spread to each side past its source's own width (m).
inline float PlumeSpreadM(const PlumeLaw& l, float x)
{
    return 0.5f * (kPlumeThickness * PlumeRiseM(l, x) + kPlumeSpreadX * std::max(x, 0.0f) + PlumeLevelM(l, x));
}

// ---- The puffs ----
// A puff is born kPuffBornM across (a few metres: smaller and many more than
// the 7 to 13 m discs before), and grows as the plume does: by kPuffGrow of
// the plume's growth in width. Its projected cover is kPuffCover of its size
// squared (the soft round texture). The set thins as it grows: a puff lives
// while PlumeSurvival is over its own draw (0..1), so that seen side-on about
// kPuffOverlap of them lie behind each other anywhere along the plume, the
// mass of those that die taken up by those that live on (PlumePuffTau). So a
// few hundred large puffs carry the far plume for kilometres.
constexpr float kPuffGrow = 0.35f;
constexpr float kPuffCover = 0.4f;
constexpr float kPuffOverlap = 3.5f;
constexpr float kPuffAlphaMax = 0.5f; // one puff's opacity at most (no puff reads as a disc)

inline float PlumePuffSizeM(const PlumeLaw& l, float x, float bornM)
{
    return bornM + kPuffGrow * 2.0f * PlumeSpreadM(l, x);
}
// The share of the puffs born (perMetre a metre of the wind's travel) still
// living x metres downwind (`overlap` of them behind each other side-on: the
// grass fire's kPuffOverlap, an object's own column kObjPuffOverlap).
inline float PlumeSurvival(const PlumeLaw& l, float x, float bornM, float perMetre, float overlap = kPuffOverlap)
{
    const float s = PlumePuffSizeM(l, x, bornM);
    // Where it leaves the plume is thinner than a few born puffs: all live.
    const float thick = std::max(PlumeTopM(l, x) - PlumeBottomM(l, x), kPlumeLeaveM + 2.5f * bornM);
    const float cover = std::max(perMetre, 1e-4f) * kPuffCover * s * s;
    return std::clamp(overlap * thick / cover, 0.0f, 1.0f);
}
// The optical depth one puff carries x metres downwind: its share `mass` of
// the source's smoke (smoke units: the fire's smoke source a second over the
// puffs born a second), spread over its cover, with the mass of those that
// died, times `opticK` (optical depth a smoke unit per square metre).
inline float PlumePuffTau(const PlumeLaw& l, float x, float bornM, float perMetre, float mass, float opticK,
                          float overlap = kPuffOverlap)
{
    const float s = PlumePuffSizeM(l, x, bornM);
    const float live = std::max(PlumeSurvival(l, x, bornM, perMetre, overlap), 1e-3f);
    return opticK * std::max(mass, 0.0f) / (live * kPuffCover * s * s);
}
// The plume's optical depth seen side-on (across its width) and from above
// (through its thickness) x metres downwind, for a source of `smoke` units a
// second carried by a wind of windMps and `widthM` wide where it starts.
inline float PlumeSideTau(const PlumeLaw& l, float x, float smoke, float windMps, float opticK)
{
    const float thick = std::max(PlumeTopM(l, x) - PlumeBottomM(l, x), 1.0f);
    return opticK * smoke / (std::max(windMps, 0.5f) * thick);
}
inline float PlumeTopTau(const PlumeLaw& l, float x, float smoke, float windMps, float widthM, float opticK)
{
    const float width = std::max(widthM + 2.0f * PlumeSpreadM(l, x), 1.0f);
    return opticK * smoke / (std::max(windMps, 0.5f) * width);
}

// All of a puff's numbers x metres downwind at once (a puff's every step:
// one power instead of a dozen), the same as the functions above.
struct PlumePoint
{
    float top = 0.0f, bottom = 0.0f, spread = 0.0f; // PlumeTopM, PlumeBottomM, PlumeSpreadM
    float size = 0.0f, survival = 1.0f, tau = 0.0f; // PlumePuffSizeM, PlumeSurvival, PlumePuffTau
};
inline PlumePoint PlumeAt(const PlumeLaw& l, float x, float bornM, float perMetre, float mass, float opticK,
                          float overlap = kPuffOverlap)
{
    PlumePoint p;
    const float h = PlumeRiseM(l, x), level = PlumeLevelM(l, x);
    p.top = l.baseM + kPlumeLeaveM + h * (1.0f + 0.5f * kPlumeThickness) + 0.5f * level;
    const float lift = l.hugM > 0.0f ? PlumeSmooth(0.3f * l.hugM, l.hugM, x) : 1.0f;
    p.bottom = std::max((0.5f * l.baseM + kPlumeUnderShare * h - 0.5f * level) * lift, 0.0f);
    p.spread = 0.5f * (kPlumeThickness * h + kPlumeSpreadX * std::max(x, 0.0f) + level);
    p.size = bornM + kPuffGrow * 2.0f * p.spread;
    const float thick = std::max(p.top - p.bottom, kPlumeLeaveM + 2.5f * bornM);
    const float cover = kPuffCover * p.size * p.size;
    p.survival = std::clamp(overlap * thick / (std::max(perMetre, 1e-4f) * cover), 0.0f, 1.0f);
    p.tau = opticK * std::max(mass, 0.0f) / (std::max(p.survival, 1e-3f) * cover);
    return p;
}

// ---- An object fire's own column (followups, 2026-11-18) ----
// A burning house's, wreck's or tanker's smoke is its own column (the consult's
// tenth answer: "a tight boiling column ... from one spot", a burning village
// "a set of separate black columns standing in or behind the pale drift of the
// grass fire"), never a share of the grass fire's puffs: with thousands of
// burning cells beside it a house drew a fraction of a percent of them and its
// column vanished (FireSpreadWorld.cpp EmitObjectPlumes). Each column sends
// enough puffs a second to stay unbroken: each puff born no more than
// kObjPlumeGap of its size behind the last at the wind near the ground
// (kObjPlumeLowWind of the 10 m wind), and kObjPuffOverlap of them behind each
// other seen side-on kObjPlumeRefM downwind (at least kObjPlumeRefBorn of its
// puffs' size out); thin smoke (under kObjPlumeSmokeFull units, a shell
// smoking at the end) sends fewer, down to kObjPlumeThinMin. At most
// kObjColumnRateMax a second (over its size factor's square far off), and all
// the columns together at most the world's budget (rateScale): with fewer,
// its puffs are born larger, so its cover and its stream hold. Far off its
// puffs are born at least kObjPlumeMinPx pixels across (at most
// kObjPlumeLodMax times their own size). Its puffs thin by its own puffs a
// metre (PlumeSurvival at kObjPuffOverlap), not the grass fire's.
constexpr float kObjPlumeGap = 0.4f, kObjPlumeLowWind = 0.75f;
constexpr float kObjPuffOverlap = 1.4f, kObjPlumeRefM = 30.0f, kObjPlumeRefBorn = 3.0f;
constexpr float kObjPlumeSmokeFull = 20.0f, kObjPlumeThinMin = 0.3f;
constexpr float kObjColumnRateMax = 12.0f, kObjPlumeMinPx = 24.0f, kObjPlumeLodMax = 3.0f, kObjPlumeBornMaxM = 30.0f;
struct ObjectColumnPlan
{
    float rate = 0.0f;     // puffs a second
    float bornM = 0.0f;    // their size at birth (m)
    float lod = 1.0f;      // its size factor for the distance
    float perMetre = 0.0f; // its puffs a metre of the 10 m wind's travel (their survival's)
    float need = 0.0f;     // the puffs a second an unbroken column of its own size needs
};
// A column on the law l with `smoke` units a second, its puffs bornM across
// at their source, in a 10 m wind of wind10 m/s, seen at pxPerM pixels a
// metre; rateScale (0..1) the share of its puffs the world's budget allows.
inline ObjectColumnPlan PlanObjectColumn(const PlumeLaw& l, float smoke, float bornM, float wind10, float pxPerM,
                                         float rateScale = 1.0f)
{
    ObjectColumnPlan p;
    const float born = std::max(bornM, 0.5f);
    p.lod = std::clamp(kObjPlumeMinPx / std::max(born * pxPerM, 1e-3f), 1.0f, kObjPlumeLodMax);
    float b = born * p.lod;
    const float uLow = kObjPlumeLowWind * std::max(wind10, 1.0f);
    const float x = std::max(kObjPlumeRefM, kObjPlumeRefBorn * b);
    const PlumePoint at = PlumeAt(l, x, b, 1.0f, 0.0f, 1.0f);
    const float thick = std::max(at.top - at.bottom, 0.5f);
    const float grow = at.size - b; // a puff's growth out to x (the same whatever its birth size)
    const float cover = kPuffCover * at.size * at.size;
    p.need = std::max(uLow / (kObjPlumeGap * b), kObjPuffOverlap * thick * uLow / std::max(cover, 1e-3f));
    const float thin = std::clamp(std::sqrt(std::max(smoke, 0.0f) / kObjPlumeSmokeFull), kObjPlumeThinMin, 1.0f);
    p.rate = std::min(p.need * thin, kObjColumnRateMax / (p.lod * p.lod)) * std::clamp(rateScale, 0.0f, 1.0f);
    p.rate = std::max(p.rate, 0.05f);
    if (p.rate < p.need)
    {
        // fewer puffs: each born larger, so its cover and its stream hold
        const float size = std::sqrt(kObjPuffOverlap * thick * uLow / (kPuffCover * p.rate));
        b = std::max({b, size - grow, uLow / (kObjPlumeGap * p.rate)});
        b = std::min(b, std::max(kObjPlumeBornMaxM, born * p.lod));
    }
    p.bornM = b;
    p.perMetre = p.rate / std::max(wind10, 1.0f);
    return p;
}
// How many puffs behind each other a column shows side-on x metres downwind
// (its puffs born bornM, perMetre a metre, that many living there).
inline float ObjectColumnOverlap(const PlumeLaw& l, float x, float bornM, float perMetre)
{
    const PlumePoint at = PlumeAt(l, x, bornM, perMetre, 0.0f, 1.0f, kObjPuffOverlap);
    const float thick = std::max(at.top - at.bottom, 0.5f);
    return perMetre * at.survival * kPuffCover * at.size * at.size / thick;
}

// ---- How it looks ----
// Colour (consult 6c and eighth 1, the checklist 1.5): grass smoke leaves grey
// with a tan cast and is pale grey-white by 500 m; shrub, timber and crown
// smoke leaves dark grey-brown, greys as it rises and cools, and stays a
// little darker for 1 to 2 km before it merges.
// `woody` is the source's woody share (0 grass .. 1 a torching crown).
struct PlumeRgb
{
    float r = 0.0f, g = 0.0f, b = 0.0f;
};
constexpr PlumeRgb kPlumeGrassRgb = {0.80f, 0.76f, 0.68f};
constexpr PlumeRgb kPlumeWoodyRgb = {0.36f, 0.33f, 0.29f};
constexpr PlumeRgb kPlumeFarRgb = {0.88f, 0.88f, 0.87f};
// (fire-objects) A burning wreck's or house's smoke is black (oil, tyres,
// paint, furnishings: the consult's tenth answer, dense black to dark brown):
// its black share takes the albedo toward kPlumeSootRgb, and it pales slower,
// staying dark for kilometres.
constexpr PlumeRgb kPlumeSootRgb = {0.20f, 0.18f, 0.16f};
inline PlumeRgb PlumeAlbedo(float woody, float x, float black = 0.0f)
{
    const float w = std::clamp(woody, 0.0f, 1.0f), k = std::clamp(black, 0.0f, 1.0f);
    PlumeRgb src{kPlumeGrassRgb.r + (kPlumeWoodyRgb.r - kPlumeGrassRgb.r) * w,
                 kPlumeGrassRgb.g + (kPlumeWoodyRgb.g - kPlumeGrassRgb.g) * w,
                 kPlumeGrassRgb.b + (kPlumeWoodyRgb.b - kPlumeGrassRgb.b) * w};
    src = {src.r + (kPlumeSootRgb.r - src.r) * k, src.g + (kPlumeSootRgb.g - src.g) * k,
           src.b + (kPlumeSootRgb.b - src.b) * k};
    // Grass smoke pales over its first 600 m; dark smoke over 0.08 to 1.5 km,
    // staying a little darker; black smoke over 0.3 to 4 km, still dark grey.
    const float paleBase = (1.0f - w) * PlumeSmooth(0.0f, 600.0f, x) + w * 0.9f * PlumeSmooth(80.0f, 1500.0f, x);
    const float pale = (1.0f - k) * paleBase + k * 0.7f * PlumeSmooth(300.0f, 4000.0f, x);
    return {src.r + (kPlumeFarRgb.r - src.r) * pale, src.g + (kPlumeFarRgb.g - src.g) * pale,
            src.b + (kPlumeFarRgb.b - src.b) * pale};
}

// The sun's share of the light on a puff by the sun's height (degrees): the
// rest is the sky's. Nothing under the horizon, most of it in a high sun.
inline float PlumeSunShare(float sunElevationDeg)
{
    return 0.75f * PlumeSmooth(-1.0f, 15.0f, sunElevationDeg);
}
// How bright a puff scatters the sun toward the eye against a flat
// (isotropic) one: cosView is the cosine between the eye's ray through the
// puff and the direction to the sun (1: looking into the sun), `thin` the light
// its own smoke lets through (exp(-tau)). Thin smoke scatters forward (a
// Henyey-Greenstein lobe, g kPlumeForwardG, over a multiple-scattering floor):
// its edges glow when the sun is behind it. Dense smoke shows its sunlit side
// when the sun is behind the eye and its shaded side when looking toward it.
constexpr float kPlumeForwardG = 0.5f;
inline float PlumePhase(float cosView, float thin)
{
    const float c = std::clamp(cosView, -1.0f, 1.0f);
    const float g = kPlumeForwardG;
    const float hg = (1.0f - g * g) / std::pow(1.0f + g * g - 2.0f * g * c, 1.5f);
    const float thinP = 0.5f + 0.5f * hg;
    const float denseP = 0.75f - 0.45f * c;
    const float t = std::clamp(thin, 0.0f, 1.0f);
    return 1.2f * (t * thinP + (1.0f - t) * denseP);
}
// The sunlight left after crossing tauToSun of the plume, per channel: the
// smoke's fine particles take blue first, so the light under and inside a
// plume goes orange-brown (consult eighth 1).
inline PlumeRgb PlumeSunThrough(float tauToSun)
{
    const float t = std::max(tauToSun, 0.0f);
    return {std::exp(-t), std::exp(-1.15f * t), std::exp(-1.45f * t)};
}

// The flames' light on a puff's underside (consult eighth 4: a grass head
// lights a face turned to it with 400 to 1100 lux at 10 m, the smoke over it
// is a large orange ceiling): each block of flames below gives it
// kPlumeGlowLuxPerHeat lux a metre of flame (its heat) over
// 1 + (d / kPlumeGlowReachM)^2, d its distance to the flames' middle, up to
// kPlumeGlowRangeM, at a soot fire's colour (kPlumeGlowRgb, about 1400 K, linear).
// The puff takes it as a fire's combat light on smoke (CombatLights.hpp
// CombatParticleLight: added at the light's hue, not scaled by the exposure),
// kPlumeGlowPerLux of it a lux, times PlumeGlowNight: it shows from about
// sunset (consult eighth 4) and is a faint warmth by day.
constexpr float kPlumeGlowLuxPerHeat = 25.0f;
constexpr float kPlumeGlowReachM = 12.0f;
constexpr float kPlumeGlowRangeM = 120.0f;
constexpr float kPlumeGlowRgb[3] = {1.0f, 0.22f, 0.03f};
constexpr float kPlumeGlowPerLux = 0.01f;
constexpr float kPlumeGlowDay = 0.03f, kPlumeGlowDayDeg = 5.0f, kPlumeGlowNightDeg = -6.0f;
inline float PlumeGlowNight(float sunElevationDeg)
{
    return kPlumeGlowDay +
           (1.0f - kPlumeGlowDay) * (1.0f - PlumeSmooth(kPlumeGlowNightDeg, kPlumeGlowDayDeg, sunElevationDeg));
}
inline float PlumeGlowKernel(float d)
{
    if (d >= kPlumeGlowRangeM)
        return 0.0f;
    const float x = d / kPlumeGlowReachM;
    const float edge = 1.0f - PlumeSmooth(0.6f * kPlumeGlowRangeM, kPlumeGlowRangeM, d);
    return edge / (1.0f + x * x);
}
} // namespace Poseidon::firespread
