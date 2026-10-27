#pragma once

#include <cstdint>
#include <vector>

// Combat light (the combatLights and combatShadows switches): muzzle flashes, cannon flashes, shell bursts,
// burning fires and wrecks, tracers, flares and missile motors as light
// sources of their own, in linear HDR, with a ball falloff instead of the
// classic quadratic one and timing envelopes per kind.
//
// Pure: no GL, no engine state. The game code spawns and moves the lights in
// the global set (GCombatLights) from its weapon, explosion, wreck, fireplace,
// tracer, flare and missile code, always, whatever the lighting setting: the
// switch never changes the simulation, so a paused frame can be shot with the
// feature off and on. World::Simulate ages them by the simulation step
// (Tick), so a paused game holds them still. The GL33 backend evaluates the
// set once per frame (Evaluate), puts the lights into the frame's tiled light
// list as HDR entries (FrameLights.hpp, FrameLight::hdr), picks up to
// kMaxShadowedCombatLights of them for the screen-space shadow mask
// (UpdateShadowSelection) and scatters the strongest in the air of the post
// pass; Object::DrawDecal lights smoke with them (CombatParticleChannel, a
// fire's light only as far as it carries into its smoke:
// CombatSmokeTransmittance).
//
// The classic light each of these sources already had (Smokes.cpp,
// Fireplace.cpp, Shots.cpp) stays: it draws the glow sprite, and it lights the
// scene whenever the combat light does not (classic lighting, the switch off,
// the gate off). Which of the two lights a frame is decided at light
// selection (Light::Kind() Combat or Fire), never at spawn.

