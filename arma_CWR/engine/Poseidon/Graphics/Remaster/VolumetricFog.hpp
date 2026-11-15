#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <memory>
#include <vector>

// Volumetric fog and atmosphere (feature switch volFog; the GL side is
// EngineGL33_PostFog.cpp and EngineGL33_PostFogShaders.cpp).
//
// The flat distance fog of the original (a linear blend toward one colour
// between two distances) is replaced by a medium that fills a froxel volume,
// a grid of cells aligned with the screen and sliced in depth, every frame:
//
//   - haze: aerosol extinction thinning with height (scale height
//     kFogHazeHeight), thin on a clear day, thicker under a grey sky and in
//     rain;
//   - Rayleigh air: the sea-level scattering of air molecules times
//     kRayleighScale (the islands are small: the blue of 20 km of air shows
//     over a few), strongest in blue and lit by the sky above the horizon, so
//     far ridges turn lighter and bluer;
//   - humid ground air: tens of metres over the ground on a misty morning, at
//     night, under a grey sky, in rain and in fog;
//   - weather fog: setFog and rain, at the density that gives the classic
//     engine's tactical visibility (WeatherVisibility) at sea level, thinning
//     upward so hilltops stand out of it;
//   - valley mist: a flat-topped layer over the lowest ground nearby (the fog
//     floor, BuildFogFloor) as deep as a share of the local relief
//     (BuildFogLayer: deep valleys hold a layer, plains, the open sea and
//     ridges none), on the mornings the weather seeds one (FogMistEvent),
//     under a grey sky, in rain and in fog, broken into drifting banks.
//
// The haze and the air take the sky's own colour at the horizon in their
// azimuth (the sky table's aerial perspective, with its glow toward the
// sun), so the far end of every line of sight melts into the sky behind it.
// The share of that light that is scattered sunlight (most of it under a
// clear sky, little under an overcast) goes dark in the shade of the shadow
// cascades, the long canopy shadows, the terrain horizon map and the cloud
// field (ridges shade the haze behind them). The sun's forward lobe (beams
// through trees, the glow toward a low sun) and the lamps, headlights and
// fires are added per pixel in a half-resolution pass in the first metres
// (EngineGL33_PostFog.cpp), where the volume's cells are too coarse for a
// shaft or a lamp's halo. The sky takes the volume where the weather fog,
// the mist and the lamps' glow fill it, and only at the horizon in clear air
// (the dome holds the clear atmosphere above it).
//
// The forward shaders keep the classic fog only for the view-distance edge
// (VolFogEdgeScale): the world's end and object fade-in stay hidden,
// everything before them is the volume's. AI sight takes the same medium and
// the same edge in place of the classic tactical fog (FogSightTransmittance),
// so the player and the AI see through the same air.
//
// Pure: no GL, no engine state, so the medium's rules, the slice layout and
// the fog floor can be tested.

