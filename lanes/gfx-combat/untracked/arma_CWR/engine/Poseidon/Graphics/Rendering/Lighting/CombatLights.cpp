// RESEARCH PROTOTYPE (track "combat"): see CombatLights.hpp.

#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>

#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace Poseidon
{
namespace
{
constexpr float kTwoPi = 6.28318530718f;
// Irradiance (linear) where a light's window ends: about the classic night
// ambient, so a light reaches as far as it can still be seen at night.
constexpr float kRangeFloor = 0.0025f;
constexpr float kMaxRange = 160.0f;

float Hash01(std::uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return static_cast<float>(x & 0xffffffU) / static_cast<float>(0x1000000);
}

// Smooth value noise in [-1, 1] over time.
float Noise1(float t, std::uint32_t seed)
{
    const float f = std::floor(t);
    const float u = t - f;
    const auto i = static_cast<std::int64_t>(f);
    const float a = Hash01(static_cast<std::uint32_t>(i) * 747796405U + seed) * 2.0f - 1.0f;
    const float b = Hash01(static_cast<std::uint32_t>(i + 1) * 747796405U + seed) * 2.0f - 1.0f;
    const float s = u * u * (3.0f - 2.0f * u);
    return a + (b - a) * s;
}

void SetColour(CombatLightSample& s, float r, float g, float b)
{
    const float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    const float inv = lum > 1e-6f ? 1.0f / lum : 0.0f;
    s.r = r * inv;
    s.g = g * inv;
    s.b = b * inv;
}

// I1 = intensity at 1 m for d >> r0 (I1 = intensity * r0^2).
void SetPower(CombatLightSample& s, float i1, float core)
{
    s.core = core;
    s.intensity = i1 / (core * core);
    float range = std::sqrt(std::max(i1, 0.0f) / kRangeFloor);
    s.range = std::min(std::max(range, core * 4.0f), kMaxRange);
}

std::vector<Ref<LightCombat>>& Bursts()
{
    static std::vector<Ref<LightCombat>> list;
    return list;
}

std::uint32_t s_seedCounter = 0x9e3779b9U;
// Tuning gains read once per frame (CombatLights::Refresh).
float s_flashGain = 1.0f;
float s_fireGain = 1.0f;
// Round 9: the remastered exposure does not adapt between day and night
// (the night is lifted about a thousandfold instead), so a light of fixed
// power reads as strong under the noon sun as it should at night. Scale by
// the scene's night factor: continuous lights (fires, tracers, flares) almost
// vanish by day, flashes keep half their punch.
float s_night = 1.0f;
} // namespace

CombatLightSample EvaluateCombatLight(CombatLightKind kind, float strength, float age, std::uint32_t seed)
{
    CombatLightSample s;
    if (age < 0.0f)
    {
        s.finished = kind != CombatLightKind::Fire && kind != CombatLightKind::Tracer && kind != CombatLightKind::Flare;
        return s;
    }
    const float jitter = 0.8f + 0.4f * Hash01(seed); // shot to shot variation
    switch (kind)
    {
        case CombatLightKind::Muzzle:
        {
            // The flash is there the frame the round leaves and gone in a few
            // frames: one fast exponential, a little flicker between shots.
            const float st = std::min(std::max(strength, 0.3f), 3.0f);
            SetColour(s, 1.0f, 0.50f, 0.16f);
            const float env = std::exp(-age / 0.030f);
            SetPower(s, 1.6f * st * jitter, 0.30f);
            s.range = std::min(s.range, 45.0f);
            s.intensity *= env;
            s.finished = age > 0.18f;
            break;
        }
        case CombatLightKind::Cannon:
        {
            const float st = std::min(std::max(strength, 0.5f), 3.0f);
            SetColour(s, 1.0f, 0.62f, 0.30f);
            const float env = 0.8f * std::exp(-age / 0.045f) + 0.2f * std::exp(-age / 0.16f);
            SetPower(s, 70.0f * st * jitter, 0.8f);
            s.intensity *= env;
            s.finished = age > 0.6f;
            break;
        }
        case CombatLightKind::Explosion:
        {
            // A white-hot flash, then the fireball: orange, flickering, fading
            // over about a second. Strength is the engine's explosion size
            // (0.25 grenade .. 1 heavy shell, 1.25 fuel).
            const float e = std::min(std::max(strength, 0.2f), 1.5f);
            const float cool = std::min(age / 0.45f, 1.0f);
            const float c = cool * cool * (3.0f - 2.0f * cool);
            SetColour(s, 1.0f, 0.86f + (0.40f - 0.86f) * c, 0.62f + (0.11f - 0.62f) * c);
            // combatFlash scales the white-hot first flash only (1 = the run-4
            // envelope); the window is sized for the peak.
            const float flash = std::max(s_flashGain, 0.0f);
            const float peak = 0.65f * flash + 0.35f;
            // Round 8: the fireball fades over about a third of a second (was
            // 0.28 s): an HE burst's dust cloud kept glowing for a second.
            float fireball = 0.35f * std::exp(-age / 0.16f);
            if (age > 0.04f)
                fireball *= 1.0f + 0.22f * Noise1(age * 14.0f, seed) + 0.1f * Noise1(age * 31.0f, seed ^ 0x51ed27U);
            float flashEnv = 0.65f * flash * std::exp(-age / 0.06f);
            if (age > 0.04f)
                flashEnv *= 1.0f + 0.22f * Noise1(age * 14.0f, seed) + 0.1f * Noise1(age * 31.0f, seed ^ 0x51ed27U);
            const float env = flashEnv + fireball;
            SetPower(s, 420.0f * e * e * jitter * peak, 0.8f + 1.8f * e);
            s.intensity *= std::max(env, 0.0f) / peak;
            s.finished = age > 0.9f;
            break;
        }
        case CombatLightKind::Fire:
        {
            // Continuous: a slow breathing, faster licks and a crackle.
            const float st = std::max(strength, 0.0f);
            SetColour(s, 1.0f, 0.46f, 0.14f);
            const float ph = Hash01(seed) * kTwoPi;
            float f = 0.80f + 0.10f * std::sin(kTwoPi * 1.3f * age + ph) +
                      0.07f * std::sin(kTwoPi * 3.7f * age + 2.1f * ph) + 0.10f * Noise1(age * 9.0f, seed) +
                      0.05f * Noise1(age * 23.0f, seed ^ 0x2545f491U);
            SetPower(s, 7.0f * st * std::max(s_fireGain, 0.0f), 0.45f);
            s.intensity *= std::max(f, 0.2f);
            s.finished = false;
            break;
        }
        case CombatLightKind::Flare:
        {
            // A magnesium flare under its parachute: lights up over a third of
            // a second, then burns with a sputtering flicker. It hangs 50-150 m
            // up, so its window reaches far beyond the other lights'.
            const float st = std::max(strength, 0.0f);
            SetColour(s, 1.0f, 0.97f, 0.90f);
            const float on = std::min(age / 0.35f, 1.0f);
            const float f = 0.84f + 0.08f * Noise1(age * 7.0f, seed) + 0.05f * Noise1(age * 19.0f, seed ^ 0x6c8e9cf5U) +
                            0.03f * std::sin(kTwoPi * 2.3f * age);
            SetPower(s, 2500.0f * st, 0.6f);
            s.range = std::min(std::sqrt(2500.0f * st / kRangeFloor), 600.0f);
            s.intensity *= on * on * std::max(f, 0.3f);
            s.finished = false;
            break;
        }
        case CombatLightKind::Tracer:
        {
            const float st = std::min(std::max(strength, 0.0f), 2.0f);
            SetColour(s, 1.0f, 0.62f, 0.30f);
            SetPower(s, 0.25f * st, 0.15f);
            s.range = std::min(s.range, 12.0f);
            s.finished = false;
            break;
        }
    }
    return s;
}

LightCombat::LightCombat(CombatLightKind kind, float strength)
    : base(HBlack, HBlack), _kind(kind), _strength(strength), _seed(s_seedCounter)
{
    s_seedCounter = s_seedCounter * 1664525U + 1013904223U;
    _sample = EvaluateCombatLight(_kind, _strength, 0.0f, _seed);
    _startAtten = _sample.core;
    _diffuse = Radiance();
}

bool LightCombat::Advance(float dt)
{
    const float gain = CombatLights::Gain();
    _age += dt;
    _sample = EvaluateCombatLight(_kind, _strength, _age, _seed);
    const bool continuous =
        _kind == CombatLightKind::Fire || _kind == CombatLightKind::Tracer || _kind == CombatLightKind::Flare;
    const float day = continuous ? 0.12f + 0.88f * s_night : 0.5f + 0.5f * s_night;
    _sample.intensity *= gain * day;
    if (_kind == CombatLightKind::Flare)
    {
        const float lum = 0.2126f * _tint[0] + 0.7152f * _tint[1] + 0.0722f * _tint[2];
        if (lum > 1e-4f)
            SetColour(_sample, _tint[0], _tint[1], _tint[2]);
    }
    _startAtten = _sample.core;
    _diffuse = Radiance();
    return !_sample.finished;
}

float LightCombat::Brightness() const
{
    // Scene::SelectLights keeps a light for an object when Brightness() > 0.02 d^2:
    // select everything inside the window while the light is lit.
    if (_sample.intensity <= 1e-5f)
        return 0.0f;
    return 0.0201f * _sample.range * _sample.range;
}

Color LightCombat::Apply(Vector3Par, Vector3Par)
{
    // CPU lighting paths (particles) use IrradianceAt instead.
    return Color(HBlack);
}

Color LightCombat::IrradianceAt(Vector3Par worldPos) const
{
    const float d2 = worldPos.Distance2(Position());
    const float r2 = _sample.range * _sample.range;
    if (d2 >= r2 || _sample.intensity <= 0.0f)
        return Color(HBlack);
    const float x = d2 / r2;
    float win = 1.0f - x * x;
    win *= win;
    const float e = _sample.intensity / (1.0f + d2 / (_sample.core * _sample.core)) * win;
    return Color(_sample.r * e, _sample.g * e, _sample.b * e, 0.0f);
}

void LightCombat::GetDescription(LightDescription& desc) const
{
    desc.type = LTPoint;
    desc.dir = Direction();
    desc.pos = Position();
    desc.startAtten = _sample.core;
    desc.ambient = Color(HBlack);
    desc.diffuse = Radiance();
    desc.phi = 0;
    desc.theta = 0;
    desc.hdrRange = _sample.intensity > 1e-5f ? _sample.range : 0.0f;
}

namespace CombatLights
{
// The tuning is read once per frame (Tick): Enabled() is asked per particle
// and per bullet.
struct Cached
{
    bool enabled = false;
    float gain = 0.0f, smoke = 1.0f, scatter = 1.0f, wreck = 1.0f;
};
static Cached s_cache;

static void Refresh()
{
    Cached c;
    if (GEngine)
    {
        const remaster::LightingTuning t = GEngine->GetLightingTuning();
        c.enabled = GEngine->LightingRemasterActive() && t.combatLights > 0.0f;
        c.gain = t.combatLights;
        c.smoke = t.combatSmoke;
        c.scatter = t.combatScatter;
        c.wreck = t.combatWreck;
        s_flashGain = t.combatFlash;
        s_fireGain = t.combatFire;
    }
    s_cache = c;
}

bool Enabled()
{
    return s_cache.enabled;
}

float Gain()
{
    return s_cache.gain;
}
float SmokeGain()
{
    return s_cache.smoke;
}
float ScatterGain()
{
    return s_cache.scatter;
}
float WreckGain()
{
    return s_cache.wreck;
}

void Spawn(CombatLightKind kind, Vector3Par pos, float strength)
{
    if (!GScene)
        return;
    Ref<LightCombat> light = new LightCombat(kind, strength);
    light->SetPosition(pos);
    light->Advance(0.0f);
    GScene->AddLight(light);
    Bursts().push_back(light);
}

Ref<LightCombat> CreateOwned(CombatLightKind kind, float strength)
{
    Ref<LightCombat> light = new LightCombat(kind, strength);
    light->Advance(0.0f);
    if (GScene)
        GScene->AddLight(light);
    return light;
}

static float s_lastTick = -1.0f;

void Tick()
{
    auto& bursts = Bursts();
    const float now = Glob.time.toFloat();
    // Age by the sim-time step; a clock jump (scripted time pin, load) is not
    // an age step.
    float dt = s_lastTick >= 0.0f ? now - s_lastTick : 0.0f;
    if (dt < 0.0f || dt > 0.25f)
        dt = 0.0f;
    s_lastTick = now;
    Refresh();
    s_night = (GScene && GScene->MainLight()) ? std::min(std::max(GScene->MainLight()->NightEffect(), 0.0f), 1.0f) : 1.0f;
    if (!Enabled())
    {
        bursts.clear();
        return;
    }
    for (size_t i = 0; i < bursts.size();)
    {
        if (!bursts[i]->Advance(dt))
        {
            bursts[i] = bursts.back();
            bursts.pop_back();
        }
        else
        {
            ++i;
        }
    }
    // Owned lights (fires, tracers) are in the scene list; update their flicker.
    if (GScene)
    {
        for (int i = 0; i < GScene->NLights(); ++i)
        {
            Light* l = GScene->GetLight(i);
            if (l && l->IsCombat())
            {
                auto* c = static_cast<LightCombat*>(l);
                if (c->Kind() == CombatLightKind::Fire || c->Kind() == CombatLightKind::Tracer ||
                    c->Kind() == CombatLightKind::Flare)
                    c->Advance(dt);
            }
        }
    }
    // Research diagnostics: how many combat lights a fight keeps alive.
    static float s_lastLog = -1e9f;
    if (GScene && now - s_lastLog > 2.0f)
    {
        s_lastLog = now;
        int byKind[6] = {0, 0, 0, 0, 0, 0};
        int other = 0;
        for (int i = 0; i < GScene->NLights(); ++i)
        {
            Light* l = GScene->GetLight(i);
            if (!l)
                continue;
            if (l->IsCombat())
                byKind[static_cast<int>(static_cast<LightCombat*>(l)->Kind())]++;
            else
                other++;
        }
        LOG_INFO(Graphics,
                 "[combat] lights muzzle={} cannon={} explosion={} fire={} tracer={} flare={} other={} slots={} night={}",
                 byKind[0], byKind[1], byKind[2], byKind[3], byKind[4], byKind[5], other, GScene->NLights(), s_night);
    }
}

bool AnyActive()
{
    if (!GScene || !Enabled())
        return false;
    for (int i = 0; i < GScene->NLights(); ++i)
    {
        Light* l = GScene->GetLight(i);
        if (l && l->IsOn() && l->IsCombat() && static_cast<LightCombat*>(l)->Intensity() > 1e-5f)
            return true;
    }
    return false;
}

const Light* ShadowCaster(Vector3Par camPos)
{
    if (!GScene || !Enabled())
        return nullptr;
    const LightCombat* best = nullptr;
    float bestScore = 0.0f;
    for (int i = 0; i < GScene->NLights(); ++i)
    {
        Light* l = GScene->GetLight(i);
        if (!l || !l->IsOn() || !l->IsCombat())
            continue;
        const auto* c = static_cast<const LightCombat*>(l);
        if (c->Intensity() <= 1e-5f)
            continue;
        const float d2 = c->Position().Distance2(camPos);
        const float reach = c->Range() + 20.0f;
        if (d2 > reach * reach)
            continue; // lights nothing near the camera
        // The light that adds the most to what the camera sees; anything
        // within 10 m counts as close.
        const float score = c->Intensity() * c->Core() * c->Core() / std::max(d2, 100.0f);
        if (score > bestScore)
        {
            bestScore = score;
            best = c;
        }
    }
    return best;
}

int Gather(ScatterLight* out, int max, Vector3Par camPos)
{
    if (!GScene || max <= 0)
        return 0;
    struct Item
    {
        LightCombat* light;
        float score;
    };
    std::vector<Item> items;
    for (int i = 0; i < GScene->NLights(); ++i)
    {
        Light* l = GScene->GetLight(i);
        if (!l || !l->IsOn() || !l->IsCombat())
            continue;
        auto* c = static_cast<LightCombat*>(l);
        if (c->Intensity() <= 1e-5f)
            continue;
        const float i1 = c->Intensity() * c->Core() * c->Core();
        const float d2 = std::max(c->Position().Distance2(camPos), 1.0f);
        items.push_back({c, i1 / d2});
    }
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.score > b.score; });
    const int n = std::min(static_cast<int>(items.size()), max);
    for (int i = 0; i < n; ++i)
    {
        LightCombat* c = items[i].light;
        const Vector3 p = c->Position();
        const Color rad = c->Radiance();
        out[i].pos[0] = p.X();
        out[i].pos[1] = p.Y();
        out[i].pos[2] = p.Z();
        // Round 7: a fire's glow is spread through the taller, thinner smoke
        // over it (a dense small ball read as a flat disc behind a wreck).
        const float k = c->Kind() == CombatLightKind::Fire ? 0.35f : 1.0f;
        out[i].rgb[0] = rad.R() * k;
        out[i].rgb[1] = rad.G() * k;
        out[i].rgb[2] = rad.B() * k;
        out[i].core = c->Core();
        out[i].range = c->Range();
        // The air that glows: muzzle gas, the dust and smoke of a blast, the
        // smoke over a fire. Lit air beyond it is left to the smoke particles.
        out[i].medium = c->Kind() == CombatLightKind::Fire
                            ? 3.0f * c->Core() + 3.0f
                            : 2.0f * c->Core() + (c->Kind() == CombatLightKind::Muzzle ? 0.6f : 1.5f);
    }
    return n;
}
} // namespace CombatLights

} // namespace Poseidon