namespace Poseidon::lighting
{

enum class CombatLightKind : std::uint8_t
{
    Muzzle,    // small arms and machine guns (a burst)
    Cannon,    // tank and artillery guns (a burst)
    Explosion, // shells, grenades, satchels, fuel explosions (a burst)
    Fire,      // camp fires and the embers under a burning wreck's smoke
    Tracer,    // a tracer round in flight
    Flare,     // an illuminating flare under its parachute, tinted by its ammo
    Motor,     // a missile's motor while it thrusts, tinted
};
constexpr int kCombatLightKinds = 7;

const char* CombatLightKindName(CombatLightKind kind);
// A burst is fired and forgotten: it lives until its envelope has finished.
// The others belong to an owner that moves them and removes them.
bool CombatLightIsBurst(CombatLightKind kind);

// How long a burst's white-hot flash holds at its peak (seconds of simulation)
// before it falls: a classic shell burst still whitens the frame 0.13 s after
// the burst (four steps at 30 fps), and a flash falling from its birth left
// that frame 14 to 24 levels under Classic at eye height. The hold goes with
// the eye's flash response (FlashExposureGain), which only runs at night:
// CombatTuning::flashHold.
constexpr float kExplosionFlashHold = 0.15f;

// The combat sub-knobs (LightingTuning combatFlash, combatFire, combatSmoke,
// combatScatter): gains on the shipped look, 1 each. The tuned look
// (combatFlash 6, combatFire 2.5, combatSmoke 0.35 over the base envelopes)
// is baked into the constants below.
struct CombatTuning
{
    float flash = 1.0f;   // the white-hot first flash of a burst
    float fire = 1.0f;    // fires and wreck embers
    float smoke = 1.0f;   // combat light on smoke and dust particles
    float scatter = 1.0f; // combat light scattered in the air (post pass)
    // Under the photoreal camera at night: how far a
    // fire's light tints the smoke it lights (the particle's own colour times
    // the light's hue, up to this share where the light is strong), so the
    // column over a camp fire reads fire-lit orange, as the classic fire's
    // decal light made it, not white steam. 0 = additive light only.
    float smokeTint = 0.0f;
    // How long a burst's white-hot flash holds at its peak (s). Under the
    // photoreal camera the hold follows its night (CombatNight), as the flash
    // response it matches does: at dusk the response is off, and a flash held
    // 0.15 s at its peak lit the plain round a shell burst into a band along
    // the horizon in every frame of it; the flash now falls from its birth.
    float flashHold = kExplosionFlashHold;
    // The steady lights' share on surfaces and in the air against the
    // twilight (CombatTwilight): 1, and under the photoreal camera less while
    // the sky still lights the ground. Their flames and smoke keep it all.
    float twilight = 1.0f;
};

// A steady light (a fire, a flare, a motor) against the twilight under the
// photoreal camera: the share of its night power that
// reads against the sky's light on the ground at a sun `sunElevationDeg`.
// The camera meters a dusk plain about as dark as a night field and exposes
// it a few stops brighter than a night, so a wreck's fire at the day scale
// still lit the plain behind it into a warm line along the horizon 1.7
// degrees after sunset. The real twilight's light on the ground falls about
// tenfold per kTwilightDecadeDeg of sun from the horizon to the night; the
// share is 1 from kTwilightNightDeg under (the night as before) and falls
// tenfold per kTwilightDecadeDeg above it, to kTwilightFloor (by day the
// day scale alone).
constexpr float kTwilightNightDeg = -6.0f;
constexpr float kTwilightDecadeDeg = 2.5f;
constexpr float kTwilightFloor = 0.1f;
float CombatTwilight(float sunElevationDeg);

// One light at one age: E(d) = intensity * window(d) / (1 + d^2 / core^2), a
// ball of radius `core` rather than a point, and the window a smooth end at
// `range` (where E falls to about kCombatRangeFloor).
struct CombatLightSample
{
    float rgb[3] = {0.0f, 0.0f, 0.0f}; // linear colour, luminance 1
    float intensity = 0.0f;            // core intensity
    float core = 1.0f;                 // r0 (m)
    float range = 1.0f;                // window end (m)
    bool finished = false;             // a burst whose envelope is over
};

// Irradiance (linear, luminance) where a light's window ends: an earlier
// 0.0025 (about the classic night ambient) lit far ground behind fires into a
// warm strip along the horizon; a light ends where it adds a few levels.
constexpr float kCombatRangeFloor = 0.006f;
constexpr float kCombatMaxRange = 120.0f;   // bursts and fires
constexpr float kCombatFlareRange = 400.0f; // a flare hangs 50 to 150 m up

// The envelope of a light `age` seconds after its birth, before the day
// scaling. `strength` is the source's (CombatStrength*), `seed` its own
// flicker phase.
CombatLightSample EvaluateCombatLight(CombatLightKind kind, float strength, float age, std::uint32_t seed,
                                      const CombatTuning& tuning);

// The remastered exposure does not adapt between day and night, so a light
// of fixed power reads by day as it does at night. Continuous lights almost
// vanish by day (0.12), flashes keep a fifth of their punch (half
// whitened the ground around a noon shell burst by 11 levels).
float CombatDayScale(CombatLightKind kind, float nightEffect);
// The night the day scale and the flash response follow. Classic's night
// effect is 1 as soon as the sun is under the horizon; the photoreal camera
// (PhotoCamera.hpp CameraNight) still exposes a twilight there, its sky still
// lighting the ground, and counts the night from 4.5 degrees under. Under that
// camera a dusk fire and shell burst lit at full night power drew a strip
// brighter than the sky along the horizon, and every rifleman's flash an oval
// on the ground: the lights follow the camera's night there, as its fog's
// lamp halos do. `cameraNight` is read only while `photoCamera` is on.
float CombatNight(float classicNight, bool photoCamera, float cameraNight);

// How much of a combat light's ball (radius `core`, its centre `d` metres off
// and at cos `cosTheta` to a surface's normal) is above that surface's
// horizon, as the share of a point light's full irradiance (cos 0 = 1): the
// irradiance of a sphere light clipped at the horizon (Lagarde and de
// Rousiers 2014, "Moving Frostbite to PBR", 4.7.2) over that of the whole
// sphere face on. cosTheta itself while the ball is wholly above the horizon;
// (1 + cosTheta) / 2 at its surface and inside it (a flash at the muzzle
// lights the shooter's arm all round); 0 once it is wholly below. A ball low
// over flat ground lights the ground far from it at a grazing angle: about
// 2 core / (3 pi d) of its light for one centred on the ground, where the
// former wrap (cos + core / d) / (1 + core / d) gave about core / d, five
// times as much, which drew the lit plain behind a burst or a fire as a band
// along the horizon. The shaders' combatLights computes the same
// (EngineGL33_LightingShadersLights.cpp).
float CombatBallCosine(float cosTheta, float core, float d);

// Strength of a muzzle or cannon flash from the fired ammo's `hit` (config);
// without one (hit <= 0) from the classic flash intensity (soldier 0.004,
// vehicle machine gun 0.4, main gun 1).
float CombatMuzzleStrength(float ammoHit, bool machineGun, float classicIntensity);

// The isotropic irradiance (luminance) of a light at squared distance d2.
float CombatIrradiance(const CombatLightSample& s, float d2);

// A smoke or dust particle's display-space channel `display` lit by combat
// irradiance `irradiance` of that channel: added in linear light with a soft
// saturation (a particle in the fireball's core glows, the cloud around it
// shades off instead of clipping to flat cream), times the smoke gain.
float CombatParticleChannel(float display, float irradiance, float smokeGain);
// The same on a colour, keeping the light's hue: the soft
// saturation is taken on the light's peak channel and scales every channel
// alike, so a puff in a fire's light glows the fire's colour however close it
// is. Per channel the three saturated toward one value, and the column over a
// camp fire went to white steam. `tint` (CombatTuning::smokeTint) deepens the
// light toward its own hue (each channel's share of the peak to the power
// 2.2) as it grows: what lights a pale smoke from inside has crossed it. The
// result is in display space and may pass 1 (KeepHueUnderOne).
void CombatParticleLight(const float display[3], const float irradiance[3], float smokeGain, float tint, float out[3]);
// The combat lights on one puff: a fire's light keeps its hue
// (CombatParticleLight, with the tint), every other light (a burst's
// white-hot flash, a muzzle flash, a flare, a tracer) saturates channel by
// channel toward white (CombatParticleChannel), as a fireball's incandescent
// core does. With every light hue-kept a dusk shell burst's cloud went one
// flat mustard yellow. `fire` and `other` are the two lights' irradiance.
void CombatParticleMix(const float display[3], const float fire[3], const float other[3], float smokeGain, float tint,
                       float out[3]);
// A display colour brought under 1 by its peak channel, so an 8-bit vertex
// colour keeps its hue instead of clipping each channel to white.
void KeepHueUnderOne(float rgb[3]);
// The flame's body: the light (linear rgb) a puff of smoke
// `d` metres from a fire's light emits as part of the flames (incandescent
// soot), added to what it scatters and not scaled by its own colour. A
// yellow-orange glow that falls off within a metre or so of the light (3
// core radii); zero for every other kind and outside it.
void CombatFlameEmission(CombatLightKind kind, const CombatLightSample& s, float d, float out[3]);

// A shell burst's fireball: the puffs of an explosion
// (CfgCloudlets Explosion, a gun's fire) carry a temperature in kelvin that
// cools from 2500 K. Classic lit them past white with the explosion's own
// light; under the combat lights they were lit smoke of their table's colour
// (lemon at 1900 K, mustard at 1600 K) at the scene's light, so a dusk burst
// read as a flat mustard cloud with no hot core and then a salmon smear. A
// puff whose table tops out (`hottestK`) between kFireballTableK and
// kFireballTableMaxK is a fireball's, in kelvin (a camp fire's smoke and a smoke
// shell's top out at 1000, grey and orange; a gun's fire runs its own scale to
// 4200, orange at 2800 and grey smoke under 2300, and keeps its look): it
// glows as a hot body at its temperature, the colour of a black body (red-
// orange to yellow-white) at kFireballEmission times exp(kFireballWienK (1 /
// kFireballRefK - 1 / T)), Wien's law at 580 nm (its visible light climbs
// about twelvefold from 1800 K to 2200 K, so only the hottest puffs of a cloud
// glow white and the rest dim to orange and red inside their soot; a power of
// 8 on the table's colour lit the whole cloud one yellow at 0.17 s), faded in
// from kFireballGlowFromK to kFireballGlowFullK. Each channel saturates softly
// at kFireballEmitCap (linear, the puff's colour and its glow), as film does,
// so the hottest core goes white-yellow. What it scatters is soot
// (kFireballSoot, display): it cools from a white-hot core through orange to
// dark grey-brown smoke. Returns false (outputs untouched) for any other puff.
constexpr float kFireballTableK = 1500.0f;
constexpr float kFireballTableMaxK = 3000.0f;
constexpr float kFireballGlowFromK = 900.0f;
constexpr float kFireballGlowFullK = 1300.0f;
constexpr float kFireballRefK = 2200.0f;
constexpr float kFireballWienK = 24800.0f; // c2 / 580 nm
constexpr float kFireballEmission = 8.0f;
constexpr float kFireballEmitCap = 2.0f;
constexpr float kFireballSoot[3] = {0.24f, 0.21f, 0.18f};
// The linear colour of a black body at `temperatureK` (1000 to 3000 K, red
// channel 1): a fit to the sRGB black-body table (1500 K (1, 0.155, 0), 2000 K
// (1, 0.255, 0.002), 2500 K (1, 0.36, 0.06), 3000 K (1, 0.47, 0.15)).
void BlackBodyRgb(float temperatureK, float out[3]);
bool FireballPuff(float temperatureK, float hottestK, float albedo[3], float emit[3]);

// Detonation physics: what a munition's charge makes of its
// flash, fireball and dust column, and a burning vehicle's fuel of its
// fireball. Classic sized every blast from `hit` (the round's direct damage),
// clamped to 0.25 .. 1, and ran the same 0.05 s of white-hot puffs for all:
// a 120 mm HEAT round (hit 700) burst bigger than the 120 mm HE shell (hit
// 200), a grenade looked like a small shell, and every blast read as a blink.
//
// The charge (TNT equivalent, kg) comes from the blast the config gives the
// round: the engine's own crater energy, indirectHit x indirectHitRange
// (Landscape::ExplosionDammage), over kChargeEnergyPerKg, calibrated so the
// 120 mm HE shell (150 x 8) is 2.5 kg, about a 120 mm HE round's filling;
// the hand grenade (18 x 7) then comes to 0.26 kg and the 120 mm HEAT round
// (300 x 1) to 0.63 kg. A round whose direct hit exceeds its blast energy is
// a shaped charge (HEAT, rockets): its jet takes much of the energy, so its
// fireball is kShapedRadiusShare of a plain charge's and its flash shorter.
//
// Blast scaling (Hopkinson-Cranz): lengths and times go with the cube root of
// the charge, s = W^(1/3). The detonation products expand to about
// kFireballRadiusPerS x s metres (1 kg TNT: a 1.6 m ball) and glow white-hot,
// a brightness temperature of kDetonationPeakK, cooling with an e-folding
// time of kFlashTimePerS x s (tens of milliseconds). A cased charge with a
// negative oxygen balance (TNT -74 %, Composition B -43 %) burns its fuel-rich
// products in the air behind the flash: an orange afterburn at kAfterburnK for
// kAfterburnTimePerS x s; a shaped charge (Octol, about -24 %) has a third of
// it. Then the gas cools to soot within kGlowFallTime. The blast throws a dust
// and smoke column kColumnHeightPerS x s high that lingers kColumnLifePerS x s
// seconds and drifts with the wind.
//
// A vehicle's fuel does not detonate; its tank bursts and the spray burns as a
// deflagration. The fireball of a burning mass M kg of hydrocarbon (CCPS /
// TNO fireball correlations): diameter kFuelDiameterPerS x M^(1/3) m and
// duration kFuelDurationPerS x M^(1/3) s; a sooty flame of kFuelFlameK, slower,
// cooler and darker than any detonation, then thick black smoke.
constexpr float kChargeEnergyPerKg = 480.0f;
constexpr float kMinChargeKg = 0.02f;
constexpr float kMaxChargeKg = 500.0f;
constexpr float kFireballRadiusPerS = 1.6f;
constexpr float kShapedRadiusShare = 0.75f;
constexpr float kFlashTimePerS = 0.03f;
constexpr float kShapedFlashShare = 0.75f;
constexpr float kAfterburnTimePerS = 0.12f;
constexpr float kShapedAfterburnShare = 0.33f;
constexpr float kDetonationPeakK = 3200.0f;
constexpr float kAfterburnK = 2000.0f;
constexpr float kGlowFallTime = 0.08f;
constexpr float kSootK = 900.0f;
constexpr float kColumnHeightPerS = 6.0f;
constexpr float kColumnLifePerS = 12.0f;
constexpr float kShapedColumnShare = 0.7f;
// The engine's classic blast size (Shots.cpp: hit x 0.003, clamped 0.25 .. 1)
// of the reference shell, for a blast that arrives without its round (a
// network copy): size 0.6 = the 2.5 kg shell.
constexpr float kReferenceBlastSize = 0.6f;
constexpr float kReferenceChargeKg = 2.5f;
// Fuel: FuelExplosion's hit (CfgAmmo 100, the engine scales it by the vehicle's
// explosives: fuel litres x 0.1 plus its ammunition) to kg of fuel in the
// fireball: hit / 0.1 litres x 0.74 kg/l, half of it burning in the fireball
// (the rest feeds the wreck's fire).
constexpr float kFuelKgPerHit = 3.7f;
constexpr float kMinFuelKg = 1.0f;
constexpr float kMaxFuelKg = 150.0f; // a heavy vehicle's tank (about 400 l), half in the ball
constexpr float kFuelDiameterPerS = 5.8f;
constexpr float kFuelDurationPerS = 0.45f;
constexpr float kFuelFlameK = 1900.0f;
constexpr float kFuelRiseShare = 0.2f;
constexpr float kFuelColumnLife = 20.0f;
struct Detonation
{
    float chargeKg = kReferenceChargeKg; // TNT equivalent; a deflagration's fuel burnt
    float scale = 1.0f;                  // the cube root of chargeKg
    bool shaped = false;                 // a shaped charge (HEAT, rockets)
    bool deflagration = false;           // a fuel fireball
    float radius = 1.6f;                 // the luminous fireball at its largest (m)
    float flashTime = 0.03f;             // the flash's e-folding time (s)
    float burnTime = 0.12f;              // afterburn, or the burning fuel fireball (s)
    float peakK = kDetonationPeakK;      // brightness temperature at its birth (K)
    float burnK = kAfterburnK;           // what the burning gas glows at (K)
    float columnHeight = 6.0f;           // the dust and smoke column (m)
    float columnLife = 12.0f;            // how long the column lingers (s)
};
// From the round's config (CfgAmmo hit, indirectHit, indirectHitRange).
Detonation DetonationFromAmmo(float hit, float indirectHit, float indirectHitRange);
// From the classic blast size alone (0.25 .. 1): s = 2.5^(1/3) x size / 0.6.
Detonation DetonationFromBlastSize(float blastSize);
// A fuel fireball from the kg of fuel that burns in it.
Detonation DeflagrationFromFuel(float fuelKg);
// The gas temperature (K) `age` seconds after the detonation: the flash's
// peakK falling with flashTime onto the afterburn's burnK, held burnTime, then
// falling to soot (kSootK) with kGlowFallTime. A deflagration rises to its
// flame over kFuelRiseShare of its burn, holds, then cools.
float DetonationTemperature(const Detonation& d, float age);
// How fast a fireball puff born at `bornK` cools (K/s, negative): to the end
// of the glow (1100 K) by the end of the burn, and over no less than
// kMinGlowSpan: a camera's 1/30 s shutter holds a flash shorter than its
// frame for the whole frame (a HEAT round's 19 ms flash was gone before its
// first frame was drawn).
constexpr float kMinGlowSpan = 0.1f;
float DetonationPuffCooling(const Detonation& d, float bornK);
// The detonation's combat light `age` seconds after it (before the day
// scaling): the visible power of a hot ball of its radius at its temperature
// (Wien at 580 nm against the peak), in the black body's colour, its ball
// kDetonationCoreShare of the fireball's radius. The flash at the peak is
// kDetonationFlashI1 x W^(2/3) (radiating area), a fuel fireball
// kDeflagrationI1 x M^(2/3) at its flame.
constexpr float kDetonationFlashI1 = 350.0f;
constexpr float kDeflagrationI1 = 60.0f;
constexpr float kDetonationCoreShare = 0.5f;
CombatLightSample EvaluateDetonationLight(const Detonation& d, float age, std::uint32_t seed,
                                          const CombatTuning& tuning);

// A fire burns inside the foot of its own smoke, and the light reaching a
// particle of that smoke has crossed the smoke below it. Soot drinks what it
// does not scatter: under a burning wreck's black column only the base glows
// and the column above stays dark against the night sky, as Classic draws it
// (lit by the embers at full strength, the column read at sky level). Pale
// smoke scatters the light on, so a camp fire's grey smoke is lit higher up.
// The reach (m) is kSootReachCores core radii for black soot, plus
// kPaleReachCores times the square of the particle's albedo (the luminance of
// its own colour, 0 to 1).
constexpr float kSootReachCores = 1.5f;
constexpr float kPaleReachCores = 14.0f;
float CombatSmokeReach(float core, float albedo);
// The share of a light's irradiance that reaches a smoke or dust particle `d`
// metres from it: for a fire or a shell burst 1 within its core, then
// exp(-(d - core) / reach) (lit alike, every puff of a burst's own cloud
// showed one flat colour; it now lights the cloud's base and near side); a muzzle or cannon flash, a tracer, a flare or
// a motor lights the smoke around it from the air, 1.
float CombatSmokeTransmittance(CombatLightKind kind, const CombatLightSample& s, float d, float albedo);

// The steady lights that have a classic light behind them: fires, flares and
// missile motors. While the combat lights are on the classic light of each of
// these sources lights nothing, so a steady light cut from the frame's list
// would light nothing at all, and a camp fire 100 m off (score 0.00175) would
// pop in and out as near tracers (0.0025 at 10 m) and muzzle flashes came and
// went.
bool CombatLightIsSteady(CombatLightKind kind);
// The candidates the frame's list keeps, at most `cap`: the `kinds` of `n`
// candidates sorted strongest at the camera first (ShadowScore); returns the
// indices kept, in that order: every steady light (the strongest `cap` if
// there are more), then the strongest of the rest in the slots left.
std::vector<int> KeepCombatLights(const CombatLightKind* kinds, int n, int cap);

// The flash response of the eye: an exposure gain while a burst's flash lights
// the camera, proportional to its irradiance there (luminance) and to the
// night, softly limited to kFlashExposureMax. 1 with no flash. Physically the
// flash lights the ground near a camera at eye height at a grazing angle, so
// the frame of a shell burst read darker than Classic's even whiteout (37
// against 63 in a measured pair); the response brings it back. It holds a moment
// after the flash (FlashHeldIrradiance): a classic burst whitened two or three
// frames.
constexpr float kFlashExposurePerIrradiance = 3.0f;
constexpr float kFlashExposureMax = 4.0f;
constexpr float kFlashExposureHold = 0.12f; // seconds of simulation
float FlashExposureGain(float flashIrradianceAtCamera, float nightEffect);
// The kinds whose flash opens the exposure: shell bursts and cannon, not small
// arms (a squad's fire would pump the frame).
bool CombatLightOpensExposure(CombatLightKind kind);
// The irradiance the response follows: this frame's flash, or the held one
// decayed by `dt` seconds of simulation since the last frame (0 while paused),
// whichever is larger.
float FlashHeldIrradiance(float held, float flashNow, float dt);

// The shadowed lights: up to kMaxShadowedCombatLights, one per channel of the
// shadow mask, picked with hysteresis so the shadow does not flip between two
// fires whose flicker crosses: a shadowed light keeps its channel unless
// another light exceeds it by kShadowSwapRatio for kShadowSwapTime seconds of
// simulation; an empty channel takes the strongest candidate at once.
constexpr int kMaxShadowedCombatLights = 4;
constexpr float kShadowSwapRatio = 1.5f;
constexpr float kShadowSwapTime = 0.3f;

// LightingTuning::debugView value that shows the combat shadow mask as each
// surface reads it (shadeRemastered): the visibility of the first shadowed
// light in grey, red where the fragment falls outside last frame's mask.
constexpr int kDebugViewCombatMask = 11; // after the reflections' two (ScreenReflections.hpp)

struct ShadowCandidate
{
    std::uint32_t id = 0; // CombatLightRecord::id
    float score = 0.0f;   // how much the light adds to what the camera sees
};

struct ShadowSelection
{
    std::uint32_t ids[kMaxShadowedCombatLights] = {}; // per channel, 0 = empty
    std::uint32_t challenger = 0;                     // the light waiting to take a channel
    float challengeTime = 0.0f;                       // how long it has exceeded the weakest
};

// The score of a light seen from `d2` square metres away: its power over the
// distance squared, anything within 10 m counting as close.
float ShadowScore(const CombatLightSample& s, float d2);

// Updates the channels from this frame's candidates (any order), `dt` seconds
// of simulation after the last update (0 while paused). `slots` channels are in
// use (1 to kMaxShadowedCombatLights). Returns the channel of each candidate
// in `channel` (0 to slots - 1, or -1), when not null.
void UpdateShadowSelection(ShadowSelection& sel, const ShadowCandidate* candidates, int n, float dt,
                           int slots = kMaxShadowedCombatLights, int* channel = nullptr);

// One light of the set.
struct CombatLightRecord
{
    std::uint32_t id = 0;
    CombatLightKind kind = CombatLightKind::Fire;
    float strength = 1.0f;
    float pos[3] = {0.0f, 0.0f, 0.0f};  // world (m)
    float age = 0.0f;                   // seconds of simulation since its birth
    std::uint32_t seed = 0;             // its flicker phase
    float tint[3] = {1.0f, 1.0f, 1.0f}; // flares and motors: the ammo's light colour
    bool owned = false;                 // moved and removed by its owner
    bool detonation = false;            // a blast with its own physics (det)
    Detonation det;
    CombatLightSample sample; // this frame's (Evaluate)
};

// The most one Tick ages the set by. A longer step (time acceleration at a low
// frame rate: setAccTime 4 under 16 fps passes 0.27 s; a scripted time pin or a
// load) ages it by this much, which ends a muzzle burst and moves the others on.
constexpr float kCombatMaxTickStep = 0.25f;

class CombatLightSet
{
  public:
    // A burst at a world position.
    std::uint32_t Spawn(CombatLightKind kind, const float pos[3], float strength);
    // A blast (kind Explosion) whose light follows its detonation physics.
    std::uint32_t SpawnDetonation(const float pos[3], const Detonation& d);
    // A light its owner moves (Update) and removes (Remove).
    std::uint32_t Create(CombatLightKind kind, const float pos[3], float strength);
    bool Update(std::uint32_t id, const float pos[3], float strength);
    void SetTint(std::uint32_t id, float r, float g, float b);
    void Remove(std::uint32_t id);
    // Ages every light by a simulation step, at most kCombatMaxTickStep, and
    // drops the finished bursts. A step of 0 (paused) ages nothing.
    void Tick(float dt);
    // This frame's samples, day scaling included; keeps the tuning for the
    // frame's other readers (the smoke gain, the scatter gain).
    void Evaluate(const CombatTuning& tuning, float nightEffect);
    const CombatTuning& Tuning() const { return _tuning; }
    // This frame's day scale of a burst (CombatDayScale): a fireball's glow
    // follows it, so by night its flames stay brighter than
    // the smoke its light lights.
    float BurstDayScale() const { return _burstDayScale; }
    // Drops the bursts at once (they end on their own within 0.9 s of
    // simulation): the scene's teardown (Scene::ResetLights, called from
    // ~Scene only).
    void ClearBursts();
    void Clear();
    // Seconds of simulation ticked so far (the hysteresis clock).
    double SimTime() const { return _simTime; }
    const std::vector<CombatLightRecord>& Records() const { return _records; }
    const CombatLightRecord* Find(std::uint32_t id) const;

