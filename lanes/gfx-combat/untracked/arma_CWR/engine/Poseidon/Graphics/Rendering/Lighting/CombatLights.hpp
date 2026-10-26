#pragma once

// RESEARCH PROTOTYPE (track "combat"): emissive combat light for the remastered
// lighting. Muzzle flashes, cannon flashes, explosions, fires and tracers become
// short-lived HDR point lights with a physical falloff (I / (1 + d^2 / r0^2),
// windowed at a range), quick attack, exponential decay and flicker. They are
// uploaded to the remastered shaders through the ordinary local-light slots
// (flagged by LightDescription::hdrRange > 0), light smoke particles on the
// CPU, and feed an analytic in-scatter term in the post pass.
//
// Only active when the remastered lighting is on and the tuning knob
// combatLights > 0: Classic keeps its original light code paths untouched.

#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>

#include <cstdint>

namespace Poseidon
{

enum class CombatLightKind : std::uint8_t
{
    Muzzle,    // small arms and machine guns
    Cannon,    // tank and artillery guns
    Explosion, // shells, grenades, satchels, fuel explosions
    Fire,      // campfires, burning wrecks (continuous)
    Tracer,    // a tracer round in flight (continuous)
    Flare,     // an illuminating flare under its parachute (continuous, tinted)
};

/// Pure envelope math (no engine state) so the look can be reasoned about and
/// unit tested: radiance colour (linear, unit luminance), intensity at the core
/// (linear radiance multiplier), core radius and range for one light at `age`
/// seconds after its birth.
struct CombatLightSample
{
    float r = 0, g = 0, b = 0; // linear colour, luminance 1
    float intensity = 0;       // core intensity: E(d) = intensity / (1 + d^2/r0^2)
    float core = 1;            // r0 (m)
    float range = 1;           // window end (m)
    bool finished = false;
};

CombatLightSample EvaluateCombatLight(CombatLightKind kind, float strength, float age, std::uint32_t seed);

class LightCombat : public LightPoint
{
    typedef LightPoint base;

public:
    LightCombat(CombatLightKind kind, float strength);

    /// Age the light by dt seconds (sim time, clamped against clock jumps) and
    /// recompute colour and intensity. Returns false once a burst has finished
    /// (continuous lights never finish on their own).
    bool Advance(float dt);

    bool IsCombat() const override { return true; }
    float Brightness() const override;
    float SortBrightness() const override { return Brightness(); }
    Color Apply(Vector3Par point, Vector3Par normal) override;
    void GetDescription(LightDescription& desc) const override;
    void ToDraw(ClipFlags, bool) override {}

    /// Linear irradiance (rgb) this light delivers at a world position,
    /// isotropic (no normal): used to light smoke particles.
    Color IrradianceAt(Vector3Par worldPos) const;

    void SetStrength(float s) { _strength = s; }
    /// Flares: the colour from the ammo's lightColor (any brightness).
    void SetTint(float r, float g, float b)
    {
        _tint[0] = r;
        _tint[1] = g;
        _tint[2] = b;
    }
    float Strength() const { return _strength; }
    CombatLightKind Kind() const { return _kind; }
    float Intensity() const { return _sample.intensity; }
    float Core() const { return _sample.core; }
    float Range() const { return _sample.range; }
    Color Radiance() const { return Color(_sample.r, _sample.g, _sample.b) * _sample.intensity; }

private:
    CombatLightKind _kind;
    float _strength;
    float _age = 0.0f;
    std::uint32_t _seed;
    CombatLightSample _sample;
    float _tint[3] = {1.0f, 1.0f, 1.0f};
};

namespace CombatLights
{
/// Remastered lighting active and the combatLights knob > 0.
bool Enabled();
/// combatLights knob (overall intensity gain).
float Gain();
/// Per-knob gains read from the tuning (smoke lighting, in-scatter).
float SmokeGain();
float ScatterGain();
float WreckGain();

/// Fire-and-forget burst (muzzle, cannon, explosion) at a world position.
void Spawn(CombatLightKind kind, Vector3Par pos, float strength);
/// A light the caller owns and moves (fires, tracers). Registered with the
/// scene; dropping the last Ref removes it.
Ref<LightCombat> CreateOwned(CombatLightKind kind, float strength);
/// Per frame, before light selection: advance every combat light, drop the
/// finished bursts.
void Tick();
/// True when at least one combat light is alive (lets the scene select lights
/// in daylight, where the classic local lights are skipped).
bool AnyActive();

struct ScatterLight
{
    float pos[3];   // world position
    float rgb[3];   // linear radiance at the core
    float core;     // r0
    float range;
    float medium;   // radius of the lit smoke/dust around the light (m)
};
/// The combat light that traces a screen-space shadow this frame (the one
/// adding the most to what the camera sees), or null.
const Light* ShadowCaster(Vector3Par camPos);

/// Up to `max` combat lights for the post-pass in-scatter, strongest first
/// (as seen from `camPos`). Returns the count.
int Gather(ScatterLight* out, int max, Vector3Par camPos);
} // namespace CombatLights

} // namespace Poseidon
