#include <Poseidon/Graphics/Remaster/SeaFlow.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <numeric>

namespace Poseidon::remaster
{

namespace
{
bool ParseNumber(const char* value, float& out)
{
    if (!value || !*value)
        return false;
    char* end = nullptr;
    const float v = std::strtof(value, &end);
    if (end == value || !std::isfinite(v))
        return false;
    out = v;
    return true;
}

float Snap(float rate)
{
    return std::round(rate * kSeaFlowTimeWrap) / kSeaFlowTimeWrap;
}

// The offset rate, in the texture's own coordinates, of a train that travels
// over the world at `layer`'s speed and heading.
void BaseRate(const SeaFlowLayer& layer, float speedScale, float& u, float& v)
{
    const float a = layer.headingDeg * 0.01745329252f;
    const float tiles = layer.speedMps * speedScale / kSeaFlowTileM;
    u = -tiles * std::cos(a);
    v = -tiles * std::sin(a);
}
} // namespace

float ParseSeaFlowXp(const char* value)
{
    float v = 0.0f;
    if (!ParseNumber(value, v) || v <= 0.0f)
        return 0.0f;
    return std::min(v, 4.0f);
}

float ParseSeaFlowXpEnv(const char* value)
{
    return (value && *value) ? ParseSeaFlowXp(value) : 1.0f;
}

SeaFlowField ParseSeaFlowField(const char* value)
{
    return value && std::strcmp(value, "river") == 0 ? SeaFlowField::River : SeaFlowField::Sea;
}

float ParseSeaFlowMorph(const char* value, SeaFlowField field)
{
    float v = 0.0f;
    if (!ParseNumber(value, v))
        return field == SeaFlowField::Sea ? kSeaFlowSeaMorphSec : 0.0f;
    if (v < 0.25f)
        return 0.0f;
    return std::min(v, 120.0f);
}

float ParseSeaFlowRho(const char* value, SeaFlowField field)
{
    const float unset = field == SeaFlowField::Sea ? kSeaFlowSeaRho : kSeaFlowRiverRho;
    float v = unset;
    if (!ParseNumber(value, v))
        return unset;
    return std::clamp(v, 0.0f, 0.95f);
}

float ParseSeaFlowHeave(const char* value, bool flowOn)
{
    const float unset = flowOn ? 0.25f : 1.0f;
    float v = unset;
    if (!ParseNumber(value, v))
        return unset;
    return std::clamp(v, 0.0f, 1.0f);
}

bool ParseSeaFlowFreezeProbe(const char* value)
{
    return value && std::strcmp(value, "freeze") == 0;
}

float SeaFlowXpHeave()
{
    static const float heave = ParseSeaFlowHeave(std::getenv("POSEIDON_XP_SEAFLOW_HEAVE"), SeaFlowXpOn());
    return heave;
}

bool SeaFlowXpFreezeProbe()
{
    static const bool freeze = !SeaFlowXpOn() && ParseSeaFlowFreezeProbe(std::getenv("POSEIDON_XP_SEAFLOW_PROBE"));
    return freeze;
}

float SeaFlowXpSpeed()
{
    static const float speed = ParseSeaFlowXpEnv(std::getenv("POSEIDON_XP_SEAFLOW"));
    return speed;
}

SeaFlowField SeaFlowXpField()
{
    static const SeaFlowField field = ParseSeaFlowField(std::getenv("POSEIDON_XP_SEAFLOW_FIELD"));
    return field;
}

float SeaFlowXpMorph()
{
    static const float morph = ParseSeaFlowMorph(std::getenv("POSEIDON_XP_SEAFLOW_MORPH"), SeaFlowXpField());
    return morph;
}

float SeaFlowXpRho()
{
    static const float rho = ParseSeaFlowRho(std::getenv("POSEIDON_XP_SEAFLOW_RHO"), SeaFlowXpField());
    return rho;
}

int SeaFlowFrameStride(int frames)
{
    for (int s = frames / 2; s > 1; --s)
    {
        if (std::gcd(s, frames) == 1)
            return s;
    }
    return 1;
}

int SeaFlowMorphFrame(long long step, int frames)
{
    if (frames <= 1)
        return 0;
    constexpr double kGolden = 1.6180339887498949;
    const long long k = static_cast<long long>(std::floor(static_cast<double>(step) * kGolden));
    const long long m = k % frames;
    return static_cast<int>(m < 0 ? m + frames : m);
}

SeaFlowFrames SeaFlowFrameBlend(double timeSec, int frames, float morphSec)
{
    SeaFlowFrames f;
    if (frames <= 1)
        return f;
    if (morphSec <= 0.0f)
    {
        f.other = SeaFlowFrameStride(frames) % frames;
        return f;
    }
    const double pos = std::max(timeSec, 0.0) / static_cast<double>(morphSec);
    const long long step = static_cast<long long>(std::floor(pos));
    f.bound = SeaFlowMorphFrame(step, frames);
    f.other = SeaFlowMorphFrame(step + 1, frames);
    f.otherWeight = std::clamp(static_cast<float>(pos - static_cast<double>(step)), 0.0f, 1.0f);
    return f;
}

float SeaFlowAnimationPhase(int frame, int frames)
{
    if (frames <= 1)
        return 0.0f;
    return (static_cast<float>(std::clamp(frame, 0, frames - 1)) + 0.5f) / static_cast<float>(frames);
}

const SeaFlowLayers& SeaFlowFieldLayers(SeaFlowField field)
{
    return field == SeaFlowField::River ? kSeaFlowRiver : kSeaFlowSea;
}

const SeaFlowShape& SeaFlowFieldShape(SeaFlowField field)
{
    return field == SeaFlowField::River ? kSeaFlowRiverShape : kSeaFlowSeaShape;
}

void SeaFlowChopMap(const SeaFlowShape& shape, float out[4])
{
    const float a = shape.chopTurnDeg * 0.01745329252f;
    const float c = std::cos(a) * shape.chopFreq;
    const float s = std::sin(a) * shape.chopFreq;
    out[0] = c; // column 0
    out[1] = s;
    out[2] = -s; // column 1
    out[3] = c;
}

void SeaFlowSwell2Map(const SeaFlowShape& shape, float out[4])
{
    const float a = (kSeaFlowSwellTurnDeg + shape.swell2TurnDeg) * 0.01745329252f;
    const float f = kSeaFlowSwellFreq * shape.swell2Freq;
    out[0] = std::cos(a) * f; // column 0
    out[1] = std::sin(a) * f;
    out[2] = -std::sin(a) * f; // column 1
    out[3] = std::cos(a) * f;
}

SeaFlowRates ComputeSeaFlowRates(float speedScale, SeaFlowField field)
{
    const SeaFlowLayers& L = SeaFlowFieldLayers(field);
    SeaFlowRates r;
    float u = 0.0f, v = 0.0f;
    BaseRate(L.base, speedScale, u, v);
    r.base[0] = Snap(u);
    r.base[1] = Snap(v);
    // The swell and the third layer sample at M * uv: their offset is M times
    // the texture-plane one.
    BaseRate(L.swell, speedScale, u, v);
    r.swell[0] = Snap(kSeaFlowSwellMap[0] * u + kSeaFlowSwellMap[2] * v);
    r.swell[1] = Snap(kSeaFlowSwellMap[1] * u + kSeaFlowSwellMap[3] * v);
    float chopMap[4];
    SeaFlowChopMap(SeaFlowFieldShape(field), chopMap);
    BaseRate(L.chop, speedScale, u, v);
    r.chop[0] = Snap(chopMap[0] * u + chopMap[2] * v);
    r.chop[1] = Snap(chopMap[1] * u + chopMap[3] * v);
    BaseRate(L.ripple, speedScale, u, v);
    r.ripple[0] = Snap(kSeaFlowRippleRepeats * u);
    r.ripple[1] = Snap(kSeaFlowRippleRepeats * v);
    float swell2Map[4];
    SeaFlowSwell2Map(SeaFlowFieldShape(field), swell2Map);
    BaseRate(L.swell2, speedScale, u, v);
    r.swell2[0] = Snap(swell2Map[0] * u + swell2Map[2] * v);
    r.swell2[1] = Snap(swell2Map[1] * u + swell2Map[3] * v);
    return r;
}

} // namespace Poseidon::remaster