  private:
    CombatLightRecord* FindMutable(std::uint32_t id);
    std::uint32_t Add(CombatLightKind kind, const float pos[3], float strength, bool owned);
    std::vector<CombatLightRecord> _records;
    CombatTuning _tuning;
    float _burstDayScale = 0.2f;
    std::uint32_t _nextId = 1;
    std::uint32_t _seed = 0x9e3779b9u;
    double _simTime = 0.0;
};

// The game's set.
CombatLightSet& GCombatLights();

// An owner's light in the game's set: created on the first Set, moved by the
// next ones, removed by Release or the destructor. Copying an owner does not
// copy its light: a copy starts without one.
class CombatLightHandle
{
  public:
    CombatLightHandle() = default;
    ~CombatLightHandle() { Release(); }
    CombatLightHandle(const CombatLightHandle&) {}
    CombatLightHandle& operator=(const CombatLightHandle& other)
    {
        if (this != &other)
            Release();
        return *this;
    }
    bool Active() const { return _id != 0; }
    std::uint32_t Id() const { return _id; }
    void Set(CombatLightKind kind, const float pos[3], float strength);
    void SetTint(float r, float g, float b);
    void Release();

  private:
    std::uint32_t _id = 0;
};

// The shadow mask's mapping (the forward shader's tap): rows that take a
// camera-relative position p of this frame to the half-resolution mask of the
// previous frame, u = (row0 . p + row0.w) / z, v = (row1 . p + row1.w) / z,
// z = row2 . p + row2.w (the previous view depth). The mask texel (i, j) is
// the 2x2 block of frame-target pixels from (2i, 2j), so a frame-target pixel
// coordinate x maps to the mask coordinate x / 2 and its uv to x / (2 * half
// width), with the half size (w + 1) / 2: on an odd frame size the last texel
// covers one pixel and the uv is scaled, never shifted.
struct CombatMaskCamera
{
    double pos[3] = {0.0, 0.0, 0.0}; // camera world position
    // World to view rotation, as rows: view x right, y up, z forward.
    float rot[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    float p11 = 1.0f, p22 = 1.0f;             // ndc.x = p11 x / z, ndc.y = p22 y / z
    float rect[4] = {0.0f, 0.0f, 1.0f, 1.0f}; // the 3D viewport in uv of the frame target
    int targetW = 0, targetH = 0;             // frame target (pixels)
};
int CombatMaskSize(int fullSize);
// The rows for a fragment of the camera at `curPos` (this frame), looking up
// the mask built from `prev`'s view.
void CombatMaskRows(const CombatMaskCamera& prev, const double curPos[3], float rows[3][4]);
// The mask uv of camera-relative point p through the rows (the shader's
// combatShadowVis); false behind the previous camera.
bool CombatMaskUv(const float rows[3][4], const float p[3], float uv[2]);
// The previous view's position of a mask texel (the mask pass's receiver):
// the centre of its 2x2 block of frame-target pixels at view depth z.
void CombatMaskTexelView(const CombatMaskCamera& prev, int i, int j, float z, float out[3]);

} // namespace Poseidon::lighting