namespace Poseidon::remaster
{

// ---- The froxel grid ---------------------------------------------------------

struct FogGrid
{
    int cellsX = 0, cellsY = 0; // over the 3D viewport
    int slices = 0;             // depth slices, a multiple of kFogSlicesPerDraw
    int cellPixels = 0;         // frame-target pixels per cell side
};

// The injection and the integration each write this many slices per draw
// (eight colour attachments, the GL 3.3 minimum).
constexpr int kFogSlicesPerDraw = 8;
// The injection samples each cell at two fixed points and never moves them:
// depth strata at a quarter and three quarters of the slice (each weighted by
// the metres of its half of the slice), on the two diagonal quarter points of
// the cell's footprint (in cells from its centre). The history keeps 0.88 of
// the last frame, read at the cell's centre through a fixed tetrahedral blur
// of kFogHistoryBlur cells and slices per axis, which gives the mist its
// softness. Nothing in the sampling changes from frame to frame, so the
// history only smooths the light's own changes and has no residual to pump
// (one jitter offset for the whole frame makes the volume flicker) or to slide
// through the fog (a per-cell offset pattern stepped every frame reads as
// wind). The shader's constants mirror these.
constexpr float kFogTapZ[2] = {0.25f, 0.75f};
constexpr float kFogTapXY[2][2] = {{-0.25f, -0.25f}, {0.25f, 0.25f}};
constexpr float kFogHistoryBlur = 0.25f;
// The grid for a viewport of w x h pixels at a quality (0 coarse, 1 shipped,
// 2 fine; the fogQuality knob, rounded and clamped): cells of 16, 12 or 8
// pixels, 48, 64 or 96 slices.
FogGrid FogGridFor(int w, int h, float quality);
// The near-field pass (the sun's beams): its march steps at a quality (12,
// 20 or 32).
int FogNearSteps(float quality);

// Depth slices are exponential: the view depth (metres along the view axis) of
// the slice coordinate t in [0, 1] is far * (exp(k t) - 1) / (exp(k) - 1), so
// the cells near the eye are about a metre deep and the last ones hundreds.
float FogSliceDepth(float t, float zFar, float k);
// The inverse: the slice coordinate of a view depth (clamped to [0, 1]).
float FogSliceCoord(float z, float zFar, float k);
// The k that makes the first of `slices` slices kFogFirstSliceMetres deep at
// 64 slices (deeper with fewer slices), for a far end at zFar.
float FogDepthK(float zFar, int slices);
constexpr float kFogFirstSliceMetres = 0.9f;

// ---- The medium ----------------------------------------------------------------

// Scale heights (m) and shapes.
constexpr float kFogHazeHeight = 1200.0f;   // aerosol haze
constexpr float kWeatherFogHeight = 800.0f; // setFog: thins upward from the fog floor (the stars go with it)
constexpr float kRainHazeHeight = 900.0f;   // rain haze
constexpr float kGroundHazeHeight = 40.0f;  // the humid air near the ground: its scale height above the ground
constexpr float kGroundHazePool = 80.0f;    // and over the fog floor: it pools in the low ground
constexpr float kFogFloorRadius = 700.0f;   // the fog floor: the lowest ground within this radius
constexpr float kFogFloorBlur = 350.0f;     // smoothed over this radius
// The valley mist's layer: the local relief (the mean ground within
// kFogReliefRadius over the fog floor, the sea counted at 0 m) times
// kMistReliefShare, at most kMistLayerMax deep, faded in from a relief of
// kMistReliefFrom to kMistReliefFull: a deep valley fills to a third of its
// depth, a plain or the open sea holds none. Above the top it falls off over
// kMistLayerFall.
constexpr float kFogReliefRadius = 600.0f;
constexpr float kMistReliefShare = 0.35f;
constexpr float kMistLayerMax = 40.0f;
constexpr float kMistReliefFrom = 12.0f;
constexpr float kMistReliefFull = 45.0f;
constexpr float kMistLayerFall = 18.0f;
// Haze extinction at sea level on a clear day (1/m), reached from
// kHazeNearTo out: a summer day's 8 km visibility, so ridges 1 to 2 km away
// pale a little and ridges 3 to 5 km away sit in the air and fade toward the
// sky.
constexpr float kHazeClear = 6.0e-4f;
// Rayleigh scattering of air at sea level (1/m, red, green, blue;
// Atmosphere.hpp's kRayleighScattering) times this: a little more than the
// physical air (the islands are small), not so much that the mid-distance
// woods turn blue.
constexpr float kRayleighScale = 1.25f;
// Dust over an arid map (FogInputs::aridity 1): 1/m at the ground, thinning
// with kDustHeight, warm (kDustTint in the shaders): the pale beige horizon
// of a desert.
constexpr float kDustHaze = 1.2e-3f;
constexpr float kDustHeight = 250.0f;
// Dust keeps kDustNearKeep near the eye, the whole from kDustNearFrom to
// kDustNearTo (it hangs low and close, the desert's horizon goes pale
// within a kilometre or two).
constexpr float kDustNearKeep = 0.3f;
constexpr float kDustNearFrom = 100.0f;
constexpr float kDustNearTo = 1500.0f;
// The sky's horizon band takes this share of the dust (the dome already
// pales toward the horizon).
constexpr float kDustSkyShare = 0.2f;
// The sun's forward lobe in the clear haze and the dust (the glow toward a
// low sun over the sea and the plain): its share of the sun's single
// scattering in the volume (the sky ring already holds the dome's own glow).
constexpr float kFogLobeGain = 0.2f;
// The sky's horizon band past the far end takes this share of it (the dome
// glows toward the sun already).
constexpr float kFogLobeSkyShare = 0.35f;
// The humid air near the ground at full weight (1/m at the ground).
constexpr float kGroundHaze = 8.0e-4f;
// Valley mist extinction inside the layer at full weight (1/m): about a
// kilometre of visibility in the layer (denser mist erases a whole misty bay
// in one wash at dawn).
constexpr float kMistSigma = 2.0e-3f;
// Inside the mist and the humid ground air the eye's own surroundings stay
// legible: their density near the eye is kMistNearKeep of it, rising to the
// whole from kMistNearFrom to kMistNearTo metres away.
constexpr float kMistNearKeep = 0.15f;
// The drawn mist's banks (EngineGL33_PostFogShaders.cpp fogMediumAt): the
// density times up to kMistBankPeak where they are thickest (at full
// mistNoise), the layer's top up to kMistBankTopMax times its depth over the
// fog floor (1.35 from the broad octave plus half of the fine one's 0.35).
constexpr float kMistBankPeak = 1.7f;
constexpr float kMistBankTopMax = 1.35f + 0.5f * 0.35f;
// The clear haze and air keep this share near the eye, the whole from
// kHazeNearFrom to kHazeNearTo away: the depth is in the distance (blue from
// about 3 km), the first kilometre keeps its colour and contrast. The air's
// light is the bright horizon sky, many times a dark wood, so an optical depth
// of even 0.05 to 0.1 over the first 300 to 600 m lays a milky blue veil over
// every wood below a clear noon vista. With these values the optical depth is
// about 0.015 at 500 m, 0.05 at 1 km, 0.27 at 2 km and 0.67 at 3 km: the near
// field is nearly clear, the far ridges keep most of their air. Dust keeps
// its own near share, kDustNearKeep.
constexpr float kHazeNearKeep = 0.06f;
constexpr float kHazeNearFrom = 200.0f;
constexpr float kHazeNearTo = 3000.0f;
// Under a grey deck, in rain and in fog the near air is thick too: the
// thinning ends at kHazeNearToGrey with kHazeNearKeepGrey at the eye.
constexpr float kHazeNearKeepGrey = 0.3f;
constexpr float kHazeNearToGrey = 800.0f;
// The humid ground air lies in the low ground: gone above the fog floor plus
// the mist layer plus kGroundAirTop (a hilltop stands in clear air), over
// kGroundAirFade metres.
constexpr float kGroundAirTop = 20.0f;
constexpr float kGroundAirFade = 30.0f;
// An arid map: the mean red of its land's bounce tint from kAridRedFrom
// (green) to kAridRedFull (sand).
constexpr float kAridRedFrom = 1.10f;
constexpr float kAridRedFull = 1.17f;
// The stars fade out over this optical depth of wet air straight up.
constexpr float kStarHideFrom = 0.05f;
constexpr float kStarHideFull = 0.3f;
constexpr float kMistNearFrom = 20.0f;
constexpr float kMistNearTo = 300.0f;
// Mie phase: the forward lobe's asymmetry and the back lobe's share.
constexpr float kFogMieG = 0.78f;
constexpr float kFogBackShare = 0.12f;
// The near-field pass marches the sun's forward lobe this far (m), times
// this gain: the morning aerosols' lobe is narrower and stronger than the
// haze's g (the shafts between crowns). The beams are linear in the gain (the
// pass's soft cap of 0.45 does not limit them); at this gain the shafts on the
// dark crowns of a morning wood read clearly while the open sky round them is
// unchanged.
constexpr float kFogBeamReach = 180.0f;
constexpr float kFogBeamGain = 35.0f;
// The classic engine's visibility in rain at full density (Landscape.cpp: 350 m).
constexpr float kRainVisibility = 350.0f;
// A grey deck thickens the clear haze by 1 + this (times the sky's greyness,
// less in rain, whose own fog carries its air). At 1.2 the ridges from 1 to
// 3 km fade one behind the other; much more turns a dry overcast into a
// 1.5 km wash with every ridge past it one flat plate.
constexpr float kHazeGreyGain = 1.2f;
// Rain's weather fog takes this share of the extinction that gives the
// classic tactical visibility (setFog keeps all of it): about a fifth of a
// surface is left at the classic fog end, where the AI's tactical sight
// ends. With the whole of it, rain 0.8 leaves a hill 300 m away a third of
// its contrast and nothing past a kilometre.
constexpr float kRainFogShare = 0.5f;
// The weather fog's extinction times the visibility it takes away: what the
// weather leaves of a surface at the classic fog end is about e^-3.2.
constexpr float kWeatherExtinction = 3.2f;
// setFog at night is this much denser at the floor (the classic engine's
// night visibility is shorter still). It holds the night's setFog at
// (1 + kNightFogBoost) x kSetFogBoost = 2 x 2.8 = 5.6 times the classic
// extinction, so the day's mist can be tuned through kSetFogBoost without
// changing the night: a house 60 to 120 m up a foggy street at setFog 0.35
// loses half to three quarters of its contrast.
constexpr float kNightFogBoost = 2.0f * 2.8f / 0.3f - 1.0f;
// setFog's own fog (not rain's) takes this share of the classic extinction,
// as kRainFogShare is rain's; like the classic fog it thickens with distance,
// not as a flat wall. The clear haze, the valley mist and the humid air stack
// on it, so the share is small: with kSetFogHazeCut and kMistFogWeight a level
// line at the fog floor under a grey deck at setFog 0.3 keeps about two
// thirds at 300 m, half at 500 m and a third at the classic fog end (a rock
// and a tree line read through a veil, the far hills fade). A larger share
// turns the light mist into a white wall where the classic fog keeps the
// first 300 m crisp.
constexpr float kSetFogBoost = 0.3f;
// A day's setFog carries the humid air the clear haze stood for: the haze is
// cut by this share at setFog 0.3 and over (by day; rain keeps its haze). The
// clear haze, about 6e-4 per metre, is otherwise as thick as setFog's own fog.
constexpr float kSetFogHazeCut = 0.5f;
// setFog's share of the valley mist's weight by day, rising to
// kMistFogWeightNight at night. By day a heavier layer doubles the weather
// fog in every hollow and thickens a light mist into fog.
constexpr float kMistFogWeight = 0.2f;
constexpr float kMistFogWeightNight = 1.2f;
// The forward fog keeps the view-distance edge: it starts at this share of
// the fog end.
constexpr float kFogEdgeStart = 0.93f;
// A misty morning is a weather event: the chance of one on a clear day, and
// what a grey sky and rain add to it (FogMistEvent).
constexpr float kMistChanceClear = 0.4f;
constexpr float kMistChanceOvercast = 0.25f;
constexpr float kMistChanceRain = 0.45f;

struct FogInputs
{
    float sunElevationDeg = 90.0f;
    float sunAzimuthDeg = 180.0f; // 0 north, 90 east
    float nightEffect = 0.0f;     // 0 day .. 1 night
    float skyThrough = 1.0f;      // 1 clear .. 0 overcast
    float fog = 0.0f;             // setFog 0..1
    float rain = 0.0f;            // rain density 0..1
    float viewDistance = 1500.0f; // the user's view distance (tactical range, m)
    float fogStart = 0.0f;        // the classic fog's start and end (Scene fog range, m)
    float fogEnd = 1500.0f;
    int year = 1985;      // the date: it seeds the misty mornings
    int dayOfYear = 180;  // 0 = 1 January
    float aridity = 0.0f; // 0 green .. 1 desert (FogAridity of the map's ground colours)
    // The classic engine's night factor on its tactical visibility
    // (Landscape.cpp: the sun's diffuse red times 4, 0.75 .. 1; 1 by day).
    float nightVisibility = 1.0f;
};

// The classic engine's fog for a weather (Landscape.cpp tactical range with
// its night factor, Scene::ResetFog ranges): the drawn linear fog runs from
// `start` to `end`, the AI's tactical fog (FogExponential) from `start` to
// `tactical`.
struct ClassicFogRange
{
    float start = 0.0f;
    float end = 0.0f;
    float tactical = 0.0f;
};

struct FogMedium
{
    // The volume's far end (m of view depth): the classic fog end, the far
    // plane of the world's projection; the sky is taken at this distance.
    float zFar = 1500.0f;
    float hazeSigma = 0.0f; // 1/m at sea level
    float hazeHeight = kFogHazeHeight;
    float rayleigh = 0.0f;    // times the physical sea-level Rayleigh scattering
    float groundSigma = 0.0f; // 1/m at the ground (the humid air near it)
    float groundHeight = kGroundHazeHeight;
    float weatherSigma = 0.0f; // 1/m at the fog floor
    float weatherHeight = kWeatherFogHeight;
    float mistSigma = 0.0f; // 1/m inside the valley mist layer
    float mistLayer = 1.0f; // times the relief's layer depth (BuildFogLayer)
    float mistFall = kMistLayerFall;
    float mistNoise = 0.0f; // 0 an even layer .. 1 broken into banks
    float dustSigma = 0.0f; // 1/m at the ground (arid maps)
    float dustHeight = kDustHeight;
    float lobe = 0.0f;                  // the sun's forward lobe in the clear haze and dust (kFogLobeGain times fogSun)
    float hazeNearTo = kHazeNearTo;     // the clear air's near thinning ends here (m)
    float hazeNearKeep = kHazeNearKeep; // and keeps this share at the eye
    float sun = 1.0f;                   // the sun's light in the air
    float sky = 1.0f;                   // the sky's light in the air
    float lights = 1.0f;                // lamps, headlights and fires
    float g = kFogMieG;
    // The forward fog factor's scale (>= 1): the classic linear fog's start
    // moves out to kFogEdgeStart of its end. 1 = the classic fog.
    float fogEdgeScale = 1.0f;
    // The morning weight (0..1) and the day's misty-morning event (0 none,
    // up to 1), for logs and the tests.
    float morning = 0.0f;
    float mistEvent = 0.0f;
    // The sky-through (1 clear .. 0 overcast): how much of the air's light is
    // scattered sunlight that shadows take away.
    float skyThrough = 1.0f;
    // The night (0..1): the air's light follows the night sky's.
    float night = 0.0f;
    // The classic fog's range (m, FogInputs::fogStart and fogEnd): with
    // fogEdgeScale, the forward fog's edge (FogEdgeKeep).
    float fogStart = 0.0f;
    float fogEnd = 1500.0f;
    // The classic weather floor (FogClassicFloor): the classic fog of this
    // weather and of a clear sky at the same darkness.
    bool classicFloor = false;
    ClassicFogRange classicWeather;
    ClassicFogRange classicClear;
};

// The classic engine's tactical visibility (m) for setFog `fog`, rain
// `rain` and a view distance (Landscape.cpp, the night factor at 1): the
// distance at which its fog ends.
float WeatherVisibility(float fog, float rain, float viewDistance);
ClassicFogRange ClassicFogRangeFor(float fog, float rain, float viewDistance, float nightVisibility);
// What the classic fog leaves of a surface `dist` metres out: the denser of
// its drawn linear fog and its tactical fog.
float ClassicFogKeep(const ClassicFogRange& r, float dist);
// How grey the sky is for the air's colour: 0 at a sky-through of 0.65 and
// over, 1 at 0.15 and under. Under a grey sky the horizon ring measures the
// deck above the horizon fill, and the apply lays the air over that fill.
float FogGreySky(float skyThrough);
// The classic horizon polygon (Landscape::DrawHorizont) stands from under the
// horizon up to this share of the far distance above the eye: the sine of its
// top's elevation (3.4 degrees). It is drawn in the classic fog colour, not
// the sky's, so under a grey sky the apply replaces it with the air.
constexpr float kFogHorizonFillTop = 0.06f;
// An optic overlay over the fogged world: the apply lays the overlay's
// darkening back as (after + eps) / (before + eps) per channel (FogSnapRatio,
// GLSL kSnapRatioEps), so a channel the overlay left alone keeps the fogged
// light whole even at 0. eps is far under the frame's darkest nonzero code,
// (1/255)^2.2 = 5e-6, so a black mask over the darkest world still darkens it
// whole. A plain floor such as max(before, 1e-5) would turn world channels at
// code 0 or 1 into a ratio of 0 or 0.5 with no overlay there: single black
// and green pixels over a fogged tree line, wherever its needles are near
// black.
constexpr float kFogSnapRatioEps = 1e-7f;
// The overlay's darkening of one channel: 1 where the overlay left it.
float FogSnapRatio(float after, float before);
// Under a grey sky the horizon ring samples from this elevation (degrees),
// above the polygon's top: the deck over the land, not the fill.
constexpr float kFogRingGreyFromDeg = 3.8f;
// The ring's light along a column at the column's own angle to the sun. The
// ring measures the sky a few degrees above the land (from 2 degrees up, the
// darker half of six samples), so toward a low sun it holds the aureole's
// flank; the haze over ground under the horizon is farther from the sun and
// takes less of the sun's forward scattering (without the gain a dawn bay
// wears a cream veil). Per channel, from the sky table's two parts at the
// ring's sample direction (r the Rayleigh and multiple scattering, m the
// haze's single scattering without its phase), the gain is
// (r + m P(column)) / (r + m P(ring)), P the sky's own Mie phase, at most
// kFogRingPhaseMax (haze in front of land right under the sun glows, but
// never more than this over the sky just above the land), and none under a
// grey sky (the sky table is the clear sky's).
constexpr float kFogRingPhaseMax = 1.5f;
float FogRingPhaseGain(float r, float m, float phaseColumn, float phaseRing);
// The morning weight: an eastern sun from 6 degrees under the horizon (full
// from 2 under to 8 above), gone by 24 degrees.
float FogMorningWeight(float sunElevationDeg, float sunAzimuthDeg);
// The day's misty-morning event: 0 on a day without one, else its strength
// (0.6 to 1). The date seeds it (the same day is misty or clear on every
// run); the chance is kMistChanceClear on a clear day, more under a grey sky
// and in rain.
float FogMistEvent(int year, int dayOfYear, float skyThrough, float rain);
// The day of the year (0 = 1 January) of a calendar date.
int FogDayOfYear(int year, int month, int day);
// The forward fog factor's scale that moves the classic start to
// kFogEdgeStart of the end (1 when the range is empty).
float VolFogEdgeScale(float fogStart, float fogEnd);
// How arid a map is (0 green .. 1 desert) from the red of its ground's
// bounce tint (TerrainHorizonMap::groundTint, (r, b) pairs; the land cells,
// heights above 0 m): sand and rock are red-heavy, grass is not. 0 without
// tints.
float FogAridity(const std::vector<float>& groundTint, const std::vector<float>& heights);
// Whether the stars still show through the wet air over the eye (1 all, 0
// none): the weather fog, the mist and the humid air's optical depth
// straight up from the eye's height.
float FogStarKeep(const FogMedium& m, float eyeY, float floorY, float aboveGround, float layer);
// The frame's medium from the weather, the time of day and the knobs.
// `applied` false (the pass does not run this frame) gives the classic fog:
// fogEdgeScale 1, nothing else read.
FogMedium ComputeFogMedium(const FogInputs& in, const LightingTuning& t, bool applied);

// ---- The fog floor and the mist layer ---------------------------------------

// The valley mist's floor over a heights grid (size x size, cellSize metres
// apart, row-major by z then x as TerrainHorizonMap::height): the lowest
// ground within `radius` metres (a square window), never below sea level (a
// lake or the sea holds its mist at the water), smoothed by a box blur of
// `blur` metres. Empty for an invalid grid.
std::vector<float> BuildFogFloor(const float* heights, int size, float cellSize, float radius = kFogFloorRadius,
                                 float blur = kFogFloorBlur);
// The mist layer's depth (m) over the floor on the same grid: the local
// relief (the mean ground within kFogReliefRadius, the sea at 0 m, over the
// floor) times kMistReliefShare, at most kMistLayerMax, faded in from a
// relief of kMistReliefFrom to kMistReliefFull. Empty for an invalid grid or
// a floor of another size.
std::vector<float> BuildFogLayer(const float* heights, const std::vector<float>& floorY, int size, float cellSize);

// Extinction (1/m) of the medium at an absolute height `y` over a fog floor
// `floorY`, `aboveGround` metres over the ground, under a mist layer `layer`
// metres deep (BuildFogLayer, before FogMedium::mistLayer), without the
// mist's noise and the eye's near thinning: the C++ mirror of the injection's
// density (Rayleigh at green), for the tests.
float FogExtinctionAt(const FogMedium& m, float y, float floorY, float aboveGround = 1e4f, float layer = kMistLayerMax);
// The share of its clear haze and Rayleigh air the eye's near thinning keeps
// `dist` metres from the eye (the injection's hazeKeep: hazeNearKeep at the
// eye, the whole from kHazeNearFrom to hazeNearTo), for the tests.
float FogHazeNearKeep(const FogMedium& m, float dist);
// The low wet air alone: the humid ground air and the valley mist (a part of
// FogExtinctionAt; the stars' and the eye's own wet air read it).
float FogSightExtinctionAt(const FogMedium& m, float y, float floorY, float aboveGround, float layer);
// The whole medium as the injection weighs it `dist` metres from the eye:
// FogExtinctionAt with each part's near thinning (the clear haze and air
// FogHazeNearKeep, the dust kDustNearKeep, the humid air and the mist
// kMistNearKeep, the weather fog none), without the mist's banks. What AI
// sight integrates.
float FogSeenExtinctionAt(const FogMedium& m, float y, float floorY, float aboveGround, float layer, float dist);
// FogSightExtinctionAt with the mist's banks at their upper bound (the
// density times 1 + (kMistBankPeak - 1) mistNoise, the layer's top at
// kMistBankTopMax of its depth): the banks drift with each machine's own
// frames, so AI sight takes the most mist any of them may draw there.
float FogSightMistBound(const FogMedium& m, float y, float floorY, float aboveGround, float layer);
// The forward fog's share left `dist` metres out: the classic linear fog over
// fogStart..fogEnd times fogEdgeScale, at most 1 (with the volume on: 1 up to
// kFogEdgeStart of the end, 0 at the end). AI sight ends where the drawn
// world ends.
float FogEdgeKeep(const FogMedium& m, float dist);
// The most the drawn air may leave of a surface `dist` metres out: the part
// of the classic fog the weather adds (the classic fog of the weather over
// that of a clear sky at the same darkness, at most 1). The drawn air and AI
// sight keep the lesser of the medium's transmittance and this, so fog and
// rain are never thinner than the classic fog drew them, while a clear sky
// keeps the clearer air (the floor is 1). Where the weather's classic range
// is shorter than the clear sky's start (heavy fog, heavy rain) the floor is
// the classic fog itself.
float FogClassicFloor(const FogMedium& m, float dist);
// The humid ground air's share at height y (1 in the low ground, 0 from
// kGroundAirFade above the fog floor plus the mist layer plus kGroundAirTop).
float FogGroundAirTop(const FogMedium& m, float y, float floorY, float layer);

// ---- AI sight ------------------------------------------------------------------
//
// AI sight is part of the simulation, not of the renderer. Once per
// simulation step (World::SimulateLandscape, Poseidon::SimulateFogSight in
// World/Terrain/TerrainLighting.hpp) the world's weather and time of day
// (FogInputs) and the island's sight map give one immutable snapshot
// (MakeFogSightSnapshot): the medium through ComputeFogMedium with the
// shipped knobs (LightingTuning{}, never a machine's dev knobs) and `applied`
// true, the very function the renderer's fog takes. No render state enters
// it: post effects, the SSAA path, a vehicle interior view, the fog's gates
// and quality, a missing renderer (a dedicated server, a headless client)
// leave AI sight as it is, and every machine computes it from the same synced
// weather and date.
//
// With the Lighting row on Remastered the renderer draws this medium and only
// the view-distance edge of the classic fog, so AI sight takes the medium's
// transmittance times that edge IN PLACE OF the classic tactical fog: the
// medium's weather fog is already the classic tactical visibility's share
// (WeatherVisibility), so multiplying both would count the weather twice.
// With the row on Classic the drawn fog is the classic fog, and AI sight
// takes the classic tactical fog only (FogSightSnapshot::volumetric false).
// The machine that simulates the AI decides; one that draws nothing takes
// the default, Remastered.

// The terrain the sight test reads: ground, fog floor and mist layer on one
// grid (x east, z north, cell (0, 0) at the world origin), and the island's
// aridity (FogAridity, for the dust).
struct FogSightMap
{
    int size = 0;
    float cellSize = 0.0f;
    std::vector<float> ground, floorY, layer;
    float aridity = 0.0f;
    bool Valid() const;
};
// The ground tint strength the terrain horizon map's tints take
// (TerrainHorizon.cpp kGroundTintStrength): the sight map's aridity takes
// the same, so it equals FogAridity on the horizon map.
constexpr float kFogAridityTintStrength = 0.75f;
// The sight map of a landscape's heights grid (range x range, `grid` metres
// apart, row-major by z then x): the grid box-downsampled as the terrain
// horizon map samples it (DownsampleTerrainGrid, at most maxSize texels per
// side), its fog floor (BuildFogFloor) and mist layer (BuildFogLayer). With
// `groundRgb` (range x range x 3, the ground's display colour at each
// vertex, as BuildTerrainHorizonMap takes it) its aridity is the one the
// horizon map's tints give; without, 0. Null for an invalid grid.
std::shared_ptr<const FogSightMap> BuildFogSightMap(const float* heights, int range, float grid, int maxSize = 1024,
                                                    const float* groundRgb = nullptr);

// One simulation step's AI sight. Immutable once published: a reader keeps
// its own reference for as long as it reads.
struct FogSightSnapshot
{
    // True: the medium and the edge replace the classic tactical fog. False
    // (Classic lighting, no island): the classic tactical fog alone.
    bool volumetric = false;
    FogMedium medium;
    std::shared_ptr<const FogSightMap> map; // also the renderer's fog floor
};
// The step's snapshot from the world's inputs (the aridity is the map's),
// the shipped knobs and the simulating machine's Lighting row (pure).
std::shared_ptr<const FogSightSnapshot> MakeFogSightSnapshot(const FogInputs& in, bool classicLighting,
                                                             std::shared_ptr<const FogSightMap> map);
// The simulation step publishes (null: no island); any thread may take the
// current one. Publishing swaps a pointer under a lock: a reader holding the
// previous snapshot keeps it whole.
void PublishFogSight(std::shared_ptr<const FogSightSnapshot> s);
std::shared_ptr<const FogSightSnapshot> FogSightNow();
// Shorthands over FogSightNow(), handing out copies: whether AI sight takes
// the medium in place of the classic fog, the medium, and the map (null
// before an island loads).
bool FogSightActive();
FogMedium FogSightMedium();
std::shared_ptr<const FogSightMap> FogSightMapInUse();
// Transmittance (0..1) along the line between two world points (x east, y
// up, z north): the medium's times the edge (FogEdgeKeep) at the line's
// length, the lesser of the drawn views from either end (FogSeenExtinctionAt
// thins the air around the eye, so from inside a mist bank the way out looks
// clearer than the way in: AI sight takes the way in, as a player there
// sees it, and is the same both ways). Eight samples along the line; parts
// that are zero cost nothing. 1 when the snapshot is not volumetric.
float FogSightTransmittance(const FogSightSnapshot& s, const float from[3], const float to[3]);
// The same for FogSightNow() (1 without one).
float FogSightTransmittance(const float from[3], const float to[3]);
// The same through a given medium over a given map (pure).
float FogSightTransmittanceThrough(const FogMedium& m, const FogSightMap& map, const float from[3], const float to[3]);
// The drawn view alone: a camera at `eye` looking at `to` (the near thinning
// around the eye), on the same eight samples (pure; for the tests and the
// probes).
float FogSeenTransmittanceFrom(const FogMedium& m, const FogSightMap& map, const float eye[3], const float to[3]);
// The medium the renderer's fog drew this frame (nullptr: it did not draw),
// for probes that compare the drawn air with AI sight (triFogSight); AI
// sight never reads it.
void SetDrawnFogMedium(const FogMedium* m);
bool GetDrawnFogMedium(FogMedium& out);

} // namespace Poseidon::remaster
