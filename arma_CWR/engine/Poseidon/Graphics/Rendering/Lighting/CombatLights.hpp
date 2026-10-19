#pragma once

#include <cstdint>
#include <vector>

// Combat light (INTEGRATION-PLAN.md section 5 B4, the combatLights and
// combatShadows switches): muzzle flashes, cannon flashes, shell bursts,
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
// combatScatter): gains on the shipped look, 1 each. The research's round-9
// look (its combatFlash 6, combatFire 2.5, combatSmoke 0.35) is the shipped
// look, baked into the constants below.
struct CombatTuning
{
    float flash = 1.0f;   // the white-hot first flash of a burst
    float fire = 1.0f;    // fires and wreck embers
    float smoke = 1.0f;   // combat light on smoke and dust particles
    float scatter = 1.0f; // combat light scattered in the air (post pass)
    // Under the photoreal camera at night (look director round 2): how far a
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
};

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

// Irradiance (linear, luminance) where a light's window ends: the research's
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
// vanish by day (0.12), flashes keep a fifth of their punch (the research's
// half whitened the ground around a noon shell burst by 11 levels).
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
// The same on a colour, keeping the light's hue (look fix "fire"): the soft
// saturation is taken on the light's peak channel and scales every channel
// alike, so a puff in a fire's light glows the fire's colour however close it
// is. Per channel the three saturated toward one value, and the column over a
// camp fire went to white steam. `tint` (CombatTuning::smokeTint) deepens the
// light toward its own hue (each channel's share of the peak to the power
// 2.2) as it grows: what lights a pale smoke from inside has crossed it. The
// result is in display space and may pass 1 (KeepHueUnderOne).
void CombatParticleLight(const float display[3], const float irradiance[3], float smokeGain, float tint, float out[3]);
// The combat lights on one puff (look director round 3, where the "fire"
// and "horizon" fixes meet): a fire's light keeps its hue
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
// The flame's body (look fix "fire"): the light (linear rgb) a puff of smoke
// `d` metres from a fire's light emits as part of the flames (incandescent
// soot), added to what it scatters and not scaled by its own colour. A
// yellow-orange glow that falls off within a metre or so of the light (3
// core radii); zero for every other kind and outside it.
void CombatFlameEmission(CombatLightKind kind, const CombatLightSample& s, float d, float out[3]);

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
// metres from it: for a fire 1 within its core, then exp(-(d - core) /
// reach); a flash, a tracer, a flare or a motor lights the smoke around it
// from the air, 1.
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
// against 63 in the research); the response brings it back. It holds a moment
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
    CombatLightSample sample;           // this frame's (Evaluate)
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
