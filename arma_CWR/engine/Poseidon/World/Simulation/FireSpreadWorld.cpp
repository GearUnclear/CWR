#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>
#include <Poseidon/World/Simulation/FireBuildingShell.hpp>
#include <Poseidon/World/Simulation/FirePlume.hpp>
#include <Poseidon/World/Simulation/FireShell.hpp>
#include <Poseidon/World/Simulation/FireSpread.hpp>

#include <Evaluator/express.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp>
#include <Poseidon/Audio/SoundScene.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Rendering/Effects/Smokes.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp>
#include <Poseidon/World/Entities/Vehicles/Air/Airplane.hpp>
#include <Poseidon/World/Entities/Vehicles/Air/Helicopter.hpp>
#include <Poseidon/World/Entities/Vehicles/Ground/Motorcycle.hpp>
#include <Poseidon/World/Entities/Vehicles/Ground/Tank.hpp>
#include <Poseidon/World/Entities/Vehicles/Misc/Ship.hpp>
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/World/MapTypes.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/Roads.hpp>
#include <Poseidon/World/World.hpp>
#include <Random/randomGen.hpp>

#include <SDL3/SDL_filesystem.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Poseidon::firespread
{

bool Enabled()
{
    static const bool on = []
    {
        const char* v = std::getenv("POSEIDON_XP_FIRESPREAD");
        return v && v[0] != '\0' && std::strcmp(v, "0") != 0;
    }();
    return on;
}

namespace
{
float EnvFloat(const char* name, float def)
{
    const char* v = std::getenv(name);
    return (v && v[0]) ? static_cast<float>(std::atof(v)) : def;
}

using Clock = std::chrono::steady_clock;
double MsSince(Clock::time_point t0)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// The smoke over and downwind of the fire on a coarse grid of blocks: fed by
// the flaming cells, carried by the wind, thinning with time. It places the
// drawn smoke and it is what AI sight looks through, so the two agree.
struct SmokeField
{
    static constexpr float kBlock = 32.0f;
    static constexpr float kTopM = 60.0f; // plume depth above the ground that blocks sight
    int x0 = 0, z0 = 0, nx = 0, nz = 0;   // block index origin and size
    std::vector<float> d, tmp, src, heat, ground;
    std::vector<float> woody; // the share of src from shrubs and timber (0.7 a unit) and torching crowns (1)

    bool Valid() const { return nx > 0 && nz > 0; }
    int Idx(int bx, int bz) const { return (bz - z0) * nx + (bx - x0); }
    bool In(int bx, int bz) const { return bx >= x0 && bz >= z0 && bx < x0 + nx && bz < z0 + nz; }
    static int Block(float v) { return static_cast<int>(std::floor(v / kBlock)); }

    // Grows to cover a box of world metres (with a margin), keeping what it holds.
    void Cover(float minX, float minZ, float maxX, float maxZ)
    {
        const int margin = 14;
        const int bx0 = Block(minX) - margin, bz0 = Block(minZ) - margin;
        const int bx1 = Block(maxX) + margin, bz1 = Block(maxZ) + margin;
        if (Valid() && bx0 >= x0 && bz0 >= z0 && bx1 < x0 + nx && bz1 < z0 + nz)
            return;
        const int nx0 = std::min(bx0, Valid() ? x0 : bx0), nz0 = std::min(bz0, Valid() ? z0 : bz0);
        const int nx1 = std::max(bx1, Valid() ? x0 + nx - 1 : bx1), nz1 = std::max(bz1, Valid() ? z0 + nz - 1 : bz1);
        SmokeField g;
        g.x0 = nx0;
        g.z0 = nz0;
        g.nx = nx1 - nx0 + 1;
        g.nz = nz1 - nz0 + 1;
        const std::size_t n = static_cast<std::size_t>(g.nx) * g.nz;
        g.d.assign(n, 0.0f);
        g.src.assign(n, 0.0f);
        g.woody.assign(n, 0.0f);
        g.heat.assign(n, 0.0f);
        g.ground.assign(n, 0.0f);
        for (int bz = g.z0; bz < g.z0 + g.nz; ++bz)
            for (int bx = g.x0; bx < g.x0 + g.nx; ++bx)
            {
                const int k = g.Idx(bx, bz);
                if (In(bx, bz))
                {
                    g.d[k] = d[Idx(bx, bz)];
                    g.ground[k] = ground[Idx(bx, bz)];
                }
                else if (GLandscape)
                {
                    g.ground[k] = GLandscape->SurfaceY((bx + 0.5f) * kBlock, (bz + 0.5f) * kBlock);
                }
            }
        *this = std::move(g);
    }

    float At(float x, float z) const
    {
        const int bx = Block(x), bz = Block(z);
        return In(bx, bz) ? d[Idx(bx, bz)] : 0.0f;
    }

    // Carries the smoke downwind (semi-Lagrangian, bilinear), thins it, adds the sources.
    void Step(float dt, float wx, float wz)
    {
        if (!Valid())
            return;
        tmp.assign(d.size(), 0.0f);
        const float sx = wx * dt / kBlock, sz = wz * dt / kBlock;
        const float keep = std::exp(-dt / 45.0f);
        for (int j = 0; j < nz; ++j)
            for (int i = 0; i < nx; ++i)
            {
                const float fx = static_cast<float>(i) - sx, fz = static_cast<float>(j) - sz;
                const int ix = static_cast<int>(std::floor(fx)), iz = static_cast<int>(std::floor(fz));
                const float ax = fx - ix, az = fz - iz;
                auto get = [&](int a, int b) { return (a >= 0 && b >= 0 && a < nx && b < nz) ? d[b * nx + a] : 0.0f; };
                const float v = (get(ix, iz) * (1 - ax) + get(ix + 1, iz) * ax) * (1 - az) +
                                (get(ix, iz + 1) * (1 - ax) + get(ix + 1, iz + 1) * ax) * az;
                const int k = j * nx + i;
                tmp[k] = std::min(4.0f, v * keep + src[k] * dt);
            }
        d.swap(tmp);
    }

    // Transmittance along a line: Beer-Lambert through the smoke below the plume top.
    float Transmittance(Vector3Par from, Vector3Par to) const
    {
        if (!Valid())
            return 1.0f;
        const float minX = x0 * kBlock, minZ = z0 * kBlock, maxX = (x0 + nx) * kBlock, maxZ = (z0 + nz) * kBlock;
        if (std::max(from.X(), to.X()) < minX || std::min(from.X(), to.X()) > maxX ||
            std::max(from.Z(), to.Z()) < minZ || std::min(from.Z(), to.Z()) > maxZ)
            return 1.0f;
        const Vector3 seg = to - from;
        const float len = seg.Size();
        if (len < 1.0f)
            return 1.0f;
        const int n = std::min(160, static_cast<int>(len / 16.0f) + 1);
        const float ds = len / n;
        float tau = 0.0f;
        for (int s = 0; s < n; ++s)
        {
            const Vector3 p = from + seg * ((s + 0.5f) / n);
            const int bx = Block(p.X()), bz = Block(p.Z());
            if (!In(bx, bz))
                continue;
            const int k = Idx(bx, bz);
            if (d[k] <= 0.002f || p.Y() - ground[k] > kTopM)
                continue;
            tau += d[k] * ds;
        }
        // Extinction 0.04 per metre at density 1: 25 m of a dense plume dims sight to a third.
        return std::exp(-0.04f * tau);
    }
};

// A plant cell's key (its 8 m cell's indices), and a plant's own (its position).
inline std::uint64_t PlantCellKey(float x, float z, float cellM)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(static_cast<int>(std::floor(x / cellM)))) << 32) |
           static_cast<std::uint32_t>(static_cast<int>(std::floor(z / cellM)));
}
inline std::uint64_t PlantKey(float x, float z)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(static_cast<int>(std::floor(x * 10.0f)))) << 32) |
           static_cast<std::uint32_t>(static_cast<int>(std::floor(z * 10.0f)));
}
// One standing in a cell (FuelAt records them): its foot (world m), its
// height and its crown's radius (m), its class and its shape's name.
struct Plant
{
    float x = 0.0f, z = 0.0f, y = 0.0f;
    float h = 0.0f, r = 0.0f;
    PlantClass cls = PlantClass::Thicket;
    std::string name;
};
// One flaring (FireWorld::Draw): the plant, when its flare began on the
// fire's clock, the midflame wind there, and what the smoke and the lights
// take from it this frame; its drawn peak for the log (its main tongues
// while it is full: the tallest tip and flame, and their means).
struct Flare
{
    Plant p;
    double start = 0.0;
    float midflame = 0.0f;
    std::uint32_t key = 0;
    float strength = 0.0f, flameM = 0.0f;
    float dirX = 0.0f, dirZ = 1.0f; // the wind's direction when it began (its feet's lee side: fixed)
    float peakTip = 0.0f, peakFlame = 0.0f, tipSum = 0.0f, flameSum = 0.0f;
    int peakN = 0;
    bool logged = false;
};
// A plant's own fire (FireWorld::UpdatePlants; FireSpreadWorld.hpp PlantKind,
// PlantLookOf and PlantTable): the plant, its kind, its slots in the table,
// the earliest moment the drawn flames of a cell seen so far reach it
// (candidate: the cell's front, its intensity and flame, its fuel's wind) and
// when they did (contact, < 0 not yet), when the surface front reached its
// trunk if it torched (torchFront), and its look.
struct PlantFire
{
    Plant p;
    PlantKind kind = PlantKind::Broadleaf;
    int slots[4] = {-1, -1, -1, -1};
    double candidate = 1.0e30, contact = -1.0, torchFront = -1.0;
    float kwM = 0.0f, flameM = 0.0f, dirX = 0.0f, dirZ = 1.0f, midflame = 0.0f;
    bool scheduled = false, flared = false, torched = false, final = false;
    PlantLook look;
};
// A plant table starts with kPlantSlotsStart slots and doubles while more
// than a third are taken (PlantTable), to kPlantSlotsMax; a plant's look is final
// kPlantFinalSec after its contact (every sweep and rise long over), within
// the shader clock's wrap.
constexpr int kPlantSlotsStart = 4096, kPlantSlotsMax = 32768;
constexpr double kPlantFinalSec = 400.0, kPlantLogM = 300.0;

// ---- Buildings (buildings builder) ----
// One building the fire knows (FuelAt registers every one it sees): its
// object, kind and box (world: its footprint's centre, its local x and z
// axes, half extents, the ground under it, its height and wall tops), whether
// a bush or a conifer stands against it; then its fire: the pass scheduled
// (the moment the drawn front reaches its apron) and the last one drawn, when
// its first flame inside began (catchAt, < 0 not caught) on the fire's clock
// and its timeline, whether the stock destruction, its embers and its heat
// on the things round it are done, its place among the burning, and its
// slots in the plant table (its burnt look).
struct Building
{
    Object* obj = nullptr;
    std::string name;
    BuildingKind kind = BuildingKind::StoneHouse;
    float cx = 0.0f, cz = 0.0f, y = 0.0f;
    float ax = 1.0f, az = 0.0f, bx = 0.0f, bz = 1.0f;
    float hx = 1.0f, hz = 1.0f;
    float h = 3.0f, wallM = 2.0f;
    bool veg = false;
    std::uint32_t seed = 0;
    double passAt = -1.0, lastPass = -1.0e9;
    float passKwM = 0.0f, passFlameM = 0.0f;
    int passes = 0;
    bool scheduled = false, ember = false;
    double catchAt = -1.0;
    // its own fire's clock (s since its first flame inside; negative through
    // its delay), run on by the frames' time times the test aid objtime, so
    // a change of that factor never jumps it
    double tb = -1.0e9;
    BuildingTimeline tl;
    bool live = false, destroyed = false, embersDone = false, heatDone = false, over = false, waiting = false;
    int slots[4] = {-1, -1, -1, -1};
    float lookDone = 0.0f; // the share of its burnt look last written (WriteBuildingLook)
    // (shell geometry, end-state agent) its burnt shell worn now
    // (FireShell.hpp ShellStageOf): -1 none, 0 piers, inner walls, rubble and
    // the collision cut while its roof falls, 1 what the fire took hidden too
    int shellStage = -1;
    float nowHeat = 0.0f; // how hard its rooms burn now (0..1; Gameplay: a man inside)
    // The point (x, z) in its own frame (lx along its x axis, lz along its z).
    void Local(float x, float z, float& lx, float& lz) const
    {
        const float dx = x - cx, dz = z - cz;
        lx = dx * ax + dz * az;
        lz = dx * bx + dz * bz;
    }
    // (x, z) lies in its footprint grown by `grow` metres.
    bool Inside(float x, float z, float grow) const
    {
        float lx, lz;
        Local(x, z, lx, lz);
        return std::fabs(lx) <= hx + grow && std::fabs(lz) <= hz + grow;
    }
    // The gap (m) from (x, z) to its footprint (0 inside).
    float Gap(float x, float z) const
    {
        float lx, lz;
        Local(x, z, lx, lz);
        const float ox = std::max(std::fabs(lx) - hx, 0.0f), oz = std::max(std::fabs(lz) - hz, 0.0f);
        return std::sqrt(ox * ox + oz * oz);
    }
    // The gap between two footprints (m), from their corners and edges.
    float GapTo(const Building& o) const
    {
        float g = 1e9f;
        for (int k = 0; k < 4; ++k)
        {
            const float sx = (k & 1) ? 1.0f : -1.0f, sz = (k & 2) ? 1.0f : -1.0f;
            g = std::min(
                g, Gap(o.cx + (o.ax * o.hx * sx + o.bx * o.hz * sz), o.cz + (o.az * o.hx * sx + o.bz * o.hz * sz)));
            g = std::min(g, o.Gap(cx + (ax * hx * sx + bx * hz * sz), cz + (az * hx * sx + bz * hz * sz)));
        }
        return g;
    }
};
// A building's slots in the plant table (its burnt look, the object main's
// burnBuilding) carry kBuildingMark in their kind field.
constexpr float kBuildingMark = 7777.0f;

// A cell that has burnt (FireWorld::Draw keeps one for each cell it has seen
// burning, until kAfterKeepSec after its front left): its last live view,
// when its front reached it on the fire's clock, its drawn residence time,
// when its torching crown went out (< 0: none), and the frame it was last
// live in. Its drawn band, tail and debris burn on from it.
struct Afterburn
{
    BurningView v;
    double caught = 0.0, torchEndAt = -1.0;
    float trDraw = 0.0f;
    float gridX = 0.0f, gridZ = 1.0f; // its rows' direction when it lit all at once (the wind when first seen)
    std::uint32_t frame = 0;
    // Its debris pieces' times off the view's age (FrontAgeAt there less the
    // view's age; kPieceBare on the road's bare surface), in the order Draw
    // visits them, read at pieceFrame (a live cell's are read again every
    // kPieceRefreshFrames: its front barely moves in that time).
    std::vector<float> pieceAge;
    std::uint32_t pieceFrame = 0;
    // Its flames' light (UpdateLights: CellFlameLight followed over
    // kLightAttackSec and kLightReleaseSec, cd), their height (m) and its
    // ash's glow (0..1); the drawn front's age at its slices
    // (CellLightSliceAges) when last read, the clock then and the caught time
    // it was read for (a view that changes phase reads them again).
    float light = 0.0f, lightH = 0.0f, ember = 0.0f;
    float lightAge[kFlameLightSlices] = {};
    double lightAgeAt = -1.0, lightAgeCaught = 0.0;
    // (fix 6) its torch's tip in tree heights, eased (a neighbour joining the
    // torch raises the model's at once: the drawn column grows into it)
    float torchTip = 0.0f;
};
constexpr std::uint32_t kLightAgeFrames = 8;
constexpr float kPieceBare = -1e30f;
// (fix 08) a piece the level of detail dropped on a reading frame: its time
// is read when it is next drawn
constexpr float kPieceUnread = -2e30f;
constexpr std::uint32_t kPieceRefreshFrames = 8;

struct Stats
{
    double msGrid = 0, msSmoke = 0, msDraw = 0, msPlay = 0, msTotal = 0;
    // Draw's parts: the live cells, the burnt-out cells' bands and tails, the debris (ms, the last frame).
    double msDrawLive = 0, msDrawAfter = 0, msDrawDebris = 0;
    double msTotalMax = 0;
    int flamePuffs = 0, smokePuffs = 0, lights = 0, damaged = 0, fled = 0, trees = 0;
    // The last frame's light (UpdateLights): the flames' candela in reach, the
    // fill's, the lights picked and fading, the halo's cells, and the picks
    // changed since the last stats line.
    float lightCd = 0.0f, fillCd = 0.0f;
    int lightsPicked = 0, lightsFading = 0, lightCells = 0, lightSwaps = 0;
    // The last frame's flame sheets, the cells they came from, and the cells
    // the sheet budget left out (faded out before it, so none pops).
    int sheets = 0, sheetCells = 0, sheetCut = 0;
    // The plants flaring now, and all that have flared.
    int flares = 0, flaresTotal = 0;
    // The last frame's lingering: tail tongues, debris flames, glow points,
    // burnt-out cells drawn on, and the cells kept (_after).
    int tail = 0, debris = 0, glows = 0, afterCells = 0, afterKept = 0;
    // The last frame's burning bed sheets (the realism audit's fix 7).
    int beds = 0;
    long long sightCalls = 0, sightHits = 0;
    // Since the last stats line: AI sight checks that reached the smoke test, and their time.
    long long winSightCalls = 0, winSightNs = 0;
};

// The fire's sound (tools/firespread/gen_fire_sounds.py: our own synthesized
// fire processed through VST plugins; fx/fire_*.ogg beside the executable,
// loaded the first time a fire burns within kSoundReach). Up to kEmitters
// 3D emitters, each on one of the smoke field's 32 m blocks, the blocks that
// sound loudest where the camera is: a looping crackle bed (every burning
// cell, by its fireline intensity) and a looping roar (intense cells and
// torching crowns), their volumes following the block's fire smoothly (the
// engine's 3D volume is acoustic power: full level within sqrt(volume) x 30
// m, then falling with distance). Near the camera, branch snaps and resin
// pops (one-shots) at burning cells, more from shrubs, timber and torching
// crowns. Missing files mean no sound (logged once).
class FireSound
{
  public:
    static constexpr int kEmitters = 4;
    static constexpr int kPops = 12;
    static constexpr float kSoundReach = 900.0f; // a fire farther than this from the camera is not heard (m)
    static constexpr float kPopReach = 90.0f;    // snaps and pops only this near (m)
    // ---- Tunables ----
    static constexpr float kCrackleGain = 0.3f;    // crackle volume per unit of crackle power
    static constexpr float kRoarGain = 0.5f;       // roar volume per unit of roar power
    static constexpr float kRoarFromKwM = 1500.0f; // cells roar above this fireline intensity (kW/m) ...
    static constexpr float kRoarTorch = 4.0f;      // ... and a torching crown adds this much
    static constexpr float kFollow = 1.5f;         // volumes follow the fire at this rate (1/s)
    static constexpr float kPopRate = 0.12f;       // pops a second from a burning grass cell at full strength
    static constexpr float kPopRateMax = 8.0f;     // at most this many a second in all

    void Update(float dt, const std::vector<BurningView>& views, Vector3Par cam);
    void Stop();
    // For the stats line: the files loaded, live emitters (crackle, roar)
    // and pops started so far.
    bool Ready() const { return _ready; }
    void Live(int& crackle, int& roar) const
    {
        crackle = roar = 0;
        for (const Emitter& e : _emit)
        {
            crackle += e.crackle ? 1 : 0;
            roar += e.roar ? 1 : 0;
        }
    }
    long long PopsPlayed() const { return _popsPlayed; }

  private:
    struct Emitter
    {
        long long block = -1; // the smoke block it sounds (bz * 2^32 + bx), -1 free
        Ref<IWave> crackle, roar;
        float crackleVol = 0.0f, roarVol = 0.0f;
        float freq = 1.0f;
        Vector3 pos = VZero;
    };
    bool Load();
    IWave* Open(const std::vector<char>& bytes, Vector3Par pos, float vol, float freq, bool loop);
    static void Release(Ref<IWave>& w)
    {
        if (w)
        {
            w->Stop();
            w = nullptr;
        }
    }

    bool _tried = false, _ready = false;
    std::vector<char> _crackleOgg, _roarOgg;
    std::vector<std::vector<char>> _popOggs;
    Emitter _emit[kEmitters];
    Ref<IWave> _pops[kPops];
    int _nextPop = 0;
    float _popCredit = 0.0f;
    long long _popsPlayed = 0;
    unsigned _rng = 0x9e3779b9u;
    float Rand()
    {
        _rng = _rng * 1664525u + 1013904223u;
        return static_cast<float>(_rng >> 8) * (1.0f / 16777216.0f);
    }
};

bool FireSound::Load()
{
    if (_tried)
        return _ready;
    _tried = true;
    const char* base = SDL_GetBasePath();
    const std::string dir = std::string(base ? base : "") + "fx/";
    auto read = [&](const std::string& name, std::vector<char>& out)
    {
        std::ifstream f(dir + name, std::ios::binary);
        if (!f)
            return false;
        out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        return !out.empty();
    };
    bool ok = read("fire_crackle_loop.ogg", _crackleOgg) && read("fire_roar_loop.ogg", _roarOgg);
    for (int i = 1; ok && i <= 6; ++i)
    {
        std::vector<char> b;
        if (read("fire_pop_" + std::to_string(i) + ".ogg", b))
            _popOggs.push_back(std::move(b));
    }
    _ready = ok && GSoundScene;
    if (_ready)
        LOG_INFO(Core, "fire spread: sounds loaded from {} ({} pops)", dir, _popOggs.size());
    else
        LOG_WARN(Core, "fire spread: no fire sounds at {}fire_*.ogg; the fire is silent", dir);
    return _ready;
}

IWave* FireSound::Open(const std::vector<char>& bytes, Vector3Par pos, float vol, float freq, bool loop)
{
    IWave* w = GSoundScene ? GSoundScene->OpenMemory(bytes.data(), bytes.size(), ".ogg") : nullptr;
    if (!w)
        return nullptr;
    w->SetKind(WaveEffect);
    w->Repeat(loop ? 1000000 : 1);
    w->SetVolume(vol, freq, true);
    w->SetPosition(pos, VZero, true);
    return w;
}

void FireSound::Stop()
{
    for (Emitter& e : _emit)
    {
        Release(e.crackle);
        Release(e.roar);
        e = Emitter{};
    }
    for (Ref<IWave>& p : _pops)
        Release(p);
}

void FireSound::Update(float dt, const std::vector<BurningView>& views, Vector3Par cam)
{
    // The fire's sound power by 32 m block near the camera: crackle from
    // every burning cell, roar from the intense ones and torching crowns.
    struct Block
    {
        float crackle = 0.0f, roar = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    };
    std::unordered_map<long long, Block> blocks;
    const float reach2 = kSoundReach * kSoundReach;
    float popWeight = 0.0f;
    int nearCells = 0;
    for (const BurningView& v : views)
    {
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        const float d2 = dx * dx + dz * dz;
        if (d2 > reach2)
            continue;
        const long long bx = static_cast<long long>(std::floor(v.x / 32.0f));
        const long long bz = static_cast<long long>(std::floor(v.z / 32.0f));
        Block& b = blocks[bz * 4294967296LL + bx];
        const float c = v.strength * (0.3f + std::min(v.intensityKwM, 4000.0f) / 1000.0f);
        b.crackle += c;
        b.roar +=
            v.strength * (std::max(v.intensityKwM - kRoarFromKwM, 0.0f) / 1000.0f + (v.torching ? kRoarTorch : 0.0f));
        b.x += c * v.x;
        b.y += c * v.y;
        b.z += c * v.z;
        b.w += c;
        if (d2 < kPopReach * kPopReach)
        {
            popWeight += v.strength * ((v.model >= 4 || v.torching) ? 3.0f : 1.0f);
            ++nearCells;
        }
    }
    bool any = !blocks.empty();
    for (const Emitter& e : _emit)
        any = any || e.block >= 0;
    if (!any || !Load())
        return;
    // The loudest blocks where the camera is: power over the squared
    // distance, never nearer than the full-level radius.
    struct Pick
    {
        long long key;
        float score;
    };
    std::vector<Pick> picks;
    for (const auto& [key, b] : blocks)
    {
        if (b.w <= 0.0f)
            continue;
        const float vol = kCrackleGain * b.crackle + kRoarGain * b.roar;
        const float px = b.x / b.w - cam.X(), pz = b.z / b.w - cam.Z();
        const float ref = std::sqrt(std::max(vol, 1e-3f)) * 30.0f;
        picks.push_back({key, vol / std::max(px * px + pz * pz, ref * ref)});
    }
    std::sort(picks.begin(), picks.end(), [](const Pick& a, const Pick& b) { return a.score > b.score; });
    if (static_cast<int>(picks.size()) > kEmitters)
        picks.resize(kEmitters);
    // Emitters keep their block while it stays among the loudest; a new one
    // takes a free emitter and fades in, a dropped one fades out.
    auto chosen = [&](long long key)
    {
        for (const Pick& p : picks)
            if (p.key == key)
                return true;
        return false;
    };
    for (const Pick& p : picks)
    {
        bool held = false;
        for (const Emitter& e : _emit)
            held = held || e.block == p.key;
        if (held)
            continue;
        for (Emitter& e : _emit)
            if (e.block < 0)
            {
                e.block = p.key;
                e.freq = 0.92f + 0.16f * Rand();
                break;
            }
    }
    const float k = 1.0f - std::exp(-kFollow * dt);
    for (Emitter& e : _emit)
    {
        if (e.block < 0)
            continue;
        float tc = 0.0f, tr = 0.0f;
        const auto it = blocks.find(e.block);
        if (it != blocks.end() && chosen(e.block) && it->second.w > 0.0f)
        {
            const Block& b = it->second;
            tc = kCrackleGain * b.crackle;
            tr = kRoarGain * b.roar;
            e.pos = Vector3(b.x / b.w, b.y / b.w + 1.5f, b.z / b.w);
        }
        e.crackleVol += (tc - e.crackleVol) * k;
        e.roarVol += (tr - e.roarVol) * k;
        if (e.crackleVol < 0.01f && e.roarVol < 0.01f && tc <= 0.0f && tr <= 0.0f)
        {
            Release(e.crackle);
            Release(e.roar);
            e = Emitter{};
            continue;
        }
        if (!e.crackle)
            e.crackle = Open(_crackleOgg, e.pos, e.crackleVol, e.freq, true);
        if (!e.roar && e.roarVol > 0.01f)
            e.roar = Open(_roarOgg, e.pos, e.roarVol, e.freq, true);
        for (Ref<IWave>* w : {&e.crackle, &e.roar})
            if (*w)
            {
                (*w)->SetVolume(w == &e.crackle ? e.crackleVol : e.roarVol, e.freq);
                (*w)->SetPosition(e.pos, VZero);
            }
    }
    // Snaps and pops near the camera, at random burning cells there.
    if (_popOggs.empty() || nearCells == 0)
        return;
    _popCredit += std::min(kPopRate * popWeight, kPopRateMax) * dt;
    while (_popCredit >= 1.0f)
    {
        _popCredit -= 1.0f;
        int n = static_cast<int>(Rand() * static_cast<float>(nearCells));
        for (const BurningView& v : views)
        {
            const float dx = v.x - cam.X(), dz = v.z - cam.Z();
            if (dx * dx + dz * dz >= kPopReach * kPopReach || n-- > 0)
                continue;
            const Vector3 pos(v.x + (Rand() - 0.5f) * 6.0f, v.y + 0.3f + Rand() * (v.torching ? 8.0f : 1.0f),
                              v.z + (Rand() - 0.5f) * 6.0f);
            const size_t which =
                std::min(static_cast<size_t>(Rand() * static_cast<float>(_popOggs.size())), _popOggs.size() - 1);
            Ref<IWave>& slot = _pops[_nextPop];
            _nextPop = (_nextPop + 1) % kPops;
            Release(slot);
            slot = Open(_popOggs[which], pos, 0.5f + 1.5f * Rand(), 0.85f + 0.35f * Rand(), false);
            _popsPlayed += slot ? 1 : 0;
            break;
        }
    }
}

// A tree's kind as the fire reads it (FireWorld::KindOf).
enum class TreeKind : std::uint8_t
{
    Broadleaf,    // scorches, never torches (birch, oak, aspen, willow, hazel, fig)
    Conifer,      // pine, fir, larch and the rest: torches
    Spruce,       // torches readily: foliage to the ground
    MountainPine, // Pinus mugo: crown fuel touching the surface
    Olive,        // the one broadleaf that can partly torch
};
// Crown base heights (m) of open-grown mountain pine and spruce (the fire
// science consult, 2026-11-16: judgement for open-grown trees).
constexpr float kMountainPineBaseM = 0.3f, kSpruceBaseM = 0.5f;
// Smoke (Simulate): grass's source a cell kSmokeGrassK x strength x
// sqrt(intensity), the other fuels that times their factor.
constexpr float kSmokeGrassK = 0.0005f;
constexpr float kSmokeShrub = 7.0f, kSmokeTimber = 4.0f, kSmokeCrown = 15.0f;
// Fresh burnt ground smokes thin and white (the consult, sixth round: the
// whole fresh burn for 1 to 3 min, strongest in the first 30 s): from the
// end of a cell's drawn band, kAfterSmokeShare of its flaming source dying
// away over kAfterSmokeSec; litter and wood, greyer, over kAfterSmokeTimberSec.
// A burnt cell's record is kept kAfterKeepSec after its front left it.
constexpr float kAfterSmokeShare = 0.15f, kAfterSmokeSec = 60.0f, kAfterSmokeTimberSec = 600.0f;
constexpr float kAfterKeepSec = 1500.0f;

// ---- The plume (EmitPlume; FirePlume.hpp the law, Smokes.hpp PlumePuff) ----
// The smoke (units: a grass fire's smoke a MW of heat released) is the fuel
// burnt times the smoke a fuel makes, so it goes with the heat: a burning
// cell's heat (kW) is its strength times its fireline intensity times its
// width, over the share of its life it flames (the front crosses its cell side
// in cellM / R, each point burns t_r). Per heat, shrub and timber fuel smoke
// kPlumeSmokeWoody times as much as grass and a torching crown kPlumeSmokeCrown
// (the consult, 6c: grass 0.2 to 0.5 kg/m2 at about 7 g/kg, maquis 1 to 3 at
// 12, crown 2 to 4 at 13 to 15: per kilogram, so per MW, 1 : 1.7 : 2). A
// torching crown releases kPlumeTorchMW at its peak (consult eighth 1: 20 to
// 30 MW for about 30 s), its smoke leaving from its flame's top. The fire's
// heat (the line's plume law) is eased over kPlumeHeatEaseSec.
constexpr float kPlumeSmokeWoody = 1.7f, kPlumeSmokeCrown = 2.0f;
constexpr float kPlumeTorchMW = 28.0f, kPlumeHeatEaseSec = 8.0f;
// Puffs a second at full smoke (kPlumeSmokeFull, a 30 m grass head;
// fireSpreadSet "smoke" and POSEIDON_XP_FIRESPREAD_SMOKE set the rate), fewer
// for less smoke, at least kPlumeRateMin while any burns. Each is born
// kPlumeBornM across (times 0.7..1.3; a torching crown's kPlumeTorchBorn times
// that), or larger and fewer while the fire is small on the screen: at least
// kPlumeMinPx pixels across, at most kPlumeLodMax times larger (and its square
// fewer).
constexpr float kPlumeRate = 30.0f, kPlumeRateMin = 1.0f, kPlumeSmokeFull = 60.0f;
constexpr float kPlumeBornM = 5.0f, kPlumeTorchBorn = 1.6f;
constexpr float kPlumeMinPx = 10.0f, kPlumeLodMax = 3.0f;
// Sources within kPlumeNearM of the eye are picked kPlumeNearBoost times more
// often (fading out by kPlumeNearEndM); a puff is born kPlumeSpacingFill as
// wide as the gap between its source's puffs at the wind near the ground
// (kPlumeLowWind of the 10 m wind), at most kPlumeBornMaxM.
constexpr float kPlumeNearBoost = 4.0f, kPlumeNearM = 60.0f, kPlumeNearEndM = 500.0f;
constexpr float kPlumeSpacingFill = 0.8f, kPlumeLowWind = 0.75f, kPlumeBornMaxM = 30.0f;
// Optical depth a smoke unit a second carries per square metre: seen side-on
// a kilometre downwind the scene's 120 m head (about kPlumeSmokeRef units in
// an 11 m/s wind, 178 m thick there) has an optical depth of 1 (consult
// eighth 1: 1 at 1 km, 0.6 at 2 km, 0.4 at 5 km).
constexpr float kPlumeSmokeRef = 250.0f;
constexpr float kPlumeOpticK = 11.0f * 178.0f / kPlumeSmokeRef;
// The plume's slow wander (PlumePuffSpec meander): two sines a side, periods (s).
constexpr float kPlumeWanderSec[4] = {73.0f, 29.0f, 47.0f, 19.0f};
constexpr float kPlumeLifeSec = 600.0f;
// The flames' light on the puffs (PlumeLightAt, lux at FirePlume.hpp's
// kPlumeGlowRgb): a block's heat is its flames' length (m, as the lights'
// heat) and a torching crown's kPlumeTorchGlow of its flame, eased over
// kPlumeGlowEaseSec.
constexpr float kPlumeTorchGlow = 3.0f, kPlumeGlowEaseSec = 1.0f;
constexpr int kPlumeGlowBlocks = 4; // blocks looked at each way (FirePlume.hpp kPlumeGlowRangeM)
// The plume's shadow (UpdatePlumeShadow): its blobs' stretches downwind (m),
// rebuilt every kPlumeShadowSec, each eased toward its rebuild over
// kPlumeShadowEaseSec (a blob keeps its stretch, so it never jumps).
constexpr float kPlumeShadowEdgesM[kPlumeShadowBlobs + 1] = {0.0f,    100.0f,  250.0f,  450.0f, 750.0f,
                                                             1150.0f, 1700.0f, 2400.0f, 3400.0f};
constexpr float kPlumeShadowSec = 0.25f, kPlumeShadowEaseSec = 0.5f;

// A plant's flare: the shape by class in FireSpreadWorld.hpp (FlareShapeOf);
// these are its start and drawing (Draw), smoke and sparks.
constexpr float kFlareMinFlameM = 0.5f, kLeafyMinShare = 0.7f, kOliveMinKwM = 1000.0f;
constexpr float kWhoompShare = 0.3f, kFlareStemShare = 0.35f;
constexpr float kFlareWindShare = 0.5f, kFlarePuffStretch = 0.1f, kFlarePuffGlow = 0.2f;
constexpr float kFlareFootFade = 0.15f, kFlareSmokeShare = 0.5f;
// (fix 6) A flare's volume tongues are drawn in toward its axis by these
// shares (main, fill): one flame body with several tips.
constexpr float kFlareConverge = 0.5f, kFlareConvergeFill = 0.3f;
// Its sparks are FireSparks.hpp's (FlareSparkPeak; EmitSparks). Each flare
// drawn within kFlareLogM of the camera logs its peak (UDFIRE;event=flare)
// when its full phase ends.
constexpr float kFlareLogM = 300.0f;
// A flare's smoke into the smoke field (Simulate), as a cell's: kSmokeGrassK x
// its strength x the square root of the intensity its flame stands for
// (Byram inverted: L = 0.0775 I^0.46), by its fuel's smoke (kSmokeShrub for
// a woody plant, grass's for a reed), kFlareSmokeShare of it (a plant is a
// few metres of front, not a cell's eight).
float FlareSmoke(const Flare& fl, float woody)
{
    if (fl.flameM <= 0.0f)
        return 0.0f;
    const float kwm = std::pow(fl.flameM / 0.0775f, 1.0f / 0.46f);
    return kFlareSmokeShare * kSmokeGrassK * fl.strength * std::sqrt(kwm) * (1.0f + (kSmokeShrub - 1.0f) * woody);
}
// Its heat (the lights and the soldiers' fear read it): a cell's is its
// flame's length a cell of front, a plant's its flame's over its width.
float FlareHeat(const Flare& fl, float cellM)
{
    return fl.strength * fl.flameM * std::max(2.0f * fl.p.r, 1.0f) / std::max(cellM, 1.0f);
}

// The tongues' feet (a debug measure, POSEIDON_XP_FIRESPREAD_FOOTSTATS=1;
// StatsLine foot): each drawn tongue's foot by its class and key, frame to
// frame, its world speed (m/s): the surface band, its tail, the debris
// flames, the glow points, the plants' flares, the torching crowns.
constexpr int kFootBand = 0, kFootTail = 1, kFootDebris = 2, kFootGlow = 3, kFootFlare = 4, kFootTorch = 5;
// (the realism audit's fix 7: the bed streaks under the band)
constexpr int kFootBed = 6;
constexpr int kFootClasses = 7;
struct FootStats
{
    struct Foot
    {
        float x, y, z;
    };
    std::unordered_map<std::uint64_t, Foot> prev, cur;
    long long n[kFootClasses] = {}, still[kFootClasses] = {};
    double sum[kFootClasses] = {};
    float max[kFootClasses] = {};
    std::uint32_t hist[kFootClasses][501] = {}; // 1 cm/s bins to 5 m/s, the last one over
    void Add(int cls, std::uint32_t key, float x, float y, float z, float dt)
    {
        const std::uint64_t k = (static_cast<std::uint64_t>(cls) << 32) | key;
        cur[k] = {x, y, z};
        const auto it = prev.find(k);
        if (it == prev.end() || dt <= 0.0f)
            return;
        const float dx = x - it->second.x, dy = y - it->second.y, dz = z - it->second.z;
        const float v = std::sqrt(dx * dx + dy * dy + dz * dz) / dt;
        n[cls]++;
        sum[cls] += v;
        max[cls] = std::max(max[cls], v);
        still[cls] += v < 0.02f ? 1 : 0;
        hist[cls][std::min(500, static_cast<int>(v / 0.01f))]++;
    }
    void EndFrame()
    {
        prev.swap(cur);
        cur.clear();
    }
    float P95(int cls) const
    {
        long long want = (n[cls] * 95 + 99) / 100, got = 0;
        for (int b = 0; b <= 500; ++b)
            if ((got += hist[cls][b]) >= want)
                return 0.01f * static_cast<float>(b + 1);
        return 0.0f;
    }
    void Reset()
    {
        for (int c = 0; c < kFootClasses; ++c)
        {
            n[c] = still[c] = 0;
            sum[c] = 0.0;
            max[c] = 0.0f;
            std::fill(std::begin(hist[c]), std::end(hist[c]), 0U);
        }
    }
};

// Roads (the fire science consult's ninth answer): a cell keeps the nearest
// road's line within kRoadReachM; the bare running surface is kRoadBareShare
// of the road's width (kRoadDefaultWidthM when its shape tells none); a road
// under kRoadMinWidthM is a two-rut track, thin fuel and no barrier.
constexpr float kRoadReachM = 14.0f, kRoadBareShare = 0.7f, kRoadMinWidthM = 2.5f, kRoadDefaultWidthM = 8.0f;
struct RoadHit
{
    float px = 0.0f, pz = 0.0f; // the nearest point on its centre line (world)
    float dx = 0.0f, dz = 1.0f; // its direction
    float halfBare = 0.0f;      // the bare surface's half width (m)
    float dist = 0.0f;          // the query point's distance from the centre line (m)
};
// The burn map's texels (UpdateBurnMap): this size while the fire fits the
// 512 texel window, coarser in steps of it beyond; each texel from the exact
// front (FrontAgeAt), its edges ramped over kBurnEdgeTexels texels so the
// bilinear map draws them where they are, not on texel lines.
constexpr float kBurnTexelM = 2.0f, kBurnEdgeTexels = 2.0f;

// ---- (followups) The buildings' run seed (BuildingDraw; FireSpreadWorld.hpp BuildingRunSeedFor) ----
// New each game: rolled at a new world (FireWorld::Ensure) and at each
// fireSpreadClear, from the clock, never from GRandGen (nothing in the game's
// own random stream moves); a fixed sequence in a deterministic world.
// fireSpreadSet "bseed" fixes it (s_bldSeedFixed) for repeated trials.
std::uint32_t s_bldRunSeed = 0, s_bldSeedRolls = 0;
bool s_bldSeedFixed = false;
void RollBuildingRunSeed()
{
    if (s_bldSeedFixed)
        return;
    const auto wall = static_cast<std::uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
    const auto mono = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
    const bool det = RandomDeterministicWorld();
    s_bldRunSeed = BuildingRunSeedFor(++s_bldSeedRolls, det, wall ^ (mono << 17) ^ (mono >> 13));
    LOG_INFO(Script, "UDFIRE;event=bseed;seed={};roll={};det={}", s_bldRunSeed, s_bldSeedRolls, det ? 1 : 0);
}

// (followups) a test aid: the wrecks' hot shell glow's light this many times
// its own (fireSpreadSet "wreckglow"; 1 its own), to judge it by eye
float s_wreckGlowScale = 1.0f;
// (smoketrim) a test aid: 0 the full world pass set-up at each return from
// the screen pass (KeepWorldPassSetup; fireSpreadSet "keeppass")
bool s_keepPass = true;

class FireWorld
{
  public:
    static FireWorld& Get()
    {
        static FireWorld w;
        return w;
    }

    FireGrid* Grid() { return _grid.get(); }

    FireGrid& Ensure()
    {
        if (!_grid || _land != GLandscape)
        {
            _land = GLandscape;
            Params p;
            p.moisture = _moisture;
            p.windWanderDeg = _wanderDeg;
            p.windWanderSeed = kWanderSeed;
            _grid = std::make_unique<FireGrid>(
                p, [this](float x, float z) { return FuelAt(x, z); },
                [](float x, float z) { return GLandscape ? GLandscape->SurfaceY(x, z) : 0.0f; });
            _smoke = SmokeField();
            _texFuel.clear();
            _texShare.clear();
            _roadWidth.clear();
            _fleeUntil.clear();
            _plants.clear();
            _flares.clear();
            _flared.clear();
            _after.clear();
            ClearPlants();
            ClearLights();
            _sparks.Clear();
            // (buildings builder) another island: none of its buildings known yet
            // (followups: and a new run seed for their draws)
            RollBuildingRunSeed();
            _bld.clear();
            ShellClear();
            _bldByObj.clear();
            _bldCells.clear();
            _bldKind.clear();
            _bldSeen.clear();
            _bldSched.clear();
            _bldLive.clear();
            // (fire-objects)
            _objectFires.clear();
            _objPending.clear();
            _wrecks.clear();
            _wreckLooks.clear();
        }
        return *_grid;
    }

    void Clear()
    {
        if (_grid)
            _grid->Clear();
        _smoke = SmokeField();
        _smokeLive = 0.0f;
        // The plume's puffs already out live on; its sources and light go.
        _plumeMW = _plumeSmoke = 0.0f;
        _glowHeat.clear();
        _glowY.clear();
        _glowNX = _glowNZ = 0;
        _sheets.clear();
        _beds.clear();
        _fleeUntil.clear();
        _plants.clear();
        _flares.clear();
        _flared.clear();
        _after.clear();
        ClearPlants();
        ClearLights();
        _sound.Stop();
        _sparks.Clear();
        // (buildings builder) the buildings' fires go out (a destroyed one
        // stays destroyed: the world's own state); they are known still
        for (Building& b : _bld)
        {
            const Building keep = b;
            b = Building();
            b.obj = keep.obj;
            b.name = keep.name;
            b.kind = keep.kind;
            b.cx = keep.cx;
            b.cz = keep.cz;
            b.y = keep.y;
            b.ax = keep.ax;
            b.az = keep.az;
            b.bx = keep.bx;
            b.bz = keep.bz;
            b.hx = keep.hx;
            b.hz = keep.hz;
            b.h = keep.h;
            b.wallM = keep.wallM;
            b.veg = keep.veg;
            b.seed = keep.seed;
        }
        _bldSeen.clear();
        _bldSched.clear();
        _bldLive.clear();
        // (end-state agent) a gutted one stays a burnt shell
        for (Building& b : _bld)
            if (b.obj && b.obj->IsGutted())
                QueueShell(b.obj, true);
        // (followups) a new fire draws anew (unless fireSpreadSet "bseed" fixed it)
        RollBuildingRunSeed();
        // (fire-objects)
        _objectFires.clear();
        _objPending.clear();
        _wrecks.clear();
        _wreckLooks.clear();
    }

    Moisture& MoistureRef() { return _moisture; }
    void SetWindOverride(bool on, float x, float z)
    {
        _windOverride = on;
        _windX = x;
        _windZ = z;
    }
    void SetBudgets(float flames, float smoke)
    {
        _flameBudget = flames;
        _smokeBudget = smoke;
    }

    // The weather's (or the override's) wind, before the wander: what the
    // grid is given (it turns it per step at its own sim time).
    void BaseWind(float& x, float& z) const
    {
        if (_windOverride || !GLandscape)
        {
            x = _windX;
            z = _windZ;
        }
        else
        {
            const Vector3 w = GLandscape->GetWind();
            x = w.X();
            z = w.Z();
        }
    }
    // The wind as the fire has it now: the base wind turned by the wander
    // (WindWanderRad, _wanderDeg: a frozen wind makes needle heads and
    // ruler-straight flanks) at the model's own time (the grid's display
    // time), so the drawing leans with the wind the spread used.
    void Wind(float& x, float& z) const
    {
        BaseWind(x, z);
        const float turn = WanderRadNow();
        if (turn != 0.0f)
        {
            const float c = std::cos(turn), s = std::sin(turn);
            const float rx = x * c - z * s, rz = x * s + z * c;
            x = rx;
            z = rz;
        }
    }
    float WanderRadNow() const
    {
        return _grid ? _grid->WanderRad(_grid->DisplayTime()) : WindWanderRad(_clock, kWanderSeed, _wanderDeg);
    }
    void SetWander(float deg)
    {
        _wanderDeg = std::max(deg, 0.0f);
        if (_grid)
            _grid->SetWander(_wanderDeg);
    }
    float WanderDeg() const { return _wanderDeg; }

    void Simulate(float dt);
    float Sight(Vector3Par from, Vector3Par to)
    {
        _stats.sightCalls++;
        const auto t0 = Clock::now();
        const float t = _smoke.Transmittance(from, to);
        _stats.winSightNs += std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t0).count();
        _stats.winSightCalls++;
        if (t < 0.999f)
            _stats.sightHits++;
        return t;
    }
    bool Active() const { return _grid && (_grid->Active() || _smokeLive > 0.0f); }
    Fuel FuelProbe(float x, float z) { return FuelAt(x, z); }
    bool BurnMap(BurnMapView& out)
    {
        if (!_grid || _burnSize == 0)
            return false;
        out.texels = _burn.data();
        out.size = _burnSize;
        out.x0 = _burnX0;
        out.z0 = _burnZ0;
        out.texelM = _burnTexel;
        out.version = _burnVersion;
        out.groundY = _groundY.empty() ? nullptr : _groundY.data();
        out.groundSize = _groundY.empty() ? 0 : kGroundSize;
        out.groundVersion = _groundVersion;
        out.plants = _pfTable.slots > 0 ? _pfTable.data.data() : nullptr;
        out.plantSlots = _pfTable.slots;
        out.plantVersion = _pfVersion;
        out.clock = _clock;
        return true;
    }
    // A tree's or a bush's own fire has burnt its leaves away (the shadow
    // pass drops them): found is false for an object the plant fire does not
    // know (a forest block), whose burn map then decides.
    bool PlantLeavesGone(const Object* obj, bool& found) const;
    // (followups) A burning (or burnt) wreck's look for the renderer
    // (DrawingWreck): its char and its shell's glow now; false for any other
    // object. HasWreckLooks: any at all (the draw asks only then).
    bool WreckLookOf(const Object* obj, float& charK, float& glow) const;
    bool HasWreckLooks() const { return !_wreckLooks.empty(); }
    bool Flames(FlameView& out)
    {
        if (!_grid || _sheets.empty())
            return false;
        out.sheets = _sheets.data();
        out.count = static_cast<int>(_sheets.size());
        out.time = static_cast<float>(_clock);
        return true;
    }
    bool Sparks(SparkView& out) const
    {
        const std::vector<SparkPoint>& p = _sparks.LastPoints();
        if (!_grid || p.empty())
            return false;
        out.points = p.data();
        out.count = static_cast<int>(p.size());
        out.glareM = _sparkGlareM;
        out.glareFromM = kSparkGlareFromM;
        out.glareToM = kSparkGlareToM;
        return true;
    }
    std::string StatsLine();
    // (buildings builder) fireSpreadBuilding and fireSpreadBuildings, and the
    // test aids fireSpreadSet "objtime" and "bseed".
    std::string BuildingCmd(float x, float z, bool ignite, bool roofFirst)
    {
        Ensure();
        return BuildingState(x, z, ignite, roofFirst);
    }
    std::string BuildingsScan(float x, float z, float r);
    // (end-state agent, TEST AID) the plant table's largest size (a power of
    // two, 256 to kPlantSlotsMax; it starts at no more): fireSpreadSet
    // "plantmax", to fill it in minutes and show a burnt look outlasting it
    void SetPlantMax(int n)
    {
        int m = 256;
        while (m < n && m < kPlantSlotsMax)
            m *= 2;
        _pfMaxSlots = m;
        _pfStartSlots = std::min(kPlantSlotsStart, m);
    }
    // (a test aid for the scenes) one number of the building nearest (x, z):
    // "tb" its fire's clock (s since its first flame inside; -1 not caught),
    // "flash", "spread", "collapse", "floor", "down" its timeline's moments
    float BuildingTime(float x, float z, const char* what);
    void SetObjTime(float k)
    {
        // (the buildings' clocks 0.01 to 1000 times; the wrecks' within their own 1 to 200)
        _objTime = std::clamp(k, 0.01f, 1000.0f);
        _objTimeK = std::clamp(k, 1.0f, 200.0f);
    }
    void BuildingHeat(float x, float z, float radiusM);
    float ObjTime() const { return _objTime; }
    bool LightCells(FlameLightView& out)
    {
        if (!_grid || _lightCells.empty())
            return false;
        out.cells = _lightCells.data();
        out.count = static_cast<int>(_lightCells.size());
        out.cellM = _grid->GetParams().cellM;
        return true;
    }
    // ---- Object fires (fire-objects) ----
    // A destroyed vehicle's wreck (NotifyWreck): its fire, and its fireball's
    // reach now; false when it is not a wreck the fire burns (not a vehicle,
    // on the water). A test wreck (fireSpreadWreck) has no vehicle.
    bool AddWreck(const Object* vehicle, float ballR);
    std::uint64_t AddTestWreck(float x, float z, WreckClass cls, float dirRad, bool fuel, bool ammo, float ballR,
                               float firstJetSec = -1.0f);
    void SetWreckBlast(bool on) { _wreckBlast = on; }
    bool WreckBlast() const { return _wreckBlast; }
    // The radiant flux (kW/m2) the object fires throw on a point: each a
    // point source at its flames' middle, kObjectRadiantShare of its heat
    // radiated, no nearer than its footprint's half size.
    float ObjectFluxAt(float x, float y, float z) const;
    // A burst of brands (FireObjects.hpp EmberThrow).
    void ThrowEmbers(const EmberThrow& e);
    // The wrecks for the scenes (fireSpreadWrecks).
    std::string WrecksLine() const;

  private:
    FireWorld()
    {
        _moisture.m1h = EnvFloat("POSEIDON_XP_FIRESPREAD_M1H", 0.06f);
        _moisture.m10h = _moisture.m1h + 0.01f;
        _moisture.m100h = _moisture.m1h + 0.02f;
        _moisture.mLive = EnvFloat("POSEIDON_XP_FIRESPREAD_MLIVE", 0.9f);
        _flameBudget = EnvFloat("POSEIDON_XP_FIRESPREAD_FLAMES", 60.0f);
        _footOn = EnvFloat("POSEIDON_XP_FIRESPREAD_FOOTSTATS", 0.0f) > 0.5f;
        _wanderDeg = std::max(0.0f, EnvFloat("POSEIDON_XP_FIRESPREAD_WANDER", kWindWanderDeg));
        _smokeBudget = EnvFloat("POSEIDON_XP_FIRESPREAD_SMOKE", kPlumeRate);
        // (buildings builder) a test aid only: the object fires' clocks run
        // this many times faster (1: real time)
        _objTime = std::clamp(EnvFloat("POSEIDON_XP_FIRESPREAD_OBJTIME", 1.0f), 0.01f, 1000.0f);
        // (fire-objects) A test aid only, off by default: the object fires'
        // timelines run this many times faster (every clip using it says so).
        _objTimeK = std::clamp(EnvFloat("POSEIDON_XP_FIRESPREAD_OBJTIME", 1.0f), 1.0f, 200.0f);
        _smokeTable = new CloudletTTable();
        _smokeTable->Add(10000.0f, HWhite);
    }

    Fuel FuelAt(float x, float z);
    // The nearest road within kRoadReachM (false: none, or only tracks).
    bool RoadNear(float x, float z, RoadHit& out);
    float RoadWidth(const RoadLink* link);
    TreeKind KindOf(const LODShape* shape, bool forest);
    std::uint8_t TextureFuel(Texture* tex);
    float TextureShare(Texture* tex);
    float SquareShare(int i, int j);
    void Draw(float dt, const std::vector<BurningView>& views);
    // The sparks (FireSparks.hpp): made from this frame's fronts, flares and
    // torching crowns, then stepped (Draw calls it after the flames).
    void EmitSparks(float dt, const std::vector<BurningView>& views, Vector3Par cam, float wx, float wz);
    // Each plant's own fire (PlantFire): registered round every cell seen
    // burning, reached when the drawn flames touch it (its flare starts then),
    // its look in the plant table the renderer reads.
    void ClearPlants();
    void RegisterPlantCell(int ci, int cj, float cellM);
    bool InsertPlant(PlantFire& pf, int reserve);
    // (end-state agent) the plant table rebuilt: every plant's and every
    // building's look written again into its new slots
    void RewriteTableLooks();
    int BuildingSlotReserve() const;
    void OfferPlant(int idx, const BurningView& v, double caught, float cellM, float wdx, float wdz, float wind10);
    void ReachPlant(PlantFire& pf, float wdx, float wdz, const Vector3& cam);
    void WritePlant(const PlantFire& pf);
    void UpdatePlants(const std::vector<BurningView>& views, float wdx, float wdz, float wind10, const Vector3& cam);
    // ---- Buildings (buildings builder) ----
    // A building FuelAt sees (registered once: its kind, box, the bushes and
    // conifers against it; -1 for a shape too small to be one).
    int RegisterBuilding(Object* obj, const LODShape* shape, MapType mt);
    BuildingKind BuildingKindOfShape(const LODShape* shape, MapType mt);
    // The building whose footprint grown by `grow` metres holds (x, z), or -1;
    // whether any footprint and apron touch the 8 m cell at (x, z) or the
    // squares beside it (its run on).
    int BuildingAt(float x, float z, float grow) const;
    bool BuildingsNear(float x, float z) const;
    // The passes (the drawn front reaching a building's apron), the catches
    // and the burning: its sources in _objectFires, its reach, its embers,
    // the stock destruction, its burnt look (Draw calls it after the plants).
    void UpdateBuildings(float dt);
    void PassBuilding(int idx, const char* why, float chance);
    void CatchBuilding(int idx, double delaySec, bool roofFirst, const char* why);
    void BuildingFires(const Building& b, float tb, const BuildingPhase& ph, float wind10, float wdx, float wdz);
    void BuildingReach(int idx);
    void BuildingEmbers(int idx, float wind10, float wdx, float wdz);
    void WriteBuildingLook(Building& b, float tb, const BuildingPhase& ph);
    // (shell geometry) its burnt shell from its roof's fall on: stage 0 at
    // the fall, stage 1 once the roof is down (FireBuildingShell.hpp)
    void StageBuildingShell(Building& b, float tb);

  public:
    // (end-state agent) FireShell.hpp NotifyGutted / NotifyUngutted
    void QueueShell(Object* obj, bool gutted)
    {
        if (obj)
            (gutted ? _shellPending : _shellRepaired).push_back(Ref<Object>(obj));
    }
    bool ShellsPending() const { return !_shellPending.empty() || !_shellRepaired.empty(); }
    ShellMode ShellModeOfShape(const LODShape* shape);
    int ShellStageFor(const Object* obj) const;
    // (script, TEST AID) what stops a line from p to q: [fire hits, view hits, visibility 0..1]
    std::string ShellRay(Vector3Par p, Vector3Par q) const;

  private:
    void DrainShells();
    // A building gutted with no fire of its own here: burnt out for good.
    void RestoreShell(int idx);
    // A building's flames on a man at (x, z) (Gameplay): a flame length (m)
    // and its strength (0..1), as a grass cell's StrengthAt gives them.
    float BuildingHeatAt(float x, float z, float& flameM) const;
    // The ember ignition chance and the day's index (FireDayIndex) now.
    float PigNow() const { return EmberIgnitionChance(_moisture.m1h); }
    float DayNow() const;
    std::string BuildingsLine() const;
    // (script) a building near (x, z): its state, or one set alight now
    std::string BuildingState(float x, float z, bool ignite, bool roofFirst);
    std::vector<Building> _bld;
    std::unordered_map<const Object*, int> _bldByObj;
    std::unordered_map<std::uint64_t, std::vector<int>> _bldCells;
    std::unordered_map<const LODShape*, BuildingKind> _bldKind;
    std::unordered_set<std::uint64_t> _bldSeen;
    std::vector<int> _bldSched, _bldLive;
    double _bldLookAt = -1.0e9;
    float _objTime = 1.0f;
    int _bldPassed = 0, _bldCaught = 0, _bldDestroyed = 0, _bldEmbersOut = 0, _bldBurning = 0, _bldParts = 0,
        _bldSmoking = 0, _bldBlack = 0;
    double _bldMs = 0.0; // UpdateBuildings' CPU this frame (StatsLine bms=)
    // (end-state agent) buildings gutted on this machine with no fire of
    // their own here (a remote machine's, a load, a clear): their burnt look
    // for good at the next frame (DrainShells); repaired ones lose it
    std::vector<Ref<Object>> _shellPending, _shellRepaired;
    int _bldShells = 0; // shells restored (StatsLine)
    // The plume (FirePlume.hpp, Smokes.hpp PlumePuff): its sources, its puffs
    // and the flames' light on them (PlumeFireLight).
    void EmitPlume(float dt, const std::vector<BurningView>& views, Vector3Par cam, float wx, float wz);
    // (followups) the object fires' own columns (FirePlume.hpp PlanObjectColumn)
    void EmitObjectPlumes(float dt, Vector3Par cam, float w10, float dirX, float dirZ, LODShapeWithShadow* shape);

    // The plume's shadow (GetPlumeShadow), from the puffs every kPlumeShadowSec
    // (_plumeShadowTarget, a blob per stretch), eased into _plumeShadow.
    void UpdatePlumeShadow(float dt, float wx, float wz);
    void RebuildPlumeShadow(float wx, float wz);

  public:
    bool PlumeLightAt(Vector3Par p, float rgb[3]) const;
    float PlumeMW() const { return _plumeMW; }
    bool PlumeShadow(PlumeShadowView& out) const
    {
        out = _plumeShadow;
        return out.count > 0;
    }

  private:
    void Gameplay(float dt, float windX, float windZ);
    // The flames' measurements since the last stats line (StatsLine fl_*):
    // per class, the model's length at each drawn tongue, the drawn flame's
    // length along its axis and its top's height over the ground (the
    // renderer draws the tip there exactly), its tilt, and what v4's flame
    // code would have drawn for the same tongue (its visible tip: the stacked
    // pieces with the flipbook's measured tip share, FlameTipShare).
    struct FlameMeasure
    {
        long long n = 0, lead = 0, lead4 = 0;
        double model = 0.0, drawn = 0.0, tilt = 0.0, size = 0.0;
        double leadModel = 0.0, leadDrawn = 0.0, leadTop = 0.0, lead4Drawn = 0.0, lead4Top = 0.0;
        float drawnMax = 0.0f, topMax = 0.0f, modelMax = 0.0f, v4Max = 0.0f, v4TopMax = 0.0f, sizeMax = 0.0f;
    };
    FlameMeasure _measure[4];
    void Measure(int cls, float model, float drawn, float top, float tilt, float share, float size = 0.0f);
    void MeasureV4(const BurningView& v, std::uint32_t cell, int k, float ox, float oz, float cellM, float wind10);

    std::unique_ptr<FireGrid> _grid;
    const Landscape* _land = nullptr;
    Moisture _moisture;
    SmokeField _smoke;
    float _smokeLive = 0.0f;
    std::unordered_map<const Texture*, std::uint8_t> _texFuel;
    std::unordered_map<const Texture*, float> _texShare;
    std::unordered_map<const LODShape*, TreeKind> _treeKind; // KindOf's verdicts
    std::unordered_map<const RoadLink*, float> _roadWidth;   // RoadWidth's, by link
    std::unordered_map<const void*, float> _fleeUntil;
    // The plants that flare (FuelAt, by their 8 m cell), those flaring now
    // and those already reached (flared or scorched), by their position.
    std::unordered_map<std::uint64_t, std::vector<Plant>> _plants;
    std::vector<Flare> _flares;
    std::unordered_set<std::uint64_t> _flared;
    std::vector<FlameSheet> _volSheets; // (fix 6) the volume tongues, moved to the end of the list far to near
    // Each plant's own fire (UpdatePlants): the plants, by their foot
    // (PlantKey), the 8 m cells registered with their plants, the cells seen
    // burning, the plants a seen cell's flames will reach (scheduled), the
    // table the renderer reads and its version (bumped at most kPlantTableHz
    // a second), and the counts for the stats line.
    std::vector<PlantFire> _pf;
    std::unordered_map<std::uint64_t, int> _pfByKey;
    std::unordered_map<std::uint64_t, std::vector<int>> _pfCells;
    std::unordered_set<std::uint64_t> _pfSeen;
    std::vector<int> _pfSched;
    PlantTable _pfTable;
    int _pfMaxSlots = kPlantSlotsMax, _pfStartSlots = kPlantSlotsStart; // (TEST AID plantmax)
    unsigned _pfVersion = 0;
    bool _pfDirty = false, _pfFull = false;
    double _pfBumpAt = -1.0e9, _pfFinalAt = 0.0;
    int _pfContacts = 0, _pfFlared = 0, _pfTorched = 0;
    // The cells that have burnt (by their 8 m cell), and Draw's frame count.
    std::unordered_map<std::uint64_t, Afterburn> _after;
    std::uint32_t _drawFrame = 0;
    // The lingering's measurements since the last stats line (StatsLine lg,
    // the consult's test 2): by time since the front (kLingerBinSec), the
    // burnt ground near the camera (m2, summed over frames), the flames
    // standing on it, and the ground under them (m2).
    double _lgArea[4] = {}, _lgPts[4] = {}, _lgFlame[4] = {};
    long long _lgFrames = 0;
    std::vector<BurningView> _views;
    std::vector<int> _pick;
    std::vector<float> _pickScore;       // by view index, this frame's
    float _lodBias = 0.0f;               // the tongues' level of detail lowered for the budget (Draw)
    std::vector<FlameSheet> _sheets;     // GetFlames, rebuilt each step in Draw
    std::vector<SurfaceTongue> _tongues; // one cell's surface tongues (Draw's scratch)
    // The burning bed (the realism audit's fix 7; Draw): its sheets, outside
    // the tongues' budget (kMaxBedSheets), first in the drawing order; and the
    // drawing order's scratch.
    std::vector<FlameSheet> _beds, _mainSheets;
    FootStats _foot; // the feet's speeds (POSEIDON_XP_FIRESPREAD_FOOTSTATS)
    bool _footOn = false;
    int _footCls = kFootBand; // the class of the tongues being pushed
    // The fire's light (UpdateLights): its slots (kFireLightsMax picked, up to
    // kLightFadeSlots fading out, and the plume's fill), the pick, and the
    // ground halo's cells (GetFlameLightCells).
    static constexpr int kLightFadeSlots = 3;
    static constexpr int kLightSlots = kFireLightsMax + kLightFadeSlots + 1;
    struct LightSlot
    {
        std::uint64_t id = 0; // its source (0: free)
        lighting::CombatLightHandle handle;
        float power = 0.0f;                 // its candela now, before the flicker
        float x = 0.0f, y = 0.0f, z = 0.0f; // where it stands (followed)
        float core = 1.0f, spread = 0.0f;   // its shape (followed)
        float swingHz = 0.0f, swingPhase = 0.0f, swingAmp = 0.0f, fastAmp = 0.0f, slowAmp = 0.0f;
        std::uint32_t seed = 0, slowSeed = 0;
        bool leaving = false;
    };
    LightSlot _lightSlots[kLightSlots];
    LightPick _lightPick;
    std::vector<FlameLightCell> _lightCells;
    void UpdateLights(float dt, Vector3Par cam, float wx, float wz);
    void ClearLights();
    FireSound _sound;
    bool _windOverride = false;
    float _windX = 0.0f, _windZ = 0.0f;
    static constexpr std::uint32_t kWanderSeed = 0x77a2d1c5u;
    float _wanderDeg = kWindWanderDeg; // the wind's wander (deg, standard deviation; 0 none)
    float _flameBudget = 60.0f, _smokeBudget = kPlumeRate;
    float _smokeCredit = 0.0f;
    // The plume (EmitPlume): its sources this frame, the fire's heat release
    // and smoke eased (the line law's), the flames' light by smoke block
    // (heat and its height, eased; PlumeLightAt) and the last emission's
    // numbers for the stats line.
    struct PlumeSource
    {
        float x = 0.0f, z = 0.0f, y = 0.0f; // where (world m; y the ground)
        float smoke = 0.0f;                 // its smoke (smoke units)
        float woody = 0.0f;                 // its woody share
        float baseM = 0.0f;                 // where its smoke leaves the flames
        float jitterM = 0.0f;               // how far round (x, z) it may leave
        float torchMW = 0.0f;               // a torching crown's heat (its own law), 0 else
        float pick = 0.0f;                  // its weight in the pick (EmitPlume)
        const BurningView* cell = nullptr;  // a surface cell's (left where its front has reached)
    };
    std::vector<PlumeSource> _plumeSrc;
    std::vector<float> _plumeCum;
    float _plumeMW = 0.0f, _plumeSmoke = 0.0f;
    std::vector<float> _glowHeat, _glowY; // by smoke block
    int _glowX0 = 0, _glowZ0 = 0, _glowNX = 0, _glowNZ = 0;
    float _plumeRate = 0.0f, _plumeLod = 1.0f, _plumeWidth = 0.0f;
    int _plumeBorn = 0;
    // (followups) The object fires' own columns (EmitObjectPlumes): one a
    // burning object (its parts' smoke together), its credit of puffs kept by
    // its key from frame to frame (a column not seen this frame goes), the
    // columns and their puffs a second this frame, the puffs born since the
    // last stats line, and the budget's share they had.
    struct ObjColumn
    {
        std::uint64_t key = 0;
        float x = 0.0f, z = 0.0f, y = 0.0f; // its smoke's middle (world m; y the ground)
        float smoke = 0.0f, black = 0.0f, mw = 0.0f, baseM = 0.0f, bornM = 0.0f, jitterM = 0.0f;
        ObjectColumnPlan plan;
    };
    struct ObjColumnCredit
    {
        float credit = 1.0f; // a new column's first puff at once
        std::uint32_t frame = 0;
    };
    std::vector<ObjColumn> _objCols;
    std::unordered_map<std::uint64_t, ObjColumnCredit> _objColCredit;
    std::uint32_t _objColFrame = 0;
    int _objPlumeBorn = 0;
    float _objPlumeRate = 0.0f, _objPlumeScale = 1.0f;
    PlumeShadowView _plumeShadow, _plumeShadowTarget;
    float _plumeShadowTimer = 0.0f;
    float _playTimer = 0.0f, _aiTimer = 0.0f;
    double _clock = 0.0;
    Ref<CloudletTTable> _smokeTable;
    // The sparks: their pool, the front within kFrontSparkFullM of the camera
    // last frame (m), and their cost (ms: the last frame's, the most since the
    // last stats line).
    SparkPool _sparks;
    float _sparkFrontM = 0.0f, _sparkGlareM = 0.0f;
    double _msSparks = 0.0, _msSparksMax = 0.0;
    Stats _stats;
    // The scorched ground map (GetBurnMap), rebuilt kBurnMapHz times a second
    // while anything flames (the char follows the front), once a second else.
    std::vector<unsigned char> _burn;
    std::vector<std::pair<float, float>> _burntScratch;
    std::vector<BurningView> _edgeScratch; // the burnt cells on the burn's edge (UpdateBurnMap)
    int _burnSize = 0;
    float _burnX0 = 0.0f, _burnZ0 = 0.0f, _burnTexel = 4.0f;
    unsigned _burnVersion = 0;
    static constexpr int kGroundSize = 128;
    std::vector<float> _groundY;
    unsigned _groundVersion = 0;
    float _burnTimer = 0.0f;
    int _burnLastBurnt = -1, _burnLastBurning = -1;
    // (the dance fix) The map before this one and its window (LatchBurnMap:
    // the char only ever grows), and the grid's clock and ignitions when it
    // was painted (fewer, or an earlier clock: the fire was cleared, the map
    // starts afresh).
    std::vector<unsigned char> _burnPrev;
    BurnWindow _burnPrevWin;
    double _burnPrevSim = 0.0;
    int _burnPrevIgn = 0;
    void UpdateBurnMap(float dt);

    // ---- Object fires (fire-objects; FireObjects.hpp ObjectFire) ----
    // This frame's object fires (cleared and filled every frame in Simulate
    // by their owners: UpdateWrecks, and the buildings' at their merge), the
    // brands waiting out their delay before they light, the draws' random
    // sources (the ground's, and the looks', so what lights never depends on
    // what the camera sees), the test aid's timeline factor
    // (POSEIDON_XP_FIRESPREAD_OBJTIME), the ground they lit by kind and the
    // flames drawn, for the stats line; the sound's views (the cells' and
    // theirs).
    std::vector<ObjectFire> _objectFires;
    struct ObjectPending
    {
        float x = 0.0f, z = 0.0f;
        double at = 0.0;
        std::uint64_t source = 0;
        ObjectIgniteKind kind = ObjectIgniteKind::Brand;
    };
    std::vector<ObjectPending> _objPending;
    ObjRand _objRand{0x0b1ec7f1U}, _objLookRand{0x2c9277b5U};
    float _objTimeK = 1.0f;
    int _objLit[kObjectIgniteKinds] = {};
    int _objDrawn = 0;
    std::vector<BurningView> _soundViews;
    // The ground at a point never takes fire from an object fire: water, a
    // road's bare surface (the cell's own fuel decides the rest).
    bool PointBare(float x, float z);
    // One draw's try (FireObjects.hpp ObjectFireDraws' `light`): true when
    // the ground there took fire, or holds fire already, or could have.
    bool ObjectLight(float x, float z, float chance, float delaySec, ObjectIgniteKind kind, std::uint64_t source);
    // The object fires' draws for dt (their heat and embers) and the brands due now.
    void ObjectDraws(float dt, float wx, float wz);
    // ---- Wrecks (fire-objects) ----
    // Each burning wreck: its vehicle (followed while it lives; once it is
    // gone its fire fades out over kWreckGoneSec), its timeline from its
    // class, where it was destroyed (the fire's clock), its box (the shape's
    // plan and height, its equivalent width), its blasts thrown, a blast's
    // flash, the ground it lit by kind and its own random source.
    struct Wreck
    {
        OLink<Object> obj;
        bool hadObject = false;
        std::uint64_t id = 0;
        WreckTimeline tl;
        double born = 0.0;
        // (followups) its own clock (s): real time times the test aid's
        // factor, accumulated, so the factor may change while it burns
        // (fireSpreadSet "objtime"; as a building's)
        double age = 0.0;
        float x = 0.0f, z = 0.0f, groundY = 0.0f, aboveM = 0.0f, ux = 1.0f, uz = 0.0f;
        float halfL = 2.0f, halfW = 0.9f, heightM = 1.5f, diamM = 2.5f;
        float gone = 0.0f;
        int blastsDone = 0;
        float ballR = 0.0f; // its fuel fireball's radius (m; 0 none)
        double flashUntil = -1.0;
        float flashX = 0.0f, flashZ = 0.0f;
        int lit[kObjectIgniteKinds] = {};
        float firstLitAge = -1.0f;
        ObjRand rng;
    };
    std::vector<Wreck> _wrecks;
    // (followups) Each wreck's look for the renderer (WreckLookOf), by its
    // vehicle: its char and its shell's glow, set by UpdateWrecks while it
    // burns and kept (its char, no glow) once its fire is over, while its
    // vehicle stands; at most kWreckLooksMax (the burnt-out ones go first).
    struct WreckLook
    {
        OLink<Object> obj;
        float charK = 0.0f, glow = 0.0f;
    };
    std::unordered_map<const Object*, WreckLook> _wreckLooks;
    bool _wreckBlast = false;
    std::uint64_t _objectIds = 0;
    void UpdateWrecks(float dt);
    void WreckBox(Wreck& w, const Object* o);
    ObjectFire WreckBodyOf(const Wreck& w) const;
    void WreckFireball(Wreck& w, float ballR);
};

std::uint8_t FireWorld::TextureFuel(Texture* tex)
{
    if (!tex)
        return 1;
    auto it = _texFuel.find(tex);
    if (it != _texFuel.end())
        return it->second;
    std::string name = (const char*)tex->GetName();
    std::string env = (const char*)tex->GetSoundEnv();
    std::string n = name + "|" + env;
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    auto has = [&](const char* s) { return n.find(s) != std::string::npos; };
    // Anderson models: cured summer grass is model 1 (short grass); crops and
    // tall meadow model 3; the forest floor model 9 or 10 (set below from the
    // forest objects); rock, sand, tarmac, concrete, mud and water nothing.
    std::uint8_t fm = 1;
    if (has("rock") || has("sand") || has("asphalt") || has("tarmac") || has("concrete") || has("road") ||
        has("water") || has("mud") || has("beton") || has("stone") || has("gravel") || has("pisek") || has("skala") ||
        has("silnice") || has("cesta") || has("more") || has("hard"))
        fm = 0;
    else if (has("pole") || has("field") || has("crop") || has("wheat"))
        fm = 3;
    else if (has("forest") || has("les") || has("wood"))
        fm = 9;
    _texFuel.emplace(tex, fm);
    LOG_INFO(Script, "UDFIRE;event=texture;name={};env={};fm={}", name.c_str(), env.c_str(), static_cast<int>(fm));
    return fm;
}

// The burnable share of a terrain texture. Island textures are blends of
// four corner materials named by two-letter codes (abel/ttshtttt.pac: three
// corners grass "tt", one rock "sh"); the share is the grass corners over
// four. A plain texture is all or nothing by its class (TextureFuel).
float FireWorld::TextureShare(Texture* tex)
{
    if (!tex)
        return 1.0f;
    auto it = _texShare.find(tex);
    if (it != _texShare.end())
        return it->second;
    std::string name = (const char*)tex->GetName();
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const std::size_t slash = name.find_last_of("\\/");
    std::string base = slash == std::string::npos ? name : name.substr(slash + 1);
    const std::size_t dot = base.find('.');
    if (dot != std::string::npos)
        base = base.substr(0, dot);
    float share = TextureFuel(tex) != 0 ? 1.0f : 0.0f;
    if (base.size() == 8 && std::all_of(base.begin(), base.end(), [](unsigned char c) { return std::isalnum(c); }))
    {
        int grass = 0;
        for (int k = 0; k < 4; ++k)
        {
            const std::string code = base.substr(2 * k, 2);
            if (code == "tt" || code == "lt" || code == "ls")
                grass++;
        }
        share = grass / 4.0f;
        // A blend the engine calls grass with no corner code known as grass: mostly grass.
        if (grass == 0 && TextureFuel(tex) != 0)
            share = 0.75f;
    }
    _texShare.emplace(tex, share);
    return share;
}

float FireWorld::SquareShare(int i, int j)
{
    const int r = LandRange;
    i = std::clamp(i, 0, r - 1);
    j = std::clamp(j, 0, r - 1);
    return TextureShare(GLandscape->GetTexture(GLandscape->GetTexture(j, i)));
}

// A tree's kind from its shape's name (the islands' trees are named in
// Czech: smrk spruce, jedle fir, borovice pine, pinie stone pine, modrin
// larch, jalovec juniper, cypris cypress, jehlicnaty coniferous; listnaty
// deciduous). A tree is a broadleaf unless its name says conifer; a forest
// block is a conifer forest unless its name says deciduous (the forest
// blocks the islands ship are spruce and pine plantations). Each shape's
// verdict is logged once (UDFIRE;event=treekind). Mountain pine (borovice
// horska, Pinus mugo) and spruce (smrk) keep their foliage to the ground;
// olive (oliva) is the one broadleaf that can partly torch (the fire science
// consult, 2026-11-16).
TreeKind FireWorld::KindOf(const LODShape* shape, bool forest)
{
    auto it = _treeKind.find(shape);
    if (it != _treeKind.end())
        return it->second;
    std::string n = shape ? (const char*)shape->GetName() : "";
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    auto has = [&](const char* s) { return n.find(s) != std::string::npos; };
    const bool needle = has("smrk") || has("jedl") || has("borov") || has("pini") || has("pine") || has("modrin") ||
                        has("jalov") || has("cypr") || has("cedr") || has("jehl") || has("spruce") || has("conif") ||
                        has("larch") || has("junip") || has("tuje") || has("thuja");
    const bool leaf = has("listn") || has("decid") || has("broadleaf");
    TreeKind kind = TreeKind::Broadleaf;
    if (forest)
        kind = leaf ? TreeKind::Broadleaf : TreeKind::Conifer;
    else if (has("horsk") || has("mugo"))
        kind = TreeKind::MountainPine;
    else if (has("smrk") || has("spruce"))
        kind = TreeKind::Spruce;
    else if (needle)
        kind = TreeKind::Conifer;
    else if (has("oliv"))
        kind = TreeKind::Olive;
    _treeKind.emplace(shape, kind);
    LOG_INFO(Script, "UDFIRE;event=treekind;shape={};forest={};kind={}", n.c_str(), forest ? 1 : 0,
             static_cast<int>(kind));
    return kind;
}

float FireWorld::RoadWidth(const RoadLink* link)
{
    auto it = _roadWidth.find(link);
    if (it != _roadWidth.end())
        return it->second;
    // From its shape: the left and right begin points (LB, PB) as the road
    // net reads them, else its roadway's extent across its local z.
    float w = 0.0f;
    Object* obj = link->GetObject();
    LODShape* shape = obj ? obj->GetShape() : nullptr;
    if (shape)
    {
        Shape* mem = shape->MemoryLevel();
        if (mem && mem->FindNamedSel("LB") >= 0 && mem->FindNamedSel("PB") >= 0)
        {
            const Vector3 d = mem->NamedPosition("LB") - mem->NamedPosition("PB");
            w = std::sqrt(d.X() * d.X() + d.Z() * d.Z()) * obj->Scale();
        }
        if (w < 0.5f)
        {
            if (Shape* road = shape->RoadwayLevel())
                w = (road->Max().X() - road->Min().X()) * obj->Scale();
        }
    }
    if (w < 0.5f)
        w = kRoadDefaultWidthM;
    w = std::clamp(w, 1.0f, 30.0f);
    _roadWidth.emplace(link, w);
    LOG_INFO(Script, "UDFIRE;event=road;shape={};width={:.2f};bare={:.2f}", shape ? (const char*)shape->GetName() : "?",
             w, kRoadBareShare * w);
    return w;
}

bool FireWorld::RoadNear(float x, float z, RoadHit& out)
{
    if (!GRoadNet)
        return false;
    const int xx = static_cast<int>(std::floor(x * InvLandGrid)), zz = static_cast<int>(std::floor(z * InvLandGrid));
    float best = kRoadReachM;
    bool found = false;
    for (int zi = zz - 1; zi <= zz + 1; ++zi)
        for (int xi = xx - 1; xi <= xx + 1; ++xi)
        {
            if (!InRange(xi, zi))
                continue;
            const RoadList& list = GRoadNet->GetRoadList(xi, zi);
            for (int k = 0; k < list.Size(); ++k)
            {
                const RoadLink* link = list[k];
                if (!link || link->NConnections() < 2)
                    continue;
                const Vector3* con = link->PosConnections();
                // Its centre line: begin to end (a crossing's second road its other pair).
                for (int p = 0; p + 1 < link->NConnections(); p += 2)
                {
                    const float ax = con[p].X(), az = con[p].Z();
                    const float ex = con[p + 1].X() - ax, ez = con[p + 1].Z() - az;
                    const float el2 = ex * ex + ez * ez;
                    if (el2 < 1e-4f)
                        continue;
                    const float t = std::clamp(((x - ax) * ex + (z - az) * ez) / el2, 0.0f, 1.0f);
                    const float px = ax + t * ex, pz = az + t * ez;
                    const float d = std::sqrt((x - px) * (x - px) + (z - pz) * (z - pz));
                    if (d >= best)
                        continue;
                    const float width = RoadWidth(link);
                    // A track too narrow to be a gap is thin fuel, not a barrier.
                    if (width < kRoadMinWidthM)
                        continue;
                    const float el = std::sqrt(el2);
                    best = d;
                    found = true;
                    out.px = px;
                    out.pz = pz;
                    out.dx = ex / el;
                    out.dz = ez / el;
                    out.halfBare = 0.5f * kRoadBareShare * width;
                    out.dist = d;
                }
            }
        }
    return found;
}

Fuel FireWorld::FuelAt(float x, float z)
{
    Fuel f;
    if (!GLandscape)
        return f;
    float dX = 0.0f, dZ = 0.0f;
    Texture* tex = nullptr;
    const float y = GLandscape->SurfaceY(x, z, &dX, &dZ, &tex);
    if (y < GLandscape->GetSeaLevelNoWave() + 0.3f)
        return f;
    // Roads are barriers, not fuel-less margins (the fire science consult's
    // ninth answer): only the bare running surface carries no fuel; the
    // verges keep theirs to its edge, and every cell near a road keeps its
    // line (the model's crossing rule, the char's and the flames' bare edge).
    {
        RoadHit road;
        if (RoadNear(x, z, road))
        {
            f.roadOffX = road.px - x;
            f.roadOffZ = road.pz - z;
            f.roadDirX = road.dx;
            f.roadDirZ = road.dz;
            f.roadHalfM = road.halfBare;
            if (road.dist < road.halfBare)
                return f;
        }
    }
    f.model = TextureFuel(tex);
    {
        // The burnable share as a continuous field between the 50 m squares'
        // centres (smoothstep weights), against a threshold that varies per
        // cell, so a burn's edge follows the visible ground instead of the
        // squares' straight edges.
        const float u = x * InvLandGrid - 0.5f, v = z * InvLandGrid - 0.5f;
        const int i0 = static_cast<int>(std::floor(u)), j0 = static_cast<int>(std::floor(v));
        float fu = u - i0, fv = v - j0;
        fu = fu * fu * (3.0f - 2.0f * fu);
        fv = fv * fv * (3.0f - 2.0f * fv);
        const float share = (SquareShare(i0, j0) * (1 - fu) + SquareShare(i0 + 1, j0) * fu) * (1 - fv) +
                            (SquareShare(i0, j0 + 1) * (1 - fu) + SquareShare(i0 + 1, j0 + 1) * fu) * fv;
        std::uint32_t hsh = static_cast<std::uint32_t>(static_cast<int>(std::floor(x / 8.0f))) * 0x8da6b343u ^
                            static_cast<std::uint32_t>(static_cast<int>(std::floor(z / 8.0f))) * 0xd8163841u;
        hsh ^= hsh >> 13;
        hsh *= 0x5bd1e995u;
        hsh ^= hsh >> 15;
        const float threshold = 0.35f + 0.3f * static_cast<float>(hsh & 0xffffu) / 65535.0f;
        if (share < threshold)
            f.model = 0;
        else if (f.model == 0)
            f.model = 1; // a grass corner of a mostly rocky square
    }
    // Map objects: forest blocks give a forest floor under a canopy, a tree
    // near the cell's centre a canopy, a bush brush, a building or wall bare ground.
    const int gx = static_cast<int>(std::floor(x * InvLandGrid)), gz = static_cast<int>(std::floor(z * InvLandGrid));
    bool forest = false, built = false;
    int trees = 0, bigTrees = 0;
    // The tallest tree standing in the cell itself (the 8 m square round the
    // query point) and the tallest shrub near it: their heights from their
    // shapes' extent, so a torching crown's flame and a shrub's are sized by
    // what grows there.
    float treeH = 0.0f, treeX = 0.0f, treeZ = 0.0f, shrubH = 0.0f, forestH = 0.0f;
    TreeKind treeKind = TreeKind::Broadleaf;
    bool forestConifer = false;
    // The plants standing in the cell (its square round the query point, the
    // cell's centre) that flare, and how much of it each kind of bush covers.
    const float cellM = _grid ? _grid->GetParams().cellM : 8.0f;
    const float half = 0.5f * cellM;
    std::vector<Plant> plants;
    float coverThicket = 0.0f, coverLeafy = 0.0f, coverReed = 0.0f, thicketH = 0.0f;
    auto plantOf = [&](const Object* obj, const LODShape* shape, PlantClass cls)
    {
        Plant p;
        p.x = obj->Position().X();
        p.z = obj->Position().Z();
        p.y = GLandscape->SurfaceY(p.x, p.z);
        p.h = (shape->Max().Y() - shape->Min().Y()) * obj->Scale();
        p.r = 0.25f * ((shape->Max().X() - shape->Min().X()) + (shape->Max().Z() - shape->Min().Z())) * obj->Scale();
        p.cls = cls;
        p.name = (const char*)shape->GetName();
        return p;
    };
    for (int cz = gz - 1; cz <= gz + 1; ++cz)
        for (int cx = gx - 1; cx <= gx + 1; ++cx)
        {
            if (!InRange(cz, cx))
                continue;
            const ObjectList& list = GLandscape->GetObjects(cz, cx);
            for (int i = 0; i < list.Size(); ++i)
            {
                Object* obj = list[i];
                if (!obj || !obj->GetShape())
                    continue;
                const LODShape* shape = obj->GetShape();
                const MapType mt = shape->GetMapType();
                const Vector3 pos = obj->Position();
                const float dx = pos.X() - x, dz = pos.Z() - z;
                const float d2 = dx * dx + dz * dz;
                if (mt == MapForestSquare || mt == MapForestTriangle || mt == MapForestBorder)
                {
                    if (d2 > 80.0f * 80.0f)
                        continue;
                    const Vector3 m = obj->WorldInvTransform().FastTransform(Vector3(x, y, z));
                    const Vector3 lo = shape->Min(), hi = shape->Max();
                    if (m.X() >= lo.X() && m.X() <= hi.X() && m.Z() >= lo.Z() && m.Z() <= hi.Z())
                    {
                        forest = true;
                        forestH = std::max(forestH, (hi.Y() - lo.Y()) * obj->Scale());
                        forestConifer = forestConifer || KindOf(shape, true) != TreeKind::Broadleaf;
                    }
                }
                else if ((mt == MapTree || mt == MapSmallTree) && d2 < 9.0f * 9.0f)
                {
                    // An olive (low, open, cultivated) is a plant that flares,
                    // never a tree of a canopy: a grove keeps its ground fuel
                    // (the consult's species answer).
                    if (KindOf(shape, false) == TreeKind::Olive)
                    {
                        if (std::fabs(dx) <= half && std::fabs(dz) <= half)
                            plants.push_back(plantOf(obj, shape, PlantClass::Olive));
                        continue;
                    }
                    trees++;
                    if (mt == MapTree)
                        bigTrees++;
                    const float h = (shape->Max().Y() - shape->Min().Y()) * obj->Scale();
                    if (std::fabs(dx) <= 4.0f && std::fabs(dz) <= 4.0f && h > treeH)
                    {
                        treeH = h;
                        treeX = dx;
                        treeZ = dz;
                        treeKind = KindOf(shape, false);
                    }
                }
                else if (mt == MapBush)
                {
                    if (d2 < 6.0f * 6.0f)
                        shrubH = std::max(shrubH, (shape->Max().Y() - shape->Min().Y()) * obj->Scale());
                    if (std::fabs(dx) <= half && std::fabs(dz) <= half)
                    {
                        // Its class by its shape's name (the consult's species answer for Abel:
                        // rakosi a reed clump, listnac a leafy bush, krovi and the rest thickets).
                        std::string n = (const char*)shape->GetName();
                        std::transform(n.begin(), n.end(), n.begin(),
                                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                        const PlantClass cls = n.find("rakosi") != std::string::npos    ? PlantClass::Reed
                                               : n.find("listnac") != std::string::npos ? PlantClass::Leafy
                                                                                        : PlantClass::Thicket;
                        Plant p = plantOf(obj, shape, cls);
                        const float cover = 3.14159265f * p.r * p.r;
                        if (cls == PlantClass::Reed)
                            coverReed += cover;
                        else if (cls == PlantClass::Leafy)
                            coverLeafy += cover;
                        else
                        {
                            coverThicket += cover;
                            thicketH = std::max(thicketH, p.h);
                        }
                        plants.push_back(std::move(p));
                    }
                }
                else if (mt == MapHouse || mt == MapBuilding || mt == MapChurch || mt == MapChapel || mt == MapBunker ||
                         mt == MapFortress || mt == MapFuelstation || mt == MapHospital)
                {
                    // (buildings builder) Its own footprint and an apron of
                    // kBuildingApronM carry no fuel (the tenth answer), not a
                    // 14 m disc round its middle whatever its size: the grass
                    // round it burns up to the apron (the cells beside a bare
                    // one draw on into it: builtSides).
                    const int bi = RegisterBuilding(obj, shape, mt);
                    if (bi >= 0 && _bld[static_cast<std::size_t>(bi)].Inside(x, z, kBuildingApronM))
                        built = true;
                }
                else if (mt == MapFence)
                    RegisterBuilding(obj, shape, mt); // (a wooden fence: no apron, the grass burns to its foot)
            }
        }
    if (built)
    {
        f.model = 0;
        f.built = true;
        return f;
    }
    {
        const std::uint64_t key = PlantCellKey(x, z, cellM);
        if (plants.empty())
            _plants.erase(key);
        else
            _plants[key] = std::move(plants);
    }
    // Three or more trees within 9 m of the cell's centre close the canopy: a
    // forest floor (timber litter and understory, model 10) in sheltered air.
    // One or two are open-grown trees in the grass: the grass fire runs under
    // them in the open wind and can torch their low crowns.
    if (forest && forestH < 0.5f)
        forestH = kTorchDefaultTreeM;
    f.treeHeightM = treeH > 0.0f ? treeH : forestH;
    const bool needles =
        treeKind == TreeKind::Conifer || treeKind == TreeKind::Spruce || treeKind == TreeKind::MountainPine;
    f.conifer = treeH > 0.0f ? needles : forestConifer;
    f.olive = treeH > 0.0f && treeKind == TreeKind::Olive;
    f.treeOffX = treeX;
    f.treeOffZ = treeZ;
    f.shrubHeightM = shrubH;
    if (forest || (f.model != 0 && trees >= 3))
    {
        f.model = 10;
        f.canopy = true;
        f.sheltered = true;
        f.canopyBaseM = treeH > 0.0f && treeKind == TreeKind::MountainPine ? kMountainPineBaseM : 4.0f;
    }
    else if (f.model != 0)
    {
        if (trees > 0)
        {
            f.canopy = true;
            // Open-grown trees keep low branches: a small tree's crown starts
            // about 1.5 m up, a grown tree's about 3 m (Van Wagner's CBH).
            f.canopyBaseM = bigTrees > 0 ? 3.0f : 1.5f;
            // Mountain pine and open-grown spruce keep foliage to the ground
            // (the consult: crown base about 0.3 m and 0.5 m).
            if (treeH > 0.0f && treeKind == TreeKind::MountainPine)
                f.canopyBaseM = kMountainPineBaseM;
            else if (treeH > 0.0f && treeKind == TreeKind::Spruce)
                f.canopyBaseM = kSpruceBaseM;
        }
        // Bushes change the cell's fuel only when they cover much of it
        // (CoverModel: the consult's cover rule).
        f.model = CoverModel(f.model, coverThicket, coverLeafy, coverReed, cellM * cellM, thicketH);
    }
    return f;
}

void FireWorld::Simulate(float dt)
{
    if (!_grid || dt <= 0.0f)
        return;
    const auto t0 = Clock::now();
    _clock += dt;
    if (ShellsPending())
        DrainShells();
    // Rain wets the fine fuel past every model's extinction moisture.
    Moisture m = _moisture;
    if (GLandscape && GLandscape->GetRainDensity() > 0.05f)
    {
        m.m1h = std::max(m.m1h, 0.35f);
        m.m10h = std::max(m.m10h, 0.35f);
        m.m100h = std::max(m.m100h, 0.35f);
    }
    if (m.m1h != _grid->GetParams().moisture.m1h)
        _grid->SetMoisture(m);
    float wx = 0.0f, wz = 0.0f;
    BaseWind(wx, wz);
    _grid->Tick(dt, wx, wz);
    Wind(wx, wz);
    const auto t1 = Clock::now();
    _stats.msGrid = MsSince(t0);

    _grid->CollectBurning(_views);
    // ---- Object fires (fire-objects): this frame's from their owners, and their draws ----
    _objectFires.clear();
    UpdateWrecks(dt);
    // (buildings builder) the buildings' passes, catches and fires
    const auto tb0 = Clock::now();
    UpdateBuildings(dt);
    _bldMs = MsSince(tb0);
    ObjectDraws(dt, wx, wz);
    float bx0, bz0, bx1, bz1;
    if (_grid->Bounds(bx0, bz0, bx1, bz1))
        _smoke.Cover(bx0, bz0, bx1, bz1);
    if (!_objectFires.empty())
    {
        // (fire-objects) the smoke field's window over them too: the plume's
        // glow is kept by its blocks (they add no smoke to it: AI sight and
        // the soldiers' fear stay the grass fire's)
        float ox0 = 1e30f, oz0 = 1e30f, ox1 = -1e30f, oz1 = -1e30f;
        for (const ObjectFire& of : _objectFires)
        {
            ox0 = std::min(ox0, of.x);
            oz0 = std::min(oz0, of.z);
            ox1 = std::max(ox1, of.x);
            oz1 = std::max(oz1, of.z);
        }
        _smoke.Cover(ox0, oz0, ox1, oz1);
    }
    if (_smoke.Valid())
    {
        std::fill(_smoke.src.begin(), _smoke.src.end(), 0.0f);
        std::fill(_smoke.heat.begin(), _smoke.heat.end(), 0.0f);
        std::fill(_smoke.woody.begin(), _smoke.woody.end(), 0.0f);
        for (const BurningView& v : _views)
        {
            const int bx = SmokeField::Block(v.x), bz = SmokeField::Block(v.z);
            if (!_smoke.In(bx, bz))
                continue;
            const int k = _smoke.Idx(bx, bz);
            // Smoke grows with the fuel burnt per second (about the square
            // root of intensity here) times the smoke a fuel makes per
            // kilogram: by fuel, relative to grass, kSmokeShrub, kSmokeTimber,
            // kSmokeCrown (the fire science consult, 2026-11-16: grass 1,
            // shrub 5 to 10, conifer crown 10 to 20 per square metre burnt,
            // from fuel load times emission factor, Akagi et al. 2011).
            const float fuel =
                v.torching ? kSmokeCrown
                           : (v.model >= 4 && v.model <= 7 ? kSmokeShrub : (v.model >= 8 ? kSmokeTimber : 1.0f));
            const float src = kSmokeGrassK * v.strength * std::sqrt(std::max(v.intensityKwM, 1.0f)) * fuel;
            _smoke.src[k] += src;
            _smoke.woody[k] += src * (v.torching || v.model >= 4 ? 1.0f : 0.0f);
            _smoke.heat[k] += v.strength * v.flameM;
        }
        // The plants flaring (Draw's last frame): FlareSmoke, FlareHeat.
        for (const Flare& fl : _flares)
        {
            const int bx = SmokeField::Block(fl.p.x), bz = SmokeField::Block(fl.p.z);
            if (fl.strength <= 0.0f || !_smoke.In(bx, bz))
                continue;
            const int k = _smoke.Idx(bx, bz);
            const float woody = FlareShapeOf(fl.p.cls, fl.p.h).woody;
            const float src = FlareSmoke(fl, woody);
            _smoke.src[k] += src;
            _smoke.woody[k] += src * woody;
            _smoke.heat[k] += FlareHeat(fl, _grid->GetParams().cellM);
        }
        // Fresh burnt ground (the records Draw keeps): kAfterSmokeShare.
        for (const auto& [key, r] : _after)
        {
            const BurningView& v = r.v;
            const float since = static_cast<float>(_clock - r.caught) - 0.5f * v.crossSec - r.trDraw;
            const bool wood = v.model >= 4;
            const float fade = since > 0.0f ? std::exp(-since / (wood ? kAfterSmokeTimberSec : kAfterSmokeSec)) : 0.0f;
            const int bx = SmokeField::Block(v.x), bz = SmokeField::Block(v.z);
            if (fade < 0.01f || !_smoke.In(bx, bz))
                continue;
            const int k = _smoke.Idx(bx, bz);
            const float fuel = wood ? (v.model <= 7 ? kSmokeShrub : kSmokeTimber) : 1.0f;
            const float src = kAfterSmokeShare * kSmokeGrassK * std::sqrt(std::max(v.intensityKwM, 1.0f)) * fuel * fade;
            _smoke.src[k] += src;
            _smoke.woody[k] += src * (wood ? 0.3f : 0.0f);
        }
        _smoke.Step(dt, wx * 0.8f, wz * 0.8f);
        float live = 0.0f;
        for (float d : _smoke.d)
            live += d;
        _smokeLive = live;
    }
    const auto t2 = Clock::now();
    _stats.msSmoke = std::chrono::duration<double, std::milli>(t2 - t1).count();

    Draw(dt, _views);
    if (GLOB_SCENE && GLOB_SCENE->GetCamera())
    {
        // (fire-objects) the object fires crackle and roar as cells of their
        // heat: a car's crackles, a truck's and a pool's roar, a jet roars
        const std::vector<BurningView>* sv = &_views;
        if (!_objectFires.empty())
        {
            _soundViews = _views;
            for (const ObjectFire& of : _objectFires)
            {
                if (of.flameM < 0.2f || of.heat <= 0.0f || of.shape == ObjectFireShape::Ball)
                    continue; // (a fireball's sound is the stock explosion's)
                BurningView v;
                v.x = of.x;
                v.z = of.z;
                v.y = of.groundY;
                v.strength = std::clamp(of.heat, 0.0f, 1.0f);
                v.intensityKwM = std::min(300.0f + 250.0f * of.heatMW, 6000.0f);
                v.model = 4;
                v.torching = of.shape == ObjectFireShape::Jet || of.heatMW > 50.0f;
                _soundViews.push_back(v);
            }
            sv = &_soundViews;
        }
        _sound.Update(dt, *sv, GLOB_SCENE->GetCamera()->Position());
    }
    const auto t3 = Clock::now();
    _stats.msDraw = std::chrono::duration<double, std::milli>(t3 - t2).count();

    Gameplay(dt, wx, wz);
    UpdateBurnMap(dt);
    _stats.msPlay = MsSince(t3);
    _stats.msTotal = MsSince(t0);
    _stats.msTotalMax = std::max(_stats.msTotalMax, _stats.msTotal);
}

namespace
{
// ---- The flames' tunables (FireWorld::Draw) ----
// A fireline this weak (kW/m: low grass flames under about 0.6 m) creeps.
constexpr float kCreepBelowKwM = 120.0f;
// The flames' tempo is FireSpreadWorld.hpp's (FeatureRiseMps, LickHz): every
// tongue's visible features climb it at the speed of a free flame of its own
// full length (its flipbook's pulses come with it: the gas rises about 0.65 L
// a pulse, gen_flame_flipbook.py, so it licks at about 2.3 / sqrt(L) Hz), a
// grass or shrub tongue quickened by its midflame wind (LickWindFactor). Each
// tongue's length and rise are fixed for its life: the renderer plays its
// flipbook from the fire's clock, so a rate that drifted would race it.
// A torching column's tongues each keep their own length's tempo, never the
// column's (a small part with the big flame's speed through its own short
// length reads as frantic flicker: the consult), quickened by kTorchDraught
// for the column's draught (up to 1.3). The column puffs as a whole
// (ColumnPuffHz: about 0.6 Hz for a lone pine), the bulge climbing it at
// kBulgeLowK sqrt(z) m/s through its lower kPuffLowShare, then steady at
// kBulgeTopK sqrt(L) (half the centreline gas speed, McCaffrey 1979), so it
// takes about 2.5 s to run a 14 m column; as it passes, the tongues swell
// (kTorchBulge, below) and brighten by up to kPuffGlow (the big, slow puff
// dominating what the eye sees, the tongues' own licks a ripple on it).
constexpr float kTorchDraught = 1.2f;
constexpr float kBulgeLowK = 3.4f, kBulgeTopK = 2.15f, kPuffLowShare = 0.4f;
constexpr float kPuffGlow = 0.25f;
// (fix 6 follow-up) A volume tongue is not stretched by the puff: its length
// and width hold steady, and the puff is the bulge the renderer runs up it
// (FlameSheet::volumeBulge: a torch tongue widens by kTorchBulge of its width
// where the bulge is, a flare's by kFlareBulge), brighter where it swells.
constexpr float kTorchBulge = 0.16f, kFlareBulge = 0.08f;
// The bulge's time (s) to climb s metres of a torch column whose tempo length
// is Lnom: kBulgeLowK sqrt(z) through its lower kPuffLowShare, then steady.
// The drawn column (Draw) and its light (UpdateLights) both read it, so the
// light brightens as the bulge passes its point.
inline float TorchBulgeSec(float s, float Lnom)
{
    s = std::max(s, 0.0f);
    const float sLow = kPuffLowShare * Lnom;
    return s <= sLow ? 2.0f * std::sqrt(s) / kBulgeLowK
                     : 2.0f * std::sqrt(sLow) / kBulgeLowK + (s - sLow) / (kBulgeTopK * std::sqrt(Lnom));
}
// (fire-objects) An object fire's flame lean (rad): the tilt law
// (FlameTiltRad) in the wind at its mid height (a log profile from the 10 m
// wind, roughness kTorchWindZ0), at most kTorchTiltMax: a car's 4 m flames
// about 33 degrees in a 4.5 m/s wind and 50 in 12.5, a pool's 22 m about 24
// and 50 (the torches' law: one lean for every big flame).
inline float ObjectFlameTilt(float wind10, float flameM)
{
    const float L = std::max(flameM, 0.1f);
    const float z = std::max(0.5f * L, 0.2f);
    const float u = std::max(wind10, 0.0f) * std::log(z / kTorchWindZ0) / std::log(10.0f / kTorchWindZ0);
    return std::min(FlameTiltRad(std::max(u, 0.0f), L), kTorchTiltMax);
}
// (fire-objects) An object fire's flames: kObjMainPerM main tongues a metre
// of its footprint's half sizes (at least kObjMainMin, a box's kObjMainBox
// more, a pool's kObjMainPool more, at most kObjMainMax), kObjFillShare as
// many fill tongues low and fat round its base; the main tongues' tips drawn
// kObjConverge of the way in toward its leaning axis (the fill's
// kObjConvergeFill), its body necking kObjNeckShare of its flames up: one
// flame body with many tips, as a pool fire burns (the consult: a car's
// flames from every opening merging over it, a tanker's pool one wide column).
constexpr float kObjMainPerM = 2.6f, kObjMainBox = 4.0f, kObjMainPool = 6.0f, kObjFillShare = 1.0f;
constexpr int kObjMainMin = 4, kObjMainMax = 28, kObjFillMax = 22;
constexpr float kObjConverge = 0.55f, kObjConvergeFill = 0.25f, kObjNeckShare = 0.3f;
// (a tongue from the footprint's edge is kObjEdgeShort shorter than one from
// its middle; a pool's tongues converge kObjPoolConverge times more: its
// flame a cone, necking above its base)
constexpr float kObjEdgeShort = 0.4f, kObjPoolConverge = 1.4f;
// (the object fires' tongues may take kObjSheetReserve past the grass fire's
// tongue budget: Draw; kMaxSheets plus it plus the beds' kMaxBedSheets stay
// under the renderer's 24576)
constexpr int kObjSheetReserve = 1500;
// Its tongues: the main ones 0.7 to 1.15 of its flames' length, the fill 0.3
// to 0.55; their bodies kObjMainWiden and kObjFillWiden of their frame's
// width; a box's feet up to kObjFootUpM into its body. A face's tongues
// leave out kObjFaceOut of their length before they climb (their axis bent
// kObjFaceBend: out first, then up the wall). A jet's are kObjJetWide of their
// length across and climb kObjJetRise faster than a free flame's (driven gas).
constexpr float kObjMainWiden = 1.15f, kObjFillWiden = 1.5f, kObjFootUpM = 0.5f;
constexpr float kObjFaceOut = 0.55f, kObjFaceBend = 0.5f, kObjJetWide = 0.32f, kObjJetRise = 1.6f;
constexpr int kMaxCells = 1200;   // burning cells drawn at most, the strongest and nearest
constexpr int kMaxSheets = 15000; // tongues at most (the renderer draws at most 24576)
// The burning bed's sheets at most, on top (the realism audit's fix 7: its own
// budget, so it never thins the tongues).
constexpr int kMaxBedSheets = 8000;
constexpr float kPieceM = 4.0f; // v4's flame pieces (MeasureV4)
// A tongue's full length varies about the model's by kTongueSpread either way
// (uniform: a standard deviation of 0.17 L, the consult's tip spread, so the
// top of a front is ragged, not a level row of tips; the realism audit's fix
// 7) and its width by kTongueWidthSpread (fixed for its life; the flipbook's
// pulses vary its tip from moment to moment by about 0.17 L more), and its
// lean's direction by up to kLeanJitterRad either side of the wind's (the
// gusts and eddies in a front): no two neighbours alike.
constexpr float kTongueSpread = 0.3f, kTongueWidthSpread = 0.25f, kLeanJitterRad = 0.17f;
// A front is many narrow tongues side by side (Finney et al. 2015: its peaks
// and troughs stand about 0.3 L apart; the consult). In each cell they stand
// on a grid fixed to the cell and turned to its front: kTongueSpacing of the
// cell's flame length apart along the front (at least kTongueSpacingMinM),
// in rows kRowSpacing of it apart through the flaming zone's depth (within
// kRowSpacingMinM..MaxM: rows at most about 1 m apart, so seen from above
// the bed between their feet is covered), each jittered by kGridJitter of
// its spacing, every other row shifted kRowStagger of a spacing along the
// front (so seen from straight downwind or upwind the rows behind fill the
// gaps between the tongues in front instead of lining up behind them: no
// picket fence; the realism audit's fix 7). A point flames as the front's
// flaming zone crosses it (SurfacePart: lit as the leading edge reaches it,
// its flame shortening as its fuel burns out), so the tongues light, burn and
// die with the front.
constexpr float kTongueSpacing = 0.3f, kTongueSpacingMinM = 0.18f, kRowStagger = 0.5f;
constexpr float kRowSpacing = 0.5f, kRowSpacingMinM = 0.25f, kRowSpacingMaxM = 1.0f;
constexpr float kGridJitter = 0.35f;
// (The feet no longer wander along the front: the consult's seventh round
// corrected its second, the peaks' sideways motion is the upper flame's:
// FireSpreadWorld.hpp kSwayBand, kPeakSwing.)
// The R2 low-discrepancy sequence's steps (1/g and 1/g^2, g the plastic
// number): any first n points of it cover the square evenly.
constexpr float kR2X = 0.7548777f, kR2Z = 0.5698403f;
// Far off the tongues thin out: past the distance where a tongue spacing
// subtends kLodRad of the reference view (FireSpreadWorld.hpp
// FlameLodDistance: 1.5 of its pixels at 1920 across, so the kept tongues
// are never told apart), every other tongue (by its index along its row, and
// every other row) fades out per doubling of the distance, the rest
// kLodWiden times wider per level, at most kLodWidenMax, and every kept one
// carries the light of those dropped (SurfaceGrid::gain, FlameSheet::gain):
// a far front keeps its cover and its light without thousands of tiny
// tongues (the realism audit's C13: the old 0.004, with the fade through the
// heat, lost about six sevenths of a front's light by 1 km).
constexpr float kLodRad = 0.0015f;
constexpr float kLodWiden = 1.4f, kLodWidenMax = 3.0f;
// The debris left burning behind the front (flamelets, glow points, stems,
// litter) thins out from twice that (3 of the reference view's pixels): its
// pieces lie scattered, not in a front, and each kept one carries the dropped
// ones' light, so a burnt field at night stays a field of points at a fifth
// of the pieces (fix 08: at 2 km by night the CPU drew about 2500 debris
// points a frame at kLodRad).
constexpr float kDebrisLodRad = 2.0f * kLodRad;
// The tongue budget: past kBudgetHigh of kMaxSheets every cell's level of
// detail is lowered (kBudgetRate levels a second), under kBudgetLow raised
// again, so the budget is met before any cell is cut.
constexpr float kBudgetHigh = 0.85f, kBudgetLow = 0.6f, kBudgetRate = 1.5f, kBudgetBiasMax = 4.0f;
// The glow points on fresh burnt ground: tiny, low and dim (kGlowM tall,
// kGlowHeat of a flame's light): kSparklePerCell a cell for kSparkleSecMin
// to Max just behind the band (the stalk bases glowing: the consult's red
// sparkle), and where each flamelet or piece of wood has burnt out.
constexpr float kGlowM = 0.12f, kGlowHeat = 0.35f;
constexpr int kSparklePerCell = 24;
constexpr float kSparkleSecMin = 10.0f, kSparkleSecMax = 20.0f;
// The debris that burns on behind the front (the wildfire consult,
// 2026-11-17, sixth round; FireWorld::Draw): in grass the flamelets
// (FlameletLifeSec) and a long burner (a stick, a dead stem or thistle) in
// kLongShare of the cells, 0.3 to 0.8 m of flame for 2 to 10 min, glowing on
// to kLongGlowMaxSec; kShrubStems stems in a shrub cell (1 to 3 min); in
// conifer litter kTwigsMin..Max twigs and cones (1 to 3 min), kBranchesMin..
// Max branches (2 to 8 min) and a log or two in most cells (10 to 20 min),
// halved in hardwood litter, and the foot of a standing trunk in
// kTrunkBaseShare of the cells with a tree (1 to 5 min); a torched tree's
// bark and the debris at its foot burn on 1 to 5 min after its crown. Each
// flames as a tuft pinned to its fuel (FireSpreadWorld.hpp TuftTiltRad and
// the kTuft tunables). Drawn within FireSpreadWorld.hpp FlameLingerRangeM of
// the camera (a few hundred metres by day, the horizon at night: bright
// points then; fix 08), measured within kLingerNearM by time since the front
// (kLingerBinSec, give or take kLingerBinHalf).
constexpr float kLongShare = 0.3f, kLongGlowMaxSec = 1200.0f;
constexpr int kShrubStems = 8;
constexpr float kTwigsMin = 10.0f, kTwigsMax = 20.0f, kBranchesMin = 5.0f, kBranchesMax = 10.0f;
constexpr float kTrunkBaseShare = 0.4f;
constexpr float kLingerNearM = 150.0f;
constexpr int kLingerBins = 4;
constexpr float kLingerBinSec[kLingerBins] = {10.0f, 30.0f, 60.0f, 120.0f};
constexpr float kLingerBinHalf[kLingerBins] = {3.0f, 5.0f, 10.0f, 15.0f};
// A tongue's visible emissivity and absorptivity (Kirchhoff: a flame hides
// what is behind it as much as it glows) seen side on: 1 - exp(-k S), S its
// thickness (kTongueThick of its length, about its base's width) and k the
// visible absorption coefficient (1/m) by its fuel (the consult: grass 2 to
// 3, so one grass tongue is about half opaque at its core and a front builds
// its density by stacking tongues; shrubs 4 to 6; a crown or torch 5 to 8).
// The renderer raises it toward end on (the sight line through more flame).
constexpr float kTongueThick = 0.2f;
constexpr float kAbsorbGrass = 2.5f, kAbsorbLitter = 3.0f, kAbsorbShrub = 5.0f, kAbsorbCrown = 6.5f;
// A torching crown (kTorchCrownShare of the tree's height across): one column
// of merging tongues (the consult: the crown itself the brightest, densest
// part, the flame wrapping it, the column streaming from its downwind side).
// kTorchTongues of them stand in the crown (feet spread through its cone, the
// R2 sequence), each leaning with the column and reaching on toward its tip
// (kTorchReach of the way from its foot, give or take kTorchReachSpread);
// kTorchFill more, kTorchFillLen of the tree's height long, stand in the
// crown's lower two thirds and rise through it, leaning less, lit kTorchCrownK
// hotter: the burning crown; kTorchShort more, short (kTorchShortLen of the
// column), escape sideways from the crown's edge. All fade in over
// kTorchFootFade of their length from a foot in the crown (not on the
// ground). The reach kTorchReach (give or take kTorchReachSpread) now sets
// only a main tongue's tempo length (its flipbook's rate, as before).
constexpr int kTorchTongues = 16, kTorchFill = 10, kTorchShort = 6;
constexpr float kTorchFootFade = 0.15f;
constexpr float kTorchCrownShare = 0.5f, kTorchReach = 0.85f, kTorchReachSpread = 0.3f;
constexpr float kTorchShortLen = 0.22f;
constexpr float kTorchFillLen = 0.45f, kTorchFillDraw = 1.2f, kTorchCrownK = 60.0f;
// (fix 6) The column's shape (FireSpreadWorld.hpp TorchColumnOf), its tongues
// drawn as volumes: the main tongues' tips stand between kTorchMainReachLo of
// the way from the tree's top to the column's tip and the tip itself (more
// toward it: the reach's share to kTorchMainReachPow), on the column's bent
// axis (TorchColumnOffset), kTorchMainSideLo to kTorchMainSideLo +
// kTorchMainSideSpan of its radius there off it, on their own side turned by
// up to half kTorchMainTurn either way, so the bundle converges raggedly;
// the fill tongues, kTorchFillDraw of their tempo's length, lean
// kTorchFillTilt of the column's lean; the
// short ones lean out from the crown's edge by kTorchShortOut against the
// column's lean. Their bodies are kTorchMainWiden, kTorchFillWiden and
// kTorchShortWiden of their frame's width (the main ones slimmer, so the
// column necks; the crown's fat), sooty to their tips (kTorchSoot). The
// column lights up the crown over kTorchClimb of its envelope, and its tip
// ratio eases toward the model's over kTorchTipEaseSec.
constexpr float kTorchMainReachLo = 0.15f, kTorchMainReachPow = 0.7f, kTorchMainTurn = 1.2f;
constexpr float kTorchMainSideLo = 0.2f, kTorchMainSideSpan = 0.6f;
constexpr float kTorchFillTilt = 0.6f, kTorchShortOut = 0.5f;
constexpr float kTorchMainWiden = 0.65f, kTorchFillWiden = 1.2f, kTorchShortWiden = 0.8f, kTorchSoot = 1.0f;
constexpr float kTorchClimb = 0.6f, kTorchTipEaseSec = 2.0f;

// The flame measurements (StatsLine fl_*): grass (fuel models 1 to 3),
// shrubs (4 to 7), timber litter (8 to 13), torching crowns.
constexpr int kMeasureGrass = 0, kMeasureShrub = 1, kMeasureTimber = 2, kMeasureTorch = 3, kMeasureClasses = 4;
// A tongue counts as the strip's leading edge (where Byram's length is
// measured in the field) from this share of its length on.
constexpr float kMeasureLead = 0.85f;

float Hash01(std::uint32_t h)
{
    h ^= h >> 16;
    h *= 0x7feb352dU;
    h ^= h >> 15;
    h *= 0x846ca68bU;
    h ^= h >> 16;
    return static_cast<float>(h & 0xffffffU) / 16777216.0f;
}
// A cell behind the camera claims kBehindShare of the budget it would in
// front, unless within kNearAnyWayM.
constexpr float kBehindShare = 0.2f, kNearAnyWayM = 25.0f;

// How much of the tongue at index i (along its row, or of a row) a level of
// detail lod keeps (0..1): the levels halve the tongues, the odd indices
// first, then those not a multiple of 4, and so on; each fades over its
// level, so none pops as the camera moves.
// How far (0..1) a band tongue is sunk now (kPeakSwing): every 2 to 5 s
// (its own period) it sinks for about a third of it, fading down and up
// again over 0.15 of it, out of step with its neighbours.
float PeakSink(std::uint32_t key, float t)
{
    const float period = 2.0f + 3.0f * Hash01(key * 2246822519U + 51U);
    const float s = std::fmod(t / period + Hash01(key * 3266489917U + 53U), 1.0f);
    auto ss = [](float e0, float e1, float x)
    {
        const float k = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
        return k * k * (3.0f - 2.0f * k);
    };
    return ss(0.0f, 0.15f, s) * (1.0f - ss(0.35f, 0.5f, s));
}
// A pulse (0..1) on for about duty of each period (its rise and fall 0.1 of
// it), its phase by the key: a flamelet's gusts and lulls.
float Pulse(std::uint32_t key, float t, float period, float duty)
{
    const float s = std::fmod(t / period + Hash01(key), 1.0f);
    auto ss = [](float e0, float e1, float x)
    {
        const float k = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
        return k * k * (3.0f - 2.0f * k);
    };
    return ss(0.0f, 0.1f, s) * (1.0f - ss(duty, duty + 0.1f, s));
}

float LodKeep(int i, float lod)
{
    if (lod <= 0.0f)
        return 1.0f;
    int level = 0;
    for (unsigned u = static_cast<unsigned>(i < 0 ? -i : i); u != 0 && (u & 1U) == 0 && level < 16; u >>= 1)
        ++level;
    if (i == 0)
        level = 16;
    return std::clamp(static_cast<float>(level) + 1.0f - lod, 0.0f, 1.0f);
}

// ---- Flames at range (fix 08): the light-keeping level of detail ----
// The keep of `count` tongues at indices first.. at a level of detail, summed.
float LodSumKeep(int first, int count, float lod)
{
    float sum = 0.0f;
    for (int i = first; i < first + count; ++i)
        sum += LodKeep(i, lod);
    return sum;
}
// A set of `count` like flames (a torch's or a flare's tongues, a kind of a
// cell's debris) at its level of detail: lod, and the gain each kept one
// carries (with its own keep) so the set's light stays that of all of them,
// over `widen` (a kept tongue drawn that much wider already brings that much
// more light).
struct LodSet
{
    float lod = 0.0f, gain = 1.0f;
};
// The last frame's draw ranges (m), for the stats line (far=...;frange=).
float s_drawRangeM = 0.0f, s_lingerRangeM = 0.0f;
LodSet FlameLodSet(int count, float lod, float widen = 1.0f)
{
    LodSet s;
    s.lod = lod;
    const float sum = LodSumKeep(0, count, lod);
    s.gain = sum > 1e-3f ? static_cast<float>(count) / (sum * std::max(widen, 1e-3f)) : 1.0f;
    return s;
}

// The kind of flame a tongue shows: by the fuel and the fire's intensity
// there, with a random mix (r, 0..1, fixed for the tongue) so no two patches
// of fire look alike. Fixed for the tongue's life (a change of style would
// swap its flipbook at once): a catching or dying tongue is short and dim
// instead. Fuel models are Anderson's 13: 1 to 3 grass, 4 to 7 shrub, 8 to
// 13 timber litter and slash.
FlameStyle PickStyle(const BurningView& v, float r)
{
    if (v.intensityKwM < kCreepBelowKwM && !v.torching) // a flank or a backing fire
        return r < 0.7f ? FlameStyle::Creep : FlameStyle::Front;
    if (v.torching)
        return r < 0.6f ? FlameStyle::Bushy : FlameStyle::Tongues;
    if (v.model >= 4 && v.model <= 7)
        return r < 0.5f ? FlameStyle::Bushy : (r < 0.8f ? FlameStyle::Tongues : FlameStyle::Front);
    if (v.model >= 8)
        return r < 0.5f ? FlameStyle::Creep : (r < 0.8f ? FlameStyle::Front : FlameStyle::Tongues);
    return r < 0.45f ? FlameStyle::Front : (r < 0.75f ? FlameStyle::Tongues : FlameStyle::Creep);
}
} // namespace

void FireWorld::Draw(float dt, const std::vector<BurningView>& views)
{
    if (!GLOB_SCENE || !GLOB_SCENE->GetCamera() || !GLOB_WORLD)
        return;
    const Vector3 cam = GLOB_SCENE->GetCamera()->Position();
    float wx = 0.0f, wz = 0.0f;
    Wind(wx, wz);

    // Flames at range (fix 08): how far they are drawn (by day the strongest
    // to a few km, the debris behind the front a few hundred metres; at night
    // all of it to the camera's far clip, the horizon), each faded out over
    // the last part of its range, and the zoom their level of detail is
    // worked at (FlameLodDistance).
    const float night = GLOB_SCENE->MainLight() ? std::clamp(GLOB_SCENE->MainLight()->NightEffect(), 0.0f, 1.0f) : 0.0f;
    const float clipFar = GLOB_SCENE->GetCamera()->ClipFar();
    const float camLeft = GLOB_SCENE->GetCamera()->Left();
    const float drawRangeM = FlameDrawRangeM(night, clipFar);
    const float lingerRangeM = FlameLingerRangeM(night, clipFar);
    s_drawRangeM = drawRangeM;
    s_lingerRangeM = lingerRangeM;

    // Flames: the renderer draws them (GetFlames) as flame tongues, many
    // narrow ones along each burning front (a grid of points per cell that
    // flame as the front's flaming zone crosses them), a torching crown as one
    // column of merging tongues; the strongest and nearest cells first, as
    // many as kMaxSheets tongues allow.
    _pick.clear();
    for (int i = 0; i < static_cast<int>(views.size()); ++i)
    {
        const BurningView& v = views[i];
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        if (dx * dx + dz * dz > drawRangeM * drawRangeM || v.strength <= 0.0f)
            continue;
        _pick.push_back(i);
    }
    // A cell's claim on the budget: its flames over its distance, and less
    // behind the camera (smoothly by the bearing, so a turning camera never
    // pops a cell; a cell within kNearAnyWayM counts whatever its bearing).
    const Vector3 camDir = GLOB_SCENE->GetCamera()->Direction();
    const float cdl = std::sqrt(camDir.X() * camDir.X() + camDir.Z() * camDir.Z());
    const float cdx = cdl > 1e-3f ? camDir.X() / cdl : 0.0f, cdz = cdl > 1e-3f ? camDir.Z() / cdl : 0.0f;
    auto score = [&](int i)
    {
        const BurningView& v = views[i];
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        const float d = std::sqrt(dx * dx + dz * dz);
        const float c = d > 1e-3f ? (dx * cdx + dz * cdz) / d : 1.0f;
        const float t = std::clamp((c + 0.3f) / 0.6f, 0.0f, 1.0f);
        const float facing = std::max(kBehindShare + (1.0f - kBehindShare) * t * t * (3.0f - 2.0f * t),
                                      std::clamp(1.0f - d / kNearAnyWayM, 0.0f, 1.0f));
        return facing * v.strength * std::max(0.3f, v.flameM) / (1.0f + d / 150.0f);
    };
    _pickScore.resize(views.size());
    for (int i : _pick)
        _pickScore[i] = score(i);
    std::sort(_pick.begin(), _pick.end(), [&](int a, int b) { return _pickScore[a] > _pickScore[b]; });
    if (static_cast<int>(_pick.size()) > kMaxCells)
        _pick.resize(kMaxCells);

    // Every flame's shape comes from the fire model, nothing is scaled for
    // looks: a tongue's length is the cell's Byram length at its point of the
    // flaming zone (SurfacePart: the tallest at the leading edge, shorter
    // behind as the fuel there burns out), it tilts downwind by the flame
    // Froude number in the cell's own midflame wind (FlameTiltRad; the fuel
    // model's wind adjustment factor, 0.15 under a closed canopy), so its
    // height is L cos(tilt) and its top leans L sin(tilt) downwind. A
    // torching crown's flame (TorchFlameM: TorchRatio x the tree's height)
    // stands on its tree, tilted by the 10 m wind. The renderer draws each
    // tongue's tip at exactly its length (the flipbook's measured tip share
    // per style, EngineGL33_FireFlames.cpp kFlipFlameShare).
    const float wind10 = std::sqrt(wx * wx + wz * wz);
    const float wdx = wind10 > 0.01f ? wx / wind10 : 0.0f, wdz = wind10 > 0.01f ? wz / wind10 : 1.0f;
    const float cellM = _grid->GetParams().cellM;
    auto cellKey = [&](const BurningView& v)
    {
        return static_cast<std::uint32_t>(static_cast<int>(std::floor(v.x / cellM)) * 73856093) ^
               static_cast<std::uint32_t>(static_cast<int>(std::floor(v.z / cellM)) * 19349663);
    };
    // The class a flame counts under in the measurements (StatsLine), and its
    // fuel's visible absorption coefficient.
    auto classOf = [](const BurningView& v)
    { return v.model >= 4 && v.model <= 7 ? kMeasureShrub : (v.model >= 8 ? kMeasureTimber : kMeasureGrass); };
    auto absorbOf = [](int cls)
    { return cls == kMeasureShrub ? kAbsorbShrub : (cls == kMeasureTimber ? kAbsorbLitter : kAbsorbGrass); };
    auto emissivity = [](float k, float pathM) { return 1.0f - std::exp(-k * std::max(pathM, 0.0f)); };

    const float t = static_cast<float>(std::fmod(_clock, 3600.0));
    _sheets.clear();
    // One tongue: its foot at (fx, fy, fz), L long, tilted downwind (its lean
    // turned by up to kLeanJitterRad from the wind, by its key), its ribbon
    // width wide (give or take kTongueWidthSpread); lengthRef its full length
    // and riseMps its features' climb (both fixed for its life: its flipbook's
    // playback); its foot fading in over baseFade (0: the renderer's, from the
    // ground). False once the budget is full. Its level of detail's gain
    // (FlameSheet::gain) is pushGain, set just before the call and back to 1
    // after it (fix 08), so a push that does not set it draws as before.
    float pushGain = 1.0f;
    // (fire-objects: the object fires' tongues may use kObjSheetReserve past the budget)
    int sheetCap = kMaxSheets;
    auto push = [&](float fx, float fz, float fy, float L, float tilt, float width, float heat, float emis,
                    std::uint32_t key, FlameStyle style, float bend, float lengthRef, float riseMps, float tempK,
                    float baseFade, float swayAmp = 0.0f, float swayHz = 0.0f)
    {
        const float gain = pushGain;
        pushGain = 1.0f;
        if (static_cast<int>(_sheets.size()) >= sheetCap)
            return false;
        FlameSheet f;
        f.gain = gain;
        f.x = fx;
        f.y = fy;
        f.z = fz;
        const float sl = std::sin(tilt), cl = std::cos(tilt);
        const float turn = kLeanJitterRad * (2.0f * Hash01(key * 747796405U + 11U) - 1.0f);
        const float ldx = wdx * std::cos(turn) - wdz * std::sin(turn),
                    ldz = wdx * std::sin(turn) + wdz * std::cos(turn);
        // the tip swings across the lean by swayAmp of the tongue's length
        // now (its foot stays: the renderer bends the axis)
        const float sway =
            swayAmp > 0.0f ? swayAmp * L * std::sin(6.2831853f * (swayHz * t + Hash01(key * 1597334677U + 7U))) : 0.0f;
        f.height = L * cl;
        f.leanX = ldx * L * sl - ldz * sway;
        f.leanZ = ldz * L * sl + ldx * sway;
        f.width = width * (1.0f - kTongueWidthSpread + 2.0f * kTongueWidthSpread * Hash01(key * 2891336453U + 29U));
        f.baseFade = baseFade;
        f.heat = std::min(1.0f, heat);
        f.emissivity = emis;
        f.seed = Hash01(key * 3266489917U + 3U);
        f.lengthRef = std::max(lengthRef, 0.05f);
        f.rise = riseMps;
        f.style = style;
        f.bend = bend;
        f.tempOffsetK = tempK;
        _sheets.push_back(f);
        if (_footOn)
            _foot.Add(_footCls, key, fx, fy, fz, dt);
        return true;
    };

    // ---- The burning bed (the realism audit's fix 7) ----
    // One bed sheet (FlameStyle::Bed): a straight ribbon lying on the ground
    // from its foot (fx, fy, fz: already lifted) along (ax, ay, az), width
    // wide, its light tempK off the bed's; into its own list (kMaxBedSheets),
    // drawn before the tongues so they stand on it. False once that is full.
    _beds.clear();
    // (fix 08: its level of detail's gain is bedGain, set just before the
    // call and back to 1 after it, as push's pushGain)
    float bedGain = 1.0f;
    auto pushBed = [&](float fx, float fy, float fz, float ax, float ay, float az, float width, float heat, float emis,
                       std::uint32_t key, float tempK, int footCls, float groundY)
    {
        const float gain = bedGain;
        bedGain = 1.0f;
        if (static_cast<int>(_beds.size()) >= kMaxBedSheets)
            return false;
        if (heat <= 0.004f)
            return true;
        FlameSheet f;
        f.gain = gain;
        f.x = fx;
        f.y = fy;
        f.z = fz;
        f.height = ay;
        f.leanX = ax;
        f.leanZ = az;
        f.width = width;
        f.heat = std::min(1.0f, heat);
        f.emissivity = emis;
        f.seed = Hash01(key * 3266489917U + 3U);
        f.lengthRef = std::max(std::sqrt(ax * ax + ay * ay + az * az), 0.05f);
        f.rise = 0.0f;
        f.bend = 1.0f; // (straight)
        f.baseFade = 0.0f;
        f.tempOffsetK = tempK;
        f.style = FlameStyle::Bed;
        _beds.push_back(f);
        if (_footOn)
            _foot.Add(footCls, key, fx, groundY, fz, dt); // (its foot on the ground: the lift is the ribbon's)
        return true;
    };

    // The burnt cells' records (_after): each live cell's last view, kept
    // after it burns out so its drawn band, tail and debris burn on.
    ++_drawFrame;
    for (const BurningView& v : views)
    {
        if (v.model == 0)
            continue;
        Afterburn& r = _after[PlantCellKey(v.x, v.z, cellM)];
        if (r.frame == 0)
        {
            r.gridX = wdx;
            r.gridZ = wdz;
        }
        r.v = v;
        r.caught = _clock - v.age;
        r.frame = _drawFrame;
        r.trDraw = BandResidenceSec(DrawResidenceSec(v.model, v.residenceSec), v.rateMps, v.flameMaxM);
        if (v.torching)
            r.torchEndAt = _clock - v.torchAge + kTorchDelaySec + kTorchSec;
    }
    auto lingerBin = [](float a)
    {
        for (int k = 0; k < kLingerBins; ++k)
            if (std::fabs(a - kLingerBinSec[k]) <= kLingerBinHalf[k])
                return k;
        return -1;
    };

    int cellsDrawn = 0;
    _stats.sheetCut = 0;
    _stats.tail = _stats.debris = _stats.glows = _stats.afterCells = 0;
    // A bed streak at a band tongue's foot (the realism audit's fix 7):
    // lying downwind from it along the tongue's own lean, as long as its full
    // flame leans, kBedWidthShare of it wide (at least kBedCoverK of the
    // tongue spacing, so streaks overlap), its heat across the band BedHeat
    // (brightest at the front, gone over the back third), cooler toward the
    // back. False once the beds' budget is full.
    // (fix 08: lodGain the band tongue's level of detail's gain, its keep with
    // the dropped streaks' light, as FlameSheet::gain)
    auto bedStreak = [&](const BurningView& v, const SurfaceTongue& s, const SurfaceGrid& g, float midflame,
                         float absorb, float trDraw, float lodGain)
    {
        const float x = s.age / std::max(trDraw, 0.1f);
        const float in = std::clamp(s.age / std::min(kLingerAttachSec, 0.3f * trDraw), 0.0f, 1.0f);
        const float heat = BedHeat(x) * in * in * (3.0f - 2.0f * in);
        if (heat <= 0.004f || lodGain <= 0.004f)
            return true;
        bedGain = lodGain;
        const float Lb = s.lmax;
        const float len = std::clamp(Lb * std::sin(FlameTiltRad(midflame, Lb)), kBedLenMinM, kBedLenMaxM) *
                          (0.75f + 0.5f * Hash01(s.key + 22U));
        const float w0 = std::max({kBedWidthShare * Lb, kBedCoverK * g.b, kBedWidthMinM});
        const float w = w0 * g.widen;
        const float turn = kLeanJitterRad * (2.0f * Hash01(s.key * 747796405U + 11U) - 1.0f);
        const float ddx = wdx * std::cos(turn) - wdz * std::sin(turn),
                    ddz = wdx * std::sin(turn) + wdz * std::cos(turn);
        const float fx = v.x + s.ox, fz = v.z + s.oz;
        const float fy = GLandscape ? GLandscape->SurfaceY(fx, fz) : v.y;
        const float ty = GLandscape ? GLandscape->SurfaceY(fx + ddx * len, fz + ddz * len) : fy;
        // (its optical path: the flaming zone's depth seen side on, at least its own width)
        const float path = std::max(w, std::min(v.rateMps * trDraw, kBedPathMaxM));
        return pushBed(fx, fy + kBedLift * w0, fz, ddx * len, ty - fy, ddz * len, w, heat, emissivity(absorb, path),
                       s.key * 2246822519U + 23U, -kBedBackCoolK * x, kFootBed, fy);
    };
    // One cell's surface flames, live (in views) or burnt out (its record,
    // drawn on while its band or tail lasts): its grid of points, each
    // flaming from when the front reaches it through the drawn band and its
    // tail (LingerFlame, trDraw the drawn residence). A live cell's band
    // tongues are measured against the model (the tail has no model flame).
    auto surface = [&](const BurningView& v, float trDraw, float gridX, float gridZ, bool live)
    {
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        const float dist = std::sqrt(dx * dx + dz * dz);
        const bool measured = dist < kLingerNearM;
        const std::uint32_t cell = cellKey(v);
        const float waf = v.sheltered ? 0.15f : Model(v.model).waf;
        const float midflame = waf * wind10;
        const float windRise = LickWindFactor(midflame); // a wind quickens the tongues' licks a little
        const int cls = classOf(v);
        const float absorb = absorbOf(cls);
        // (the level of detail at the zoom's distance; its kept tongues carry
        // the dropped ones' light, faded out at the edge of the range)
        const SurfaceGrid g = CellSurfaceTongues(v, trDraw, cellM, cell, gridX, gridZ, FlameLodDistance(dist, camLeft),
                                                 _lodBias, _tongues);
        // (buildings builder) no flame foot inside a building's footprint and apron
        if (!_bld.empty() && BuildingsNear(v.x, v.z))
            _tongues.erase(std::remove_if(_tongues.begin(), _tongues.end(), [&](const SurfaceTongue& s)
                                          { return BuildingAt(v.x + s.ox, v.z + s.oz, kBuildingApronM) >= 0; }),
                           _tongues.end());
        const float cellGain = g.gain * FlameRangeFade(dist, drawRangeM);
        int counted = 0;
        for (const SurfaceTongue& s : _tongues)
        {
            const bool band = s.age < trDraw;
            // a band tongue sinks now and then (the peaks change place by
            // dying and being born); the tail's only shrink where they stand
            const float L = s.lmax * s.flame.length * (band ? 1.0f - kPeakSwing * PeakSink(s.key, t) : 1.0f);
            const float heat = s.flame.heat;
            const float lodGain = s.keep * cellGain;
            // the burning bed under the band (the realism audit's fix 7), with
            // the tongue's level of detail's gain (fix 08)
            if (band && Hash01(s.key + 21U) < kBedShare)
                bedStreak(v, s, g, midflame, absorb, trDraw, lodGain);
            if (heat <= 0.004f || lodGain <= 0.004f || L < 0.04f)
                continue;
            const float fx = v.x + s.ox, fz = v.z + s.oz;
            const float fy = GLandscape ? GLandscape->SurfaceY(fx, fz) : v.y;
            const FlameStyle style = PickStyle(v, Hash01(s.key + 5U));
            // a small tail tongue is a tuft: wider, its lean capped
            const float tuft = band ? 0.0f : TuftShare(L);
            const float width =
                FlameFrameWidth(static_cast<int>(style), L) * g.widen * (1.0f + (kTuftWiden - 1.0f) * tuft);
            const float tilt = band ? FlameTiltRad(midflame, L) : std::min(FlameTiltRad(midflame, L), kTuftTiltMax);
            const float q = Hash01(s.key + 8U);
            const float swayAmp =
                band ? kSwayBand + (kSwayBandMax - kSwayBand) * q : kSwayTail + (kSwayTailMax - kSwayTail) * q;
            const float swayHz = band ? 1.0f / SwayPeriodSec(s.lmax) : 1.0f / (0.5f + 0.5f * Hash01(s.key + 9U));
            _footCls = band ? kFootBand : kFootTail;
            pushGain = lodGain;
            if (!push(fx, fz, fy, L, tilt, width, heat * (0.8f + 0.2f * Hash01(s.key + 6U)),
                      emissivity(absorb, kTongueThick * L), s.key, style, 0.0f, s.lmax,
                      FeatureRiseMps(s.lmax) * windRise, 0.0f, 0.0f, swayAmp, swayHz))
                return;
            if (!band)
                _stats.tail++;
            if (measured)
            {
                const int bin = lingerBin(s.age);
                if (bin >= 0)
                {
                    _lgPts[bin] += 1.0;
                    _lgFlame[bin] += g.a * g.b; // the ground the tongue stands for
                }
            }
            if (live && band)
            {
                Measure(cls, L, L, L * std::cos(tilt), tilt, s.flame.length, cls == kMeasureShrub ? v.shrubH : 0.0f);
                if (counted < 12)
                    MeasureV4(v, cell, counted, s.ox, s.oz, cellM, wind10);
                ++counted;
            }
        }
    };
    const auto tLive = Clock::now();
    for (std::size_t n = 0; n < _pick.size(); ++n)
    {
        if (static_cast<int>(_sheets.size()) >= kMaxSheets)
        {
            _stats.sheetCut = static_cast<int>(_pick.size() - n);
            break;
        }
        ++cellsDrawn;
        const BurningView& v = views[_pick[n]];
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        const float dist = std::sqrt(dx * dx + dz * dz);
        const std::uint32_t cell = cellKey(v);
        const auto rit = _after.find(PlantCellKey(v.x, v.z, cellM));
        surface(v, BandResidenceSec(DrawResidenceSec(v.model, v.residenceSec), v.rateMps, v.flameMaxM),
                rit != _after.end() ? rit->second.gridX : wdx, rit != _after.end() ? rit->second.gridZ : wdz, true);
        // A torching crown: one column of merging tongues (the consult),
        // drawn as volumes (fix 6: TorchColumnOf, the kTorch tunables). Their
        // feet stand through the crown's cone (fixed for their life); each
        // main tongue reaches for a point in the column's upper part on its
        // own side of the axis, inside the column's radius there, so the
        // bundle wraps the crown, necks above its top and tapers to the tip
        // TorchRatio tree heights above the ground (not a flag of parallel
        // streaks); the fill tongues rise through the crown, leaning less;
        // the short ones escape sideways from its edge. The column lights up
        // the crown from its foot (kTorchClimb) and dies back from its top;
        // the whole column puffs (a bulge climbing it), each tongue flickering
        // on its own flipbook at its own full length's tempo (Lown: fixed, as
        // before).
        if (v.torchM > 0.05f)
        {
            const float tx = v.x + v.treeOX, tz = v.z + v.treeOZ;
            const float H = std::max(v.treeH, 1.0f);
            const float crown = kTorchCrownShare * H;
            const float env = v.torchM / std::max(v.torchMaxM, 0.01f);
            // the tip in tree heights (the model's TorchRatio of the trees
            // torching round it now), eased: a neighbour joining grows it
            float tipRatio = (v.torchMaxM + kTorchFootShare * H) / H;
            if (rit != _after.end())
            {
                float& eased = rit->second.torchTip;
                eased =
                    eased <= 0.0f ? tipRatio : eased + (tipRatio - eased) * (1.0f - std::exp(-dt / kTorchTipEaseSec));
                tipRatio = eased;
            }
            const TorchColumn col = TorchColumnOf(H, crown, tipRatio, wind10);
            const float tilt = col.tilt;
            const float sl = std::sin(tilt), cl = std::cos(tilt);
            const float g0 = GLandscape ? GLandscape->SurfaceY(tx, tz) : v.y;
            const float baseY = g0 + kTorchFootShare * H;
            // the column's full length along its lean, from the crown's foot
            const float Lcol = (col.tipM - col.crownBaseM) / cl;
            // The tempo's lengths are the lone tree's full flame (fixed by the
            // tree: they never drift as the torch grows and dies or its
            // neighbours join it, which would race the tongues' flipbooks).
            const float Lnom = std::max(0.5f, (kTorchRatio - kTorchFootShare) * H);
            const float puffHz = ColumnPuffHz(crown, Lnom);
            const float puffPhase = 6.2831853f * Hash01(cell * 2166136261U + 23U);
            // The bulge's time to climb s metres of the column: 3.4 sqrt(z)
            // through its lower part, then steady.
            auto bulgeSec = [&](float s) { return TorchBulgeSec(s, Lnom); };
            const float lod =
                std::log2(std::max(1.0f, FlameLodDistance(dist, camLeft) * kLodRad / (0.15f * crown))) + _lodBias;
            const float widenT = std::min(kLodWidenMax, std::pow(kLodWiden, std::max(lod, 0.0f)));
            // (the kept tongues carry the dropped ones' light: fix 08)
            const LodSet torchLod = FlameLodSet(kTorchTongues + kTorchFill + kTorchShort, lod, widenT);
            const float torchFade = FlameRangeFade(dist, drawRangeM);
            float topMax = 0.0f, drawnMax = 0.0f;
            for (int j = 0; j < kTorchTongues + kTorchFill + kTorchShort; ++j)
            {
                const float keep = LodKeep(j, lod);
                if (keep <= 0.0f)
                    continue;
                const std::uint32_t key = cell * 15485863U + static_cast<std::uint32_t>(j) * 32452843U + 977U;
                const bool fill = j >= kTorchTongues && j < kTorchTongues + kTorchFill;
                const bool shortOne = j >= kTorchTongues + kTorchFill;
                const float qa = std::fmod(0.5f + kR2X * static_cast<float>(j), 1.0f);
                const float qb = std::fmod(0.5f + kR2Z * static_cast<float>(j), 1.0f);
                // its foot: up the crown (0 its foot, 1 its top), within the
                // cone's radius there (a short one at the cone's edge)
                const float up = shortOne ? 0.2f + 0.7f * qa : (fill ? 0.65f * qa : 0.9f * qa);
                const float coneR = 0.5f * crown * (1.0f - 0.75f * up);
                const float rr = coneR * (shortOne ? 0.85f + 0.15f * Hash01(key) : std::sqrt(Hash01(key)));
                const float ang = 6.2831853f * qb;
                const float rx = std::cos(ang), rz = std::sin(ang);
                const float fx = tx + rx * rr, fz = tz + rz * rr;
                const float fy = baseY + up * (1.0f - kTorchFootShare) * H;
                // how far up the column its foot is (along the leaning axis)
                const float s = (fy - baseY) * cl + ((fx - tx) * wdx + (fz - tz) * wdz) * sl;
                // the bulge climbing the column
                // (the puff: a bulge climbing the tongue, at this phase at its foot now)
                const float puffTurns = puffHz * (t - bulgeSec(s)) + puffPhase / 6.2831853f;
                // the crown lights from its foot up and dies back from its top
                const float envJ = std::clamp(env * (1.0f + kTorchClimb) - kTorchClimb * up, 0.0f, 1.0f);
                if (envJ <= 0.0f)
                    continue;
                // (no puff in its length or width: a steady body)
                const float stretch = envJ;
                // its full reach from its foot, then grown to now
                float cx = 0.0f, cy = 0.0f, cz = 0.0f;
                if (shortOne)
                {
                    // out from the crown's edge, leaning with the column
                    const float Ls = kTorchShortLen * Lcol * (0.7f + 0.6f * Hash01(key + 1U));
                    const float dx = kTorchShortOut * rx + wdx * sl, dy = cl, dz = kTorchShortOut * rz + wdz * sl;
                    const float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
                    cx = dx / dl * Ls;
                    cy = dy / dl * Ls;
                    cz = dz / dl * Ls;
                }
                else if (fill)
                {
                    // up through the crown, leaning less than the column
                    const float Lf = kTorchFillDraw * kTorchFillLen * H * (0.7f + 0.6f * Hash01(key + 1U));
                    const float ft = kTorchFillTilt * tilt;
                    cx = wdx * std::sin(ft) * Lf;
                    cy = std::cos(ft) * Lf;
                    cz = wdz * std::sin(ft) * Lf;
                }
                else
                {
                    // into the column's upper part on its own side: the neck
                    // and the taper (most reach near the tip)
                    const float reach =
                        kTorchMainReachLo + (1.0f - kTorchMainReachLo) * std::pow(Hash01(key + 1U), kTorchMainReachPow);
                    const float zt = H + (col.tipM - H) * reach;
                    const float lean = TorchColumnOffset(col, zt);
                    const float side =
                        TorchColumnRadius(col, zt) * (kTorchMainSideLo + kTorchMainSideSpan * Hash01(key + 3U));
                    const float sa = ang + kTorchMainTurn * (Hash01(key + 4U) - 0.5f);
                    cx = tx + wdx * lean + std::cos(sa) * side - fx;
                    cy = g0 + zt - fy;
                    cz = tz + wdz * lean + std::sin(sa) * side - fz;
                }
                cx *= stretch;
                cy *= stretch;
                cz *= stretch;
                const float L = std::sqrt(cx * cx + cy * cy + cz * cz);
                if (L < 0.05f)
                    continue;
                // its own full length (the tree's, fixed): its tempo's
                const float sNom = up * (1.0f - kTorchFootShare) * H;
                const float Lown =
                    shortOne ? kTorchShortLen * Lnom * (0.7f + 0.6f * Hash01(key + 1U))
                    : fill
                        ? kTorchFillLen * H * (0.7f + 0.6f * Hash01(key + 1U))
                        : std::max(0.2f * Lnom, (Lnom - sNom) * (kTorchReach +
                                                                 kTorchReachSpread * (2.0f * Hash01(key + 1U) - 1.0f)));
                const FlameStyle style = Hash01(key + 2U) < 0.7f ? FlameStyle::Bushy : FlameStyle::Tongues;
                const float widen = shortOne ? kTorchShortWiden : (fill ? kTorchFillWiden : kTorchMainWiden);
                _footCls = kFootTorch;
                // (its level of detail's keep in the gain, the dropped tongues'
                // light carried: fix 08)
                pushGain = keep * torchLod.gain * torchFade;
                if (!push(fx, fz, fy, L, tilt, widen * FlameFrameWidth(static_cast<int>(style), L) * widenT,
                          (0.6f + 0.4f * envJ) * (1.0f - 0.5f * kPuffGlow), emissivity(kAbsorbCrown, 0.3f * L), key,
                          style, 1.0f, std::max(Lown, 0.5f), FeatureRiseMps(std::max(Lown, 0.5f)) * kTorchDraught,
                          fill ? kTorchCrownK : -60.0f * up, // the column's upper tongues are its cooler tips
                          kTorchFootFade * L))
                    break;
                // its own direction (into the column), not the wind's: a volume
                FlameSheet& f = _sheets.back();
                f.height = cy;
                f.leanX = cx;
                f.leanZ = cz;
                f.volumeK = kAbsorbCrown;
                f.volumeR = 1.0f;
                f.volumeSoot = kTorchSoot;
                f.volumeLen = L / stretch; // its full reach: its place in the drawing order
                f.volumePuff = puffTurns;
                f.volumeWaveM = kBulgeTopK * std::sqrt(Lnom) / std::max(puffHz, 0.05f);
                f.volumeBulge = kTorchBulge;
                topMax = std::max(topMax, fy + cy - g0);
                drawnMax = std::max(drawnMax, L);
            }
            Measure(kMeasureTorch, v.torchM, drawnMax, topMax, tilt, env, v.treeH);
        }
    }
    // Burnt-out cells whose drawn band or tail outlast the fire model's
    // window (their records), drawn on as the live ones.
    const auto tAfter = Clock::now();
    _stats.msDrawLive = std::chrono::duration<double, std::milli>(tAfter - tLive).count();
    for (auto& [ckey, r] : _after)
    {
        if (r.frame == _drawFrame || static_cast<int>(_sheets.size()) >= kMaxSheets)
            continue;
        const float ageNow = static_cast<float>(_clock - r.caught);
        if (ageNow >= r.v.crossSec + kTailEnd * r.trDraw)
            continue;
        const float dx = r.v.x - cam.X(), dz = r.v.z - cam.Z();
        if (dx * dx + dz * dz > drawRangeM * drawRangeM)
            continue;
        BurningView v = r.v;
        AgeView(v, ageNow);
        v.torching = false;
        v.torchM = 0.0f;
        _stats.afterCells++;
        surface(v, r.trDraw, r.gridX, r.gridZ, false);
    }
    // The debris burning on near the camera (kGlowM and the debris tunables):
    // each piece where it lies in its cell, lit born seconds after the front
    // reached that point (cellTime: after the cell's own front), flaming
    // life seconds, its flame shrinking from h0 to h1 (base across), then
    // glowing glow seconds; gain its level of detail's (fix 08: a kept piece
    // carries the light of those its kind's level drops, faded at the range's
    // edge). False once the budget is full.
    struct Debris
    {
        float ox = 0.0f, oz = 0.0f, up = 0.0f;
        float born = 0.0f, life = 0.0f, h0 = 0.0f, h1 = 0.0f, glow = 0.0f, base = 0.3f;
        bool cellTime = false;
        float gain = 1.0f;
    };
    // The record whose pieces are visited, the next piece's slot in its
    // pieceAge, and whether its times are read again this frame.
    Afterburn* pieceRec = nullptr;
    std::size_t pieceSlot = 0;
    bool pieceRead = false;
    auto debris = [&](const BurningView& v, float age, std::uint32_t key, const Debris& d, float midflame,
                      float windRise, bool measured)
    {
        // (FrontAgeAt reads the view's own time: the record's age now shifts it)
        std::vector<float>& cache = pieceRec->pieceAge;
        const std::size_t slot = pieceSlot++;
        if (pieceRead || slot >= cache.size() || cache[slot] == kPieceUnread)
        {
            // (buildings builder: none inside a building's footprint and apron either)
            const float off = BareAt(v, d.ox, d.oz) || (!_bld.empty() && BuildingsNear(v.x, v.z) &&
                                                        BuildingAt(v.x + d.ox, v.z + d.oz, kBuildingApronM) >= 0)
                                  ? kPieceBare
                                  : (d.cellTime ? 0.0f : FrontAgeAt(v, d.ox, d.oz, cellM) - v.age);
            if (slot < cache.size())
                cache[slot] = off;
            else
                cache.push_back(off);
        }
        const float off = cache[slot];
        if (off == kPieceBare)
            return true;
        const float pa = d.cellTime ? age : off + age;
        const float since = pa - d.born;
        if (since <= 0.0f || since >= d.life + d.glow)
            return true;
        const float fx = v.x + d.ox, fz = v.z + d.oz;
        const float fy = (GLandscape ? GLandscape->SurfaceY(fx, fz) : v.y) + d.up;
        if (since < d.life)
        {
            // a tuft (the consult's seventh round): short, wide, leaning hard
            // downwind of its fuel and flapping in place, its length by
            // kTuftFlutter at about kTuftFlapHz; a gust presses it down and
            // gutters it (one in four at any time), a lull stands it up
            float L = d.h0 + (d.h1 - d.h0) * since / d.life;
            const float flap = 0.6f * std::sin(6.2831853f * (kTuftFlapHz * t + Hash01(key + 8U))) +
                               0.4f * std::sin(6.2831853f * (1.43f * kTuftFlapHz * t + Hash01(key + 9U)));
            const float gust = Pulse(key * 747796405U + 61U, t, 4.0f + 4.0f * Hash01(key + 10U), 0.25f);
            const float lull =
                Pulse(key * 2891336453U + 67U, t, 3.0f + 3.0f * Hash01(key + 11U), 0.25f) * (1.0f - gust);
            L *= (1.0f + kTuftFlutter * flap) * (1.0f - kTuftGutter * gust);
            const float tilt = TuftTiltRad(wind10, L, lull, gust);
            const FlameStyle style = FlameStyle::Creep;
            const float width = std::max(kTuftWiden * FlameFrameWidth(static_cast<int>(style), L), 0.6f * d.base);
            // (seen through its base: a tussock's flame is as thick as the clump under it)
            const float heat = (0.6f + 0.3f * Hash01(key + 6U)) * std::min(1.0f, 0.5f + 0.5f * since);
            _footCls = kFootDebris;
            pushGain = d.gain;
            if (!push(fx, fz, fy, L, tilt, width, heat,
                      emissivity(kAbsorbGrass, std::max(kTongueThick * L, 0.8f * d.base)), key, style, 0.0f, d.h0,
                      kTuftRise, 0.0f, 0.0f, kSwayTuft + (kSwayTuftMax - kSwayTuft) * Hash01(key + 12U),
                      2.5f + 2.5f * Hash01(key + 13U)))
                return false;
            _stats.debris++;
            if (measured)
            {
                const int bin = lingerBin(pa);
                if (bin >= 0)
                {
                    _lgPts[bin] += 1.0;
                    _lgFlame[bin] += 0.785 * d.base * d.base;
                }
            }
            return true;
        }
        const float g = std::clamp(1.0f - (since - d.life) / std::max(d.glow, 0.1f), 0.0f, 1.0f);
        if (g <= 0.01f)
            return true;
        _footCls = kFootGlow;
        pushGain = d.gain;
        if (!push(fx, fz, fy, kGlowM, 0.0f, 1.5f * FlameFrameWidth(static_cast<int>(FlameStyle::Creep), kGlowM),
                  g * kGlowHeat, emissivity(kAbsorbGrass, 0.36f), key, FlameStyle::Creep, 0.0f, kGlowM,
                  FeatureRiseMps(kGlowM), 0.0f, 0.0f))
            return false;
        _stats.glows++;
        return true;
    };
    const auto tDebris = Clock::now();
    _stats.msDrawAfter = std::chrono::duration<double, std::milli>(tDebris - tAfter).count();
    for (auto it = _after.begin(); it != _after.end();)
    {
        Afterburn& r = it->second;
        const float age = static_cast<float>(_clock - r.caught);
        if (age > r.v.crossSec + kAfterKeepSec)
        {
            it = _after.erase(it);
            continue;
        }
        ++it;
        const BurningView& v = r.v;
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        const float dist = std::sqrt(dx * dx + dz * dz);
        if (dist > lingerRangeM + cellM)
            continue;
        const bool measured = dist < kLingerNearM;
        if (measured)
            for (int k = 0; k < kLingerBins; ++k)
            {
                // the share of the cell the front reached within the bin's times ago
                const float lo = kLingerBinSec[k] - kLingerBinHalf[k], hi = kLingerBinSec[k] + kLingerBinHalf[k];
                float share = age >= lo && age <= hi ? 1.0f : 0.0f;
                if (v.crossSec > 0.0f)
                    share = std::max(0.0f,
                                     std::min(1.0f, (age - lo) / v.crossSec) - std::max(0.0f, (age - hi) / v.crossSec));
                _lgArea[k] += share * cellM * cellM;
            }
        if (static_cast<int>(_sheets.size()) >= kMaxSheets)
            continue;
        const std::uint32_t cell = cellKey(v);
        const float tr = r.trDraw;
        const float midflame = (v.sheltered ? 0.15f : Model(v.model).waf) * wind10;
        const float windRise = LickWindFactor(midflame);
        bool ok = true;
        pieceRec = &r;
        pieceSlot = 0;
        pieceRead = r.pieceAge.empty() || (r.frame > r.pieceFrame && _drawFrame - r.pieceFrame >= kPieceRefreshFrames);
        if (pieceRead)
            r.pieceFrame = _drawFrame;
        auto place = [&](std::uint32_t key, Debris& d)
        {
            d.ox = (Hash01(key + 1U) - 0.5f) * (cellM - 0.5f);
            d.oz = (Hash01(key + 2U) - 0.5f) * (cellM - 0.5f);
        };
        auto span = [&](std::uint32_t key, float lo, float hi) { return lo + (hi - lo) * Hash01(key); };
        // Each kind of n pieces, spread over the cell about cellM / sqrt(n)
        // apart, thins out far off by its own level of detail (at the zoom's
        // distance, never from the budget: near the camera every piece
        // stays), its kept pieces carrying the light of the dropped ones and
        // fading out at the range's edge (fix 08).
        const float lodDist = FlameLodDistance(dist, camLeft);
        const float rangeFade = FlameRangeFade(dist, lingerRangeM);
        auto kind = [&](int n)
        {
            const float spacing = cellM / std::sqrt(static_cast<float>(std::max(n, 1)));
            return FlameLodSet(n, std::log2(std::max(1.0f, lodDist * kDebrisLodRad / spacing)));
        };
        // A piece kept draws through debris(); a dropped one still takes its
        // slot in the record's pieceAge (fix 4's cache follows the visiting
        // order, so a dropped piece must not shift the next ones' times), its
        // time marked unread on a reading frame (read when next drawn).
        auto kept = [&](const LodSet& s, int k, Debris& d)
        {
            const float keep = LodKeep(k, s.lod);
            d.gain = keep * s.gain * rangeFade;
            if (keep > 0.0f && d.gain > 0.004f)
                return true;
            std::vector<float>& cache = pieceRec->pieceAge;
            const std::size_t slot = pieceSlot++;
            if (slot >= cache.size())
                cache.push_back(kPieceUnread);
            else if (pieceRead)
                cache[slot] = kPieceUnread;
            return false;
        };
        // the glow sparkle just behind the band
        const LodSet sparkleLod = kind(kSparklePerCell);
        for (int k = 0; k < kSparklePerCell && ok; ++k)
        {
            const std::uint32_t key = cell * 2971215073U + static_cast<std::uint32_t>(k) * 433494437U + 61U;
            Debris d;
            if (!kept(sparkleLod, k, d))
                continue;
            place(key, d);
            d.born = tr * span(key + 3U, 0.9f, 1.2f);
            d.glow = span(key + 4U, kSparkleSecMin, kSparkleSecMax);
            ok = debris(v, age, key, d, midflame, windRise, false);
        }
        // grass: the flamelets
        const int lets =
            static_cast<int>(std::lround(kFlameletsPerCell * FlameletScale(v.model) * (0.5f + Hash01(cell + 71U))));
        const LodSet letLod = kind(lets);
        for (int k = 0; k < lets && ok; ++k)
        {
            const std::uint32_t key = cell * 1597334677U + static_cast<std::uint32_t>(k) * 3812015801U + 17U;
            Debris d;
            if (!kept(letLod, k, d))
                continue;
            place(key, d);
            d.born = tr * span(key + 3U, 1.0f, 1.5f);
            d.life = FlameletLifeSec(Hash01(key + 10U), Hash01(key + 11U));
            d.h0 = 0.15f + 0.35f * std::pow(Hash01(key + 12U), 1.5f);
            d.h1 = 0.1f;
            d.glow = d.life * span(key + 13U, 1.0f, 3.0f);
            d.base = span(key + 14U, 0.2f, 0.5f);
            ok = debris(v, age, key, d, midflame, windRise, measured);
        }
        // grass and shrub: a long burner
        if (ok && v.model >= 1 && v.model <= 7 && Hash01(cell + 73U) < kLongShare)
        {
            const std::uint32_t key = cell * 3266489917U + 29U;
            Debris d;
            d.gain = rangeFade;
            place(key, d);
            d.born = tr * span(key + 3U, 1.0f, 1.5f);
            d.life = span(key + 10U, 120.0f, 600.0f);
            d.h0 = span(key + 12U, 0.3f, 0.8f);
            d.h1 = 0.2f;
            d.glow = std::max(60.0f, kLongGlowMaxSec - d.life);
            d.base = span(key + 14U, 0.2f, 0.4f);
            ok = debris(v, age, key, d, midflame, windRise, measured);
        }
        // shrub: the stems
        const LodSet stemLod = kind(kShrubStems);
        for (int k = 0; v.model >= 4 && v.model <= 7 && k < kShrubStems && ok; ++k)
        {
            const std::uint32_t key = cell * 2246822519U + static_cast<std::uint32_t>(k) * 374761393U + 31U;
            Debris d;
            if (!kept(stemLod, k, d))
                continue;
            place(key, d);
            d.born = tr * span(key + 3U, 0.8f, 1.2f);
            d.life = span(key + 10U, 60.0f, 180.0f);
            d.h0 = span(key + 12U, 0.2f, 0.5f);
            d.h1 = 0.1f;
            d.glow = span(key + 13U, 300.0f, 900.0f);
            d.base = span(key + 14U, 0.2f, 0.4f);
            ok = debris(v, age, key, d, midflame, windRise, measured);
        }
        // litter: twigs and cones, branches, logs (halved in hardwood
        // litter), and the foot of a standing trunk
        if (v.model >= 8)
        {
            const float share = v.model >= 10 ? 1.0f : 0.5f;
            const int twigs = static_cast<int>(std::lround(share * span(cell + 81U, kTwigsMin, kTwigsMax)));
            const int branches = static_cast<int>(std::lround(share * span(cell + 83U, kBranchesMin, kBranchesMax)));
            const float lq = Hash01(cell + 85U);
            const int logs = share >= 1.0f ? (lq < 0.25f ? 0 : (lq < 0.75f ? 1 : 2)) : (lq < 0.5f ? 0 : 1);
            const LodSet litterLod = kind(twigs + branches + logs);
            for (int k = 0; k < twigs + branches + logs && ok; ++k)
            {
                const std::uint32_t key = cell * 2654435761U + static_cast<std::uint32_t>(k) * 2246822519U + 37U;
                const bool twig = k < twigs, log = k >= twigs + branches;
                Debris d;
                if (!kept(litterLod, k, d))
                    continue;
                place(key, d);
                d.born = tr * span(key + 3U, 0.6f, 1.2f);
                d.life = twig ? span(key + 10U, 60.0f, 180.0f)
                              : (log ? span(key + 10U, 600.0f, 1200.0f) : span(key + 10U, 120.0f, 480.0f));
                d.h0 = twig ? span(key + 12U, 0.2f, 0.4f)
                            : (log ? span(key + 12U, 0.3f, 1.0f) : span(key + 12U, 0.2f, 0.6f));
                d.h1 = 0.1f;
                d.glow = twig ? span(key + 13U, 300.0f, 600.0f) : span(key + 13U, 300.0f, 900.0f);
                d.base = log ? span(key + 14U, 0.6f, 1.2f) : span(key + 14U, 0.2f, 0.4f);
                ok = debris(v, age, key, d, midflame, windRise, measured);
            }
            if (ok && v.treeH > 0.0f && Hash01(cell + 87U) < kTrunkBaseShare)
                for (int k = 0; k < 2 && ok; ++k)
                {
                    const std::uint32_t key = cell * 3812015801U + static_cast<std::uint32_t>(k) * 1597334677U + 41U;
                    Debris d;
                    d.gain = rangeFade;
                    d.ox = v.treeOX + span(key + 1U, -0.3f, 0.3f);
                    d.oz = v.treeOZ + span(key + 2U, -0.3f, 0.3f);
                    d.born = tr * span(key + 3U, 0.8f, 1.2f);
                    d.life = span(key + 10U, 60.0f, 300.0f);
                    d.h0 = span(key + 12U, 0.3f, 1.0f);
                    d.h1 = 0.15f;
                    d.glow = span(key + 13U, 600.0f, 1200.0f);
                    d.base = 0.4f;
                    ok = debris(v, age, key, d, midflame, windRise, measured);
                }
        }
        // a torched tree: its bark (up to 2 m) and the burning debris
        // fallen round its foot, from about when its crown goes out
        if (r.torchEndAt > 0.0)
        {
            const float out = static_cast<float>(r.torchEndAt - r.caught);
            // (eight pieces within a metre or so of the trunk)
            const LodSet treeLod = FlameLodSet(8, std::log2(std::max(1.0f, lodDist * kDebrisLodRad / 1.0f)));
            for (int k = 0; k < 8 && ok; ++k)
            {
                const std::uint32_t key = cell * 2166136261U + static_cast<std::uint32_t>(k) * 16777619U + 43U;
                const bool bark = k < 3;
                const float ang = 6.2831853f * Hash01(key + 1U);
                const float rr = bark ? 0.25f : span(key + 2U, 0.5f, 1.5f);
                Debris d;
                if (!kept(treeLod, k, d))
                    continue;
                d.ox = v.treeOX + rr * std::cos(ang);
                d.oz = v.treeOZ + rr * std::sin(ang);
                d.up = bark ? span(key + 4U, 0.3f, 2.0f) : 0.0f;
                d.cellTime = true;
                d.born = out - (bark ? 0.0f : 20.0f) + span(key + 3U, 0.0f, 10.0f);
                d.life = bark ? span(key + 10U, 60.0f, 300.0f) : span(key + 10U, 120.0f, 300.0f);
                d.h0 = bark ? span(key + 12U, 0.3f, 1.0f) : span(key + 12U, 0.3f, 0.6f);
                d.h1 = 0.15f;
                d.glow = span(key + 13U, 300.0f, 900.0f);
                d.base = 0.4f;
                ok = debris(v, age, key, d, midflame, windRise, measured);
            }
        }
    }
    _stats.afterKept = static_cast<int>(_after.size());
    _stats.msDrawDebris = MsSince(tDebris);
    _lgFrames++;
    // Plants: every tree, bush, reed clump and olive the fire comes near has
    // its own fire (UpdatePlants). A bush, reed clump or olive flares as a
    // body from the moment the drawn flames touch it, if the surface fire
    // that reached it is strong enough for its class, and burns on by itself
    // after the grass round it is out (_flares): a line of burning bushes
    // standing in the black behind the front.
    UpdatePlants(views, wdx, wdz, wind10, cam);
    for (std::size_t n = 0; n < _flares.size();)
    {
        Flare& fl = _flares[n];
        const Plant& p = fl.p;
        const FlareShape s = FlareShapeOf(p.cls, p.h);
        const float a = static_cast<float>(_clock - fl.start);
        const float tFull = s.peakSec + s.fullSec, tDown = tFull + s.downSec, tEnd = tDown + s.stemSec;
        const float dx = p.x - cam.X(), dz = p.z - cam.Z();
        const float dist = std::sqrt(dx * dx + dz * dz);
        if (!fl.logged && a > tFull)
        {
            // Its peak against its height (the realism check: a thicket's tip
            // about 1.7 to 2 h up), for those drawn near the camera.
            if (fl.peakN > 0 && dist < kFlareLogM)
                LOG_INFO(Script,
                         "UDFIRE;event=flare;shape={};class={};h={:.2f};width={:.2f};x={:.1f};z={:.1f};flame={:.2f};"
                         "drawnMean={:.2f};drawnMax={:.2f};tipMean={:.2f};tipMax={:.2f};tipMeanOverH={:.2f};"
                         "tipMaxOverH={:.2f};start={:.1f}",
                         p.name.c_str(), static_cast<int>(p.cls), p.h, 2.0f * p.r, p.x, p.z, s.flameM,
                         fl.flameSum / std::max(fl.peakN, 1), fl.peakFlame, fl.tipSum / std::max(fl.peakN, 1),
                         fl.peakTip, fl.tipSum / std::max(fl.peakN, 1) / std::max(p.h, 0.1f),
                         fl.peakTip / std::max(p.h, 0.1f), fl.start);
            fl.logged = true;
        }
        if (a > tEnd)
        {
            _flares[n] = std::move(_flares.back());
            _flares.pop_back();
            continue;
        }
        ++n;
        auto ss = [](float e0, float e1, float x)
        {
            const float k = std::clamp((x - e0) / std::max(e1 - e0, 1e-3f), 0.0f, 1.0f);
            return k * k * (3.0f - 2.0f * k);
        };
        float env = 0.0f;
        if (a < s.peakSec)
            env = kWhoompShare * ss(0.0f, 0.66f * s.peakSec, a) +
                  (1.0f - kWhoompShare) * ss(0.6f * s.peakSec, s.peakSec, a);
        else if (a < tFull)
            env = 1.0f;
        else if (a < tDown)
            env = 1.0f - ss(0.0f, s.downSec, a - tFull);
        const float stem = s.stemSec > 0.0f && a >= tFull
                               ? kFlareStemShare * (1.0f - ss(0.5f * s.stemSec, s.stemSec, a - tDown))
                               : 0.0f;
        fl.strength = std::max(env, stem);
        fl.flameM = s.flameM * fl.strength;
        if (dist > drawRangeM || fl.strength <= 0.004f)
            continue;
        const float d = std::max(2.0f * p.r, 0.5f);
        const float lod = std::log2(std::max(1.0f, FlameLodDistance(dist, camLeft) * kLodRad / (0.15f * d))) + _lodBias;
        // (the kept tongues carry the dropped ones' light: fix 08)
        const LodSet flareLod = FlameLodSet(s.mainN + s.fillN, lod);
        const float flareFade = FlameRangeFade(dist, drawRangeM);
        const float puffHz = ColumnPuffHz(d, s.flameM);
        const float puffPhase = 6.2831853f * Hash01(fl.key + 41U);
        const float windRise = LickWindFactor(fl.midflame);
        // (fix 6) Its tongues of kVolumeFromM or more (their full length) are
        // volumes, their tips drawn kFlareConverge (main) or
        // kFlareConvergeFill of the way in toward the flare's own axis (from
        // its body's middle, leaning at its full flame's tilt), so the flare
        // reads as one flame body with several tips, not a spray of streaks.
        const float axisTilt = FlameTiltRad(kFlareWindShare * wind10, std::max(s.flameM, 0.3f));
        const float axisTan = std::tan(axisTilt), axisY = p.y + 0.5f * p.h;
        for (int j = 0; j < s.mainN + s.fillN; ++j)
        {
            const float keep = LodKeep(j, lod);
            if (keep <= 0.0f)
                continue;
            const bool main = j < s.mainN;
            const std::uint32_t key = fl.key + static_cast<std::uint32_t>(j) * 2246822519U + 13U;
            const float q1 = Hash01(key + 1U), q2 = Hash01(key + 2U), q3 = Hash01(key + 3U), q4 = Hash01(key + 4U),
                        q5 = Hash01(key + 5U);
            // its foot: in the plant's body, most in its lee half
            const float footH = (main ? s.mainFoot0 + (s.mainFoot1 - s.mainFoot0) * q1
                                      : s.fillFoot0 + (s.fillFoot1 - s.fillFoot0) * q1) *
                                p.h;
            const float turn = (q3 < s.leeShare ? 0.0f : 3.14159265f) + (q4 - 0.5f) * 3.14159265f;
            const float ddx = fl.dirX * std::cos(turn) - fl.dirZ * std::sin(turn),
                        ddz = fl.dirX * std::sin(turn) + fl.dirZ * std::cos(turn);
            const float rr = s.spread * d * std::sqrt(q2);
            const float fx = p.x + ddx * rr, fz = p.z + ddz * rr, fy = p.y + footH;
            // its own full length (fixed: its tempo's) and now
            const float Lown = main ? s.flameM * (0.7f + 0.4f * q5) : 0.5f * s.flameM * (0.5f + 0.5f * q5);
            // (a volume keeps a steady size: its puff is the bulge the renderer runs up it)
            const bool volume = Lown >= kVolumeFromM;
            const float puffTurns = puffHz * (t - footH / FeatureRiseMps(s.flameM)) + puffPhase / 6.2831853f;
            const float puff = volume ? 0.5f : 0.5f + 0.5f * std::sin(6.2831853f * puffTurns);
            const float L =
                Lown * (main ? env : std::max(env, stem)) * (1.0f + kFlarePuffStretch * (puff - 0.5f) * 2.0f);
            if (L < 0.05f)
                continue;
            const float tilt = FlameTiltRad(kFlareWindShare * wind10, std::max(L, 0.3f));
            const FlameStyle style =
                Hash01(key + 6U) < s.bushyShare
                    ? FlameStyle::Bushy
                    : (p.cls == PlantClass::Reed && Hash01(key + 7U) < 0.5f ? FlameStyle::Front : FlameStyle::Tongues);
            const float width = s.widthK * FlameFrameWidth(static_cast<int>(style), L);
            const float heat = (main ? 0.55f + 0.45f * env : std::max(0.5f + 0.5f * env, stem > 0.0f ? 0.5f : 0.0f)) *
                               (1.0f - kFlarePuffGlow + kFlarePuffGlow * puff);
            _footCls = kFootFlare;
            pushGain = keep * flareLod.gain * flareFade;
            if (!push(fx, fz, fy, L, tilt, width, heat, emissivity(s.absorb, 0.3f * L), key, style,
                      volume ? 1.0f : (main ? 1.3f : 1.6f), std::max(Lown, 0.3f),
                      FeatureRiseMps(std::max(Lown, 0.3f)) * windRise, 0.0f, kFlareFootFade * L,
                      kSwayBand + (kSwayBandMax - kSwayBand) * Hash01(key + 8U), 1.0f / SwayPeriodSec(Lown)))
                break;
            if (volume)
            {
                // a volume, its tip drawn in toward the flare's axis at its height
                FlameSheet& f = _sheets.back();
                const float lean = std::max(fy + f.height - axisY, 0.0f) * axisTan;
                const float ax = p.x + fl.dirX * lean, az = p.z + fl.dirZ * lean;
                const float converge = main ? kFlareConverge : kFlareConvergeFill;
                f.leanX += (ax - (fx + f.leanX)) * converge;
                f.leanZ += (az - (fz + f.leanZ)) * converge;
                f.volumeK = s.absorb;
                f.volumeR = 1.0f;
                f.volumeSoot = s.woody;
                f.volumeLen = Lown; // its place in the drawing order
                f.volumePuff = puffTurns;
                f.volumeWaveM = FeatureRiseMps(s.flameM) / std::max(puffHz, 0.05f);
                f.volumeBulge = kFlareBulge;
            }
            if (main && a >= s.peakSec && a <= tFull)
            {
                const float tip = footH + L * std::cos(tilt);
                fl.peakTip = std::max(fl.peakTip, tip);
                fl.peakFlame = std::max(fl.peakFlame, L);
                fl.tipSum += tip;
                fl.flameSum += L;
                fl.peakN++;
            }
        }
    }
    _stats.flares = static_cast<int>(_flares.size());
    // ---- Object fires (fire-objects): their flames ----
    // Each object fire's flames (FireObjects.hpp ObjectFire) through the same
    // pass as the grass's and as volumes like a torch's: a box's or a pool's
    // one flame body standing on its footprint (its main tongues from feet
    // over it converging on its leaning axis, fill tongues low and fat round
    // its base, the body puffing as a whole at a pool fire's rate,
    // ColumnPuffHz of its width), a face's leaving its opening and climbing the
    // wall, a jet's straight along its way; a hot shell's glow points by night.
    // Their level of detail is the flares' (fix 08: the kept carry the light
    // of the dropped).
    _objDrawn = 0;
    // A few dozen tongues each, near the eye and the scene's subject: they
    // never wait on the grass fire's budget (a reserve of kObjSheetReserve
    // past it, within the renderer's 24576 with the beds' 8000) nor take its
    // lowered level of detail (_lodBias), only their own distance's.
    sheetCap = kMaxSheets + kObjSheetReserve;
    for (const ObjectFire& of : _objectFires)
    {
        if (static_cast<int>(_sheets.size()) >= sheetCap)
            break;
        const float odx = of.x - cam.X(), odz = of.z - cam.Z();
        const float dist = std::sqrt(odx * odx + odz * odz);
        if (dist > drawRangeM)
            continue;
        const std::uint32_t okey =
            static_cast<std::uint32_t>(of.id * 2654435761ULL) ^ static_cast<std::uint32_t>(of.id >> 29) ^ 0x9e3779b9U;
        const float ofade = FlameRangeFade(dist, drawRangeM);
        _footCls = kFootFlare;
        // a hot shell's glow: a few dim points low round its foot
        if (of.glow > 0.02f && dist < lingerRangeM)
        {
            const float lfade = FlameRangeFade(dist, lingerRangeM);
            for (int j = 0; j < 6; ++j)
            {
                const std::uint32_t key = okey * 2246822519U + static_cast<std::uint32_t>(j) * 3266489917U + 17U;
                const float a = (Hash01(key) * 2.0f - 1.0f) * of.halfL,
                            c = (Hash01(key + 1U) < 0.5f ? -1.0f : 1.0f) * of.halfW;
                const float gx = of.x + a * of.ux - c * of.uz, gz = of.z + a * of.uz + c * of.ux;
                pushGain = lfade;
                push(gx, gz, of.groundY + 0.05f, kGlowM * (0.7f + 0.6f * Hash01(key + 2U)), 0.0f,
                     FlameFrameWidth(static_cast<int>(FlameStyle::Creep), kGlowM) * kTuftWiden, kGlowHeat * of.glow,
                     0.5f, key, FlameStyle::Creep, 1.6f, kGlowM, kTuftRise, -150.0f, 0.0f);
            }
        }
        if (of.flameM < 0.05f || of.heat <= 0.0f)
            continue;
        _objDrawn++;
        const float L = of.flameM;
        const float Lref = of.flameRefM > 0.05f ? of.flameRefM : L; // its tempo's length (fixed for its life)
        if (of.shape == ObjectFireShape::Jet)
        {
            // a straight bright body along its way, two thinner beside it
            const float jl = std::sqrt(of.jetX * of.jetX + of.jetY * of.jetY + of.jetZ * of.jetZ);
            const float jx = jl > 1e-3f ? of.jetX / jl : 0.0f, jy = jl > 1e-3f ? of.jetY / jl : 1.0f,
                        jz = jl > 1e-3f ? of.jetZ / jl : 0.0f;
            for (int j = 0; j < 3; ++j)
            {
                const std::uint32_t key = okey * 15485863U + static_cast<std::uint32_t>(j) * 32452843U + 991U;
                const float share = j == 0 ? 1.0f : 0.55f + 0.25f * Hash01(key);
                const float Lj = L * share, Ljref = Lref * share;
                const float sx = j == 0 ? 0.0f : 0.25f * (Hash01(key + 1U) - 0.5f);
                const float sz = j == 0 ? 0.0f : 0.25f * (Hash01(key + 2U) - 0.5f);
                // bent a little downwind by the wind over its length
                const float bendW = std::min(0.04f * wind10, 0.4f);
                float dx = jx + sx + wdx * bendW, dy = jy, dz = jz + sz + wdz * bendW;
                const float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
                dx /= dl;
                dy /= dl;
                dz /= dl;
                pushGain = ofade;
                if (!push(of.x, of.z, of.y, Lj, 0.0f, kObjJetWide * Lj * (j == 0 ? 1.0f : 0.65f), of.heat,
                          emissivity(of.absorb, 0.2f * Lj), key, FlameStyle::Tongues, 1.0f, Ljref,
                          FeatureRiseMps(Ljref) * kObjJetRise, of.tempK, 0.05f * Lj))
                    break;
                FlameSheet& f = _sheets.back();
                f.height = dy * Lj;
                f.leanX = dx * Lj;
                f.leanZ = dz * Lj;
                f.volumeK = of.absorb;
                f.volumeR = 0.7f;
                f.volumeSoot = of.soot;
                f.volumeLen = Lj;
            }
            continue;
        }
        if (of.shape == ObjectFireShape::Ball)
        {
            // billowing out and up from the ball's lower part: fat volume
            // tongues, their feet through its lower two thirds, each leaning out
            // from its centre and up, their tips over its top
            const float R = std::max(of.halfL, 0.3f);
            const int n = std::clamp(static_cast<int>(std::lround(14.0f + 1.5f * R)), 14, 48);
            const float lod = std::log2(std::max(1.0f, FlameLodDistance(dist, camLeft) * kLodRad / (0.15f * R)));
            const LodSet set = FlameLodSet(n, lod);
            for (int j = 0; j < n; ++j)
            {
                const float keep = LodKeep(j, lod);
                if (keep <= 0.0f)
                    continue;
                const std::uint32_t key = okey * 15485863U + static_cast<std::uint32_t>(j) * 32452843U + 977U;
                const float qa = std::fmod(0.5f + kR2X * static_cast<float>(j), 1.0f);
                const float qb = std::fmod(0.5f + kR2Z * static_cast<float>(j), 1.0f);
                // a point of its lower two thirds (cos of the polar angle from -1 to 0.35), 0.4 to 0.8 of R out
                const float cz = -1.0f + 1.35f * qa, sz = std::sqrt(std::max(1.0f - cz * cz, 0.0f));
                const float ang = 6.2831853f * qb, rr = R * (0.3f + 0.4f * Hash01(key + 1U));
                const float ox = sz * std::cos(ang), oz = sz * std::sin(ang);
                const float fx = of.x + ox * rr, fy = of.y + cz * rr, fz = of.z + oz * rr;
                float dx = 0.3f * ox, dy = 1.0f + 0.3f * std::max(cz, 0.0f), dz = 0.3f * oz;
                const float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
                const float share = 0.65f + 0.45f * Hash01(key + 2U);
                const float Lj = R * share, Ljref = Lref * share;
                const FlameStyle style = Hash01(key + 3U) < 0.7f ? FlameStyle::Bushy : FlameStyle::Tongues;
                pushGain = keep * set.gain * ofade;
                if (!push(fx, fz, fy, Lj, 0.0f, 1.8f * FlameFrameWidth(static_cast<int>(style), Lj), of.heat,
                          emissivity(of.absorb, 0.3f * Lj), key, style, 1.0f, Ljref, FeatureRiseMps(Ljref) * 1.4f,
                          of.tempK, 0.25f * Lj))
                    break;
                FlameSheet& f = _sheets.back();
                f.height = dy / dl * Lj;
                f.leanX = dx / dl * Lj;
                f.leanZ = dz / dl * Lj;
                f.volumeK = of.absorb;
                f.volumeR = 1.2f;
                f.volumeSoot = of.soot;
                f.volumeLen = Ljref;
            }
            continue;
        }
        if (of.shape == ObjectFireShape::Face)
        {
            // out of the opening's upper half, out along the face, then up the wall
            const int n = std::clamp(static_cast<int>(std::lround(2.0f + 2.5f * of.halfL)), 3, 12);
            const float lod = std::log2(
                std::max(1.0f, FlameLodDistance(dist, camLeft) * kLodRad / (0.3f * std::max(of.halfL, 0.3f))));
            const LodSet set = FlameLodSet(n, lod);
            const float puffHz = ColumnPuffHz(std::max(2.0f * of.halfL, 0.5f), Lref);
            const float puffPhase = Hash01(okey + 41U);
            for (int j = 0; j < n; ++j)
            {
                const float keep = LodKeep(j, lod);
                if (keep <= 0.0f)
                    continue;
                const std::uint32_t key = okey * 15485863U + static_cast<std::uint32_t>(j) * 32452843U + 977U;
                const float qa = std::fmod(0.5f + kR2X * static_cast<float>(j), 1.0f);
                const float a = (2.0f * qa - 1.0f) * 0.9f * of.halfL;
                const float fy = of.y + of.halfW * (1.0f + 0.9f * Hash01(key + 1U));
                const float fx = of.x + a * of.ux + of.faceX * 0.1f, fz = of.z + a * of.uz + of.faceZ * 0.1f;
                const float share = 0.7f + 0.45f * Hash01(key + 2U);
                const float Lown = L * share, Lt = Lref * share;
                const float out = kObjFaceOut + 0.2f * wind10 * std::max(of.faceX * wdx + of.faceZ * wdz, 0.0f) / 10.0f;
                float dx = of.faceX * out + 0.15f * wdx, dy = 1.0f, dz = of.faceZ * out + 0.15f * wdz;
                const float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
                const FlameStyle style = Hash01(key + 3U) < 0.6f ? FlameStyle::Bushy : FlameStyle::Tongues;
                const float puffTurns = puffHz * t + puffPhase;
                pushGain = keep * set.gain * ofade;
                if (!push(fx, fz, fy, Lown, 0.0f, kObjMainWiden * FlameFrameWidth(static_cast<int>(style), Lown),
                          of.heat, emissivity(of.absorb, 0.3f * Lown), key, style, kObjFaceBend, Lt, FeatureRiseMps(Lt),
                          of.tempK, kFlareFootFade * Lown))
                    break;
                FlameSheet& f = _sheets.back();
                f.height = dy / dl * Lown;
                f.leanX = dx / dl * Lown;
                f.leanZ = dz / dl * Lown;
                if (Lown >= kVolumeFromM)
                {
                    f.volumeK = of.absorb;
                    f.volumeR = 1.0f;
                    f.volumeSoot = of.soot;
                    f.volumeLen = Lt;
                    f.volumePuff = puffTurns;
                    f.volumeWaveM = FeatureRiseMps(Lref) / std::max(puffHz, 0.05f);
                    f.volumeBulge = kFlareBulge;
                    f.bend = 1.0f;
                }
            }
            continue;
        }
        // a box's or a pool's flame body
        const bool pool = of.shape == ObjectFireShape::Pool;
        const float hl = std::max(of.halfL, 0.2f), hw = std::max(of.halfW, 0.2f);
        const float D = 2.0f * std::sqrt(hl * hw);
        const int mainN =
            std::clamp(static_cast<int>(std::lround((pool ? kObjMainPool : kObjMainBox) + kObjMainPerM * (hl + hw))),
                       kObjMainMin, kObjMainMax);
        const int fillN =
            std::clamp(static_cast<int>(std::lround(kObjFillShare * static_cast<float>(mainN))), 3, kObjFillMax);
        const float lod = std::log2(std::max(1.0f, FlameLodDistance(dist, camLeft) * kLodRad / (0.15f * D)));
        const LodSet set = FlameLodSet(mainN + fillN, lod);
        const float puffHz = ColumnPuffHz(D, Lref);
        const float puffPhase = Hash01(okey + 41U);
        const float axisTilt = ObjectFlameTilt(wind10, L);
        const float axisTan = std::tan(axisTilt), axisY = of.y + kObjNeckShare * L;
        const float windRise = LickWindFactor(0.5f * wind10);
        for (int j = 0; j < mainN + fillN; ++j)
        {
            const float keep = LodKeep(j, lod);
            if (keep <= 0.0f)
                continue;
            const bool main = j < mainN;
            const std::uint32_t key = okey * 15485863U + static_cast<std::uint32_t>(j) * 32452843U + 977U;
            const float qa = std::fmod(0.5f + kR2X * static_cast<float>(j), 1.0f);
            const float qb = std::fmod(0.5f + kR2Z * static_cast<float>(j), 1.0f);
            // its foot over the footprint (the fill's toward its edge), a box's a little up its body
            float a = 0.0f, c = 0.0f;
            if (pool)
            {
                const float rr = main ? std::sqrt(qa) : 0.7f + 0.3f * qa, ang = 6.2831853f * qb;
                a = hl * rr * std::cos(ang);
                c = hw * rr * std::sin(ang);
            }
            else
            {
                a = (2.0f * qa - 1.0f) * hl;
                c = (2.0f * qb - 1.0f) * hw;
                if (!main)
                    c = (c < 0.0f ? -1.0f : 1.0f) * hw * (0.8f + 0.2f * Hash01(key + 9U));
            }
            const float fx = of.x + a * of.ux - c * of.uz, fz = of.z + a * of.uz + c * of.ux;
            const float fy = of.y + (pool ? 0.0f : kObjFootUpM * Hash01(key + 1U) * (main ? 1.0f : 0.3f));
            // (a pool fire is a cone: its tongues from the middle reach highest, those
            // from its edge less, kObjEdgeShort of it at the edge)
            const float edge = std::min(1.0f, (a * a) / (hl * hl) + (c * c) / (hw * hw));
            const float share = (main ? 0.7f + 0.45f * Hash01(key + 2U) : 0.3f + 0.25f * Hash01(key + 2U)) *
                                (1.0f - kObjEdgeShort * edge);
            const float Lown = L * share, Lt = Lref * share;
            // (a volume by its full length: one tongue never turns from a ribbon to a volume as the fire dies)
            const bool volume = Lt >= kVolumeFromM && Lown >= 0.5f * kVolumeFromM;
            const float puffTurns = puffHz * (t - (fy - of.y) / FeatureRiseMps(Lref)) + puffPhase;
            const float puff = 0.5f + 0.5f * std::sin(6.2831853f * puffTurns);
            const float Lnow = volume ? Lown : Lown * (1.0f + kFlarePuffStretch * (puff - 0.5f) * 2.0f);
            if (Lnow < 0.05f)
                continue;
            const float tilt = ObjectFlameTilt(wind10, Lnow) * (main ? 1.0f : 0.7f);
            const FlameStyle style = Hash01(key + 3U) < 0.7f ? FlameStyle::Bushy : FlameStyle::Tongues;
            const float width = (main ? kObjMainWiden : kObjFillWiden) * FlameFrameWidth(static_cast<int>(style), Lnow);
            const float heat = std::min(1.0f, of.heat * (main ? 0.9f + 0.1f * Hash01(key + 4U) : 1.0f)) *
                               (1.0f - kFlarePuffGlow + kFlarePuffGlow * puff);
            pushGain = keep * set.gain * ofade;
            if (!push(fx, fz, fy, Lnow, tilt, width, heat, emissivity(of.absorb, 0.3f * Lnow), key, style,
                      volume ? 1.0f : 1.3f, std::max(Lt, 0.3f), FeatureRiseMps(std::max(Lt, 0.3f)) * windRise, of.tempK,
                      kFlareFootFade * Lnow, kSwayBand + (kSwayBandMax - kSwayBand) * Hash01(key + 8U),
                      1.0f / SwayPeriodSec(Lt)))
                break;
            if (volume)
            {
                // a volume, its tip drawn in toward the fire's leaning axis at its height
                FlameSheet& f = _sheets.back();
                const float lean = std::max(fy + f.height - axisY, 0.0f) * axisTan;
                const float ax = of.x + wdx * lean, az = of.z + wdz * lean;
                const float converge = (main ? kObjConverge : kObjConvergeFill) * (pool ? kObjPoolConverge : 1.0f);
                f.leanX += (ax - (fx + f.leanX)) * converge;
                f.leanZ += (az - (fz + f.leanZ)) * converge;
                f.volumeK = of.absorb;
                f.volumeR = 1.0f;
                f.volumeSoot = of.soot;
                f.volumeLen = Lt;
                f.volumePuff = puffTurns;
                f.volumeWaveM = FeatureRiseMps(Lref) / std::max(puffHz, 0.05f);
                f.volumeBulge = pool ? kTorchBulge : kFlareBulge;
            }
        }
    }
    // (fix 6) The volume tongues (torches and flares) are drawn last, far to
    // near: the flame target composites in drawing order, and a near torch
    // drawn before a far one would be covered by it. The rest keep their order.
    // Each stands in that order by its middle at its full length
    // (FlameSheet::volumeLen), not its length now: the tongues of one column
    // overlap, and sorted by their length now they swapped places as each
    // puffed, a whole patch of the column changing from one frame to the next.
    _volSheets.clear();
    for (const FlameSheet& f : _sheets)
        if (f.volumeK > 0.0f)
            _volSheets.push_back(f);
    if (!_volSheets.empty())
    {
        _sheets.erase(
            std::remove_if(_sheets.begin(), _sheets.end(), [](const FlameSheet& f) { return f.volumeK > 0.0f; }),
            _sheets.end());
        const float cx = cam.X(), cy = cam.Y(), cz = cam.Z();
        auto far2 = [&](const FlameSheet& f)
        {
            const float now = std::sqrt(f.leanX * f.leanX + f.height * f.height + f.leanZ * f.leanZ);
            const float h = f.volumeLen > 0.0f && now > 1e-3f ? 0.5f * f.volumeLen / now : 0.5f;
            const float x = f.x + h * f.leanX - cx, y = f.y + h * f.height - cy, z = f.z + h * f.leanZ - cz;
            return x * x + y * y + z * z;
        };
        std::sort(_volSheets.begin(), _volSheets.end(),
                  [&](const FlameSheet& u, const FlameSheet& w) { return far2(u) > far2(w); });
        _sheets.insert(_sheets.end(), _volSheets.begin(), _volSheets.end());
    }
    if (_footOn)
        _foot.EndFrame();
    // The level of detail follows the budget: lowered while the tongues fill
    // most of it (or a cell was cut), raised again when they need far less.
    {
        const float used = static_cast<float>(_sheets.size()) / static_cast<float>(kMaxSheets);
        if (used > kBudgetHigh || _stats.sheetCut > 0)
            _lodBias = std::min(kBudgetBiasMax, _lodBias + kBudgetRate * dt);
        else if (used < kBudgetLow)
            _lodBias = std::max(0.0f, _lodBias - kBudgetRate * dt);
    }
    _stats.sheets = static_cast<int>(_sheets.size());
    _stats.sheetCells = cellsDrawn;
    // The drawing order (the realism audit's fix 7): the burning bed first, so
    // the tongues standing on it composite over it, then the tongues as fix
    // 6 ordered them.
    _stats.beds = static_cast<int>(_beds.size());
    if (!_beds.empty())
    {
        _mainSheets.clear();
        _mainSheets.reserve(_beds.size() + _sheets.size());
        _mainSheets.insert(_mainSheets.end(), _beds.begin(), _beds.end());
        _mainSheets.insert(_mainSheets.end(), _sheets.begin(), _sheets.end());
        _sheets.swap(_mainSheets);
    }

    // Sparks: thrown up by this frame's flames (EmitSparks).
    EmitSparks(dt, views, cam, wx, wz);

    // Smoke: the plume (EmitPlume) and its shadow.
    EmitPlume(dt, views, cam, wx, wz);
    UpdatePlumeShadow(dt, wx, wz);

    // Light: the flames' own (UpdateLights).
    UpdateLights(dt, cam, wx, wz);
}

void FireWorld::ClearLights()
{
    for (LightSlot& s : _lightSlots)
    {
        s.handle.Release();
        s = LightSlot{};
    }
    _lightPick = LightPick{};
    _lightCells.clear();
}

// The fire's light (FireSpreadWorld.hpp, "The fire's light"): each cell's
// flames as drawn (CellFlameLight, followed), summed into 32 m blocks, the
// torching crowns and flaring plants on their own, the strongest picked
// with hysteresis (LightPick) into persistent lights whose power, place and
// shape follow their sources, the rest lent to the nearest block light, and
// one broad fill from the lit smoke over the fire. Strengths are candela
// (CombatLights.hpp kind Wildfire puts them on the rig's scale, by day and
// by night); the flicker is laid on after the follow, so the follow never
// smooths it away.
void FireWorld::UpdateLights(float dt, Vector3Par cam, float wx, float wz)
{
    const float cellM = _grid->GetParams().cellM;
    const float t = static_cast<float>(std::fmod(_clock, 3600.0));
    const float wind10 = std::sqrt(wx * wx + wz * wz);
    const float wdx = wind10 > 0.01f ? wx / wind10 : 0.0f, wdz = wind10 > 0.01f ? wz / wind10 : 1.0f;
    constexpr std::uint64_t kBlockKind = 1, kTorchKind = 2, kFlareKind = 3, kFillKind = 4, kObjectKind = 5;
    auto idOf = [](std::uint64_t kind, std::uint64_t key) { return (kind << 60) | (key & 0x0FFFFFFFFFFFFFFFULL); };
    auto blockKey = [](int bx, int bz)
    {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(bx) & 0x0FFFFFFFU) << 28) |
               (static_cast<std::uint64_t>(static_cast<std::uint32_t>(bz) & 0x0FFFFFFFU));
    };
    auto blockSeed = [](int bx, int bz)
    { return static_cast<std::uint32_t>(bx) * 73856093U ^ static_cast<std::uint32_t>(bz) * 19349663U; };
    // the whole fire's gusts (every light and the halo breathe with them)
    const float gust = 1.0f + kFireSlowAmp * LightNoise(t, kFireSlowRate, 0x2545f491U);

    // Each cell's flames as drawn, followed (never a step), into the halo's
    // cells (flickered) and the blocks (not: the light's own flicker is laid
    // on the light).
    struct Block
    {
        double cd = 0.0, x = 0.0, z = 0.0, y = 0.0, h = 0.0, r2 = 0.0;
        int bx = 0, bz = 0;
    };
    std::unordered_map<std::uint64_t, Block> blocks;
    _lightCells.clear();
    for (auto& [key, r] : _after)
    {
        BurningView v = r.v;
        const bool live = r.frame == _drawFrame;
        if (!live)
        {
            const float ageNow = static_cast<float>(_clock - r.caught);
            // long done: its flamelets and glow are out too
            if (r.light < 1.0f && r.ember < 0.002f && ageNow > v.crossSec + 2.0f * r.trDraw + 600.0f)
                continue;
            AgeView(v, ageNow); // as Draw ages a record (fix 04's front fields too)
            v.torching = false;
            v.torchM = 0.0f;
        }
        // The drawn front's age at the slices (fix 04's field, as the tongues
        // read it), read again every kLightAgeFrames frames (and when the view
        // changed phase) and run on with the clock between; a cell whose band
        // and tail are long over or not near takes the planar estimate.
        float ages[kFlameLightSlices];
        {
            const float aEnd = kTailEnd * r.trDraw;
            const bool near = v.age > -30.0f && v.age - v.crossSec < aEnd + 30.0f;
            const std::uint32_t stagger = static_cast<std::uint32_t>(key ^ (key >> 32)) % kLightAgeFrames;
            if (near &&
                (r.lightAgeAt < 0.0 || r.lightAgeCaught != r.caught || (_drawFrame + stagger) % kLightAgeFrames == 0))
            {
                CellLightSliceAges(v, cellM, r.lightAge);
                r.lightAgeAt = _clock;
                r.lightAgeCaught = r.caught;
            }
            if (near && r.lightAgeAt >= 0.0)
            {
                const float run = static_cast<float>(_clock - r.lightAgeAt);
                for (int k = 0; k < kFlameLightSlices; ++k)
                    ages[k] = r.lightAge[k] + run;
            }
            else
                for (int k = 0; k < kFlameLightSlices; ++k)
                    ages[k] = v.crossSec > 0.0f ? v.age - (static_cast<float>(k) + 0.5f) /
                                                              static_cast<float>(kFlameLightSlices) * v.crossSec
                                                : v.age;
        }
        const float midflame = (v.sheltered ? 0.15f : Model(v.model).waf) * wind10;
        const CellLight target = CellFlameLight(v, r.trDraw, cellM, midflame, ages);
        r.light = LightFollow(r.light, target.cd, dt, kLightAttackSec, kLightReleaseSec);
        r.ember = LightFollow(r.ember, target.ember, dt, kLightAttackSec, kLightReleaseSec);
        if (target.heightM > 0.0f)
            r.lightH = target.heightM;
        if (r.light < 1.0f && r.ember < 0.002f)
            continue;
        const int bx = static_cast<int>(std::floor(v.x / kLightBlockM)),
                  bz = static_cast<int>(std::floor(v.z / kLightBlockM));
        const std::uint32_t cs = static_cast<std::uint32_t>(key ^ (key >> 29)) * 2654435761U;
        const float slow = gust * (1.0f + kBlockSlowAmp * LightNoise(t, kLightSlowRate, blockSeed(bx, bz) + 3U));
        const float fast = 1.0f + kHaloFastAmp * LightFlicker(t, cs + 1U);
        FlameLightCell c;
        c.x = v.x;
        c.z = v.z;
        c.cd = r.light * slow * fast;
        c.ember = std::clamp(r.ember, 0.0f, 1.0f);
        _lightCells.push_back(c);
        const float dx = v.x - cam.X(), dz = v.z - cam.Z();
        if (r.light < 1.0f || dx * dx + dz * dz > kLightReachM * kLightReachM)
            continue;
        Block& b = blocks[blockKey(bx, bz)];
        b.bx = bx;
        b.bz = bz;
        const double w = r.light;
        b.cd += w;
        b.x += w * v.x;
        b.z += w * v.z;
        b.y += w * (v.y + 0.5f * r.lightH);
        b.h += w * r.lightH;
        b.r2 += w * (static_cast<double>(v.x) * v.x + static_cast<double>(v.z) * v.z);
    }
    _stats.lightCells = static_cast<int>(_lightCells.size());

    // The candidates: the blocks, the torching crowns, the flaring plants.
    struct Cand
    {
        std::uint64_t id = 0, kind = 0;
        float cd = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f;
        float core = 1.0f, spread = 0.0f;
        float swingHz = 0.0f, swingPhase = 0.0f, swingAmp = 0.0f, fastAmp = 0.0f, slowAmp = 0.0f;
        std::uint32_t seed = 0, slowSeed = 0;
        bool picked = false;
    };
    std::vector<Cand> cands;
    cands.reserve(blocks.size() + 16);
    for (const auto& [bk, b] : blocks)
    {
        if (b.cd < 1.0)
            continue;
        Cand c;
        c.kind = kBlockKind;
        c.id = idOf(kBlockKind, bk);
        c.cd = static_cast<float>(b.cd);
        const double cx = b.x / b.cd, cz = b.z / b.cd;
        c.x = static_cast<float>(cx);
        c.z = static_cast<float>(cz);
        c.y = static_cast<float>(b.y / b.cd);
        const float rms = static_cast<float>(std::sqrt(std::max(b.r2 / b.cd - cx * cx - cz * cz, 0.0)));
        // its falloff from the flames' spread over it (a line across it:
        // about 16 m, smooth along a front of blocks); its cosine from half
        // a block off (the halo lights the ground nearer)
        c.core = std::clamp(1.7f * rms, 8.0f, 0.5f * kLightBlockM);
        c.spread = 0.5f * kLightBlockM;
        c.fastAmp = kBlockFastAmp;
        c.slowAmp = kBlockSlowAmp;
        c.seed = blockSeed(b.bx, b.bz) * 2246822519U + 7U;
        c.slowSeed = blockSeed(b.bx, b.bz) + 3U;
        cands.push_back(c);
    }
    for (const BurningView& v : _views)
    {
        if (v.torchM <= 0.05f || v.treeH <= 0.0f)
            continue;
        const float tx = v.x + v.treeOX, tz = v.z + v.treeOZ;
        const float dx = tx - cam.X(), dz = tz - cam.Z();
        if (dx * dx + dz * dz > kLightReachM * kLightReachM)
            continue;
        const float H = std::max(v.treeH, 1.0f);
        const float crown = kTorchCrownShare * H;
        const float Lcol = v.torchM;
        const float g0 = GLandscape ? GLandscape->SurfaceY(tx, tz) : v.y;
        const float Lnom = std::max(0.5f, (kTorchRatio - kTorchFootShare) * H);
        const std::uint32_t cell = static_cast<std::uint32_t>(static_cast<int>(std::floor(v.x / cellM)) * 73856093) ^
                                   static_cast<std::uint32_t>(static_cast<int>(std::floor(v.z / cellM)) * 19349663);
        Cand c;
        c.kind = kTorchKind;
        c.id = idOf(kTorchKind, PlantCellKey(v.x, v.z, cellM));
        c.cd = TorchLightCd(Lcol, v.torchMaxM, H);
        // a point up its column, on its axis: fix 06's drawn column (TorchColumnOf, bent over by
        // TorchColumnOffset), 0.4 of the way from the crown's foot to its tip as the torch stands now
        const TorchColumn col = TorchColumnOf(H, crown, (v.torchMaxM + kTorchFootShare * H) / H, wind10);
        const float zc = col.crownBaseM +
                         0.4f * (col.tipM - col.crownBaseM) * std::min(v.torchM / std::max(v.torchMaxM, 0.01f), 1.0f);
        const float off = TorchColumnOffset(col, zc);
        c.x = tx + wdx * off;
        c.z = tz + wdz * off;
        c.y = g0 + zc;
        c.core = std::max(0.25f * Lcol, 2.0f);
        // In step with the drawn column's puff (Draw): a bulge that climbs the column (fix 6's follow-up: the
        // column itself no longer stretches), brighter where it is. The light brightens as the bulge passes
        // its point, so its phase lags the crown's foot by the bulge's climb to 0.4 of the tree's full column
        // (a fixed lag: one that followed the growing torch would drag the light's rate down while it grows).
        c.swingHz = ColumnPuffHz(crown, Lnom);
        c.swingPhase = 6.2831853f * (Hash01(cell * 2166136261U + 23U) - c.swingHz * TorchBulgeSec(0.4f * Lnom, Lnom));
        c.swingAmp = kTorchSwing;
        c.fastAmp = 0.04f;
        c.seed = cell * 2654435761U + 5U;
        cands.push_back(c);
    }
    for (const Flare& fl : _flares)
    {
        if (fl.strength <= 0.004f || fl.flameM <= 0.05f)
            continue;
        const Plant& p = fl.p;
        const float dx = p.x - cam.X(), dz = p.z - cam.Z();
        if (dx * dx + dz * dz > kLightReachM * kLightReachM)
            continue;
        const FlareShape s = FlareShapeOf(p.cls, p.h);
        const float width = std::max(2.0f * p.r, 0.5f);
        const float tilt = FlameTiltRad(kFlareWindShare * wind10, std::max(fl.flameM, 0.3f));
        Cand c;
        c.kind = kFlareKind;
        c.id = idOf(kFlareKind, fl.key);
        c.cd = FlareLightCd(fl.flameM, width, tilt);
        c.x = p.x + fl.dirX * std::sin(tilt) * 0.4f * fl.flameM;
        c.z = p.z + fl.dirZ * std::sin(tilt) * 0.4f * fl.flameM;
        c.y = p.y + s.mainFoot0 * p.h + std::cos(tilt) * 0.4f * fl.flameM;
        c.core = std::max(0.4f * fl.flameM, 1.0f);
        // (as its drawn bulge passes the light's point, 0.4 of its full flame up, a fixed lag: Draw's flare loop)
        c.swingHz = ColumnPuffHz(width, s.flameM);
        c.swingPhase = 6.2831853f * (Hash01(fl.key + 41U) - c.swingHz * 0.4f * s.flameM / FeatureRiseMps(s.flameM));
        c.swingAmp = kFlareSwing;
        c.fastAmp = 0.06f;
        c.seed = fl.key * 2246822519U + 9U;
        cands.push_back(c);
    }

    // ---- Object fires (fire-objects): their light ----
    // Each object fire with flames is a light of its own (kObjectLightNits
    // over its flames as drawn: their length times its broad side, leaning at
    // its tilt; a jet kJetLightNits over its body), puffing with its body
    // (ColumnPuffHz of its width), its ground halo beside it; a hot shell's
    // glow a dim one.
    for (const ObjectFire& of : _objectFires)
    {
        const bool flame = of.flameM > 0.05f && of.heat > 0.0f;
        if ((!flame && of.glow < 0.02f) ||
            of.shape == ObjectFireShape::Ball) // (a fireball's light is the stock detonation's)
            continue;
        const float dx = of.x - cam.X(), dz = of.z - cam.Z();
        if (dx * dx + dz * dz > kLightReachM * kLightReachM)
            continue;
        const std::uint32_t okey =
            static_cast<std::uint32_t>(of.id * 2654435761ULL) ^ static_cast<std::uint32_t>(of.id >> 29) ^ 0x9e3779b9U;
        const float L = std::max(of.flameM, 0.0f);
        const float width = std::max(2.0f * std::max(of.halfL, of.halfW), 0.5f);
        Cand c;
        c.kind = kObjectKind;
        c.id = idOf(kObjectKind, of.id);
        if (of.shape == ObjectFireShape::Jet && flame)
        {
            c.cd = kJetLightNits * kObjJetWide * L * L * of.heat;
            c.x = of.x + of.jetX * 0.4f * L;
            c.y = of.y + of.jetY * 0.4f * L;
            c.z = of.z + of.jetZ * 0.4f * L;
        }
        else if (flame)
        {
            const float tilt = ObjectFlameTilt(wind10, L);
            c.cd = FlareLightCd(L, width, tilt) * (kObjectLightNits / kFlareLightNits) * (0.3f + 0.7f * of.heat);
            c.x = of.x + wdx * std::sin(tilt) * 0.4f * L;
            c.z = of.z + wdz * std::sin(tilt) * 0.4f * L;
            c.y = of.y + std::cos(tilt) * 0.4f * L;
        }
        else
        {
            c.cd = kObjectGlowCd * of.glow * width;
            c.x = of.x;
            c.z = of.z;
            c.y = of.groundY + 0.5f;
        }
        c.core = std::max(0.4f * L, 1.0f);
        c.swingHz = ColumnPuffHz(width, std::max(of.flameRefM > 0.05f ? of.flameRefM : L, 0.3f));
        c.swingPhase = 6.2831853f * Hash01(okey + 41U);
        c.swingAmp = of.shape == ObjectFireShape::Jet ? 0.1f : kFlareSwing;
        c.fastAmp = 0.06f;
        c.seed = okey * 2246822519U + 11U;
        cands.push_back(c);
        FlameLightCell hc;
        hc.x = of.x;
        hc.z = of.z;
        hc.cd = c.cd * gust * (1.0f + kHaloFastAmp * LightFlicker(t, okey + 1U));
        hc.ember = std::clamp(of.glow, 0.0f, 1.0f);
        _lightCells.push_back(hc);
    }
    _stats.lightCells = static_cast<int>(_lightCells.size());
    // The whole fire in reach, for the fill: its candela, centre and spread.
    double tot = 0.0, fx = 0.0, fz = 0.0, f2 = 0.0;
    for (const Cand& c : cands)
    {
        tot += c.cd;
        fx += static_cast<double>(c.cd) * c.x;
        fz += static_cast<double>(c.cd) * c.z;
        f2 += static_cast<double>(c.cd) * (static_cast<double>(c.x) * c.x + static_cast<double>(c.z) * c.z);
    }

    // The pick, with hysteresis.
    std::vector<LightCandidate> pc(cands.size());
    for (std::size_t i = 0; i < cands.size(); ++i)
    {
        const float dx = cands[i].x - cam.X(), dy = cands[i].y - cam.Y(), dz = cands[i].z - cam.Z();
        const float d2 = std::max(dx * dx + dy * dy + dz * dz, kLightNearM * kLightNearM);
        pc[i] = {cands[i].id, cands[i].cd / d2};
    }
    std::uint64_t before[kFireLightsMax];
    std::copy(std::begin(_lightPick.ids), std::end(_lightPick.ids), before);
    _lightPick.Update(pc.data(), static_cast<int>(pc.size()), dt);
    for (std::uint64_t id : _lightPick.ids)
        if (id != 0 && std::find(std::begin(before), std::end(before), id) == std::end(before))
            _stats.lightSwaps++;
    for (Cand& c : cands)
        c.picked = _lightPick.Has(c.id);
    // A picked source whose light has no slot yet waits for one (a fading
    // light frees it: a light is never cut off, so it never steps), the
    // strongest first; meanwhile it lends its power as an unpicked one does,
    // and the fading lights go twice as fast.
    bool scarce = false;
    {
        int open = 0;
        for (int i = 0; i < kLightSlots - 1; ++i)
            open += _lightSlots[i].id == 0 ? 1 : 0;
        std::vector<std::size_t> waiting;
        for (std::size_t i = 0; i < cands.size(); ++i)
        {
            if (!cands[i].picked)
                continue;
            bool has = false;
            for (int k = 0; k < kLightSlots - 1 && !has; ++k)
                has = _lightSlots[k].id == cands[i].id;
            if (!has)
                waiting.push_back(i);
        }
        std::sort(waiting.begin(), waiting.end(),
                  [&](std::size_t a, std::size_t b) { return pc[a].score > pc[b].score; });
        for (std::size_t i : waiting)
        {
            if (open > 0)
                --open;
            else
            {
                cands[i].picked = false;
                scarce = true;
            }
        }
    }

    // A source without a light lends its power to the nearest picked block
    // (any picked light when no block is picked): the light keeps its place
    // and grows its core to cover what it took (at most kLentCoreMaxM).
    constexpr float kLentCoreMaxM = 40.0f;
    struct Lent
    {
        double cd = 0.0, x = 0.0, z = 0.0, r2 = 0.0;
    };
    std::vector<Lent> lent(cands.size());
    for (std::size_t i = 0; i < cands.size(); ++i)
    {
        const Cand& c = cands[i];
        if (c.picked)
            continue;
        int best = -1;
        float bestD = 0.0f;
        for (int pass = 0; pass < 2 && best < 0; ++pass)
            for (std::size_t j = 0; j < cands.size(); ++j)
            {
                if (!cands[j].picked || (pass == 0 && cands[j].kind != kBlockKind))
                    continue;
                const float dx = cands[j].x - c.x, dz = cands[j].z - c.z;
                const float d = dx * dx + dz * dz;
                if (best < 0 || d < bestD)
                {
                    best = static_cast<int>(j);
                    bestD = d;
                }
            }
        if (best < 0)
            continue;
        Lent& l = lent[static_cast<std::size_t>(best)];
        l.cd += c.cd;
        l.x += static_cast<double>(c.cd) * c.x;
        l.z += static_cast<double>(c.cd) * c.z;
        l.r2 += static_cast<double>(c.cd) * (bestD + c.core * c.core);
    }
    for (std::size_t i = 0; i < cands.size(); ++i)
    {
        Cand& c = cands[i];
        const Lent& l = lent[i];
        if (!c.picked || l.cd <= 0.0)
            continue;
        const double all = c.cd + l.cd;
        const double r2 = (static_cast<double>(c.cd) * c.core * c.core + l.r2) / all;
        if (c.kind == kBlockKind)
        {
            c.x = static_cast<float>((static_cast<double>(c.cd) * c.x + l.x) / all);
            c.z = static_cast<float>((static_cast<double>(c.cd) * c.z + l.z) / all);
            c.core = std::clamp(static_cast<float>(std::sqrt(r2)), c.core, kLentCoreMaxM);
            c.spread = std::max(c.spread, c.core);
        }
        c.cd = static_cast<float>(all);
    }

    // The slots: a picked source keeps its light; a new one takes a free
    // slot (there is one: above); a light whose source lost its place fades
    // out; the last slot is the fill's.
    LightSlot& fillSlot = _lightSlots[kLightSlots - 1];
    for (int i = 0; i < kLightSlots - 1; ++i)
        if (_lightSlots[i].id != 0)
            _lightSlots[i].leaving = true;
    for (const Cand& c : cands)
    {
        if (!c.picked)
            continue;
        LightSlot* slot = nullptr;
        for (int i = 0; i < kLightSlots - 1 && !slot; ++i)
            if (_lightSlots[i].id == c.id)
                slot = &_lightSlots[i];
        const bool fresh = slot == nullptr;
        if (!slot)
        {
            for (int i = 0; i < kLightSlots - 1 && !slot; ++i)
                if (_lightSlots[i].id == 0)
                    slot = &_lightSlots[i];
            if (!slot)
                continue;
            slot->id = c.id;
            slot->power = 0.0f;
        }
        slot->leaving = false;
        const float k = fresh ? 1.0f : 1.0f - std::exp(-std::max(dt, 0.0f) / kLightMoveSec);
        slot->x += (c.x - slot->x) * k;
        slot->y += (c.y - slot->y) * k;
        slot->z += (c.z - slot->z) * k;
        slot->core += (c.core - slot->core) * k;
        slot->spread += (c.spread - slot->spread) * k;
        slot->swingHz = c.swingHz;
        slot->swingPhase = c.swingPhase;
        slot->swingAmp = c.swingAmp;
        slot->fastAmp = c.fastAmp;
        slot->slowAmp = c.slowAmp;
        slot->seed = c.seed;
        slot->slowSeed = c.slowSeed;
        slot->power = LightFollow(slot->power, c.cd, dt, kLightAttackSec, kLightReleaseSec);
    }
    // The fill: the lit smoke over the fire, downwind of its centre.
    {
        LightSlot& s = fillSlot;
        constexpr float kFillFollowSec = 3.0f;
        const float target = static_cast<float>(kFillShare * tot);
        if (tot > 1.0)
        {
            const double cx = fx / tot, cz = fz / tot;
            const float rms = static_cast<float>(std::sqrt(std::max(f2 / tot - cx * cx - cz * cz, 0.0)));
            const float px = static_cast<float>(cx) + wdx * kFillDownwindM;
            const float pz = static_cast<float>(cz) + wdz * kFillDownwindM;
            const float py = (GLandscape ? GLandscape->SurfaceY(px, pz) : cam.Y()) + kFillHeightM;
            const float core = kFillCoreMinM + 0.5f * rms;
            const bool fresh = s.id == 0;
            const float k = fresh ? 1.0f : 1.0f - std::exp(-std::max(dt, 0.0f) / kFillFollowSec);
            s.id = idOf(kFillKind, 1);
            s.leaving = false;
            s.x += (px - s.x) * k;
            s.y += (py - s.y) * k;
            s.z += (pz - s.z) * k;
            s.core += (core - s.core) * k;
            s.spread = 0.0f;
            s.slowAmp = kFillSlowAmp;
            s.slowSeed = 0x5bd1e995U;
        }
        if (s.id != 0)
            s.power = LightFollow(s.power, tot > 1.0 ? target : 0.0f, dt, kFillFollowSec, kFillFollowSec);
        _stats.fillCd = s.power;
    }
    // Light them; a fading light that has all but gone frees its slot. The
    // fill goes last, with the flicker of the lights it is lit by.
    _stats.lightsPicked = _stats.lightsFading = 0;
    _stats.lightCd = static_cast<float>(tot);
    int used = 0;
    double modSum = 0.0, modPower = 0.0;
    for (int i = 0; i < kLightSlots; ++i)
    {
        LightSlot& s = _lightSlots[i];
        if (s.id == 0)
            continue;
        const bool fill = i == kLightSlots - 1;
        if (s.leaving)
        {
            s.power =
                LightFollow(s.power, 0.0f, dt, kLightAttackSec, scarce ? 0.5f * kLightReleaseSec : kLightReleaseSec);
            if (s.power < std::max(10.0f, 0.002f * static_cast<float>(tot)))
            {
                s.handle.Release();
                s = LightSlot{};
                continue;
            }
            _stats.lightsFading++;
        }
        else if (i < kLightSlots - 1)
            _stats.lightsPicked++;
        float mod = fill ? (modPower > 0.0 ? static_cast<float>(modSum / modPower) : gust) : gust;
        if (s.swingAmp > 0.0f)
            mod *= 1.0f + s.swingAmp * std::sin(6.2831853f * s.swingHz * t + s.swingPhase);
        if (s.fastAmp > 0.0f)
            mod *= 1.0f + s.fastAmp * LightFlicker(t, s.seed);
        if (s.slowAmp > 0.0f)
            mod *= 1.0f + s.slowAmp * LightNoise(t, fill ? 0.2f : kLightSlowRate, s.slowSeed);
        if (!fill)
        {
            modSum += static_cast<double>(s.power) * mod;
            modPower += s.power;
        }
        const float p[3] = {s.x, s.y, s.z};
        const float range = std::clamp(std::sqrt(std::max(s.power, 0.0f) / kLightRangeLux), 60.0f, kLightRangeMaxM);
        s.handle.Set(lighting::CombatLightKind::Wildfire, p, std::max(s.power * mod, 0.0f));
        s.handle.SetShape(s.core, range, s.spread);
        ++used;
    }
    _stats.lights = used;
}

namespace
{
// ---- The sparks' sources (FireWorld::EmitSparks; the laws are FireSparks.hpp's) ----
// fireSpreadSet "flames" (the old ember budget, default kSparkBudgetRef)
// scales every count: kSparkBudgetRef makes the consult's.
constexpr float kSparkBudgetRef = 60.0f;
// A front's sparks are born along its tongues' axes, between kFrontBirthLo
// and kFrontBirthHi of the way to the tip (kFrontBirthTries tries each for a
// flaming point of the cell).
constexpr float kFrontBirthLo = 0.15f, kFrontBirthHi = 0.9f;
constexpr int kFrontBirthTries = 8;
// A flare's: from its body between kFlareBirthLo and kFlareBirthHi of the
// plant's height, its burst at the whoomp (kFlareBurstAt of its rise); all
// made out to kFlareSparkFullM, none past kFlareSparkMaxM (a flaring thicket
// reads to about a kilometre by day, its single sparks far less).
constexpr float kFlareBirthLo = 0.3f, kFlareBirthHi = 1.0f, kFlareBurstAt = 0.66f;
constexpr float kFlareSparkFullM = 100.0f, kFlareSparkMaxM = 600.0f;
// A torch's: torn off the crown and the column's lower part, on fix 6's
// drawn column (TorchColumnOf: its axis bent over by TorchColumnOffset,
// within TorchColumnRadius of it), from the crown's foot up to
// kTorchBirthReach of the column as it stands now (more low: the share
// squared); more leave with each puff of the column (kTorchPuffPeak times at
// a puff's peak at the crown, kTorchPuffLow at its trough, as the puff
// squared, over their mean), and up the column more are born where the
// bulge is now (06b's TorchBulgeSec: kTorchBulgeTries draws, each kept by
// the bulge's own weight there); the count follows the torch's own flame as
// (now / peak)^kTorchEnvPow.
constexpr float kTorchBirthReach = 0.6f, kTorchPuffPeak = 1.7f, kTorchPuffLow = 0.3f, kTorchEnvPow = 1.5f;
constexpr int kTorchBulgeTries = 4;
SparkKind SparkKindOf(PlantClass c)
{
    switch (c)
    {
        case PlantClass::Leafy:
            return SparkKind::Leafy;
        case PlantClass::Reed:
            return SparkKind::Reed;
        case PlantClass::Olive:
            return SparkKind::Olive;
        default:
            return SparkKind::Thicket;
    }
}
} // namespace

void FireWorld::EmitSparks(float dt, const std::vector<BurningView>& views, Vector3Par cam, float wx, float wz)
{
    const auto t0 = Clock::now();
    const float cellM = _grid->GetParams().cellM;
    const float wind10 = std::sqrt(wx * wx + wz * wz);
    const float wdx = wind10 > 0.01f ? wx / wind10 : 0.0f, wdz = wind10 > 0.01f ? wz / wind10 : 1.0f;
    const float scale = std::max(_flameBudget, 0.0f) / kSparkBudgetRef;
    const float t = static_cast<float>(std::fmod(_clock, 3600.0));
    auto ground = [](float x, float z, float y) { return GLandscape ? GLandscape->SurfaceY(x, z) : y; };
    auto distTo = [&](float x, float z)
    {
        const float dx = x - cam.X(), dz = z - cam.Z();
        return std::sqrt(dx * dx + dz * dz);
    };
    auto count = [&](float expected)
    {
        const float whole = std::floor(expected);
        return static_cast<int>(whole) + (_sparks.Uniform() < expected - whole ? 1 : 0);
    };
    float frontM = 0.0f;
    if (scale > 0.0f && dt > 0.0f)
    {
        // The fronts: a live cell whose leading edge is in it (one lit all at
        // once while its band burns) is cellM metres of front, its sparks
        // FrontSparksPerM of its flame now (more near the eye), each born on
        // a flaming point of the cell along its tongue's axis.
        const SparkKindShape front = SparkKindShapeOf(SparkKind::Front);
        for (const BurningView& v : views)
        {
            if (v.model == 0 || v.strength <= 0.0f || v.flameM < 0.1f || v.age < 0.0f)
                continue;
            const bool onFront =
                v.crossSec > 0.0f ? v.age < v.crossSec : v.age < DrawResidenceSec(v.model, v.residenceSec);
            if (!onFront)
                continue;
            const float d = distTo(v.x, v.z);
            if (d < kFrontSparkFullM)
                frontM += cellM;
            const float keep = SparkKeep(d, kFrontSparkFullM, kFrontSparkMaxM);
            if (keep <= 0.0f)
                continue;
            const float lifeK = FrontSparkLifeScale(v.flameM);
            const float perSec =
                cellM * FrontSparksPerM(v.flameM) * SparkNearBoost(d) / std::max(SparkMeanLifeSec(front) * lifeK, 0.1f);
            const int n = count(perSec * dt * keep * scale);
            const float midflame = (v.sheltered ? 0.15f : Model(v.model).waf) * wind10;
            for (int i = 0; i < n; ++i)
            {
                float ox = 0.0f, oz = 0.0f, share = 0.0f;
                for (int tries = 0; tries < kFrontBirthTries && share <= 0.05f; ++tries)
                {
                    ox = (_sparks.Uniform() - 0.5f) * cellM;
                    oz = (_sparks.Uniform() - 0.5f) * cellM;
                    const PartFlame pf = SurfacePart(v, ox, oz, cellM);
                    share = pf.heat > 0.05f ? pf.length : 0.0f;
                }
                if (share <= 0.05f)
                    continue;
                const float L = std::max(0.2f, v.flameMaxM * share);
                const float tilt = FlameTiltRad(midflame, L);
                const float fx = v.x + ox, fz = v.z + oz;
                const float gy = ground(fx, fz, v.y);
                const float up = kFrontBirthLo + (kFrontBirthHi - kFrontBirthLo) * _sparks.Uniform();
                SparkBirth b;
                b.kind = SparkKind::Front;
                b.x = fx + wdx * L * std::sin(tilt) * up;
                b.z = fz + wdz * L * std::sin(tilt) * up;
                b.y = gy + L * std::cos(tilt) * up;
                b.groundY = gy;
                b.tipY = gy + L * std::cos(tilt);
                b.riseMps = SparkRiseMps(L);
                const float u = wind10 * SparkWindShare(b.y - gy);
                b.vx = wdx * u;
                b.vz = wdz * u;
                b.vy = b.riseMps;
                b.weight = 1.0f / keep;
                b.lifeScale = lifeK;
                _sparks.Emit(b, 1);
            }
        }
        // The plants flaring: a burst of FlareSparkPeak at the whoomp (over
        // the kind's burstSec), and a thin stream (its streamShare of that
        // in the air, by the flare's strength) all through it.
        for (const Flare& fl : _flares)
        {
            if (fl.strength <= 0.004f || fl.flameM <= 0.05f)
                continue;
            const Plant& p = fl.p;
            const float d = distTo(p.x, p.z);
            const float keep = SparkKeep(d, kFlareSparkFullM, kFlareSparkMaxM);
            if (keep <= 0.0f)
                continue;
            const FlareShape s = FlareShapeOf(p.cls, p.h);
            const SparkKind kind = SparkKindOf(p.cls);
            const SparkKindShape k = SparkKindShapeOf(kind);
            const float peak = FlareSparkPeak(kind, s.flameM);
            const float a = static_cast<float>(_clock - fl.start);
            const float burstAt = kFlareBurstAt * s.peakSec;
            float perSec = k.streamShare * peak * fl.strength / std::max(SparkMeanLifeSec(k), 0.1f);
            if (a >= burstAt && a < burstAt + k.burstSec)
                perSec += peak / k.burstSec;
            const float L = std::max(fl.flameM, 0.3f);
            const float tilt = FlameTiltRad(kFlareWindShare * wind10, L);
            SparkBirth b;
            b.kind = kind;
            b.x = p.x;
            b.z = p.z;
            b.spreadXZ = 2.0f * std::max(p.r, 0.3f);
            b.y = p.y + kFlareBirthLo * p.h;
            b.spreadY = (kFlareBirthHi - kFlareBirthLo) * p.h;
            b.groundY = p.y;
            b.tipY = p.y + std::max(p.h, s.mainFoot1 * p.h + L * std::cos(tilt));
            b.riseMps = SparkRiseMps(L);
            const float u = wind10 * SparkWindShare(0.6f * p.h);
            b.vx = wdx * u;
            b.vz = wdz * u;
            b.vy = b.riseMps;
            b.weight = 1.0f / keep;
            _sparks.EmitExpected(b, perSec * dt * keep * scale);
        }
        // The torching crowns: a stream of TorchSparkPeak at the torch's
        // peak, more with each puff of the column (ColumnPuffHz, the drawn
        // column's own phase), torn off the crown and the column's lower
        // part on fix 6's drawn column, more where its bulge is now.
        const SparkKindShape torch = SparkKindShapeOf(SparkKind::Torch);
        const float puffMean = kTorchPuffLow + (kTorchPuffPeak - kTorchPuffLow) * 0.375f; // sin-squared's mean
        for (const BurningView& v : views)
        {
            if (v.torchM <= 0.05f)
                continue;
            const float tx = v.x + v.treeOX, tz = v.z + v.treeOZ;
            const float d = distTo(tx, tz);
            const float keep = SparkKeep(d, kTorchSparkFullM, kTorchSparkMaxM);
            if (keep <= 0.0f)
                continue;
            const float H = std::max(v.treeH, 1.0f);
            const float crown = kTorchCrownShare * H;
            const float env = std::clamp(v.torchM / std::max(v.torchMaxM, 0.01f), 0.0f, 1.0f);
            const float Lnom = std::max(0.5f, (kTorchRatio - kTorchFootShare) * H);
            const std::uint32_t cell =
                static_cast<std::uint32_t>(static_cast<int>(std::floor(v.x / cellM)) * 73856093) ^
                static_cast<std::uint32_t>(static_cast<int>(std::floor(v.z / cellM)) * 19349663);
            const float puffHz = ColumnPuffHz(crown, Lnom);
            const float puffPhase = 6.2831853f * Hash01(cell * 2166136261U + 23U);
            // the puff s metres up the column now (the bulge climbing it), as Draw's torch block puffs
            auto puffAt = [&](float sAlong)
            { return 0.5f + 0.5f * std::sin(6.2831853f * puffHz * (t - TorchBulgeSec(sAlong, Lnom)) + puffPhase); };
            const float puff = puffAt(0.0f);
            const float puffK = (kTorchPuffLow + (kTorchPuffPeak - kTorchPuffLow) * puff * puff) / puffMean;
            const float perSec = TorchSparkPeak(v.torchMaxM) * std::pow(env, kTorchEnvPow) * puffK /
                                 std::max(SparkMeanLifeSec(torch), 0.1f);
            const int n = count(perSec * dt * keep * scale);
            if (n <= 0)
                continue;
            // fix 6's column (its tip ratio as Draw's torch block eased it, read only)
            float tipRatio = (v.torchMaxM + kTorchFootShare * H) / H;
            const auto rit = _after.find(PlantCellKey(v.x, v.z, cellM));
            if (rit != _after.end() && rit->second.torchTip > 0.0f)
                tipRatio = rit->second.torchTip;
            const TorchColumn col = TorchColumnOf(H, crown, tipRatio, wind10);
            const float cl = std::cos(col.tilt);
            const float g0 = ground(tx, tz, v.y);
            // the column as it stands now: lit from the crown's foot up to its tip times env
            const float topM = col.crownBaseM + (col.tipM - col.crownBaseM) * env;
            const float rise = SparkRiseMps((topM - col.crownBaseM) / std::max(cl, 0.2f));
            for (int i = 0; i < n; ++i)
            {
                float zM = col.crownBaseM, wBest = -1.0f;
                for (int k = 0; k < kTorchBulgeTries; ++k)
                {
                    const float q = _sparks.Uniform();
                    const float z = col.crownBaseM + kTorchBirthReach * (topM - col.crownBaseM) * q * q;
                    const float pz = puffAt((z - col.crownBaseM) / std::max(cl, 0.2f));
                    const float w = kTorchPuffLow + (kTorchPuffPeak - kTorchPuffLow) * pz * pz;
                    if (w > wBest)
                    {
                        wBest = w;
                        zM = z;
                    }
                    if (_sparks.Uniform() * kTorchPuffPeak < w)
                        break;
                }
                const float lean = TorchColumnOffset(col, zM);
                const float rr = TorchColumnRadius(col, zM) * std::sqrt(_sparks.Uniform());
                const float ang = 6.2831853f * _sparks.Uniform();
                SparkBirth b;
                b.kind = SparkKind::Torch;
                b.x = tx + wdx * lean + std::cos(ang) * rr;
                b.z = tz + wdz * lean + std::sin(ang) * rr;
                b.y = g0 + zM;
                b.spreadY = 0.5f;
                b.groundY = g0;
                b.tipY = g0 + topM;
                b.riseMps = rise;
                const float u = wind10 * SparkWindShare(b.y - g0);
                b.vx = wdx * u;
                b.vz = wdz * u;
                b.vy = rise;
                b.weight = 1.0f / keep;
                _sparks.Emit(b, 1);
            }
        }
        // ---- Object fires (fire-objects): their sparks ----
        // sparkRate a second from its flames (a jet's along it, as a torch's), born through their lower body.
        for (const ObjectFire& of : _objectFires)
        {
            if (of.sparkRate <= 0.0f || of.flameM < 0.1f || of.heat <= 0.0f)
                continue;
            const float keep = SparkKeep(distTo(of.x, of.z), kTorchSparkFullM, kTorchSparkMaxM);
            if (keep <= 0.0f)
                continue;
            const bool jet = of.shape == ObjectFireShape::Jet;
            const int n = count(of.sparkRate * dt * keep * scale);
            for (int i = 0; i < n; ++i)
            {
                SparkBirth b;
                b.kind = jet ? SparkKind::Torch : SparkKind::Thicket;
                float px = of.x, pz = of.z;
                if (!jet)
                    ObjectFootPoint(of, 0.0f, _objLookRand, px, pz);
                const float up = of.flameM * (jet ? _sparks.Uniform() : 0.15f + 0.5f * _sparks.Uniform());
                b.x = px + (jet ? of.jetX * up : 0.0f);
                b.z = pz + (jet ? of.jetZ * up : 0.0f);
                b.y = of.y + (jet ? of.jetY * up : up);
                b.spreadXZ = jet ? 0.6f : 0.5f;
                b.spreadY = 0.4f;
                b.groundY = of.groundY;
                b.tipY = of.y + of.flameM * (jet ? std::max(of.jetY, 0.3f) : 1.0f);
                b.riseMps = SparkRiseMps(of.flameM) * (jet ? kObjJetRise : 1.0f);
                const float u = wind10 * SparkWindShare(b.y - of.groundY);
                b.vx = wdx * u;
                b.vz = wdz * u;
                b.vy = b.riseMps;
                b.weight = 1.0f / keep;
                _sparks.Emit(b, 1);
            }
        }
    }
    _sparkFrontM = frontM;
    const float night =
        GLOB_SCENE && GLOB_SCENE->MainLight() ? std::clamp(GLOB_SCENE->MainLight()->NightEffect(), 0.0f, 1.0f) : 0.0f;
    _sparkGlareM = kSparkGlareM * night;
    _sparks.Step(dt, wx, wz, [](float x, float z) { return GLandscape ? GLandscape->SurfaceY(x, z) : 0.0f; });
    _sparks.BuildPoints();
    _stats.flamePuffs = static_cast<int>(std::min<long long>(_sparks.Made(), 0x7fffffff));
    _msSparks = MsSince(t0);
    _msSparksMax = std::max(_msSparksMax, _msSparks);
}

// The plume (the realism audit's fix 2): the smoke of every burning cell,
// flaring plant and fresh burnt cell (as the smoke field's sources: the fuel
// burnt times the smoke a fuel makes) leaves as puffs (Smokes.hpp PlumePuff)
// that follow the bent-over plume of the whole fire's heat (FirePlume.hpp),
// a torching crown's from its flame's top on its own crown's heat. The puffs
// carry the source's smoke between them, so the plume is as dense as the
// fire makes it however many are drawn; fewer and larger ones while the fire
// is small on the screen.
void FireWorld::EmitPlume(float dt, const std::vector<BurningView>& views, Vector3Par cam, float wx, float wz)
{
    const float cellM = _grid->GetParams().cellM;
    const float w10 = std::sqrt(wx * wx + wz * wz);
    const float dirX = w10 > 1e-3f ? wx / w10 : 0.0f, dirZ = w10 > 1e-3f ? wz / w10 : 1.0f;
    // The sources, the fire's heat and its width across the wind.
    _plumeSrc.clear();
    float heatKw = 0.0f, smoke = 0.0f;
    float acrossMin = 1e30f, acrossMax = -1e30f;
    float bx0 = 1e30f, bz0 = 1e30f, bx1 = -1e30f, bz1 = -1e30f;
    auto add = [&](const PlumeSource& s)
    {
        if (s.smoke <= 0.0f)
            return;
        _plumeSrc.push_back(s);
        smoke += s.smoke;
        const float across = -dirZ * s.x + dirX * s.z;
        acrossMin = std::min(acrossMin, across);
        acrossMax = std::max(acrossMax, across);
        bx0 = std::min(bx0, s.x);
        bz0 = std::min(bz0, s.z);
        bx1 = std::max(bx1, s.x);
        bz1 = std::max(bz1, s.z);
    };
    // A cell's heat (kW) at full strength: its fireline intensity times its
    // width over the share of its life it flames.
    auto cellHeatKw = [&](const BurningView& v)
    {
        const float depth = cellM + std::max(v.rateMps, 0.0f) * std::max(v.residenceSec, 0.0f);
        return std::max(v.intensityKwM, 0.0f) * cellM * cellM / std::max(depth, 1.0f);
    };
    for (const BurningView& v : views)
    {
        if (v.strength <= 0.0f)
            continue;
        const bool woody = v.model >= 4;
        const float heat = v.strength * cellHeatKw(v);
        // The surface fire.
        PlumeSource s;
        s.x = v.x;
        s.z = v.z;
        s.y = v.y;
        s.smoke = 0.001f * heat * (woody ? kPlumeSmokeWoody : 1.0f);
        s.woody = woody ? 1.0f : 0.0f;
        s.baseM = 0.5f * v.strength * v.flameM;
        s.jitterM = 0.5f * cellM;
        s.cell = &v;
        add(s);
        heatKw += heat;
        // A torching crown: its own smoke from its flame's top, on its own heat.
        if (v.torching && v.torchM > 0.0f)
        {
            PlumeSource t;
            t.x = v.x + v.treeOX;
            t.z = v.z + v.treeOZ;
            t.y = v.y;
            t.torchMW = kPlumeTorchMW * std::clamp(v.torchM / std::max(v.torchMaxM, 1.0f), 0.05f, 1.0f);
            t.smoke = t.torchMW * kPlumeSmokeCrown;
            t.woody = 1.0f;
            t.baseM = v.treeH + 0.5f * v.torchM;
            t.jitterM = 2.0f;
            add(t);
        }
    }
    // The plants flaring: their own smoke from their top, their heat from
    // their flame (Byram inverted: I = (L / 0.0775)^(1 / 0.46)) over their width.
    for (const Flare& fl : _flares)
    {
        if (fl.strength <= 0.0f || fl.flameM <= 0.0f)
            continue;
        const float woody = FlareShapeOf(fl.p.cls, fl.p.h).woody;
        const float heat = fl.strength * std::pow(fl.flameM / 0.0775f, 1.0f / 0.46f) * std::max(2.0f * fl.p.r, 1.0f);
        PlumeSource s;
        s.x = fl.p.x;
        s.z = fl.p.z;
        s.y = fl.p.y;
        s.smoke = 0.001f * heat * (1.0f + (kPlumeSmokeWoody - 1.0f) * woody);
        s.woody = woody;
        s.baseM = fl.p.h;
        s.jitterM = fl.p.r;
        add(s);
        heatKw += heat;
    }
    // (fire-objects; followups) The object fires' smoke is not here: each
    // burning object has its own column on its own budget (EmitObjectPlumes).
    // The fresh burnt ground: thin white smoke (grey from litter and wood),
    // kAfterSmokeShare of its flaming smoke dying away (as the smoke field's).
    for (const auto& [key, r] : _after)
    {
        const BurningView& v = r.v;
        const float since = static_cast<float>(_clock - r.caught) - 0.5f * v.crossSec - r.trDraw;
        const bool wood = v.model >= 4;
        const float fade = since > 0.0f ? std::exp(-since / (wood ? kAfterSmokeTimberSec : kAfterSmokeSec)) : 0.0f;
        if (fade < 0.01f)
            continue;
        PlumeSource s;
        s.x = v.x;
        s.z = v.z;
        s.y = v.y;
        s.smoke = 0.001f * kAfterSmokeShare * cellHeatKw(v) * (wood ? kPlumeSmokeWoody : 1.0f) * fade;
        s.woody = wood ? 0.3f : 0.0f;
        s.baseM = 0.3f;
        s.jitterM = 0.5f * cellM;
        add(s);
    }
    const float ease = dt > 0.0f ? 1.0f - std::exp(-dt / kPlumeHeatEaseSec) : 0.0f;
    _plumeMW += (heatKw / 1000.0f - _plumeMW) * (_plumeMW <= 0.0f ? 1.0f : ease);
    _plumeSmoke = smoke;
    _plumeWidth = acrossMax >= acrossMin ? std::max(acrossMax - acrossMin + cellM, cellM) : 0.0f;

    // The flames' light on the puffs: heat by smoke block, eased.
    if (_smoke.Valid())
    {
        const int n = _smoke.nx * _smoke.nz;
        if (_glowX0 != _smoke.x0 || _glowZ0 != _smoke.z0 || _glowNX != _smoke.nx || _glowNZ != _smoke.nz)
        {
            std::vector<float> heat(n, 0.0f), y(n, 0.0f);
            for (int k = 0; k < n; ++k)
            {
                const int bx = _smoke.x0 + k % _smoke.nx, bz = _smoke.z0 + k / _smoke.nx;
                const int gx = bx - _glowX0, gz = bz - _glowZ0;
                y[k] = _smoke.ground[k];
                if (gx >= 0 && gz >= 0 && gx < _glowNX && gz < _glowNZ)
                {
                    heat[k] = _glowHeat[gz * _glowNX + gx];
                    y[k] = _glowY[gz * _glowNX + gx];
                }
            }
            _glowHeat.swap(heat);
            _glowY.swap(y);
            _glowX0 = _smoke.x0;
            _glowZ0 = _smoke.z0;
            _glowNX = _smoke.nx;
            _glowNZ = _smoke.nz;
        }
        std::vector<float> heat(n, 0.0f), hy(n, 0.0f);
        auto put = [&](float x, float z, float h, float y)
        {
            const int bx = SmokeField::Block(x), bz = SmokeField::Block(z);
            if (h <= 0.0f || !_smoke.In(bx, bz))
                return;
            const int k = _smoke.Idx(bx, bz);
            heat[k] += h;
            hy[k] += h * y;
        };
        for (const BurningView& v : views)
        {
            put(v.x, v.z, v.strength * v.flameM, v.y + 0.4f * v.flameM);
            if (v.torching && v.torchM > 0.0f)
                put(v.x + v.treeOX, v.z + v.treeOZ, kPlumeTorchGlow * v.torchM, v.y + v.treeH);
        }
        for (const Flare& fl : _flares)
            put(fl.p.x, fl.p.z, FlareHeat(fl, cellM), fl.p.y + 0.5f * fl.p.h);
        // (fire-objects) the object fires light their own smoke
        for (const ObjectFire& of : _objectFires)
            if (of.flameM > 0.05f)
                put(of.x, of.z,
                    2.0f * of.heat * of.flameM * std::max(2.0f * std::max(of.halfL, of.halfW), 1.0f) / cellM,
                    of.y + 0.5f * of.flameM);
        const float glowEase = dt > 0.0f ? 1.0f - std::exp(-dt / kPlumeGlowEaseSec) : 0.0f;
        for (int k = 0; k < n; ++k)
        {
            _glowHeat[k] += (heat[k] - _glowHeat[k]) * glowEase;
            if (heat[k] > 0.0f)
                _glowY[k] = hy[k] / heat[k];
        }
    }

    // The puffs: how many, and how large.
    _plumeRate = 0.0f;
    LODShapeWithShadow* shape = GLOB_SCENE->Preloaded(CloudletBasic);
    // (followups) the object fires' own columns first, on their own budget
    if (shape && dt > 0.0f)
        EmitObjectPlumes(dt, cam, w10, dirX, dirZ, shape);
    if (!shape || _plumeSrc.empty() || smoke <= 0.0f || dt <= 0.0f)
        return;
    // The fire's size on the screen: born puffs at least kPlumeMinPx across.
    const Camera& camera = *GLOB_SCENE->GetCamera();
    const float nx = std::clamp(cam.X(), bx0, bx1), nz = std::clamp(cam.Z(), bz0, bz1);
    const float dCam = std::max(std::sqrt((cam.X() - nx) * (cam.X() - nx) + (cam.Z() - nz) * (cam.Z() - nz)), 1.0f);
    const float pxPerM = 0.5f * static_cast<float>(GEngine ? GEngine->Width() : 1920) / (dCam * camera.Left());
    _plumeLod = std::clamp(kPlumeMinPx / std::max(kPlumeBornM * pxPerM, 1e-3f), 1.0f, kPlumeLodMax);
    const float rate =
        std::max(kPlumeRateMin, _smokeBudget * std::min(1.0f, smoke / kPlumeSmokeFull)) / (_plumeLod * _plumeLod);
    _plumeRate = rate;
    _smokeCredit = std::min(_smokeCredit + rate * dt, 2.0f + 0.5f * rate);
    if (_smokeCredit < 1.0f)
        return;
    // A source is picked by the square root of its smoke, kPlumeNearBoost
    // times more often near the eye, and its puff carries its smoke over the
    // chance it had (smoke / (rate x chance)), so the plume keeps the fire's
    // smoke: a weak source (a flank, fresh burnt ground) sends more puffs, each
    // thinner, and the smoke round the eye is finer. A puff is born as wide as
    // kPlumeSpacingFill of the gap the wind leaves between its source's puffs
    // (at least kPlumeBornM, at most kPlumeBornMaxM), so a source's smoke is a
    // stream, never a row of discs: thin smoke from a weak source comes as wide
    // faint puffs.
    _plumeCum.resize(_plumeSrc.size());
    float cum = 0.0f;
    for (std::size_t i = 0; i < _plumeSrc.size(); ++i)
    {
        PlumeSource& s = _plumeSrc[i];
        const float d = std::sqrt((s.x - cam.X()) * (s.x - cam.X()) + (s.z - cam.Z()) * (s.z - cam.Z()));
        s.pick = std::sqrt(s.smoke) * (1.0f + kPlumeNearBoost * (1.0f - PlumeSmooth(kPlumeNearM, kPlumeNearEndM, d)));
        _plumeCum[i] = (cum += s.pick);
    }
    const PlumeLaw lineLaw = MakePlumeLaw(_plumeMW, w10, 0.0f, true);
    const float perMetre = rate / std::max(w10, 1.0f);
    const float acrossMid = 0.5f * (acrossMin + acrossMax);
    const float t = static_cast<float>(_clock);
    auto wander = [&](float pa, float pb)
    { return 0.7f * std::sin(6.2831853f * t / pa + 1.3f) + 0.3f * std::sin(6.2831853f * t / pb + 4.1f); };
    const float wanderSide = wander(kPlumeWanderSec[0], kPlumeWanderSec[1]);
    const float wanderUp = wander(kPlumeWanderSec[2], kPlumeWanderSec[3]);
    while (_smokeCredit >= 1.0f)
    {
        _smokeCredit -= 1.0f;
        const float r = GFxRandGen.RandomValue() * cum;
        const std::size_t i = std::min<std::size_t>(
            std::lower_bound(_plumeCum.begin(), _plumeCum.end(), r) - _plumeCum.begin(), _plumeSrc.size() - 1);
        const PlumeSource& s = _plumeSrc[i];
        // Where it leaves: a surface cell's smoke only where its front has reached.
        float ox = 0.0f, oz = 0.0f;
        for (int tries = 0; tries < 3; ++tries)
        {
            ox = (GFxRandGen.RandomValue() * 2.0f - 1.0f) * s.jitterM;
            oz = (GFxRandGen.RandomValue() * 2.0f - 1.0f) * s.jitterM;
            if (!s.cell || PartStrength(*s.cell, ox, oz, cellM) > 0.05f)
                break;
            if (tries == 2)
                ox = oz = 0.0f;
        }
        const float px = s.x + ox, pz = s.z + oz;
        PlumePuffSpec spec;
        spec.source = Vector3(px, GLandscape ? GLandscape->SurfaceY(px, pz) : s.y, pz);
        spec.windX = dirX;
        spec.windZ = dirZ;
        const PlumeLaw law = s.torchMW > 0.0f ? MakePlumeLaw(s.torchMW, w10, s.baseM, false)
                                              : PlumeLaw{lineLaw.riseK, lineLaw.finalX, s.baseM, lineLaw.hugM};
        spec.riseK = law.riseK;
        spec.finalX = law.finalX;
        spec.baseM = law.baseM;
        spec.hugM = law.hugM;
        const float chance = std::max(s.pick, 1e-6f) / cum;
        const float gap = kPlumeLowWind * std::max(w10, 1.0f) / (rate * chance);
        spec.bornM = std::clamp(kPlumeSpacingFill * gap, kPlumeBornM * _plumeLod, kPlumeBornMaxM) *
                     (0.8f + 0.4f * GFxRandGen.RandomValue()) * (s.torchMW > 0.0f ? kPlumeTorchBorn : 1.0f);
        spec.perMetre = perMetre;
        spec.mass = s.smoke / (rate * chance);
        spec.load = smoke / std::max(w10, 1.0f);
        spec.opticK = kPlumeOpticK;
        spec.widthM = std::max(_plumeWidth, 2.0f * spec.bornM);
        spec.acrossM = -dirZ * px + dirX * pz - acrossMid;
        spec.lateral = GFxRandGen.RandomValue() + GFxRandGen.RandomValue() - 1.0f;
        spec.vertical = 0.65f * GFxRandGen.RandomValue() + 0.35f * GFxRandGen.RandomValue();
        spec.draw = GFxRandGen.RandomValue();
        spec.woody = s.woody;
        spec.meanderSide = wanderSide;
        spec.meanderUp = wanderUp;
        spec.lifeSec = kPlumeLifeSec;
        GLOB_WORLD->AddCloudlet(new PlumePuff(shape, spec));
        _stats.smokePuffs++;
        _plumeBorn++;
    }
}

// ---- (followups) The object fires' own columns ----
// Each burning object's smoke (its parts' together: a wreck's body, pool and
// jets, a building's) rises as its own column on its own budget
// (FirePlume.hpp PlanObjectColumn), never a share of the grass fire's puffs,
// as its flames have their own reserve (kObjSheetReserve): a burning house,
// wreck or tanker stands under its own column whatever the grass fire round
// it does. All the columns together send at most kObjPlumeRateMax puffs a
// second (each column's puffs born larger when that cuts them). Its puffs
// leave nearer its middle than the grass fire's (kObjPuffCore of the plume's
// spread across and through it: one coherent column, not a scatter of round
// puffs with sky between them) and live kObjPuffLifeSec (its far end, about
// a kilometre downwind in a strong wind, merges into the haze). Their cost is
// their fill, which goes with their cover (FirePlume.hpp kObjPuffOverlap) and
// their length, not their count (fewer puffs are born larger): a worst-day
// village's eight to a dozen columns at 40 min cost about 9 ms of GPU a frame
// from the helicopter with 1.6 behind each other for 150 s (the followups'
// story); these bounds are the compromise.
constexpr float kObjPlumeRateMax = 36.0f, kObjPuffCore = 0.6f, kObjPuffLifeSec = 100.0f;
void FireWorld::EmitObjectPlumes(float dt, Vector3Par cam, float w10, float dirX, float dirZ, LODShapeWithShadow* shape)
{
    _objPlumeRate = 0.0f;
    _objPlumeScale = 1.0f;
    _objCols.clear();
    ++_objColFrame;
    for (const ObjectFire& of : _objectFires)
    {
        if (of.smoke <= 0.0f || of.shape == ObjectFireShape::Ball)
            continue;
        // one column a burning object: a wreck's parts share its id but the
        // last 3 bits, a building's but the last 8
        const std::uint64_t key = of.owner == kObjectOwnerBuilding ? (of.id & ~0xffULL) : (of.id & ~0x7ULL);
        ObjColumn* c = nullptr;
        for (ObjColumn& k : _objCols)
            if (k.key == key)
            {
                c = &k;
                break;
            }
        if (!c)
        {
            _objCols.emplace_back();
            c = &_objCols.back();
            c->key = key;
        }
        const float w = of.smoke;
        const float baseM = std::max(of.y - of.groundY, 0.0f) + 0.85f * std::max(of.flameM, 0.0f);
        const float sum = c->smoke + w;
        c->x = (c->x * c->smoke + of.x * w) / sum;
        c->z = (c->z * c->smoke + of.z * w) / sum;
        c->y = (c->y * c->smoke + of.groundY * w) / sum;
        c->black = (c->black * c->smoke + std::clamp(of.black, 0.0f, 1.0f) * w) / sum;
        c->baseM = (c->baseM * c->smoke + baseM * w) / sum;
        c->smoke = sum;
        c->mw += std::max(of.heatMW, 0.0f);
        // born about its fire's width (a smouldering shell's about its own)
        c->bornM = std::max(c->bornM,
                            std::clamp(std::max(1.4f * std::max(of.halfL, of.halfW), 0.35f * of.flameM), 2.0f, 14.0f));
        c->jitterM = std::max(c->jitterM, 0.5f * std::max(std::max(of.halfL, of.halfW), 0.5f));
    }
    if (_objCols.empty())
    {
        _objColCredit.clear();
        return;
    }
    const Camera& camera = *GLOB_SCENE->GetCamera();
    const float halfW = 0.5f * static_cast<float>(GEngine ? GEngine->Width() : 1920);
    auto plan = [&](ObjColumn& c, float scale)
    {
        const float dx = c.x - cam.X(), dz = c.z - cam.Z(), dy = c.y + c.baseM - cam.Y();
        const float d = std::max(std::sqrt(dx * dx + dy * dy + dz * dz), 1.0f);
        const PlumeLaw law = MakePlumeLaw(std::max(c.mw, 0.05f), w10, c.baseM, false);
        c.plan = PlanObjectColumn(law, c.smoke, c.bornM, w10, halfW / (d * camera.Left()), scale);
    };
    float total = 0.0f;
    for (ObjColumn& c : _objCols)
    {
        plan(c, 1.0f);
        total += c.plan.rate;
    }
    if (total > kObjPlumeRateMax)
    {
        _objPlumeScale = kObjPlumeRateMax / total;
        total = 0.0f;
        for (ObjColumn& c : _objCols)
        {
            plan(c, _objPlumeScale);
            total += c.plan.rate;
        }
    }
    _objPlumeRate = total;
    // the columns' slow wander, each on its own phase
    const float t = static_cast<float>(_clock);
    auto wander = [&](float pa, float pb, float ph)
    { return 0.7f * std::sin(6.2831853f * (t + ph) / pa + 1.3f) + 0.3f * std::sin(6.2831853f * (t + ph) / pb + 4.1f); };
    for (const ObjColumn& c : _objCols)
    {
        ObjColumnCredit& cr = _objColCredit[c.key];
        cr.frame = _objColFrame;
        const float rate = c.plan.rate;
        cr.credit = std::min(cr.credit + rate * dt, 2.0f + 0.5f * rate);
        _plumeSmoke += c.smoke;
        if (cr.credit < 1.0f)
            continue;
        const PlumeLaw law = MakePlumeLaw(std::max(c.mw, 0.05f), w10, c.baseM, false);
        const float ph = 300.0f * Hash01(static_cast<std::uint32_t>(c.key * 2654435761ULL) ^
                                         static_cast<std::uint32_t>(c.key >> 32));
        const float wanderSide = wander(kPlumeWanderSec[0], kPlumeWanderSec[1], ph);
        const float wanderUp = wander(kPlumeWanderSec[2], kPlumeWanderSec[3], ph);
        while (cr.credit >= 1.0f)
        {
            cr.credit -= 1.0f;
            const float ox = (GFxRandGen.RandomValue() * 2.0f - 1.0f) * c.jitterM;
            const float oz = (GFxRandGen.RandomValue() * 2.0f - 1.0f) * c.jitterM;
            const float px = c.x + ox, pz = c.z + oz;
            PlumePuffSpec spec;
            spec.source = Vector3(px, GLandscape ? GLandscape->SurfaceY(px, pz) : c.y, pz);
            spec.windX = dirX;
            spec.windZ = dirZ;
            spec.riseK = law.riseK;
            spec.finalX = law.finalX;
            spec.baseM = law.baseM;
            spec.hugM = law.hugM;
            spec.bornM = c.plan.bornM * (0.8f + 0.4f * GFxRandGen.RandomValue());
            spec.perMetre = c.plan.perMetre;
            spec.mass = c.smoke / std::max(rate, 1e-3f);
            // its own column's load and width (its shade inside it), not the grass fire's
            spec.load = c.smoke / std::max(w10, 1.0f);
            spec.opticK = kPlumeOpticK;
            spec.widthM = 2.0f * c.plan.bornM;
            spec.acrossM = -dirZ * ox + dirX * oz;
            spec.lateral = kObjPuffCore * (GFxRandGen.RandomValue() + GFxRandGen.RandomValue() - 1.0f);
            spec.vertical =
                0.5f + kObjPuffCore * (0.65f * GFxRandGen.RandomValue() + 0.35f * GFxRandGen.RandomValue() - 0.5f);
            spec.draw = GFxRandGen.RandomValue();
            spec.woody = 1.0f;
            spec.black = c.black;
            spec.meanderSide = wanderSide;
            spec.meanderUp = wanderUp;
            spec.lifeSec = kObjPuffLifeSec;
            spec.overlap = kObjPuffOverlap;
            GLOB_WORLD->AddCloudlet(new PlumePuff(shape, spec));
            _stats.smokePuffs++;
            _plumeBorn++;
            _objPlumeBorn++;
        }
    }
    // a column no longer burning keeps no credit
    for (auto it = _objColCredit.begin(); it != _objColCredit.end();)
        it = it->second.frame != _objColFrame ? _objColCredit.erase(it) : std::next(it);
}

// The plume's shadow (GetPlumeShadow): the showing puffs grouped by how far
// they have come (kPlumeShadowEdgesM), each group one soft blob: its puffs'
// middle weighted by their smoke (optical depth times cover), spread along the
// wind over its stretch and across it as its puffs are (plus a third of their
// size), its vertical optical depth their smoke over the blob's footprint.
// Rebuilt every kPlumeShadowSec (the plume moves a few metres meanwhile).
void FireWorld::UpdatePlumeShadow(float dt, float wx, float wz)
{
    _plumeShadowTimer -= dt;
    if (_plumeShadowTimer <= 0.0f || _plumeShadowTarget.count == 0)
    {
        _plumeShadowTimer = kPlumeShadowSec;
        RebuildPlumeShadow(wx, wz);
    }
    // ---- No steps round the eye (fix 02 follow-up): the shadow eased ----
    // Each blob (one per stretch downwind, the same slot always) moves
    // toward its rebuild; one that comes or goes fades in place.
    const float k = dt > 0.0f ? 1.0f - std::exp(-dt / kPlumeShadowEaseSec) : 0.0f;
    bool any = false;
    for (int b = 0; b < kPlumeShadowBlobs; ++b)
    {
        PlumeShadowBlob& c = _plumeShadow.blobs[b];
        const PlumeShadowBlob& t = _plumeShadowTarget.blobs[b];
        if (c.tau <= 1e-4f && t.tau > 1e-4f)
        {
            const float from = c.tau;
            c = t;
            c.tau = from + (t.tau - from) * k;
        }
        else if (t.tau <= 1e-4f)
        {
            c.tau -= c.tau * k;
        }
        else
        {
            c.x += (t.x - c.x) * k;
            c.z += (t.z - c.z) * k;
            c.y += (t.y - c.y) * k;
            c.alongM += (t.alongM - c.alongM) * k;
            c.acrossM += (t.acrossM - c.acrossM) * k;
            c.dirX += (t.dirX - c.dirX) * k;
            c.dirZ += (t.dirZ - c.dirZ) * k;
            c.tau += (t.tau - c.tau) * k;
        }
        if (c.tau <= 1e-4f && t.tau <= 1e-4f)
            c.tau = 0.0f;
        any = any || c.tau > 0.0f;
    }
    _plumeShadow.count = any ? kPlumeShadowBlobs : 0;
    // ---- end the shadow eased ----
}

void FireWorld::RebuildPlumeShadow(float wx, float wz)
{
    _plumeShadowTarget = PlumeShadowView();
    if (!GLOB_WORLD || PlumePuff::Alive() <= 0)
        return;
    const float wl = std::sqrt(wx * wx + wz * wz);
    const float ux = wl > 1e-3f ? wx / wl : 0.0f, uz = wl > 1e-3f ? wz / wl : 1.0f;
    struct Acc
    {
        double w = 0.0, x = 0.0, z = 0.0, y = 0.0, a = 0.0, a2 = 0.0, size = 0.0;
    };
    Acc acc[kPlumeShadowBlobs];
    for (int i = 0; i < GLOB_WORLD->NCloudlets(); ++i)
    {
        const PlumePuff* p = dynamic_cast<const PlumePuff*>(GLOB_WORLD->GetCloudlet(i));
        if (!p || p->BodyOpacity() < 0.01f)
            continue;
        const float x = p->TravelM();
        int b = 0;
        while (b < kPlumeShadowBlobs && x >= kPlumeShadowEdgesM[b + 1])
            ++b;
        if (b >= kPlumeShadowBlobs)
            continue;
        const float s = p->Scale();
        const double w = -std::log(1.0 - std::min(p->BodyOpacity(), 0.99f)) * kPuffCover * s * s;
        const Vector3 pos = p->Position();
        const double across = -uz * pos.X() + ux * pos.Z();
        Acc& c = acc[b];
        c.w += w;
        c.x += w * pos.X();
        c.z += w * pos.Z();
        c.y += w * pos.Y();
        c.a += w * across;
        c.a2 += w * across * across;
        c.size += w * s;
    }
    for (int b = 0; b < kPlumeShadowBlobs; ++b)
    {
        const Acc& c = acc[b];
        if (c.w <= 1e-3)
            continue;
        // A blob per stretch, in its own slot (UpdatePlumeShadow eases them).
        PlumeShadowBlob& o = _plumeShadowTarget.blobs[b];
        _plumeShadowTarget.count = kPlumeShadowBlobs;
        o.x = static_cast<float>(c.x / c.w);
        o.z = static_cast<float>(c.z / c.w);
        o.y = static_cast<float>(c.y / c.w);
        const double mean = c.a / c.w;
        const float sd = static_cast<float>(std::sqrt(std::max(c.a2 / c.w - mean * mean, 0.0)));
        o.alongM = 0.5f * (kPlumeShadowEdgesM[b + 1] - kPlumeShadowEdgesM[b]);
        o.acrossM = std::max(sd + static_cast<float>(c.size / c.w) / 3.0f, 5.0f);
        o.dirX = ux;
        o.dirZ = uz;
        o.tau = static_cast<float>(c.w / (6.2831853 * o.alongM * o.acrossM));
    }
}

bool FireWorld::PlumeLightAt(Vector3Par p, float rgb[3]) const
{
    rgb[0] = rgb[1] = rgb[2] = 0.0f;
    if (_glowNX <= 0 || _glowNZ <= 0 || _glowHeat.empty())
        return false;
    const int bx = SmokeField::Block(p.X()), bz = SmokeField::Block(p.Z());
    float e = 0.0f;
    for (int dz = -kPlumeGlowBlocks; dz <= kPlumeGlowBlocks; ++dz)
        for (int dx = -kPlumeGlowBlocks; dx <= kPlumeGlowBlocks; ++dx)
        {
            const int gx = bx + dx - _glowX0, gz = bz + dz - _glowZ0;
            if (gx < 0 || gz < 0 || gx >= _glowNX || gz >= _glowNZ)
                continue;
            const int k = gz * _glowNX + gx;
            const float h = _glowHeat[k];
            if (h <= 0.01f)
                continue;
            // A block's flames are spread over it: the distance to its nearest
            // part, sideways, and to their middle's height.
            const float cx = (bx + dx + 0.5f) * SmokeField::kBlock, cz = (bz + dz + 0.5f) * SmokeField::kBlock;
            const float side = std::max(
                std::sqrt((p.X() - cx) * (p.X() - cx) + (p.Z() - cz) * (p.Z() - cz)) - 0.4f * SmokeField::kBlock, 0.0f);
            const float up = p.Y() - _glowY[k];
            e += kPlumeGlowLuxPerHeat * h * PlumeGlowKernel(std::sqrt(side * side + up * up));
        }
    if (e <= 0.0f)
        return false;
    for (int i = 0; i < 3; ++i)
        rgb[i] = e * kPlumeGlowRgb[i];
    return true;
}

void FireWorld::Measure(int cls, float model, float drawn, float top, float tilt, float share, float size)
{
    FlameMeasure& m = _measure[std::clamp(cls, 0, kMeasureClasses - 1)];
    m.n++;
    m.model += model;
    m.drawn += drawn;
    m.tilt += tilt;
    m.size += size;
    m.modelMax = std::max(m.modelMax, model);
    m.drawnMax = std::max(m.drawnMax, drawn);
    m.topMax = std::max(m.topMax, top);
    m.sizeMax = std::max(m.sizeMax, size);
    if (share >= kMeasureLead)
    {
        m.lead++;
        m.leadModel += model;
        m.leadDrawn += drawn;
        m.leadTop += top;
    }
}

// What v4's flame code drew for this tongue (FireSpreadWorld.cpp before
// fire-B): 1.8 times the cell's flame length (a torching cell's at least
// 18.2 m), times 0.65..1.35, grown by PartStrength, leaned by
// atan(0.8 U / sqrt(g L)) on 0.4 of the 10 m wind (0.5 when torching) at most
// 0.6 rad, stacked in pieces of 4 m whose top piece's flame tip sits at the
// flipbook's measured share of the frame, not the 0.3 v4 assumed.
void FireWorld::MeasureV4(const BurningView& v, std::uint32_t cell, int k, float ox, float oz, float cellM,
                          float wind10)
{
    const float flameMax = v.torching ? std::max(v.flameMaxM, 14.0f * 1.3f) : v.flameMaxM;
    const float lmax = std::max(1.0f, 1.8f * flameMax) * (0.65f + 0.7f * Hash01(cell * 3266489917U + k * 61U + 3U));
    const float s = std::min(1.0f, PartStrength(v, ox, oz, cellM));
    if (s <= 0.004f)
        return;
    const float grow = 0.35f + 0.65f * std::sqrt(s);
    const float L = lmax * (0.3f + 0.7f * s) * grow;
    const float froude = 0.4f * wind10 / std::sqrt(9.81f * L);
    const float lean = std::min(0.6f, std::atan((v.torching ? 0.5f : 0.8f) * froude));
    const int pieces = std::clamp(static_cast<int>(std::ceil(lmax / kPieceM)), 1, 5);
    const float pieceL = L / (1.0f + 0.7f * static_cast<float>(pieces - 1));
    FlameStyle top = PickStyle(v, Hash01(cell * 374761393U + k * 23U + 5U));
    if (pieces > 1)
        top = Hash01(cell * 1103515245U + k * 53U + (pieces - 1) * 211U + 13U) < 0.6f ? FlameStyle::Tongues
                                                                                      : FlameStyle::Front;
    const float visible =
        0.7f * pieceL * static_cast<float>(pieces - 1) + pieceL * FlameTipShare(static_cast<int>(top)) / 0.3f;
    const float vtop = visible * std::cos(lean);
    FlameMeasure& m =
        _measure[v.torching ? kMeasureTorch
                            : (v.model >= 4 && v.model <= 7 ? kMeasureShrub
                                                            : (v.model >= 8 ? kMeasureTimber : kMeasureGrass))];
    m.v4Max = std::max(m.v4Max, visible);
    m.v4TopMax = std::max(m.v4TopMax, vtop);
    if (s >= kMeasureLead)
    {
        m.lead4++;
        m.lead4Drawn += visible;
        m.lead4Top += vtop;
    }
}

namespace
{
void IssueSoloMove(AIGroup* grp, AIUnit* unit, Vector3 tgt)
{
    if (!grp || !unit)
        return;
    if (GLandscape)
    {
        float dx, dz;
        tgt[1] = GLOB_LAND->SurfaceYAboveWater(tgt[0], tgt[2], &dx, &dz);
    }
    Command mv;
    mv._message = Command::Move;
    mv._destination = tgt;
    mv._discretion = Command::Undefined;
    mv._context = Command::CtxMission;
    mv._id = grp->GetNextCmdId();
    PackedBoolArray one;
    one.Set(unit->ID() - 1, true);
    grp->IssueCommand(mv, one);
    if (AISubgroup* sub = unit->GetSubgroup())
        sub->SetSpeedMode(SpeedFull);
}
} // namespace

void FireWorld::Gameplay(float dt, float windX, float windZ)
{
    if (!GLOB_WORLD || !_grid)
        return;
    // The fire fells no tree (StatsLine's trees stays 0). A live tree stands
    // through its torch and the rest of the day (the consult's eighth answer,
    // 5), so its crown flame always burns on a standing tree and ends with
    // its crown. A fall would belong only to a tree already dead, and no
    // island marks one.

    _playTimer += dt;
    _aiTimer += dt;
    const bool doDamage = _playTimer >= 0.5f;
    const bool doAi = _aiTimer >= 1.0f;
    if (!doDamage && !doAi)
        return;
    const float dmgDt = _playTimer;
    if (doDamage)
        _playTimer = 0.0f;
    if (doAi)
        _aiTimer = 0.0f;
    const float wind = std::sqrt(windX * windX + windZ * windZ);

    for (int i = 0; i < GLOB_WORLD->NVehicles(); ++i)
    {
        Entity* ent = GLOB_WORLD->GetVehicle(i);
        Man* man = dyn_cast<Man>(ent);
        if (!man || man->IsDammageDestroyed())
            continue;
        const Vector3 pos = man->Position();
        float flame = 0.0f;
        float s = _grid->StrengthAt(pos.X(), pos.Z(), &flame);
        // (buildings builder) a burning building's flames on him (in it, or beside a lit window)
        if (!_bldLive.empty())
        {
            float bf = 0.0f;
            const float bs = BuildingHeatAt(pos.X(), pos.Z(), bf);
            if (bs > s)
            {
                s = bs;
                flame = bf;
            }
        }
        if (doDamage && s > 0.0f && man->IsLocal())
        {
            // In the flames: a metre-high grass fire kills in a few seconds,
            // a creeping backing fire burns more slowly.
            const float rate = std::clamp(0.10f * flame, 0.04f, 0.5f) * s;
            man->SetDammage(std::min(1.0f, man->GetTotalDammage() + rate * dmgDt));
            _stats.damaged++;
        }
        // (fire-objects) In or beside an object fire's flames (within its
        // hurtM of its footprint, under their tips): burnt as in the grass
        // fire's, by its flames' length.
        if (doDamage && man->IsLocal() && !_objectFires.empty())
        {
            float orate = 0.0f;
            for (const ObjectFire& of : _objectFires)
            {
                if (of.flameM < 0.1f || of.heat <= 0.0f || of.shape == ObjectFireShape::Ball ||
                    pos.Y() < of.groundY - 1.0f || pos.Y() > of.y + of.flameM)
                    continue; // (a fireball's harm is the stock blast's)
                float d = 0.0f;
                if (of.shape == ObjectFireShape::Jet)
                {
                    // off the jet's line
                    const float rx = pos.X() - of.x, ry = pos.Y() + 1.0f - of.y, rz = pos.Z() - of.z;
                    const float along = std::clamp(rx * of.jetX + ry * of.jetY + rz * of.jetZ, 0.0f, of.flameM);
                    const float ex = rx - of.jetX * along, ey = ry - of.jetY * along, ez = rz - of.jetZ * along;
                    d = std::max(std::sqrt(ex * ex + ey * ey + ez * ez) - kObjJetWide * of.flameM, 0.0f);
                }
                else
                    d = ObjectOutside(of, pos.X(), pos.Z());
                if (d > of.hurtM)
                    continue;
                orate = std::max(orate, std::clamp(0.10f * of.flameM, 0.04f, 0.5f) * of.heat *
                                            (1.0f - 0.5f * d / std::max(of.hurtM, 0.1f)));
            }
            if (orate > 0.0f)
            {
                man->SetDammage(std::min(1.0f, man->GetTotalDammage() + orate * dmgDt));
                _stats.damaged++;
            }
        }
        if (!doAi)
            continue;
        AIUnit* unit = man->Brain();
        if (!unit || unit->IsPlayer() || !man->IsLocal())
            continue;
        auto fit = _fleeUntil.find(man);
        if (fit != _fleeUntil.end() && fit->second > _clock)
            continue;
        // The flames a soldier would notice: within about 80 m round him, and
        // up to 200 m upwind, where a head fire comes from. Their heat-weighted centre.
        if (!_smoke.Valid())
            continue;
        const float wdx = wind > 0.5f ? windX / wind : 0.0f, wdz = wind > 0.5f ? windZ / wind : 0.0f;
        const int bx = SmokeField::Block(pos.X()), bz = SmokeField::Block(pos.Z());
        float cx = 0.0f, cz = 0.0f, h = 0.0f;
        for (int z = bz - 6; z <= bz + 6; ++z)
            for (int x = bx - 6; x <= bx + 6; ++x)
            {
                if (!_smoke.In(x, z))
                    continue;
                const float hk = _smoke.heat[_smoke.Idx(x, z)];
                if (hk <= 0.0f)
                    continue;
                const float ox = (x + 0.5f) * SmokeField::kBlock - pos.X();
                const float oz = (z + 0.5f) * SmokeField::kBlock - pos.Z();
                const float d = std::sqrt(ox * ox + oz * oz);
                const float upwind = -(ox * wdx + oz * wdz); // > 0: the fire is upwind of him
                if (d > 80.0f && !(upwind > 0.0f && d < 200.0f && std::fabs(ox * wdz - oz * wdx) < 80.0f))
                    continue;
                cx += (pos.X() + ox) * hk;
                cz += (pos.Z() + oz) * hk;
                h += hk;
            }
        if (h < 0.5f && s <= 0.0f)
            continue;
        if (h > 0.0f)
        {
            cx /= h;
            cz /= h;
        }
        else
        {
            cx = pos.X() - windX;
            cz = pos.Z() - windZ;
        }
        // Where to run. A head fire outruns a man in the long run and is
        // deepest along the wind, so with the fire upwind of him he runs across
        // the wind to the nearer flank (and a little downwind); beside or
        // behind it he runs straight away from it. Never back into the head.
        float rx = pos.X() - cx, rz = pos.Z() - cz;
        const float rl = std::max(0.1f, std::sqrt(rx * rx + rz * rz));
        rx /= rl;
        rz /= rl;
        float ax = rx, az = rz;
        if (wind > 0.5f && rx * wdx + rz * wdz > 0.3f)
        {
            // The crosswind side he already stands on (the flank nearer him).
            const float side = (rx * -wdz + rz * wdx) >= 0.0f ? 1.0f : -1.0f;
            ax = -wdz * side + 0.35f * wdx;
            az = wdx * side + 0.35f * wdz;
            const float l = std::sqrt(ax * ax + az * az);
            ax /= l;
            az /= l;
        }
        // Try that way, then 40 and 80 degrees off it (away from the wind's source first), for ground not burning.
        const float angles[5] = {0.0f, 0.7f, -0.7f, 1.4f, -1.4f};
        Vector3 tgt = pos + Vector3(ax, 0, az) * 100.0f;
        for (float a : angles)
        {
            const float c = std::cos(a), sn = std::sin(a);
            const float dx = ax * c - az * sn, dz = ax * sn + az * c;
            if (wind > 0.5f && dx * wdx + dz * wdz < -0.5f)
                continue; // never into the wind's teeth
            const Vector3 t = pos + Vector3(dx, 0, dz) * 100.0f;
            if (_grid->StrengthAt(t.X(), t.Z()) <= 0.0f &&
                _grid->StrengthAt(pos.X() + dx * 50.0f, pos.Z() + dz * 50.0f) <= 0.0f)
            {
                tgt = t;
                break;
            }
        }
        IssueSoloMove(unit->GetGroup(), unit, tgt);
        _fleeUntil[man] = static_cast<float>(_clock) + 5.0f;
        _stats.fled++;
    }
}

void FireWorld::UpdateBurnMap(float dt)
{
    constexpr float kBurnMapHz = 8.0f;
    const firespread::Stats& g = _grid->GetStats();
    _burnTimer += dt;
    if (_burnTimer < (g.burning > 0 ? 1.0f / kBurnMapHz : 1.0f) && _burnSize != 0)
        return;
    _burnTimer = 0.0f;
    // (end-state agent) a burnt shell with no fire known: the map's window
    // round it all the same (the renderer reads its look from the map's atlas)
    const Building* shell = nullptr;
    if (_burnSize == 0)
        for (const Building& sb : _bld)
            if (sb.slots[0] >= 0 && (sb.destroyed || sb.live))
            {
                shell = &sb;
                break;
            }
    if (g.burning == 0 && g.burnt == _burnLastBurnt && g.burning == _burnLastBurning && !shell)
        return;
    float bx0, bz0, bx1, bz1;
    if (!_grid->Bounds(bx0, bz0, bx1, bz1))
    {
        // (buildings builder) a building alight with no grass fire known yet:
        // the map's window round it all the same (the renderer reads its
        // burnt look from the map's atlas; (end-state agent) a burnt shell
        // too); (followups) or a wreck burning (its char and its glow are read
        // with the map bound)
        if (_bldLive.empty() && !shell && _wrecks.empty())
            return;
        float cx = 0.0f, cz = 0.0f;
        if (!_bldLive.empty() || shell)
        {
            const Building& b = _bldLive.empty() ? *shell : _bld[static_cast<std::size_t>(_bldLive.front())];
            cx = b.cx;
            cz = b.cz;
        }
        else
        {
            cx = _wrecks.front().x;
            cz = _wrecks.front().z;
        }
        bx0 = cx - 30.0f;
        bz0 = cz - 30.0f;
        bx1 = cx + 30.0f;
        bz1 = cz + 30.0f;
    }
    _burnLastBurnt = g.burnt;
    _burnLastBurning = g.burning;
    // A 512 texel window, kBurnTexelM texels (1 km), widened in steps of it when the fire outgrows it.
    const int size = 512;
    const float extent = std::max(bx1 - bx0, bz1 - bz0) + 64.0f;
    const float texel = std::max(kBurnTexelM, kBurnTexelM * std::ceil(extent / (size * kBurnTexelM)));
    const float cx = 0.5f * (bx0 + bx1), cz = 0.5f * (bz0 + bz1);
    // The window moves only when the fire nears its edge (keeps the texels still on screen).
    const float half = 0.5f * size * texel;
    const bool fits = _burnSize == size && texel == _burnTexel && bx0 > _burnX0 + 32.0f && bz0 > _burnZ0 + 32.0f &&
                      bx1 < _burnX0 + 2.0f * half - 32.0f && bz1 < _burnZ0 + 2.0f * half - 32.0f;
    if (!fits)
    {
        _burnTexel = texel;
        _burnX0 = texel * std::floor((cx - half) / texel);
        _burnZ0 = texel * std::floor((cz - half) / texel);
        _burnSize = size;
        // The land's height under the window, for the renderer's char on
        // walls and trunks (how far above the ground a fragment is).
        _groundY.resize(static_cast<std::size_t>(kGroundSize) * kGroundSize);
        const float gstep = size * texel / kGroundSize;
        for (int j = 0; j < kGroundSize; ++j)
            for (int i = 0; i < kGroundSize; ++i)
                _groundY[static_cast<std::size_t>(j) * kGroundSize + i] =
                    GLandscape ? GLandscape->SurfaceY(_burnX0 + (i + 0.5f) * gstep, _burnZ0 + (j + 0.5f) * gstep)
                               : 0.0f;
        _groundVersion++;
    }
    // (the dance fix) the map before this one kept for LatchBurnMap below
    _burnPrev.swap(_burn);
    _burn.assign(static_cast<std::size_t>(size) * size, 0);
    const float cell = _grid->GetParams().cellM;
    const float hc = 0.5f * cell;
    auto put = [&](int i, int j, float b)
    {
        if (i < 0 || j < 0 || i >= size || j >= size || b <= 0.0f)
            return;
        unsigned char& t = _burn[static_cast<std::size_t>(j) * size + i];
        t = std::max(t, static_cast<unsigned char>(std::clamp(b, 0.0f, 255.0f)));
    };
    // Burnt-out cells with only burnt ground round them: char all over, every
    // texel whose centre lies in a cell's square, whatever the texel
    // (PaintBurntInterior, the lattice fix).
    _grid->CollectBurntInterior(_burntScratch);
    PaintBurntInterior(_burntScratch, cell, _burnX0, _burnZ0, _burnTexel, size, _burn.data());
    // Every other caught cell texel by texel from its exact front
    // (FrontAgeAt: the arrival field with metre-scale edge noise), out to a
    // texel past its square so the edge's ramp has both sides: a texel the
    // front has passed by s metres (FrontDepthAt) ramps up over kBurnEdgeTexels texels
    // behind the front (BurnEdgeRamp: nothing ahead of it, so the char never
    // runs in front of the flames), then chars from a scorch (110) toward the
    // ash (230) over the point's drawn residence, and to black (255) over as
    // long again.
    // The bare surface of a road never chars (its edge ramps the same way, so
    // a verge burns black to it); other bare ground beside the cell takes
    // the char's edge on the cell's side, ragged by the same noise.
    const float W = kBurnEdgeTexels * _burnTexel;
    _grid->CollectBurntEdges(_edgeScratch);
    auto texels = [&](const BurningView& v0)
    {
        BurningView v = v0;
        BurningView raw = v0;
        raw.roadHalf = 0.0f; // the front read on the road too (its ramp under the road's edge)
        const FrontDepthSupport depthSup = FrontDepthSupportOf(raw);
        const float d = std::max(DrawResidenceSec(v.model, v.residenceSec), 0.5f);
        // the square and a texel round it, wider on a side that borders a road cell (its verge runs on to the bare
        // edge)
        const float xLo0 = -hc - (v.roadSides & 2 ? cell : _burnTexel),
                    xHi0 = hc + (v.roadSides & 1 ? cell : _burnTexel);
        const float zLo0 = -hc - (v.roadSides & 8 ? cell : _burnTexel),
                    zHi0 = hc + (v.roadSides & 4 ? cell : _burnTexel);
        // (buildings builder) and on a side that borders a building's bare
        // cell, into the quarter of its square on this side (BuiltRunOn)
        const float xLo = v.builtSides & 2 ? std::min(xLo0, -hc - cell) : xLo0;
        const float xHi = v.builtSides & 1 ? std::max(xHi0, hc + cell) : xHi0;
        const float zLo = v.builtSides & 8 ? std::min(zLo0, -hc - cell) : zLo0;
        const float zHi = v.builtSides & 4 ? std::max(zHi0, hc + cell) : zHi0;
        const int i0 = static_cast<int>(std::floor((v.x + xLo - _burnX0) / _burnTexel));
        const int i1 = static_cast<int>(std::ceil((v.x + xHi - _burnX0) / _burnTexel));
        const int j0 = static_cast<int>(std::floor((v.z + zLo - _burnZ0) / _burnTexel));
        const int j1 = static_cast<int>(std::ceil((v.z + zHi - _burnZ0) / _burnTexel));
        for (int j = std::max(j0, 0); j < std::min(j1, size); ++j)
            for (int i = std::max(i0, 0); i < std::min(i1, size); ++i)
            {
                const float wx = _burnX0 + (i + 0.5f) * _burnTexel, wz = _burnZ0 + (j + 0.5f) * _burnTexel;
                const float ox = wx - v.x, oz = wz - v.z;
                const bool runOn = ox < xLo0 || ox > xHi0 || oz < zLo0 || oz > zHi0;
                if (runOn && !BuiltRunOn(v.builtSides, ox, oz, cell))
                    continue;
                float speed = 0.0f;
                const float a = FrontAgeAt(raw, ox, oz, cell, &speed);
                // metres behind the front (negative: ahead of it): on a seed's
                // field no deeper than its burnt area's nearest edge
                // (FrontDepthAt: beside an ignition line the head's speed put
                // the backing front's ground tens of metres in, follow-up 13)
                const float s = a > -1e8f ? FrontDepthAt(raw, depthSup, ox, oz, a, speed) : -1e9f;
                float e = BurnEdgeRamp(s, W);
                if (e <= 0.0f)
                    continue;
                // a road's bare surface: the ramp from its edge (positive on the verge)
                if (v.roadHalf > 0.0f)
                {
                    const float sr = ((ox - v.roadX) * v.roadDZ - (oz - v.roadZ) * v.roadDX) * v.roadSide - v.roadHalf;
                    e *= std::clamp(0.5f + sr / W, 0.0f, 1.0f);
                }
                // other bare ground beside it, and the square's own edge where no road cell lies beyond
                float ex = 0.0f, ez = 0.0f;
                if (v.bareSides)
                    EdgeWarp(wx + 37.0f, wz - 11.0f, ex, ez);
                auto side = [&](float into, bool bare)
                {
                    // into: metres inside the square from that side (negative: past it)
                    if (bare)
                        e *= std::clamp(0.5f + (into + ex) / W, 0.0f, 1.0f);
                };
                // (a building's bare cell is no edge: the char runs on into it)
                side(hc - ox, (v.bareSides & ~v.builtSides & 1) != 0);
                side(hc + ox, (v.bareSides & ~v.builtSides & 2) != 0);
                side(hc - oz, (v.bareSides & ~v.builtSides & 4) != 0);
                side(hc + oz, (v.bareSides & ~v.builtSides & 8) != 0);
                // past the square into a neighbour: only the edge's ramp (that neighbour, if caught, draws its own
                // char); into a building's bare cell (its quarter on this side) all of it
                const bool out = ((ox > hc && !(v.roadSides & 1)) || (ox < -hc && !(v.roadSides & 2)) ||
                                  (oz > hc && !(v.roadSides & 4)) || (oz < -hc && !(v.roadSides & 8))) &&
                                 !(v.builtSides && BuiltRunOn(v.builtSides, ox, oz, cell));
                float b = 110.0f;
                if (a > 0.0f)
                    b += 120.0f * std::clamp(a / d, 0.0f, 1.0f) + 25.0f * std::clamp((a - d) / d, 0.0f, 1.0f);
                if (out)
                    b = std::min(b, 109.0f);
                put(i, j, e * b);
            }
    };
    for (const BurningView& v : _views)
        texels(v);
    for (const BurningView& v : _edgeScratch)
        texels(v);
    // (the dance fix) Ground once burnt stays burnt: every texel at least
    // what the map before read there (LatchBurnMap), unless the fire was
    // cleared since (the grid's clock went back or it has fewer ignitions).
    BurnWindow win;
    win.x0 = _burnX0;
    win.z0 = _burnZ0;
    win.texelM = _burnTexel;
    win.size = size;
    if (g.simTime < _burnPrevSim || g.ignitions < _burnPrevIgn)
        _burnPrevWin = BurnWindow();
    if (_burnPrev.size() == static_cast<std::size_t>(_burnPrevWin.size) * static_cast<std::size_t>(_burnPrevWin.size))
        LatchBurnMap(_burnPrev.data(), _burnPrevWin, win, _burn.data());
    _burnPrevWin = win;
    _burnPrevSim = g.simTime;
    _burnPrevIgn = g.ignitions;
    _burnVersion++;
}

// ---- Each plant's own fire (the realism audit's fix 3) ----
// The plant table's version changes at most this often (the renderer
// re-uploads it then); a plant's times are in it, so the shader's look moves
// on smoothly between uploads.
constexpr double kPlantTableHz = 8.0;

// A plant's look from what reached it (FireSpreadWorld.hpp PlantLookOf): its
// flare's shape if it flared, the torch's start if it torched.
PlantLook PlantFireLook(const PlantFire& pf)
{
    const FlareShape s = FlareShapeOf(pf.p.cls, pf.p.h);
    const float torchSec = pf.torched ? static_cast<float>(pf.torchFront + kTorchDelaySec - pf.contact) : 0.0f;
    return PlantLookOf(pf.kind, pf.p.h, pf.kwM, pf.flameM, pf.flared ? &s : nullptr, pf.torched, torchSec);
}

void FireWorld::ClearPlants()
{
    _pf.clear();
    _pfByKey.clear();
    _pfCells.clear();
    _pfSeen.clear();
    _pfSched.clear();
    _pfTable = PlantTable();
    for (Building& bd : _bld)
        std::fill(std::begin(bd.slots), std::end(bd.slots), -1);
    ++_pfVersion;
    _pfDirty = _pfFull = false;
    _pfBumpAt = -1.0e9;
    _pfFinalAt = 0.0;
    _pfContacts = _pfFlared = _pfTorched = 0;
}

void FireWorld::WritePlant(const PlantFire& pf)
{
    const PlantLook& k = pf.look;
    for (const int s : pf.slots)
    {
        if (s < 0)
            continue;
        float* d = _pfTable.Slot(s);
        d[kPlantY] = pf.p.y;
        d[kPlantH] = pf.p.h;
        d[kPlantContact] = pf.contact < 0.0 ? kPlantNotYet
                           : pf.final       ? kPlantLongAgo
                                            : static_cast<float>(std::fmod(pf.contact, kPlantClockWrap));
        d[kPlantDirX] = pf.dirX;
        d[kPlantDirZ] = pf.dirZ;
        d[kPlantScorchTop] = k.scorchTopM;
        d[kPlantScorchRise] = k.scorchRiseSec;
        d[kPlantBarkWind] = k.barkWindM;
        d[kPlantBarkLee] = k.barkLeeM;
        d[kPlantBurn0] = k.burn0Sec;
        d[kPlantBurn1] = k.burn1Sec;
        d[kPlantSweepMax] = k.sweepMax;
        d[kPlantGoneThreshold] =
            k.strands ? PlantStrandHalfWidth(1.0f - k.goneShare) : PlantNoiseThreshold(k.goneShare);
        d[kPlantKeptChar] = k.keptChar;
        d[kPlantFrameTop] = k.frameTopM;
        d[kPlantKeepBelow] = k.keepBelowM;
        d[kPlantClump] = k.clumpM;
        d[kPlantGlow] = k.glow;
        d[kPlantScorchChar] = k.scorchChar;
        d[kPlantStrands] = k.strands ? 1.0f : 0.0f;
    }
    _pfDirty = true;
}

bool FireWorld::InsertPlant(PlantFire& pf, int reserve)
{
    if (_pfTable.slots == 0)
        _pfTable.Reset(_pfStartSlots);
    if (_pfTable.Insert(pf.p.x, pf.p.z, pf.slots, reserve))
        return true;
    // Too full: a table twice the size holds every foot again, the buildings'
    // first (end-state agent: a burnt building's look lasts as long as the
    // building; a plant refused at the largest table leaves every slot as it
    // was, where it once cleared every building's).
    std::vector<PlantFootRef> blds, plants;
    blds.reserve(_bld.size());
    for (Building& bd : _bld)
        if (bd.slots[0] >= 0)
        {
            const Vector3 p = bd.obj ? bd.obj->Position() : Vector3(bd.cx, bd.y, bd.cz);
            blds.push_back({p.X(), p.Z(), bd.slots});
        }
    plants.reserve(_pf.size());
    for (PlantFire& o : _pf)
        plants.push_back({o.p.x, o.p.z, o.slots});
    bool grew = false;
    int dropped = 0;
    if (PlantTableAdd(_pfTable, {pf.p.x, pf.p.z, pf.slots}, false, reserve, _pfMaxSlots, blds, plants, grew, dropped))
    {
        if (grew)
        {
            RewriteTableLooks();
            LOG_INFO(Script, "UDFIRE;event=planttable;slots={};plants={}", _pfTable.slots, _pf.size() + 1);
        }
        return true;
    }
    if (!_pfFull)
        LOG_WARN(Script, "UDFIRE;event=planttable;full;slots={};plants={}", _pfTable.slots, _pf.size());
    _pfFull = true;
    return false;
}

int FireWorld::BuildingSlotReserve() const
{
    int without = 0;
    for (const Building& bd : _bld)
        if (bd.slots[0] < 0 && bd.kind != BuildingKind::Never && bd.kind != BuildingKind::Fence)
            ++without;
    return PlantBuildingReserve(without);
}

void FireWorld::RewriteTableLooks()
{
    for (const PlantFire& o : _pf)
        WritePlant(o);
    for (Building& bd : _bld)
        if (bd.slots[0] >= 0)
        {
            const float tb = static_cast<float>(bd.tb);
            WriteBuildingLook(bd, tb, BuildingPhaseAt(bd.tl, tb));
        }
    _pfDirty = true;
}

void FireWorld::RegisterPlantCell(int ci, int cj, float cellM)
{
    if (!GLandscape)
        return;
    const std::uint64_t ck =
        (static_cast<std::uint64_t>(static_cast<std::uint32_t>(ci)) << 32) | static_cast<std::uint32_t>(cj);
    const auto ins = _pfCells.emplace(ck, std::vector<int>());
    if (!ins.second)
        return;
    std::vector<int>& list = ins.first->second;
    // (end-state agent) room held back for the buildings with no slots yet
    const int reserve = BuildingSlotReserve();
    const float x0 = static_cast<float>(ci) * cellM, z0 = static_cast<float>(cj) * cellM;
    const int gx0 = static_cast<int>(std::floor(x0 * InvLandGrid)),
              gx1 = static_cast<int>(std::floor((x0 + cellM) * InvLandGrid));
    const int gz0 = static_cast<int>(std::floor(z0 * InvLandGrid)),
              gz1 = static_cast<int>(std::floor((z0 + cellM) * InvLandGrid));
    for (int cz = gz0; cz <= gz1; ++cz)
        for (int cx = gx0; cx <= gx1; ++cx)
        {
            if (!InRange(cz, cx))
                continue;
            const ObjectList& objs = GLandscape->GetObjects(cz, cx);
            for (int i = 0; i < objs.Size(); ++i)
            {
                Object* obj = objs[i];
                if (!obj || !obj->GetShape())
                    continue;
                const LODShape* shape = obj->GetShape();
                const MapType mt = shape->GetMapType();
                if (mt != MapTree && mt != MapSmallTree && mt != MapBush)
                    continue;
                const Vector3 pos = obj->Position();
                if (pos.X() < x0 || pos.X() >= x0 + cellM || pos.Z() < z0 || pos.Z() >= z0 + cellM)
                    continue;
                const std::uint64_t key = PlantKey(pos.X(), pos.Z());
                if (_pfByKey.count(key) || _pfTable.Find(pos.X(), pos.Z()) >= 0)
                    continue;
                PlantFire pf;
                Plant& p = pf.p;
                p.x = pos.X();
                p.z = pos.Z();
                p.y = GLandscape->SurfaceY(p.x, p.z);
                p.h = (shape->Max().Y() - shape->Min().Y()) * obj->Scale();
                p.r = 0.25f * ((shape->Max().X() - shape->Min().X()) + (shape->Max().Z() - shape->Min().Z())) *
                      obj->Scale();
                p.name = (const char*)shape->GetName();
                if (mt == MapBush)
                {
                    // Its class by its shape's name, as FuelAt reads it (the
                    // consult's species answer for Abel).
                    std::string n = p.name;
                    std::transform(n.begin(), n.end(), n.begin(),
                                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                    p.cls = n.find("rakosi") != std::string::npos    ? PlantClass::Reed
                            : n.find("listnac") != std::string::npos ? PlantClass::Leafy
                                                                     : PlantClass::Thicket;
                    pf.kind = p.cls == PlantClass::Reed    ? PlantKind::Reed
                              : p.cls == PlantClass::Leafy ? PlantKind::Leafy
                                                           : PlantKind::Thicket;
                }
                else
                {
                    const TreeKind tk = KindOf(shape, false);
                    if (tk == TreeKind::Olive)
                    {
                        p.cls = PlantClass::Olive;
                        pf.kind = PlantKind::Olive;
                    }
                    else
                        pf.kind = tk == TreeKind::Broadleaf ? PlantKind::Broadleaf : PlantKind::Pine;
                }
                if (!InsertPlant(pf, reserve))
                    continue;
                const int idx = static_cast<int>(_pf.size());
                _pf.push_back(std::move(pf));
                WritePlant(_pf.back());
                _pfByKey.emplace(key, idx);
                list.push_back(idx);
            }
        }
}

void FireWorld::OfferPlant(int idx, const BurningView& v, double caught, float cellM, float wdx, float wdz,
                           float wind10)
{
    PlantFire& pf = _pf[idx];
    if (pf.contact >= 0.0)
        return;
    const Plant& p = pf.p;
    // How near the drawn flames must come: a bush's, a reed clump's or an
    // olive's crown edge (its crown hangs low), a tree's ground under its crown.
    const bool tree = pf.kind == PlantKind::Pine || pf.kind == PlantKind::Broadleaf;
    const float rc = std::max(0.2f, tree ? std::min(kPlantTreeReachShare * p.r, kPlantTreeReachMaxM) : p.r);
    float fx = v.frontX, fz = v.frontZ;
    if (v.crossSec <= 0.0f || fx * fx + fz * fz < 0.25f)
    {
        fx = wdx;
        fz = wdz;
    }
    const float half = 0.5f * cellM;
    // The cell's square must come within that reach of the plant ...
    const float nx = std::clamp(p.x, v.x - half, v.x + half), nz = std::clamp(p.z, v.z - half, v.z + half);
    if ((nx - p.x) * (nx - p.x) + (nz - p.z) * (nz - p.z) > rc * rc)
        return;
    // ... and its front reaches first the point of it on the side it comes from.
    float qx = std::clamp(p.x - rc * fx, v.x - half, v.x + half);
    float qz = std::clamp(p.z - rc * fz, v.z - half, v.z + half);
    if ((qx - p.x) * (qx - p.x) + (qz - p.z) * (qz - p.z) > rc * rc * 1.0001f)
    {
        qx = nx;
        qz = nz;
    }
    const double t = caught + static_cast<double>(DrawnReachSec(v, qx, qz, cellM));
    if (t < pf.candidate)
    {
        pf.candidate = t;
        pf.kwM = v.intensityKwM;
        pf.flameM = v.flameMaxM;
        pf.dirX = fx;
        pf.dirZ = fz;
        pf.midflame = (v.sheltered ? 0.15f : Model(v.model).waf) * wind10;
    }
    if (!pf.scheduled)
    {
        pf.scheduled = true;
        _pfSched.push_back(idx);
    }
}

void FireWorld::ReachPlant(PlantFire& pf, float wdx, float wdz, const Vector3& cam)
{
    // A reach found late (its cell seen after the moment) starts now, so no
    // flare or scorch ever jumps part way in.
    const double late = _clock - pf.candidate;
    pf.contact = late > 0.5 ? _clock : pf.candidate;
    _pfContacts++;
    const Plant& p = pf.p;
    if (pf.kind == PlantKind::Thicket || pf.kind == PlantKind::Leafy || pf.kind == PlantKind::Reed ||
        pf.kind == PlantKind::Olive)
    {
        // Its flare (a bush, a reed clump or an olive), if the flames that
        // reached it are strong enough for its class; it starts as they touch it.
        const bool flares = p.cls == PlantClass::Olive   ? pf.kwM >= kOliveMinKwM
                            : p.cls == PlantClass::Leafy ? pf.flameM >= kLeafyMinShare * p.h
                                                         : pf.flameM >= kFlareMinFlameM;
        if (flares)
        {
            const std::uint64_t pk = PlantKey(p.x, p.z);
            Flare fl;
            fl.p = p;
            fl.start = pf.contact;
            fl.midflame = pf.midflame;
            fl.dirX = wdx;
            fl.dirZ = wdz;
            fl.key = static_cast<std::uint32_t>(pk ^ (pk >> 29)) * 2654435761U;
            _flares.push_back(std::move(fl));
            _stats.flaresTotal++;
            pf.flared = true;
            _pfFlared++;
        }
    }
    pf.look = PlantFireLook(pf);
    WritePlant(pf);
    const float dx = p.x - cam.X(), dz = p.z - cam.Z();
    if (dx * dx + dz * dz < kPlantLogM * kPlantLogM)
        LOG_INFO(Script,
                 "UDFIRE;event=plantfire;shape={};kind={};h={:.2f};r={:.2f};x={:.1f};z={:.1f};contact={:.2f};"
                 "late={:.2f};kwm={:.0f};flame={:.2f};flare={};torch={};scorch={:.2f};barkW={:.2f};barkL={:.2f};"
                 "gone={:.2f};sweep={:.1f}-{:.1f}",
                 p.name.c_str(), static_cast<int>(pf.kind), p.h, p.r, p.x, p.z, pf.contact, late, pf.kwM, pf.flameM,
                 pf.flared ? 1 : 0, pf.torched ? 1 : 0, pf.look.scorchTopM, pf.look.barkWindM, pf.look.barkLeeM,
                 pf.look.goneShare, pf.look.burn0Sec, pf.look.burn1Sec);
}

void FireWorld::UpdatePlants(const std::vector<BurningView>& views, float wdx, float wdz, float wind10,
                             const Vector3& cam)
{
    const float cellM = _grid->GetParams().cellM;
    auto cellKeyOf = [](int ci, int cj)
    { return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(ci)) << 32) | static_cast<std::uint32_t>(cj); };
    // A cell seen burning for the first time: the plants in it and round it
    // are known from now on, and the moment its drawn flames reach each one.
    for (const BurningView& v : views)
    {
        if (v.model == 0 || !_pfSeen.insert(PlantCellKey(v.x, v.z, cellM)).second)
            continue;
        const int ci = static_cast<int>(std::floor(v.x / cellM)), cj = static_cast<int>(std::floor(v.z / cellM));
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
                RegisterPlantCell(ci + di, cj + dj, cellM);
        const double caught = _clock - v.age;
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
            {
                const auto it = _pfCells.find(cellKeyOf(ci + di, cj + dj));
                if (it == _pfCells.end())
                    continue;
                for (const int idx : it->second)
                    OfferPlant(idx, v, caught, cellM, wdx, wdz, wind10);
            }
    }
    // A torching crown: its tree (the cell's, as the fire model chose it).
    for (const BurningView& v : views)
    {
        if (!v.torching || v.treeH <= 0.0f)
            continue;
        const float tx = v.x + v.treeOX, tz = v.z + v.treeOZ;
        const auto it = _pfCells.find(PlantCellKey(tx, tz, cellM));
        if (it == _pfCells.end())
            continue;
        int best = -1;
        float bestD2 = 1.0f;
        for (const int idx : it->second)
        {
            const PlantFire& o = _pf[idx];
            const float d2 = (o.p.x - tx) * (o.p.x - tx) + (o.p.z - tz) * (o.p.z - tz);
            if (o.kind == PlantKind::Pine && d2 < bestD2)
            {
                bestD2 = d2;
                best = idx;
            }
        }
        if (best < 0 || _pf[best].torched)
            continue;
        PlantFire& pf = _pf[best];
        pf.torched = true;
        pf.torchFront = _clock - v.torchAge;
        _pfTorched++;
        if (pf.contact >= 0.0)
        {
            pf.look = PlantFireLook(pf);
            WritePlant(pf);
        }
        const float dx = pf.p.x - cam.X(), dz = pf.p.z - cam.Z();
        if (dx * dx + dz * dz < kPlantLogM * kPlantLogM)
            LOG_INFO(Script, "UDFIRE;event=planttorch;shape={};h={:.2f};x={:.1f};z={:.1f};front={:.2f};contact={:.2f}",
                     pf.p.name.c_str(), pf.p.h, pf.p.x, pf.p.z, pf.torchFront, pf.contact);
    }
    // The plants the drawn flames reach now.
    for (std::size_t n = 0; n < _pfSched.size();)
    {
        PlantFire& pf = _pf[_pfSched[n]];
        if (_clock < pf.candidate)
        {
            ++n;
            continue;
        }
        ReachPlant(pf, wdx, wdz, cam);
        _pfSched[n] = _pfSched.back();
        _pfSched.pop_back();
    }
    // Looks long settled are final (the shader's clock wraps).
    if (_clock - _pfFinalAt >= 2.0)
    {
        _pfFinalAt = _clock;
        for (PlantFire& pf : _pf)
            if (pf.contact >= 0.0 && !pf.final && _clock - pf.contact > kPlantFinalSec)
            {
                pf.final = true;
                WritePlant(pf);
            }
    }
    if (_pfDirty && _clock - _pfBumpAt >= 1.0 / kPlantTableHz)
    {
        ++_pfVersion;
        _pfDirty = false;
        _pfBumpAt = _clock;
    }
}

bool FireWorld::PlantLeavesGone(const Object* obj, bool& found) const
{
    found = false;
    const Vector3 p = obj->Position();
    const auto it = _pfByKey.find(PlantKey(p.X(), p.Z()));
    if (it == _pfByKey.end())
        return false;
    found = true;
    const PlantFire& pf = _pf[it->second];
    const PlantLook& k = pf.look;
    if (pf.contact < 0.0 || k.goneShare < 0.7f || k.burn1Sec <= k.burn0Sec)
        return false;
    return _clock - pf.contact >= k.burn0Sec + 0.8f * (k.burn1Sec - k.burn0Sec);
}

// ---- Buildings (buildings builder; the wildfire consult, 2026-11-18, tenth answer) ----
namespace
{
// A building's pass is looked for from the cells seen burning within this of
// its footprint (its apron and a cell's run on), and drawn again only this
// long after the last (another fire passing it).
constexpr float kBuildingPassReachM = kBuildingApronM + 4.0f;
constexpr double kBuildingRepassSec = 600.0;
// A shape whose footprint is under this (m2) is no building (a sign, a post).
constexpr float kBuildingMinAreaM2 = 1.5f;
// Its wall tops as a share of its height (a pitched roof above them); a
// church's nave under its tower lower.
constexpr float kBuildingWallShare = 0.6f, kChurchWallShare = 0.35f, kChurchRidgeShare = 0.55f;
// A bush or a conifer this near its footprint stands against it.
constexpr float kBuildingVegM = 2.5f;
// Its windows: one per kWindowSpacingM of wall (at least one on a long side),
// the ground floor's opening kWindowSillM to kWindowTopM up, an upper
// floor's kWindowUpperM higher where its walls stand over kWindowUpperFromM;
// at most kWindowsMax.
constexpr float kWindowSpacingM = 3.5f, kWindowSillM = 1.0f, kWindowTopM = 2.1f, kWindowUpperM = 2.9f;
constexpr float kWindowUpperFromM = 5.0f;
constexpr int kWindowsMax = 12;
// Its smoke per MW of its peak (PlumeSource units: furnishings, paint, tar
// and timber burn dense and black; a grass cell's 0.001 a kW), and its
// smoke's share once burnt down (a shell smoking thinly; a barn's hay white
// and thicker).
constexpr float kBuildingSmokePerMW = 3.0f;
constexpr float kShellSmoke = 0.06f, kHaySmoke = 0.25f, kHeapSmoke = 0.03f;
// Its heat lights the cured grass round it each point in this mean time (s)
// at its peak (the shared draws; the wrecks builder's heatMeanSec).
constexpr float kBuildingHeatMeanSec = 60.0f;
// Its burnt look is written to the plant table at most this often (s).
constexpr double kBuildingLookSec = 2.0;
// A deterministic draw (0..1): the building's seed, the draw's number and
// the run's seed (s_bldRunSeed: new each game, or fireSpreadSet "bseed"), so
// a run repeats with one run seed.
float BuildingDraw(std::uint32_t seed, int k)
{
    return BuildingDrawFor(seed, k, s_bldRunSeed);
}
// A stone house's, church's or masonry shed's roof falls in over this from
// its collapse (WriteBuildingLook: the shader cuts it in ragged holes); its
// burnt shell's stage 1 is worn from kShellDoneRoof of the fall on (the cut
// has taken every roof face by 0.875: burnBuilding's roof * 1.2 - 0.05).
constexpr float kRoofFallSec = 90.0f;
constexpr float kShellDoneRoof = 0.9f;

float Ss(float e0, float e1, float x)
{
    const float k = std::clamp((x - e0) / std::max(e1 - e0, 1e-3f), 0.0f, 1.0f);
    return k * k * (3.0f - 2.0f * k);
}
} // namespace

BuildingKind FireWorld::BuildingKindOfShape(const LODShape* shape, MapType mt)
{
    const auto it = _bldKind.find(shape);
    if (it != _bldKind.end())
        return it->second;
    std::string n = (const char*)shape->GetName();
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const int m = mt == MapChurch                        ? 1
                  : mt == MapChapel                      ? 2
                  : mt == MapBunker || mt == MapFortress ? 3
                  : mt == MapFuelstation                 ? 4
                  : mt == MapHospital                    ? 5
                  : mt == MapFence                       ? 6
                                                         : 0;
    // (lower than kBuildingMinHeightM: no building, an obstacle course's
    // timber or a low wall; it never burns as one)
    const BuildingKind k = m != 6 && shape->Max().Y() - shape->Min().Y() < kBuildingMinHeightM
                               ? BuildingKind::Never
                               : BuildingKindOf(m, n.c_str());
    _bldKind.emplace(shape, k);
    LOG_INFO(Script, "UDFIRE;event=buildingkind;shape={};map={};kind={};w={:.1f};l={:.1f};h={:.1f}", n.c_str(),
             static_cast<int>(mt), static_cast<int>(k), shape->Max().X() - shape->Min().X(),
             shape->Max().Z() - shape->Min().Z(), shape->Max().Y() - shape->Min().Y());
    return k;
}

int FireWorld::RegisterBuilding(Object* obj, const LODShape* shape, MapType mt)
{
    const auto it = _bldByObj.find(obj);
    if (it != _bldByObj.end())
        return it->second;
    const Vector3 lo = shape->Min(), hi = shape->Max();
    const float sc = obj->Scale();
    Building b;
    b.obj = obj;
    b.name = (const char*)shape->GetName();
    b.kind = BuildingKindOfShape(shape, mt);
    b.hx = 0.5f * (hi.X() - lo.X()) * sc;
    b.hz = 0.5f * (hi.Z() - lo.Z()) * sc;
    if ((b.kind == BuildingKind::Fence ? 4.0f * std::max(b.hx, b.hz) * 0.25f : 4.0f * b.hx * b.hz) <
            kBuildingMinAreaM2 ||
        (b.kind == BuildingKind::Never && mt == MapFence) || !GLandscape)
    {
        _bldByObj.emplace(obj, -1);
        return -1;
    }
    const Vector3 mid = obj->PositionModelToWorld((lo + hi) * 0.5f);
    b.cx = mid.X();
    b.cz = mid.Z();
    const Vector3 xa = obj->DirectionAside(), za = obj->Direction();
    const float xl = std::sqrt(xa.X() * xa.X() + xa.Z() * xa.Z()), zl = std::sqrt(za.X() * za.X() + za.Z() * za.Z());
    b.ax = xl > 1e-4f ? xa.X() / xl : 1.0f;
    b.az = xl > 1e-4f ? xa.Z() / xl : 0.0f;
    b.bx = zl > 1e-4f ? za.X() / zl : -b.az;
    b.bz = zl > 1e-4f ? za.Z() / zl : b.ax;
    b.y = GLandscape->SurfaceY(b.cx, b.cz);
    const float top = mid.Y() + 0.5f * (hi.Y() - lo.Y()) * sc;
    b.h = std::clamp(top - b.y, 1.5f, 60.0f);
    b.wallM = (b.kind == BuildingKind::Church ? kChurchWallShare : kBuildingWallShare) * b.h;
    // (end-state agent) a house's own eave line (FireShell.hpp ShellWallTopOf:
    // the lowest edge of its roof faces), where its shell is cut and its roof
    // drops in the burnt look; a church keeps its share (its tower)
    if (b.kind != BuildingKind::Church && b.kind != BuildingKind::Fence && b.kind != BuildingKind::Never)
    {
        const float eave =
            obj->PositionModelToWorld(Vector3(0.0f, ShellWallTopOf(ShellSourceOf(shape)), 0.0f)).Y() - b.y;
        if (eave > 1.5f && eave < b.h)
            b.wallM = eave;
    }
    b.seed = static_cast<std::uint32_t>(static_cast<int>(std::floor(b.cx * 4.0f))) * 73856093U ^
             static_cast<std::uint32_t>(static_cast<int>(std::floor(b.cz * 4.0f))) * 19349663U;
    // A bush or a conifer against it (a hedge, a pine at its wall).
    const int gx = static_cast<int>(std::floor(b.cx * InvLandGrid)),
              gz = static_cast<int>(std::floor(b.cz * InvLandGrid));
    for (int cz = gz - 1; cz <= gz + 1 && !b.veg; ++cz)
        for (int cx = gx - 1; cx <= gx + 1 && !b.veg; ++cx)
        {
            if (!InRange(cz, cx))
                continue;
            const ObjectList& list = GLandscape->GetObjects(cz, cx);
            for (int i = 0; i < list.Size() && !b.veg; ++i)
            {
                const Object* o = list[i];
                if (!o || !o->GetShape())
                    continue;
                const MapType om = o->GetShape()->GetMapType();
                const bool plant = om == MapBush || ((om == MapTree || om == MapSmallTree) &&
                                                     KindOf(o->GetShape(), false) != TreeKind::Broadleaf &&
                                                     KindOf(o->GetShape(), false) != TreeKind::Olive);
                if (plant && b.Gap(o->Position().X(), o->Position().Z()) < kBuildingVegM)
                    b.veg = true;
            }
        }
    const int idx = static_cast<int>(_bld.size());
    _bld.push_back(b);
    _bldByObj.emplace(obj, idx);
    // The 8 m cells its footprint and apron touch.
    constexpr float kCellM = 8.0f;
    const float r = std::sqrt(b.hx * b.hx + b.hz * b.hz) + kBuildingApronM + kCellM;
    for (int j = static_cast<int>(std::floor((b.cz - r) / kCellM));
         j <= static_cast<int>(std::floor((b.cz + r) / kCellM)); ++j)
        for (int i = static_cast<int>(std::floor((b.cx - r) / kCellM));
             i <= static_cast<int>(std::floor((b.cx + r) / kCellM)); ++i)
        {
            const float qx = (static_cast<float>(i) + 0.5f) * kCellM, qz = (static_cast<float>(j) + 0.5f) * kCellM;
            // (its own cells and those whose ground may run on into them)
            if (b.Gap(qx, qz) > kBuildingApronM + 0.7072f * kCellM + kCellM)
                continue;
            const std::uint64_t key =
                (static_cast<std::uint64_t>(static_cast<std::uint32_t>(i)) << 32) | static_cast<std::uint32_t>(j);
            _bldCells[key].push_back(idx);
        }
    return idx;
}

int FireWorld::BuildingAt(float x, float z, float grow) const
{
    const auto it = _bldCells.find(PlantCellKey(x, z, 8.0f));
    if (it == _bldCells.end())
        return -1;
    for (const int idx : it->second)
        if (_bld[static_cast<std::size_t>(idx)].kind != BuildingKind::Fence &&
            _bld[static_cast<std::size_t>(idx)].Inside(x, z, grow))
            return idx;
    return -1;
}

bool FireWorld::BuildingsNear(float x, float z) const
{
    // (a building is listed in the cells round it out to a cell's run on)
    return _bldCells.count(PlantCellKey(x, z, 8.0f)) != 0;
}

void FireWorld::BuildingHeat(float x, float z, float radiusM)
{
    // a fireball: under it a shed or a barn lights, a house through its
    // windows and eaves (a tanker's 0.7, a car's 0.25); a big one's heat out
    // to 2 radii half that
    Ensure();
    BuildingsScan(x, z, 2.0f * radiusM + 30.0f);
    const bool big = radiusM > kFireballBigM;
    for (int k = 0; k < static_cast<int>(_bld.size()); ++k)
    {
        Building& b = _bld[static_cast<std::size_t>(k)];
        if (b.kind == BuildingKind::Never || b.catchAt >= 0.0)
            continue;
        const float gap = b.Gap(x, z);
        if (gap > (big ? 2.0f : 1.0f) * radiusM)
            continue;
        const bool wood = b.kind == BuildingKind::Shed || b.kind == BuildingKind::Barn;
        float p = wood ? 1.0f : (big ? 0.7f : 0.25f) * (b.kind == BuildingKind::Masonry ? 0.5f : 1.0f);
        if (gap > radiusM)
            p *= 0.5f;
        b.passAt = _clock;
        PassBuilding(k, "fireball", p);
    }
}

float FireWorld::DayNow() const
{
    float wx = 0.0f, wz = 0.0f;
    BaseWind(wx, wz);
    return FireDayIndex(PigNow(), std::sqrt(wx * wx + wz * wz));
}

void FireWorld::PassBuilding(int idx, const char* why, float chance)
{
    Building& b = _bld[static_cast<std::size_t>(idx)];
    b.scheduled = false;
    b.lastPass = _clock;
    b.passes++;
    _bldPassed++;
    float wx = 0.0f, wz = 0.0f;
    BaseWind(wx, wz);
    const float pig = PigNow(), wind10 = std::sqrt(wx * wx + wz * wz);
    const float p = chance >= 0.0f ? chance : BuildingPassChance(b.kind, b.veg, pig, wind10);
    const float u = BuildingDraw(b.seed, 100 + b.passes);
    const bool caught = u < p && b.catchAt < 0.0 && !(b.obj && b.obj->IsDammageDestroyed());
    LOG_INFO(Script,
             "UDFIRE;event=buildingpass;shape={};kind={};x={:.1f};z={:.1f};why={};veg={};pig={:.2f};wind={:.1f};"
             "day={:.2f};chance={:.3f};u={:.3f};caught={};kwm={:.0f};flame={:.2f};t={:.1f}",
             b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, why, b.veg ? 1 : 0, pig, wind10,
             FireDayIndex(pig, wind10), p, u, caught ? 1 : 0, b.passKwM, b.passFlameM, _clock);
    if (!caught)
        return;
    // Its first flame inside after its delay (on its own clock: the test aid
    // POSEIDON_XP_FIRESPREAD_OBJTIME runs it faster), its fire starting in
    // the roof for kRoofFirstShare of the ember starts (embers under the
    // tiles) and for every one lit by a neighbour's heat on its eaves.
    constexpr float kRoofFirstShare = 0.4f;
    // (a neighbour's heat on the eaves shows within minutes)
    const bool heat = std::strcmp(why, "heat") == 0;
    const float du = BuildingDraw(b.seed, 200 + b.passes);
    const float delay = heat && b.kind != BuildingKind::Shed && b.kind != BuildingKind::Barn
                            ? 120.0f + 480.0f * du
                            : BuildingDelaySec(b.kind, du);
    const bool roofFirst = heat || BuildingDraw(b.seed, 300 + b.passes) < kRoofFirstShare;
    b.ember = false;
    // (a pass found late: its delay runs from the pass)
    const double late = std::clamp(_clock - b.passAt, 0.0, 1.0) * static_cast<double>(_objTime);
    CatchBuilding(idx, static_cast<double>(delay) - late, roofFirst, why);
}

void FireWorld::CatchBuilding(int idx, double delaySec, bool roofFirst, const char* why)
{
    Building& b = _bld[static_cast<std::size_t>(idx)];
    if (b.catchAt >= 0.0 || b.kind == BuildingKind::Never)
        return;
    float u[kBuildingTimelineDraws];
    for (int k = 0; k < kBuildingTimelineDraws; ++k)
        u[k] = BuildingDraw(b.seed, 400 + k);
    b.catchAt = _clock;
    b.tb = -std::max(delaySec, 0.0);
    b.tl = BuildingTimelineOf(b.kind, roofFirst, u);
    b.live = true;
    _bldLive.push_back(idx);
    _bldCaught++;
    LOG_INFO(Script,
             "UDFIRE;event=buildingcatch;shape={};kind={};x={:.1f};z={:.1f};why={};roofFirst={};delay={:.1f};"
             "now={:.1f};objtime={:.2f};flash={:.0f};spread={:.0f};collapse={:.0f};floor={:.0f};down={:.0f};"
             "peak={:.1f};window={:.1f};roof={:.1f};mw={:.0f}",
             b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, why, b.tl.roofFirst ? 1 : 0, -b.tb, _clock, _objTime,
             b.tl.flashSec, b.tl.spreadSec, b.tl.collapseSec, b.tl.floorSec, b.tl.downSec, b.tl.peakM, b.tl.windowM,
             b.tl.roofM, b.tl.peakMW);
}

void FireWorld::BuildingReach(int idx)
{
    // At its peak its heat lights wood round it (the next shed, barn, fence
    // or house's eaves: BuildingHeatChance by the gap); the cured grass round
    // it is its parts' heatReachM (the shared draws).
    // (a copy: an ignition below may register a building and move _bld)
    const Building b = _bld[static_cast<std::size_t>(idx)];
    const bool small = b.kind == BuildingKind::Shed;
    const float grassM = small ? kHeatGrassShedM : kHeatGrassHouseM;
    const float reach = std::max(kHeatWoodM, grassM) + std::sqrt(b.hx * b.hx + b.hz * b.hz);
    const float day = DayNow();
    std::vector<int> near;
    for (int k = 0; k < static_cast<int>(_bld.size()); ++k)
    {
        if (k == idx)
            continue;
        const Building& o = _bld[static_cast<std::size_t>(k)];
        const float dx = o.cx - b.cx, dz = o.cz - b.cz;
        if (dx * dx + dz * dz > (reach + 30.0f) * (reach + 30.0f) || o.catchAt >= 0.0 || o.kind == BuildingKind::Never)
            continue;
        // a shed's heat reaches half as far
        const float gap = b.GapTo(o) * (b.kind == BuildingKind::Fence ? 4.0f : (small ? 2.0f : 1.0f));
        const float p = BuildingHeatChance(o.kind, gap, day);
        if (p <= 0.0f)
            continue;
        Building& w = _bld[static_cast<std::size_t>(k)];
        w.passAt = _clock;
        PassBuilding(k, "heat", p);
    }
}

void FireWorld::BuildingEmbers(int idx, float wind10, float wdx, float wdz)
{
    // The roof falls in: 1 to 3 embers on the typical day (median 15 m, to
    // 80 m), 3 to 10 on the worst (median 40 m, to 300 m), downwind within
    // about 30 degrees, a shed half; each a spot fire with the ember chance,
    // or a pass for a building it lands on.
    // (a copy: an ignition below may register a building and move _bld)
    const Building b = _bld[static_cast<std::size_t>(idx)];
    const float s = DayNow();
    float n = DayBlend(kCollapseEmbersTypical, kCollapseEmbersWorst, s);
    if (b.kind == BuildingKind::Shed)
        n *= 0.5f;
    if (b.kind == BuildingKind::Fence)
        n = 0.0f;
    const float median = DayBlend(kCollapseEmberMedianTypicalM, kCollapseEmberMedianWorstM, s);
    const float maxM = DayBlend(kCollapseEmberMaxTypicalM, kCollapseEmberMaxWorstM, s);
    const int count = static_cast<int>(std::floor(n + BuildingDraw(b.seed, 600)));
    const float pig = PigNow();
    for (int k = 0; k < count; ++k)
    {
        const float u1 = std::max(BuildingDraw(b.seed, 610 + 3 * k), 1e-4f), u2 = BuildingDraw(b.seed, 611 + 3 * k);
        const float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(6.2831853f * u2);
        const float d = std::min(maxM, median * std::exp(kSpotLogSigma * z));
        const float turn = 0.52f * (2.0f * BuildingDraw(b.seed, 612 + 3 * k) - 1.0f);
        const float c = std::cos(turn), sn = std::sin(turn);
        const float dx = wind10 > 0.5f ? wdx * c - wdz * sn : std::cos(6.2831853f * u2),
                    dz = wind10 > 0.5f ? wdx * sn + wdz * c : std::sin(6.2831853f * u2);
        const float x = b.cx + dx * (d + 0.5f * (b.hx + b.hz)), zz = b.cz + dz * (d + 0.5f * (b.hx + b.hz));
        _bldEmbersOut++;
        const int hit = BuildingAt(x, zz, kBuildingApronM);
        bool lit = false;
        if (hit >= 0 && hit != idx)
        {
            Building& o = _bld[static_cast<std::size_t>(hit)];
            o.passAt = _clock;
            o.ember = true;
            PassBuilding(hit, "ember", -1.0f);
        }
        else
            lit = ObjectLight(x, zz, pig, 0.0f, ObjectIgniteKind::Ember,
                              (1ULL << 63) | static_cast<std::uint64_t>(static_cast<std::uint32_t>(idx)));
        LOG_INFO(Script, "UDFIRE;event=buildingember;from={};x={:.1f};z={:.1f};d={:.1f};onBuilding={};lit={}",
                 b.name.c_str(), x, zz, d, hit >= 0 && hit != idx ? 1 : 0, lit ? 1 : 0);
    }
}

void FireWorld::BuildingFires(const Building& b, float tb, const BuildingPhase& ph, float wind10, float wdx, float wdz)
{
    // Its object fires (the shared interface): its windows, the flames above
    // its walls, its whole body (a shed, a barn, a fuel station), the low
    // flames inside its shell, and its smoke.
    // (bit 63 set: a building's, never a wreck's, the wrecks builder's ids)
    const std::uint64_t base =
        (1ULL << 63) | ((static_cast<std::uint64_t>(static_cast<std::uint32_t>(&b - _bld.data())) + 1ULL) << 8);
    const float g = b.y;
    auto put = [&](std::uint64_t part, float x, float y, float z, float ux, float uz, float halfL, float halfW,
                   float flameM, float refM, float heat, float mw)
    {
        ObjectFire o;
        o.id = base | part;
        o.x = x;
        o.y = y;
        o.z = z;
        o.ux = ux;
        o.uz = uz;
        o.halfL = halfL;
        o.halfW = halfW;
        o.flameM = flameM;
        o.flameRefM = std::max(refM, 0.3f);
        o.heat = std::clamp(heat, 0.0f, 1.0f);
        o.heatMW = mw;
        o.owner = kObjectOwnerBuilding;
        o.groundY = g;
        o.absorb = 6.0f;
        o.soot = 1.0f;
        o.black = ph.black;
        // people in or beside its flames burn (the grass fire's rate)
        o.hurtM = 1.0f;
        _objectFires.push_back(o);
        return &_objectFires.back();
    };
    // Its windows: on its four walls, the long sides more, the first to flash over the one with share 0.
    if (ph.windows > 0.0f && b.tl.windowM > 0.0f)
    {
        const int floors = b.wallM > kWindowUpperFromM ? 2 : 1;
        int k = 0;
        float minShare = 1.0f;
        int first = 0;
        struct Win
        {
            float x, y, z, nx, nz, share, ux, uz;
        };
        Win wins[kWindowsMax];
        for (int side = 0; side < 4 && k < kWindowsMax; ++side)
        {
            const bool alongX = side < 2; // a face along its x axis (normal along z)
            const float len = 2.0f * (alongX ? b.hx : b.hz);
            const int n = std::clamp(static_cast<int>(std::lround(len / kWindowSpacingM)), alongX ? 1 : 0, 6);
            const float sgn = (side & 1) ? -1.0f : 1.0f;
            const float nx = (alongX ? b.bx : b.ax) * sgn, nz = (alongX ? b.bz : b.az) * sgn;
            const float ux = alongX ? b.ax : b.bx, uz = alongX ? b.az : b.bz;
            const float off = (alongX ? b.hz : b.hx) + 0.08f;
            for (int fl = 0; fl < floors; ++fl)
                for (int i = 0; i < n && k < kWindowsMax; ++i)
                {
                    const float along = (static_cast<float>(i) + 0.5f) / static_cast<float>(n) - 0.5f +
                                        0.15f * (BuildingDraw(b.seed, 700 + k) - 0.5f) / static_cast<float>(n);
                    Win& w = wins[k];
                    w.x = b.cx + nx * off + ux * along * len;
                    w.z = b.cz + nz * off + uz * along * len;
                    // (a Face part: its base the opening's middle, halfW its half height:
                    // the shared draw stands its flames' feet from the opening's head up)
                    w.y = g + 0.5f * (kWindowSillM + kWindowTopM) + kWindowUpperM * static_cast<float>(fl);
                    w.nx = nx;
                    w.nz = nz;
                    w.ux = ux;
                    w.uz = uz;
                    // the upper floor later (the fire climbs)
                    w.share = std::min(1.0f, BuildingDraw(b.seed, 720 + k) * (fl ? 1.0f : 0.8f) + 0.2f * fl);
                    if (w.share < minShare)
                    {
                        minShare = w.share;
                        first = k;
                    }
                    ++k;
                }
        }
        for (int i = 0; i < k; ++i)
        {
            const Win& w = wins[i];
            const float share = i == first ? 0.0f : w.share;
            const float L = BuildingWindowFlameM(b.tl, share, tb) * (0.8f + 0.4f * BuildingDraw(b.seed, 740 + i));
            if (L < 0.05f)
                continue;
            ObjectFire* o =
                put(0x10 + static_cast<std::uint64_t>(i), w.x, w.y, w.z, w.ux, w.uz, 0.5f,
                    0.5f * (kWindowTopM - kWindowSillM), L, b.tl.windowM, L / std::max(b.tl.windowM, 0.1f), 1.0f);
            o->shape = ObjectFireShape::Face;
            o->faceX = w.nx;
            o->faceZ = w.nz;
        }
    }
    const float inset = 0.85f;
    // Its heat on the cured grass round it (the shared draws light it, each
    // point in heatMeanSec): out to kHeatGrassHouseM from its walls as it
    // burns fully (a shed's kHeatGrassShedM), less before (the main part's).
    const float full = std::clamp((ph.heatMW / std::max(b.tl.peakMW, 0.1f) - 0.2f) / 0.6f, 0.0f, 1.0f);
    const float heatReach = (b.kind == BuildingKind::Shed ? kHeatGrassShedM : kHeatGrassHouseM) * full;
    auto heatOn = [&](ObjectFire* o)
    {
        if (heatReach <= 0.5f)
            return;
        o->heatReachM = heatReach;
        o->heatMeanSec = kBuildingHeatMeanSec / std::max(_objTime, 0.01f);
    };
    // Above the walls: through the roof (roof first: holes in it), then out of the roofless shell.
    if (ph.topM > 0.05f)
    {
        const bool church = b.kind == BuildingKind::Church;
        const bool throughRoof = !ph.collapsed;
        const float baseY = church        ? g + kChurchRidgeShare * b.h - 0.5f
                            : throughRoof ? g + b.wallM + 0.5f * (b.h - b.wallM)
                                          : g + b.wallM - 0.8f;
        const float k = throughRoof && !church ? 0.55f : inset;
        heatOn(put(0x01, b.cx, baseY, b.cz, b.ax, b.az, k * b.hx, k * b.hz, ph.topM, b.tl.peakM,
                   ph.topM / std::max(b.tl.peakM, 0.1f), ph.heatMW));
    }
    // A shed, a barn, a fuel station: its whole body.
    if (ph.bodyM > 0.05f && b.kind == BuildingKind::Fence)
    {
        // a fence: its flames along itself from its foot (its long axis)
        const bool alongX = b.hx >= b.hz;
        put(0x02, b.cx, g + 0.05f, b.cz, alongX ? b.ax : b.bx, alongX ? b.az : b.bz, std::max(b.hx, b.hz), 0.15f,
            ph.bodyM, b.tl.peakM, ph.bodyM / std::max(b.tl.peakM, 0.1f), ph.heatMW);
    }
    else if (ph.bodyM > 0.05f)
        heatOn(put(0x02, b.cx, g + 0.2f, b.cz, b.ax, b.az, inset * b.hx, inset * b.hz, ph.bodyM, b.tl.peakM,
                   ph.bodyM / std::max(b.tl.peakM, 0.1f), ph.heatMW));
    // Low inside its shell once the floor has fallen.
    if (ph.innerM > 0.05f)
        put(0x03, b.cx, g + 0.3f, b.cz, b.ax, b.az, 0.7f * b.hx, 0.7f * b.hz, ph.innerM, 2.5f, ph.innerM / 2.5f,
            0.1f * ph.heatMW);
    // Its smoke: from a window or the eaves, then between the tiles all along
    // the roof, then out of the shell; black once a room flashes over, grey
    // as it burns down, white from a barn's smouldering hay.
    if (ph.smoke > 0.0f)
    {
        const float smoke = ph.smoke * kBuildingSmokePerMW * std::max(b.tl.peakMW, 1.0f);
        const float y = ph.collapsed ? g + b.wallM : g + b.h;
        ObjectFire* o = put(0x04, b.cx, y, b.cz, b.ax, b.az, 0.8f * b.hx, 0.8f * b.hz, 0.0f, b.tl.peakM, 0.0f,
                            std::max(ph.heatMW, 0.3f));
        o->smoke = smoke;
        o->black = ph.black;
        o->glow = ph.glow;
        o->hurtM = 0.0f;
    }
    (void)wind10;
    (void)wdx;
    (void)wdz;
}

void FireWorld::WriteBuildingLook(Building& b, float tb, const BuildingPhase& ph)
{
    // Its burnt look (the object main's burnBuilding): in the plant table,
    // found by its origin as a plant is. Soot rising up its walls with its
    // fire and fans above its windows, the whole shell black once its roof is
    // in, its rubble's glow, a shed or barn burnt away to its lowest metre.
    if (b.kind == BuildingKind::Fence)
        return; // (a fence falls: the stock destruction is its end)
    if (b.slots[0] < 0)
    {
        // (end-state agent) a building always finds room: the table grows
        // or, at its largest, the plants make room (PlantTableAdd)
        const Vector3 p = b.obj ? b.obj->Position() : Vector3(b.cx, b.y, b.cz);
        if (_pfTable.slots == 0)
            _pfTable.Reset(_pfStartSlots);
        if (!_pfTable.Insert(p.X(), p.Z(), b.slots))
        {
            std::vector<PlantFootRef> blds, plants;
            for (Building& bd : _bld)
                if (&bd != &b && bd.slots[0] >= 0)
                {
                    const Vector3 q = bd.obj ? bd.obj->Position() : Vector3(bd.cx, bd.y, bd.cz);
                    blds.push_back({q.X(), q.Z(), bd.slots});
                }
            plants.reserve(_pf.size());
            for (PlantFire& o : _pf)
                plants.push_back({o.p.x, o.p.z, o.slots});
            bool grew = false;
            int dropped = 0;
            if (!PlantTableAdd(_pfTable, {p.X(), p.Z(), b.slots}, true, 0, _pfMaxSlots, blds, plants, grew, dropped))
                return;
            if (dropped > 0)
                LOG_WARN(Script, "UDFIRE;event=planttable;buildingroom;slots={};plantsdropped={}", _pfTable.slots,
                         dropped);
            if (grew)
            {
                // every other foot moved: their looks go in again (this
                // building's is written below)
                for (const PlantFire& o : _pf)
                    WritePlant(o);
                for (Building& bd : _bld)
                    if (&bd != &b && bd.slots[0] >= 0)
                    {
                        const float otb = static_cast<float>(bd.tb);
                        WriteBuildingLook(bd, otb, BuildingPhaseAt(bd.tl, otb));
                    }
            }
        }
    }
    const BuildingTimeline& tl = b.tl;
    float soot = 0.0f;
    if (tb > 0.0f)
        soot = tl.kind == BuildingKind::Shed || tl.kind == BuildingKind::Barn || tl.kind == BuildingKind::FuelStation
                   ? Ss(0.0f, std::max(tl.flashSec, 1.0f), tb)
                   : 0.15f + 0.45f * Ss(tl.flashSec, tl.collapseSec, tb) + 0.4f * Ss(tl.collapseSec, tl.floorSec, tb);
    const float charred = ph.collapsed ? Ss(tl.collapseSec, tl.downSec, tb) : 0.0f;
    const bool body = tl.kind == BuildingKind::Shed || tl.kind == BuildingKind::Barn;
    std::string lower = b.name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const bool standing = body && BuildingWallsStand(lower.c_str());
    const float gone = body && !standing && ph.collapsed ? Ss(tl.collapseSec, tl.collapseSec + 120.0f, tb) : 0.0f;
    // a stone house's, church's or masonry shed's roof falling in over
    // kRoofFallSec from its collapse (its faces above the wall tops dropped,
    // a ragged hole first); once its burnt shell's stage 1 is worn the
    // roof's faces are gone from the shape itself and nothing is cut
    const bool shell = tl.kind == BuildingKind::StoneHouse || tl.kind == BuildingKind::Church ||
                       tl.kind == BuildingKind::Masonry || standing;
    const float roof = shell && ph.collapsed && !(b.obj && ShellHidesRoof(b.obj->GetShape()))
                           ? Ss(tl.collapseSec, tl.collapseSec + kRoofFallSec, tb)
                           : 0.0f;
    const Vector3 p = b.obj ? b.obj->Position() : Vector3(b.cx, b.y, b.cz);
    for (int s : b.slots)
    {
        if (s < 0)
            continue;
        float* d = _pfTable.Slot(s);
        std::fill(d, d + kPlantSlotFloats, 0.0f);
        d[kPlantX] = p.X();
        d[kPlantZ] = p.Z();
        d[kPlantY] = b.y;
        d[kPlantH] = b.h;
        d[4] = kPlantLongAgo; // (no plant's contact: the plant paths never read a building's slot)
        d[5] = std::clamp(soot, 0.0f, 1.0f);
        d[6] = charred;
        d[7] = ph.glow;
        d[8] = ph.windows;
        d[9] = b.wallM;
        d[10] = gone;
        d[11] = b.ax;
        d[12] = b.az;
        d[13] = b.hx;
        d[14] = b.hz;
        d[15] = roof;
        // (end-state agent) 1: gutted, its shell (FireShell.hpp) for good;
        // the rubble's look in burnBuilding keys on it
        d[16] = b.obj && b.obj->IsGutted() ? 1.0f : 0.0f;
        d[kPlantSlotFloats - 1] = kBuildingMark;
    }
    _pfDirty = true;
    b.lookDone = soot;
}

void FireWorld::StageBuildingShell(Building& b, float tb)
{
    // (end-state agent: one clone, one swap) the shell is the gutted
    // object's (FireShell.hpp ShellWear: the geometry agent's piers, inner
    // walls and hidden roof, the collision cut and the rubble on one copy);
    // GutByFire put stage 0 on at the collapse, stage 1 goes on once the
    // shader's ragged cut has taken the roof
    if (!b.obj || !b.obj->IsGutted())
        return;
    const BuildingTimeline& tl = b.tl;
    const int cur = ShellStageOf(b.obj->GetShape());
    const int want = Ss(tl.collapseSec, tl.collapseSec + kRoofFallSec, tb) >= kShellDoneRoof ? 1 : 0;
    if (cur >= 0 && want > cur)
    {
        ShellWear(b.obj, want);
        _pfDirty = true;
        LOG_INFO(Script, "UDFIRE;event=buildingshellstage;shape={};stage={};x={:.1f};z={:.1f};tb={:.0f}",
                 b.name.c_str(), want, b.cx, b.cz, tb);
    }
    b.shellStage = ShellStageOf(b.obj->GetShape());
}

int FireWorld::ShellStageFor(const Object* obj) const
{
    const auto it = _bldByObj.find(obj);
    if (it == _bldByObj.end() || it->second < 0)
        return 1;
    const Building& b = _bld[static_cast<std::size_t>(it->second)];
    // its own fire here, its roof falling in: stage 0 until the cut is done
    if (!b.live || !b.destroyed)
        return 1;
    return Ss(b.tl.collapseSec, b.tl.collapseSec + kRoofFallSec, static_cast<float>(b.tb)) >= kShellDoneRoof ? 1 : 0;
}

void FireWorld::UpdateBuildings(float dt)
{
    const std::vector<BurningView>& views = _views;
    float wx = 0.0f, wz = 0.0f;
    Wind(wx, wz);
    const float wind10 = std::sqrt(wx * wx + wz * wz);
    const float wdx = wind10 > 0.01f ? wx / wind10 : 0.0f, wdz = wind10 > 0.01f ? wz / wind10 : 1.0f;
    _bldBurning = 0;
    _bldParts = 0;
    _bldSmoking = _bldBlack = 0;
    if (_bld.empty())
        return;
    const float cellM = _grid->GetParams().cellM;
    const float half = 0.5f * cellM;
    // A cell seen burning for the first time: the buildings round it are
    // passed when its drawn front reaches the point of its square (and its
    // run on) nearest them.
    for (const BurningView& v : views)
    {
        if (v.model == 0 || !_bldSeen.insert(PlantCellKey(v.x, v.z, cellM)).second)
            continue;
        const int ci = static_cast<int>(std::floor(v.x / cellM)), cj = static_cast<int>(std::floor(v.z / cellM));
        const double caught = _clock - v.age;
        const float xLo = v.x - half - (v.builtSides & 2 ? cellM : 0.0f),
                    xHi = v.x + half + (v.builtSides & 1 ? cellM : 0.0f);
        const float zLo = v.z - half - (v.builtSides & 8 ? cellM : 0.0f),
                    zHi = v.z + half + (v.builtSides & 4 ? cellM : 0.0f);
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
            {
                const std::uint64_t key = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(ci + di)) << 32) |
                                          static_cast<std::uint32_t>(cj + dj);
                const auto it = _bldCells.find(key);
                if (it == _bldCells.end())
                    continue;
                for (const int idx : it->second)
                {
                    Building& b = _bld[static_cast<std::size_t>(idx)];
                    if (b.kind == BuildingKind::Never || b.catchAt >= 0.0 ||
                        (!b.scheduled && b.passes > 0 && _clock - b.lastPass < kBuildingRepassSec))
                        continue;
                    const float qx = std::clamp(b.cx, xLo, xHi), qz = std::clamp(b.cz, zLo, zHi);
                    if (b.Gap(qx, qz) > kBuildingPassReachM)
                        continue;
                    const double t = caught + static_cast<double>(DrawnReachSec(v, qx, qz, cellM));
                    if (!b.scheduled || t < b.passAt)
                    {
                        b.passAt = t;
                        b.passKwM = v.intensityKwM;
                        b.passFlameM = v.flameMaxM;
                    }
                    if (!b.scheduled)
                    {
                        b.scheduled = true;
                        _bldSched.push_back(idx);
                    }
                }
            }
    }
    // The passes due now.
    for (std::size_t n = 0; n < _bldSched.size();)
    {
        const int idx = _bldSched[n];
        Building& b = _bld[static_cast<std::size_t>(idx)];
        if (b.scheduled && _clock < b.passAt)
        {
            ++n;
            continue;
        }
        if (b.scheduled)
            PassBuilding(idx, b.ember ? "ember" : "front", -1.0f);
        _bldSched[n] = _bldSched.back();
        _bldSched.pop_back();
    }
    // The burning: its phase now on its own clock, the stock destruction and
    // its embers as its roof falls in, its heat on what is round it at its
    // peak, its fires and its look.
    const bool look = _clock - _bldLookAt >= kBuildingLookSec;
    if (look)
        _bldLookAt = _clock;
    int flaming = 0;
    for (std::size_t n = 0; n < _bldLive.size();)
    {
        const int idx = _bldLive[n];
        Building& b = _bld[static_cast<std::size_t>(idx)];
        // At most kBuildingFiresMax alight at once: one past it waits (its
        // clock held before its first flashover).
        b.waiting = flaming >= kBuildingFiresMax && b.tb >= 0.0 && static_cast<double>(b.tl.flashSec) > b.tb;
        if (!b.waiting)
            b.tb += static_cast<double>(dt) * static_cast<double>(_objTime);
        const float tb = static_cast<float>(b.tb);
        if (tb < 0.0f || b.waiting)
        {
            ++n;
            continue;
        }
        const BuildingPhase ph = BuildingPhaseAt(b.tl, tb);
        if (ph.over)
        {
            b.live = false;
            b.over = true;
            WriteBuildingLook(b, tb, ph);
            _bldLive[n] = _bldLive.back();
            _bldLive.pop_back();
            continue;
        }
        ++n;
        if (ph.flaming)
            ++flaming;
        if (ph.collapsed && !b.destroyed)
        {
            // its roof (a shed's or barn's whole body) falls in: the stock
            // damage path, as a shell's hit would destroy it (cover, people
            // inside), without the stock ruin's crumple (that sinks a house
            // a few metres into the ground): a building gutted by fire
            // stands as the tenth answer's shell, its walls, gables and
            // chimney up and its roof gone (burnBuilding drops the roof's
            // faces; a shed or barn burns away to its lowest metre). A
            // fence falls as the stock ruin does.
            b.destroyed = true;
            _bldDestroyed++;
            // (end-state agent) gutted: on every machine (the replicated
            // damage, its ammo name FireShell.hpp kShellAmmo) the object
            // takes its shell (collision, shots and sight with the roof
            // gone, its rubble inside), marked for a savegame; never the
            // stock crumple. A fence falls as the stock ruin does.
            if (b.obj && !b.obj->IsGutted())
            {
                if (b.kind == BuildingKind::Fence)
                {
                    if (!b.obj->IsDammageDestroyed())
                        b.obj->SetDammageNetAware(1.0f);
                }
                else
                    b.obj->GutByFire();
            }
            LOG_INFO(Script, "UDFIRE;event=buildingcollapse;shape={};kind={};x={:.1f};z={:.1f};t={:.1f};tb={:.0f}",
                     b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, _clock, tb);
            // a burst of sparks over it
            SparkBirth sb;
            sb.kind = SparkKind::Torch;
            sb.x = b.cx;
            sb.z = b.cz;
            sb.y = b.y + b.wallM;
            sb.spreadXZ = b.hx + b.hz;
            sb.spreadY = 1.0f;
            sb.vy = 6.0f;
            sb.tipY = b.y + b.wallM + b.tl.peakM;
            sb.riseMps = 6.0f;
            sb.groundY = b.y;
            _sparks.Emit(sb, std::clamp(static_cast<int>(60.0f * (b.hx + b.hz)), 100, 900));
        }
        // (shell geometry) its burnt shell: its chimney carried down to the
        // ground floor and its walls' inner sides while its roof falls in,
        // then everything the fire took hidden, in the shadow pass too
        if (ph.collapsed)
            StageBuildingShell(b, tb);
        // (its embers and its heat may light cells whose fuel registers more
        // buildings: _bld may move, so it is looked up again after them)
        if (ph.collapsed && !b.embersDone)
        {
            b.embersDone = true;
            BuildingEmbers(idx, wind10, wdx, wdz);
        }
        if (!_bld[static_cast<std::size_t>(idx)].heatDone && ph.flaming &&
            ph.heatMW >= 0.5f * _bld[static_cast<std::size_t>(idx)].tl.peakMW)
        {
            _bld[static_cast<std::size_t>(idx)].heatDone = true;
            BuildingReach(idx);
        }
        Building& bb = _bld[static_cast<std::size_t>(idx)];
        if (ph.flaming)
            _bldBurning++;
        // its column: smoking (a column seen from afar), black (dense dark smoke)
        if (ph.smoke > 0.0f)
            _bldSmoking++;
        if (ph.smoke >= 0.3f && ph.black >= 0.6f)
            _bldBlack++;
        bb.nowHeat = ph.flaming ? std::clamp(std::max({ph.windows, ph.topM / std::max(bb.tl.peakM, 0.1f),
                                                       ph.bodyM / std::max(bb.tl.peakM, 0.1f), ph.innerM / 2.5f}),
                                             0.0f, 1.0f)
                                : 0.0f;
        const std::size_t before = _objectFires.size();
        BuildingFires(bb, tb, ph, wind10, wdx, wdz);
        _bldParts += static_cast<int>(_objectFires.size() - before);
        if (look)
            WriteBuildingLook(bb, tb, ph);
    }
}

ShellMode FireWorld::ShellModeOfShape(const LODShape* shape)
{
    const BuildingKind k = BuildingKindOfShape(shape, shape->GetMapType());
    if (k == BuildingKind::Shed || k == BuildingKind::Barn)
    {
        std::string n = (const char*)shape->GetName();
        std::transform(n.begin(), n.end(), n.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (!BuildingWallsStand(n.c_str()))
            return ShellMode::Stub;
    }
    return k == BuildingKind::Church ? ShellMode::Church : ShellMode::Walls;
}

void FireWorld::RestoreShell(int idx)
{
    // (end-state agent) gutted with no fire of its own here (a remote
    // machine's, a load, a clear): its timeline as its seed draws it and the
    // look of its end (sooted, charred, roofless, cold), for good
    Building& b = _bld[static_cast<std::size_t>(idx)];
    float u[kBuildingTimelineDraws];
    for (int k = 0; k < kBuildingTimelineDraws; ++k)
        u[k] = BuildingDraw(b.seed, 400 + k);
    b.tl = BuildingTimelineOf(b.kind, false, u);
    const float end = b.tl.downSec + std::max(b.tl.glowSec, b.tl.smokeSec) + 1.0f;
    b.tb = end;
    b.catchAt = _clock;
    b.live = false;
    b.destroyed = b.over = b.embersDone = b.heatDone = true;
    _bldShells++;
    if (b.obj)
        ShellWear(b.obj, 1);
    b.shellStage = b.obj ? ShellStageOf(b.obj->GetShape()) : -1;
    WriteBuildingLook(b, end, BuildingPhaseAt(b.tl, end));
    LOG_INFO(Script, "UDFIRE;event=shellrestore;shape={};kind={};x={:.1f};z={:.1f};wallM={:.2f};slot={}",
             b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, b.wallM, b.slots[0]);
}

void FireWorld::DrainShells()
{
    std::vector<Ref<Object>> gutted, repaired;
    gutted.swap(_shellPending);
    repaired.swap(_shellRepaired);
    for (Ref<Object>& o : gutted)
    {
        Object* obj = o;
        if (!obj || !obj->IsGutted() || !obj->GetShape())
            continue;
        const LODShape* shape = obj->GetShape();
        const int idx = RegisterBuilding(obj, shape, shape->GetMapType());
        if (idx < 0)
            continue;
        const Building& b = _bld[static_cast<std::size_t>(idx)];
        if (b.live || b.destroyed)
            continue; // its own fire here writes its look
        RestoreShell(idx);
    }
    for (Ref<Object>& o : repaired)
    {
        const auto it = _bldByObj.find(o.GetRef());
        if (it == _bldByObj.end() || it->second < 0)
            continue;
        const int idx = it->second;
        Building& b = _bld[static_cast<std::size_t>(idx)];
        // its fire forgotten, its look none (the slots stay its, blank)
        _bldLive.erase(std::remove(_bldLive.begin(), _bldLive.end(), idx), _bldLive.end());
        b.live = b.destroyed = b.over = b.embersDone = b.heatDone = b.waiting = false;
        b.catchAt = -1.0;
        b.tb = -1.0e9;
        b.tl = BuildingTimeline();
        if (b.slots[0] >= 0)
            WriteBuildingLook(b, -1.0f, BuildingPhase());
        LOG_INFO(Script, "UDFIRE;event=shellrepaired;shape={};x={:.1f};z={:.1f}", b.name.c_str(), b.cx, b.cz);
    }
}

std::string FireWorld::ShellRay(Vector3Par p, Vector3Par q) const
{
    if (!GLandscape)
        return "none";
    CollisionBuffer fire, view;
    GLandscape->ObjectCollision(fire, nullptr, nullptr, p, q, 0.0f, ObjIntersectFire);
    GLandscape->ObjectCollision(view, nullptr, nullptr, p, q, 0.0f, ObjIntersectView);
    const float vis = GLandscape->Visible(p, q, 0.0f, nullptr, nullptr, ObjIntersectView);
    std::string firstFire = "none", firstView = "none";
    if (fire.Size() > 0 && fire[0].object && fire[0].object->GetShape())
        firstFire = (const char*)fire[0].object->GetShape()->GetName();
    if (view.Size() > 0 && view[0].object && view[0].object->GetShape())
        firstView = (const char*)view[0].object->GetShape()->GetName();
    char buf[320];
    std::snprintf(buf, sizeof(buf), "fire=%d;view=%d;visible=%.2f;fireObj=%s;viewObj=%s", fire.Size(), view.Size(), vis,
                  firstFire.c_str(), firstView.c_str());
    return buf;
}

float FireWorld::BuildingHeatAt(float x, float z, float& flameM) const
{
    // (its parts' flames burn people within their hurtM: the shared Gameplay)
    flameM = 0.0f;
    float s = 0.0f;
    // inside a building whose rooms burn
    for (const int idx : _bldLive)
    {
        const Building& b = _bld[static_cast<std::size_t>(idx)];
        if (b.nowHeat <= 0.0f || !b.Inside(x, z, 0.3f))
            continue;
        s = std::max(s, 0.4f + 0.6f * b.nowHeat);
        flameM = std::max(flameM, 2.0f + 2.0f * b.nowHeat);
    }
    return s;
}

std::string FireWorld::BuildingsLine() const
{
    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "bld=%d;bpass=%d;bcaught=%d;bburning=%d;bsmoking=%d;bblack=%d;bdestroyed=%d;bembers=%d;bparts=%d;"
                  "bms=%.3f",
                  static_cast<int>(_bld.size()), _bldPassed, _bldCaught, _bldBurning, _bldSmoking, _bldBlack,
                  _bldDestroyed, _bldEmbersOut, _bldParts, _bldMs);
    return buf;
}

std::string FireWorld::BuildingsScan(float x, float z, float r)
{
    // Registers every building within r of (x, z) and counts them by kind
    // (with bushes or a conifer against them), for the scenes.
    Ensure();
    int count[kBuildingKinds] = {}, veg[kBuildingKinds] = {};
    if (GLandscape)
    {
        const int g0x = static_cast<int>(std::floor((x - r) * InvLandGrid)),
                  g1x = static_cast<int>(std::floor((x + r) * InvLandGrid));
        const int g0z = static_cast<int>(std::floor((z - r) * InvLandGrid)),
                  g1z = static_cast<int>(std::floor((z + r) * InvLandGrid));
        for (int cz = g0z; cz <= g1z; ++cz)
            for (int cx = g0x; cx <= g1x; ++cx)
            {
                if (!InRange(cz, cx))
                    continue;
                const ObjectList& list = GLandscape->GetObjects(cz, cx);
                for (int i = 0; i < list.Size(); ++i)
                {
                    Object* obj = list[i];
                    if (!obj || !obj->GetShape())
                        continue;
                    const MapType mt = obj->GetShape()->GetMapType();
                    if (mt != MapHouse && mt != MapBuilding && mt != MapChurch && mt != MapChapel && mt != MapBunker &&
                        mt != MapFortress && mt != MapFuelstation && mt != MapHospital && mt != MapFence)
                        continue;
                    const float dx = obj->Position().X() - x, dz = obj->Position().Z() - z;
                    if (dx * dx + dz * dz > r * r)
                        continue;
                    const int bi = RegisterBuilding(obj, obj->GetShape(), mt);
                    if (bi < 0)
                        continue;
                    const Building& b = _bld[static_cast<std::size_t>(bi)];
                    count[static_cast<int>(b.kind)]++;
                    veg[static_cast<int>(b.kind)] += b.veg ? 1 : 0;
                    LOG_INFO(
                        Script,
                        "UDFIRE;event=building;shape={};kind={};x={:.1f};z={:.1f};w={:.1f};l={:.1f};h={:.1f};veg={}",
                        b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, 2.0f * b.hx, 2.0f * b.hz, b.h,
                        b.veg ? 1 : 0);
                }
            }
    }
    char buf[256];
    std::snprintf(buf, sizeof(buf), "stone=%d/%d;shed=%d;barn=%d;church=%d;fuel=%d;masonry=%d;never=%d;fence=%d",
                  count[0], veg[0], count[1], count[2], count[3], count[4], count[5], count[6], count[7]);
    return buf;
}

float FireWorld::BuildingTime(float x, float z, const char* what)
{
    int best = -1;
    float bestD = 30.0f;
    for (int k = 0; k < static_cast<int>(_bld.size()); ++k)
    {
        const float d = _bld[static_cast<std::size_t>(k)].Gap(x, z);
        if (d < bestD)
        {
            bestD = d;
            best = k;
        }
    }
    if (best < 0)
        return -1.0f;
    const Building& b = _bld[static_cast<std::size_t>(best)];
    if (!strcmpi(what, "tb"))
        return b.catchAt >= 0.0 ? static_cast<float>(b.tb) : -1.0f;
    if (!strcmpi(what, "flash"))
        return b.tl.flashSec;
    if (!strcmpi(what, "spread"))
        return b.tl.spreadSec;
    if (!strcmpi(what, "collapse"))
        return b.tl.collapseSec;
    if (!strcmpi(what, "floor"))
        return b.tl.floorSec;
    if (!strcmpi(what, "down"))
        return b.tl.downSec;
    return -1.0f;
}

std::string FireWorld::BuildingState(float x, float z, bool ignite, bool roofFirst)
{
    // the building nearest (x, z) within 30 m that the fire knows; fireSpreadBuilding registers the land's round it
    if (GLandscape)
    {
        const int gx = static_cast<int>(std::floor(x * InvLandGrid)),
                  gz = static_cast<int>(std::floor(z * InvLandGrid));
        for (int cz = gz - 1; cz <= gz + 1; ++cz)
            for (int cx = gx - 1; cx <= gx + 1; ++cx)
            {
                if (!InRange(cz, cx))
                    continue;
                const ObjectList& list = GLandscape->GetObjects(cz, cx);
                for (int i = 0; i < list.Size(); ++i)
                {
                    Object* obj = list[i];
                    if (!obj || !obj->GetShape())
                        continue;
                    const MapType mt = obj->GetShape()->GetMapType();
                    if (mt == MapHouse || mt == MapBuilding || mt == MapChurch || mt == MapChapel || mt == MapBunker ||
                        mt == MapFortress || mt == MapFuelstation || mt == MapHospital)
                        RegisterBuilding(obj, obj->GetShape(), mt);
                }
            }
    }
    int best = -1;
    float bestD = 30.0f;
    for (int k = 0; k < static_cast<int>(_bld.size()); ++k)
    {
        const float d = _bld[static_cast<std::size_t>(k)].Gap(x, z);
        if (d < bestD)
        {
            bestD = d;
            best = k;
        }
    }
    if (best < 0)
        return "none";
    Building& b = _bld[static_cast<std::size_t>(best)];
    if (ignite && b.catchAt < 0.0 && b.kind != BuildingKind::Never)
    {
        b.passAt = _clock;
        CatchBuilding(best, 0.0, roofFirst, "script");
    }
    const float tb = b.catchAt >= 0.0 ? static_cast<float>(b.tb) : -1.0f;
    const BuildingPhase ph = BuildingPhaseAt(b.tl, tb);
    char buf[640];
    std::snprintf(buf, sizeof(buf),
                  "shape=%s;kind=%d;x=%.1f;z=%.1f;w=%.1f;l=%.1f;h=%.1f;wall=%.1f;veg=%d;passes=%d;caught=%d;tb=%.0f;"
                  "flash=%.0f;collapse=%.0f;down=%.0f;windows=%.2f;windowM=%.2f;top=%.2f;body=%.2f;inner=%.2f;"
                  "mw=%.1f;smoke=%.2f;black=%.2f;glow=%.2f;collapsed=%d;destroyed=%d;chanceNow=%.3f",
                  b.name.c_str(), static_cast<int>(b.kind), b.cx, b.cz, 2.0f * b.hx, 2.0f * b.hz, b.h, b.wallM,
                  b.veg ? 1 : 0, b.passes, b.catchAt >= 0.0 ? 1 : 0, tb, b.tl.flashSec, b.tl.collapseSec, b.tl.downSec,
                  ph.windows, ph.windowM, ph.topM, ph.bodyM, ph.innerM, ph.heatMW, ph.smoke, ph.black, ph.glow,
                  ph.collapsed ? 1 : 0, b.obj && b.obj->IsDammageDestroyed() ? 1 : 0,
                  BuildingPassChance(b.kind, b.veg, PigNow(),
                                     [&]
                                     {
                                         float wx = 0.0f, wz = 0.0f;
                                         BaseWind(wx, wz);
                                         return std::sqrt(wx * wx + wz * wz);
                                     }()));
    return buf;
}

// The renderer's last frame of flames by how it drew them (fix 08,
// NoteFlameDraw; StatsLine far=).
struct FlameDrawCounts
{
    int nearRibbons = 0, farRibbons = 0, splats = 0;
};
FlameDrawCounts s_flameDraw;

std::string FireWorld::StatsLine()
{
    char buf[768];
    const Stats& s = _stats;
    const firespread::Stats g = _grid ? _grid->GetStats() : firespread::Stats();
    float wx = 0.0f, wz = 0.0f;
    Wind(wx, wz);
    int crackles = 0, roars = 0;
    _sound.Live(crackles, roars);
    std::snprintf(
        buf, sizeof(buf),
        "burning=%d;front=%d;burnt=%d;known=%d;tiles=%d;ignitions=%d;torching=%d;spots=%d;deferred=%d;"
        "simt=%.1f;ms=%.3f;msmax=%.3f;msgrid=%.3f;mssmoke=%.3f;msdraw=%.3f;msplay=%.3f;flames=%d;smokepuffs=%d;"
        "lights=%d;cloudlets=%d;damaged=%d;fled=%d;trees=%d;sheets=%d;sheetcells=%d;sheetcut=%d;flares=%d/"
        "%d;sight=%lld/"
        "%lld;wsight=%lld;wsightus=%.1f;wind=%.1f,%.1f;m1h=%.3f;smoke=%.1f;sound=%d;crackles=%d;roars=%d;pops=%lld",
        g.burning, g.front, g.burnt, g.known, g.tiles, g.ignitions, g.torching, g.spots, g.deferred, g.simTime,
        s.msTotal, s.msTotalMax, s.msGrid, s.msSmoke, s.msDraw, s.msPlay, s.flamePuffs, s.smokePuffs, s.lights,
        GLOB_WORLD ? GLOB_WORLD->NCloudlets() : -1, s.damaged, s.fled, s.trees, s.sheets, s.sheetCells, s.sheetCut,
        s.flares, s.flaresTotal, s.sightHits, s.sightCalls, s.winSightCalls, s.winSightNs / 1000.0, wx, wz,
        _moisture.m1h, _smokeLive, _sound.Ready() ? 1 : 0, crackles, roars, _sound.PopsPlayed());
    // The flames against the model (fl_<class>=tongues/at the strip's
    // leading edge: tongues, model L, drawn L, drawn top/drawn L max, top
    // max/mean tilt (deg)/v4 at the leading edge: drawn L, top/v4 max L, top
    // max/size (tree or shrub height) mean, max), metres.
    std::string out = buf;
    {
        // The spread (the realism targets): embers, spot fires' farthest
        // throw (m), the burnt and burning extent (cell centres, m), the
        // area (ha) and the build-up's rate (per minute).
        char sp[256];
        const float cellM = _grid ? _grid->GetParams().cellM : 8.0f;
        std::snprintf(sp, sizeof(sp), ";embers=%d;spotmax=%.0f;ext=%.0f,%.0f,%.0f,%.0f;area=%.3f;buildup=%.3f",
                      g.embers, g.spotMaxM, g.extMinX, g.extMinZ, g.extMaxX, g.extMaxZ,
                      g.ignitions * cellM * cellM / 10000.0f, _grid ? _grid->GetParams().buildUpPerMin : 0.0f);
        out += sp;
        // The front (rays walked, anchors slid, offers from a giver's own
        // front), the roads (cells drawing for embers at a gap, embers across,
        // spot fires lit beyond, steps by flame contact) and the wind's wander
        // (deg, its turn now).
        std::snprintf(sp, sizeof(sp), ";front4=%lld/%d/%d;road=%d/%d/%d;wander=%.1f/%.1f;msd=%.2f/%.2f/%.2f", g.rays,
                      g.slides, g.localOffers, g.roadDraws, g.roadEmbers, g.roadSpots, _wanderDeg,
                      57.29578f * WanderRadNow(), s.msDrawLive, s.msDrawAfter, s.msDrawDebris);
        out += sp;
    }
    {
        // Each plant's own fire: the plants known, reached by the drawn
        // flames, flared and torched; the plant table's slots taken and all.
        char pf[160];
        std::snprintf(pf, sizeof(pf), ";plants=%d/%d/%d/%d;ptable=%d/%d%s", static_cast<int>(_pf.size()), _pfContacts,
                      _pfFlared, _pfTorched, _pfTable.used, _pfTable.slots, _pfFull ? "/full" : "");
        out += pf;
    }
    {
        // (fire-objects) The object fires this frame, the wrecks, the object
        // flames drawn, and the ground they lit by kind (heat, ember,
        // fireball, debris, rain, fragment, brand) and brands waiting.
        char ob[200];
        std::snprintf(ob, sizeof(ob),
                      ";obj=%d;wrecks=%d;objdrawn=%d;objlit=%d/%d/%d/%d/%d/%d/%d;objwait=%d;objtime=%.0f",
                      static_cast<int>(_objectFires.size()), static_cast<int>(_wrecks.size()), _objDrawn, _objLit[0],
                      _objLit[1], _objLit[2], _objLit[3], _objLit[4], _objLit[5], _objLit[6],
                      static_cast<int>(_objPending.size()), _objTimeK);
        out += ob;
        // (followups) the object fires' own columns: how many, their puffs a
        // second, the budget's share, the puffs born since the last line
        std::snprintf(ob, sizeof(ob), ";objplume=%d/%.1f/%.2f/%d;wrecklooks=%d", static_cast<int>(_objCols.size()),
                      _objPlumeRate, _objPlumeScale, _objPlumeBorn, static_cast<int>(_wreckLooks.size()));
        out += ob;
        _objPlumeBorn = 0;
    }
    {
        // The lingering: the last frame's tail tongues, debris flames, glow
        // points, burnt-out cells drawn on and cells kept; then, by time
        // since the front (s), the burnt ground near the camera (m2, a
        // frame's mean), its flames per 100 m2 and the share of it under a
        // flame (%), since the last stats line (the consult's test 2).
        char lg[400];
        int len = std::snprintf(lg, sizeof(lg), ";linger=%d/%d/%d/%d/%d;lg=", _stats.tail, _stats.debris, _stats.glows,
                                _stats.afterCells, _stats.afterKept);
        const double frames = std::max<double>(1.0, static_cast<double>(_lgFrames));
        for (int k = 0; k < kLingerBins && len > 0 && len < static_cast<int>(sizeof(lg)); ++k)
        {
            const double area = _lgArea[k];
            len += std::snprintf(lg + len, sizeof(lg) - static_cast<std::size_t>(len), "%s%.0f:%.0f:%.2f:%.2f",
                                 k ? "|" : "", kLingerBinSec[k], area / frames,
                                 area > 0.0 ? 100.0 * _lgPts[k] / area : 0.0,
                                 area > 0.0 ? 100.0 * _lgFlame[k] / area : 0.0);
            _lgArea[k] = _lgPts[k] = _lgFlame[k] = 0.0;
        }
        _lgFrames = 0;
        out += lg;
    }
    {
        // The light (UpdateLights, the last frame): the flames' candela in
        // reach (kcd), the fill's, the lights picked and fading, the halo's
        // cells, the picks changed since the last stats line, and the night's
        // gain the renderer gave it (CombatTuning::wildfire).
        char li[160];
        std::snprintf(li, sizeof(li), ";light=%.1f/%.1f/%d/%d/%d/%d/%.1f", _stats.lightCd / 1000.0f,
                      _stats.fillCd / 1000.0f, _stats.lightsPicked, _stats.lightsFading, _stats.lightCells,
                      _stats.lightSwaps, lighting::GCombatLights().Tuning().wildfire);
        _stats.lightSwaps = 0;
        out += li;
    }
    {
        // The burning bed (the realism audit's fix 7): the last frame's sheets.
        char bd[48];
        std::snprintf(bd, sizeof(bd), ";beds=%d", _stats.beds);
        out += bd;
    }
    {
        // The renderer's last frame of flames by how it drew them (fix 08):
        // ribbons from the near flipbook, from the far book, and splats.
        char fd[96];
        std::snprintf(fd, sizeof(fd), ";far=%d/%d/%d;frange=%.0f/%.0f", s_flameDraw.nearRibbons, s_flameDraw.farRibbons,
                      s_flameDraw.splats, s_drawRangeM, s_lingerRangeM);
        out += fd;
    }
    {
        // The sparks (FireSparks.hpp): in the air now by what threw them (a
        // front, the plants' flares, the torching crowns), made, dropped (the
        // pool full) and landed since the start; the front's in the air
        // within kFrontSparkFullM of the camera, the front there (m) and
        // their count a metre of it (the consult's 5 to 20); their cost (ms:
        // the last frame's, the most since the last stats line).
        const SparkCounts c = _sparks.Counts();
        int nearFront = 0;
        if (GLOB_SCENE && GLOB_SCENE->GetCamera())
        {
            const Vector3 cp = GLOB_SCENE->GetCamera()->Position();
            nearFront = _sparks.CountNear(SparkKind::Front, cp.X(), cp.Z(), kFrontSparkFullM);
        }
        const int flares = c.byKind[static_cast<int>(SparkKind::Thicket)] +
                           c.byKind[static_cast<int>(SparkKind::Leafy)] + c.byKind[static_cast<int>(SparkKind::Reed)] +
                           c.byKind[static_cast<int>(SparkKind::Olive)];
        char sk[256];
        std::snprintf(
            sk, sizeof(sk),
            ";sparks=%d/%d/%d/%d;sparkmade=%lld;sparkdrop=%lld;sparkland=%lld;sp100=%d/%.0f/%.2f;spms=%.3f/%.3f",
            c.alive, c.byKind[static_cast<int>(SparkKind::Front)], flares, c.byKind[static_cast<int>(SparkKind::Torch)],
            c.made, c.dropped, c.landed, nearFront, _sparkFrontM, _sparkFrontM > 0.0f ? nearFront / _sparkFrontM : 0.0f,
            _msSparks, _msSparksMax);
        out += sk;
        _msSparksMax = 0.0;
    }
    if (_footOn)
    {
        // The feet (m/s, since the last stats line): by class, the frame
        // pairs measured, the share under 2 cm/s, max, p95, mean.
        static const char* kFootNames[kFootClasses] = {"band", "tail", "debris", "glow", "flare", "torch", "bed"};
        out += ";foot=";
        for (int c = 0; c < kFootClasses; ++c)
        {
            char fs[160];
            const long long n = _foot.n[c];
            std::snprintf(fs, sizeof(fs), "%s%s:%lld/%.3f/%.3f/%.3f/%.3f", c ? "|" : "", kFootNames[c], n,
                          n ? static_cast<double>(_foot.still[c]) / static_cast<double>(n) : 0.0,
                          static_cast<double>(_foot.max[c]), static_cast<double>(n ? _foot.P95(c) : 0.0f),
                          n ? _foot.sum[c] / static_cast<double>(n) : 0.0);
            out += fs;
        }
        _foot.Reset();
    }
    static const char* kNames[kMeasureClasses] = {"grass", "shrub", "timber", "torch"};
    for (int c = 0; c < kMeasureClasses; ++c)
    {
        const FlameMeasure& m = _measure[c];
        auto avg = [](double sum, long long n) { return n > 0 ? sum / static_cast<double>(n) : 0.0; };
        char seg[320];
        std::snprintf(seg, sizeof(seg), ";fl_%s=%lld/%lld,%.2f,%.2f,%.2f/%.2f,%.2f/%.1f/%.2f,%.2f/%.2f,%.2f/%.1f,%.1f",
                      kNames[c], m.n, m.lead, avg(m.leadModel, m.lead), avg(m.leadDrawn, m.lead),
                      avg(m.leadTop, m.lead), m.drawnMax, m.topMax, avg(m.tilt, m.n) * 57.29578,
                      avg(m.lead4Drawn, m.lead4), avg(m.lead4Top, m.lead4), m.v4Max, m.v4TopMax, avg(m.size, m.n),
                      m.sizeMax);
        out += seg;
        _measure[c] = FlameMeasure();
    }
    // The plume (EmitPlume): live puffs, those born since the last line, the
    // fire's heat (MW), smoke (units a second), width across the wind (m),
    // puffs a second and their size factor; then in bands round 100 m, 1, 2
    // and 5 km downwind of their sources (the surface fire's puffs): how many
    // show (alpha over 0.02), their middle's mean height over their source's
    // ground, its 5 and 95 percent heights, the plume's width across the wind
    // (5 to 95 percent of the puffs' places plus their mean size) and their
    // mean alpha; and the farthest a showing puff has come.
    {
        float wxs = 0.0f, wzs = 0.0f;
        Wind(wxs, wzs);
        const float wl = std::sqrt(wxs * wxs + wzs * wzs);
        const float ux = wl > 1e-3f ? wxs / wl : 0.0f, uz = wl > 1e-3f ? wzs / wl : 1.0f;
        constexpr int kBands = 4;
        const float bandX[kBands] = {100.0f, 1000.0f, 2000.0f, 5000.0f};
        const char* bandName[kBands] = {"100", "1k", "2k", "5k"};
        std::vector<float> hs[kBands], across[kBands], agl[kBands];
        double sizeSum[kBands] = {}, alphaSum[kBands] = {};
        float farthest = 0.0f, fireMax = 0.0f;
        for (int i = 0; GLOB_WORLD && i < GLOB_WORLD->NCloudlets(); ++i)
        {
            const PlumePuff* p = dynamic_cast<const PlumePuff*>(GLOB_WORLD->GetCloudlet(i));
            if (!p || p->Opacity() < 0.02f)
                continue;
            farthest = std::max(farthest, p->TravelM());
            fireMax = std::max(fireMax, p->FireLight());
            if (p->Crown())
                continue;
            for (int b = 0; b < kBands; ++b)
                if (std::fabs(p->TravelM() - bandX[b]) <= 0.1f * bandX[b])
                {
                    hs[b].push_back(p->RiseM());
                    agl[b].push_back(GLandscape ? p->Position().Y() -
                                                      GLandscape->SurfaceY(p->Position().X(), p->Position().Z())
                                                : p->RiseM());
                    across[b].push_back(-uz * p->Position().X() + ux * p->Position().Z());
                    sizeSum[b] += p->Scale();
                    alphaSum[b] += p->Opacity();
                }
        }
        float glowHeat = 0.0f;
        for (float h : _glowHeat)
            glowHeat = std::max(glowHeat, h);
        char seg[240];
        std::snprintf(seg, sizeof(seg), ";plume=%d/%d/%.1f/%.3f/%.0f/%.1f/%.2f;plumefar=%.0f;plumeglow=%.1f/%.4f",
                      PlumePuff::Alive(), _plumeBorn, _plumeMW, _plumeSmoke, _plumeWidth, _plumeRate, _plumeLod,
                      farthest, glowHeat, fireMax);
        out += seg;
        _plumeBorn = 0;
        auto pct = [](std::vector<float>& v, float q)
        {
            if (v.empty())
                return 0.0f;
            const std::size_t k = std::min(v.size() - 1, static_cast<std::size_t>(q * (v.size() - 1) + 0.5f));
            std::nth_element(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k), v.end());
            return v[k];
        };
        for (int b = 0; b < kBands; ++b)
        {
            const std::size_t n = hs[b].size();
            double mean = 0.0;
            for (float h : hs[b])
                mean += h;
            mean = n ? mean / static_cast<double>(n) : 0.0;
            const float lo = pct(hs[b], 0.05f), hi = pct(hs[b], 0.95f);
            const float wlo = pct(across[b], 0.05f), whi = pct(across[b], 0.95f);
            const float alo = pct(agl[b], 0.05f), amid = pct(agl[b], 0.5f), ahi = pct(agl[b], 0.95f);
            const double size = n ? sizeSum[b] / static_cast<double>(n) : 0.0;
            std::snprintf(seg, sizeof(seg), ";plume%s=%zu/%.0f/%.0f/%.0f/%.0f/%.2f/agl:%.0f,%.0f,%.0f", bandName[b], n,
                          mean, static_cast<double>(lo), static_cast<double>(hi),
                          n ? static_cast<double>(whi - wlo) + size : 0.0,
                          n ? alphaSum[b] / static_cast<double>(n) : 0.0, static_cast<double>(alo),
                          static_cast<double>(amid), static_cast<double>(ahi));
            out += seg;
        }
    }
    // (buildings builder) the buildings the fire knows, passed, caught, burning, destroyed
    out += ";";
    out += BuildingsLine();
    _stats.winSightCalls = 0;
    _stats.winSightNs = 0;
    _stats.msTotalMax = 0.0;
    return out;
}

// ---- Object fires (fire-objects; the shared part) ----
bool FireWorld::PointBare(float x, float z)
{
    if (!GLandscape)
        return true;
    if (GLandscape->SurfaceY(x, z) < GLandscape->GetSeaLevelNoWave() + 0.3f)
        return true;
    RoadHit road;
    if (RoadNear(x, z, road) && road.dist < road.halfBare)
        return true;
    // (buildings builder) a building's own footprint and apron
    if (!_bld.empty() && BuildingAt(x, z, kBuildingApronM) >= 0)
        return true;
    return false;
}

bool FireWorld::ObjectLight(float x, float z, float chance, float delaySec, ObjectIgniteKind kind, std::uint64_t source)
{
    if (!_grid || PointBare(x, z))
        return false;
    const CellState st = _grid->StateAt(x, z);
    if (st == CellState::Burning || st == CellState::Burnt)
        return true; // fire there already: the heat's try stops
    if (_objRand.U() >= chance)
        return true;
    if (delaySec > 0.0f)
    {
        ObjectPending pend;
        pend.x = x;
        pend.z = z;
        pend.at = _clock + delaySec;
        pend.source = source;
        pend.kind = kind;
        _objPending.push_back(pend);
        return true;
    }
    if (!_grid->Ignite(x, z))
        return false; // not burnable there (its cell's fuel): the heat tries further out
    _objLit[static_cast<int>(kind)]++;
    for (Wreck& w : _wrecks)
        if (w.id == (source >> 3))
        {
            w.lit[static_cast<int>(kind)]++;
            const float age = static_cast<float>(w.age);
            if (w.firstLitAge < 0.0f)
                w.firstLitAge = age;
            LOG_INFO(Script, "UDFIRE;event=objlit;id={};cls={};kind={};age={:.1f};x={:.1f};z={:.1f};d={:.1f}", w.id,
                     WreckClassName(w.tl.cls), static_cast<int>(kind), age, x, z,
                     std::sqrt((x - w.x) * (x - w.x) + (z - w.z) * (z - w.z)));
            break;
        }
    return true;
}

void FireWorld::ObjectDraws(float dt, float wx, float wz)
{
    if (!_grid || dt <= 0.0f)
        return;
    for (const ObjectFire& of : _objectFires)
        ObjectFireDraws(of, dt, wx, wz, _objRand,
                        [&](float x, float z, float chance, float delay, ObjectIgniteKind kind)
                        { return ObjectLight(x, z, chance, delay, kind, of.id); });
    // the brands whose smoulder is over light now (their chance already drawn)
    for (std::size_t i = 0; i < _objPending.size();)
    {
        const ObjectPending pend = _objPending[i];
        if (pend.at > _clock)
        {
            ++i;
            continue;
        }
        _objPending[i] = _objPending.back();
        _objPending.pop_back();
        ObjectLight(pend.x, pend.z, 1.0f, 0.0f, pend.kind, pend.source);
    }
}

void FireWorld::ThrowEmbers(const EmberThrow& e)
{
    if (!_grid)
        return;
    const float pig = EmberPig(_grid->GetParams().moisture.m1h);
    EmberThrowDraws(e, pig, _objRand, [&](float x, float z, float chance, float delay, ObjectIgniteKind kind)
                    { return ObjectLight(x, z, chance, delay, kind, e.source); });
}

float FireWorld::ObjectFluxAt(float x, float y, float z) const
{
    float q = 0.0f;
    for (const ObjectFire& of : _objectFires)
    {
        if (of.heatMW <= 0.0f || of.flameM <= 0.05f)
            continue;
        const float cy = of.y + 0.4f * of.flameM;
        const float dx = x - of.x, dy = y - cy, dz = z - of.z;
        const float r = std::max(std::sqrt(dx * dx + dy * dy + dz * dz), std::max(std::max(of.halfL, of.halfW), 0.5f));
        q += kObjectRadiantShare * 1000.0f * of.heatMW / (4.0f * 3.14159265f * r * r);
    }
    return q;
}

// ---- Wrecks (fire-objects) ----
namespace
{
// What the wreck code reads off a destroyed vehicle: its class's config
// (FireObjects.hpp WreckClassOf: CfgVehicles transportFuel, armor, its shape's
// length, its mass; tracked or an aircraft), the fuel in its tank and a
// tanker's load (l). Not a wreck the fire burns: not a vehicle, a ship, or
// over the water.
struct WreckRead
{
    bool ok = false;
    WreckConfig cfg;
    float fuelL = 0.0f, loadL = 0.0f;
};
// The wreck's body in its own frame: its geometry level's box (the hull the
// game collides with), else its first level's; the LOD shape's own box spans
// every level and its memory points (a jeep's came out 4 x 2.8 x 3.2 m).
bool WreckBounds(const LODShape* sh, Vector3& lo, Vector3& hi)
{
    if (!sh)
        return false;
    const Shape* lv = sh->GeometryLevel();
    if (!lv || lv->NVertex() < 4)
        lv = sh->NLevels() > 0 ? sh->Level(0) : nullptr;
    if (lv && lv->NVertex() >= 4)
    {
        lo = lv->Min();
        hi = lv->Max();
    }
    else
    {
        lo = sh->Min();
        hi = sh->Max();
    }
    return hi.X() > lo.X() && hi.Z() > lo.Z();
}
WreckRead ReadWreck(const Object* o)
{
    WreckRead r;
    const Transport* t = dynamic_cast<const Transport*>(o);
    if (!t || !GLandscape || !o->GetShape() || dynamic_cast<const Ship*>(o))
        return r;
    const Vector3 p = o->Position();
    if (GLandscape->SurfaceY(p.X(), p.Z()) < GLandscape->GetSeaLevelNoWave() + 0.3f)
        return r;
    Vector3 lo, hi;
    if (!WreckBounds(o->GetShape(), lo, hi))
        return r;
    const float len = (hi.Z() - lo.Z()) * o->Scale();
    const bool tracked = dynamic_cast<const Tank*>(o) != nullptr;
    const bool air = dynamic_cast<const Helicopter*>(o) != nullptr || dynamic_cast<const Airplane*>(o) != nullptr;
    r.fuelL = std::max(t->GetFuel(), 0.0f);
    r.loadL = std::max(t->GetFuelCargo(), 0.0f);
    r.cfg.cls = air ? WreckClass::Truck : WreckClassOf(r.loadL, tracked, t->GetArmor(), len, o->GetMass());
    r.cfg.fuelBurning = r.fuelL > 1.0f || r.cfg.cls == WreckClass::Tanker;
    r.cfg.ammo = t->GetAmmoHit() > 0.0f || t->GetAmmoCargo() > 0.0f;
    r.cfg.fuelCargoL = r.loadL;
    r.cfg.sizeK = dynamic_cast<const Motorcycle*>(o) != nullptr ? 0.5f : 1.0f;
    r.ok = true;
    return r;
}
// A seed from a point (a wreck's look and its draws).
std::uint32_t PointSeed(float x, float z)
{
    std::uint32_t h = static_cast<std::uint32_t>(static_cast<int>(std::floor(x * 4.0f))) * 0x8da6b343U ^
                      static_cast<std::uint32_t>(static_cast<int>(std::floor(z * 4.0f))) * 0xd8163841U;
    h ^= h >> 13;
    h *= 0x5bd1e995U;
    h ^= h >> 15;
    return h ? h : 1U;
}
} // namespace

void FireWorld::WreckBox(Wreck& w, const Object* o)
{
    Vector3 lo, hi;
    if (!o || !GLandscape || !WreckBounds(o->GetShape(), lo, hi))
        return;
    const float sc = o->Scale();
    const Vector3 c = o->PositionModelToWorld((lo + hi) * 0.5f);
    const Vector3 dir = o->Direction();
    const float dl = std::sqrt(dir.X() * dir.X() + dir.Z() * dir.Z());
    w.ux = dl > 1e-3f ? dir.X() / dl : 0.0f;
    w.uz = dl > 1e-3f ? dir.Z() / dl : 1.0f;
    w.x = c.X();
    w.z = c.Z();
    w.groundY = GLandscape->SurfaceY(w.x, w.z);
    w.halfL = std::clamp(0.5f * (hi.Z() - lo.Z()) * sc, 0.5f, 8.0f);
    w.halfW = std::clamp(0.5f * (hi.X() - lo.X()) * sc, 0.4f, 2.5f);
    w.heightM = std::clamp((hi.Y() - lo.Y()) * sc, 0.8f, 4.0f);
    w.aboveM = std::max(c.Y() - 0.5f * w.heightM - w.groundY, 0.0f);
    w.diamM = std::sqrt(16.0f * w.halfL * w.halfW / 3.14159265f);
}

ObjectFire FireWorld::WreckBodyOf(const Wreck& w) const
{
    ObjectFire f;
    f.id = w.id << 3;
    f.owner = kObjectOwnerWreck;
    f.shape = ObjectFireShape::Box;
    f.x = w.x;
    f.z = w.z;
    f.groundY = w.groundY;
    f.y = w.groundY + w.aboveM + kWreckFootShare * w.heightM;
    f.ux = w.ux;
    f.uz = w.uz;
    // its flames over most of its plan (not its very ends)
    f.halfL = kWreckFlameShare * w.halfL;
    f.halfW = kWreckFlameShare * w.halfW;
    // a fuel and plastics fire: dense, sooty, deep orange
    f.absorb = kWreckAbsorb;
    f.soot = 1.0f;
    f.tempK = kWreckTempK;
    return f;
}

void FireWorld::WreckFireball(Wreck& w, float ballR)
{
    if (!_grid || ballR <= 0.0f || w.aboveM > ballR)
        return;
    const WreckDay day = WreckDayOf(_grid->GetParams().moisture.m1h);
    const ObjectFire body = WreckBodyOf(w);
    WreckFireballDraws(
        w.tl, day, body, ballR, w.rng, [&](float x, float z, float chance, float delay, ObjectIgniteKind kind)
        { return ObjectLight(x, z, chance, delay, kind, body.id); },
        [&](float x, float z, float r)
        {
            const int n = _grid->IgniteArea(x, z, r);
            _objLit[static_cast<int>(ObjectIgniteKind::Fireball)] += n > 0 ? 1 : 0;
            w.lit[static_cast<int>(ObjectIgniteKind::Fireball)] += n;
            if (n > 0 && w.firstLitAge < 0.0f)
                w.firstLitAge = 0.0f;
            LOG_INFO(Script, "UDFIRE;event=objlit;id={};cls={};kind=ring;r={:.1f};cells={}", w.id,
                     WreckClassName(w.tl.cls), r, n);
        });
    // (buildings builder) the buildings the fireball touches: a shed or a barn
    // lights, a house through its windows and eaves (BuildingHeat: a car's
    // 0.25, a tanker's 0.7, a tanker's heat out to 2 radii at half that)
    BuildingHeat(w.x, w.z, ballR);
}

bool FireWorld::AddWreck(const Object* o, float ballR)
{
    const WreckRead rd = ReadWreck(o);
    if (!rd.ok)
        return false;
    Ensure();
    if (static_cast<int>(_wrecks.size()) >= kWrecksMax)
    {
        // the one furthest burnt out goes
        std::size_t oldest = 0;
        for (std::size_t i = 1; i < _wrecks.size(); ++i)
            if (_wrecks[i].born < _wrecks[oldest].born)
                oldest = i;
        _wrecks.erase(_wrecks.begin() + static_cast<std::ptrdiff_t>(oldest));
    }
    Wreck w;
    w.obj = const_cast<Object*>(o);
    w.hadObject = true;
    w.id = ++_objectIds;
    w.born = _clock;
    WreckBox(w, o);
    const std::uint32_t seed = PointSeed(w.x, w.z) ^ static_cast<std::uint32_t>(w.id * 2654435761ULL);
    w.rng = ObjRand(seed ^ 0x7f4a7c15U);
    w.tl = WreckTimelineOf(rd.cfg, seed);
    _wrecks.push_back(w);
    Wreck& nw = _wrecks.back();
    LOG_INFO(Script,
             "UDFIRE;event=wreck;id={};cls={};fuel={:.0f};load={:.0f};ammo={};x={:.1f};z={:.1f};len={:.1f};"
             "width={:.1f};h={:.1f};ball={:.1f};fuelFlame={:.1f}/{:.0f}s;body={:.1f}@{:.0f}s;flameEnd={:.0f};"
             "smokeEnd={:.0f};pool={:.0f}m2/{:.1f}m/{:.0f}s;jets={};blasts={};m1h={:.3f}",
             nw.id, WreckClassName(nw.tl.cls), rd.fuelL, rd.loadL, nw.tl.ammo ? 1 : 0, nw.x, nw.z, 2.0f * nw.halfL,
             2.0f * nw.halfW, nw.heightM, ballR, nw.tl.fuelFlameM, nw.tl.fuelSec, nw.tl.bodyFlameM, nw.tl.bodyPeakSec,
             nw.tl.flameEndSec, nw.tl.smokeEndSec, nw.tl.poolAreaM2, nw.tl.poolFlameM, nw.tl.poolSec, nw.tl.jets,
             nw.tl.blasts, _grid ? _grid->GetParams().moisture.m1h : 0.0f);
    nw.ballR = ballR;
    WreckFireball(nw, ballR);
    return true;
}

std::uint64_t FireWorld::AddTestWreck(float x, float z, WreckClass cls, float dirRad, bool fuel, bool ammo, float ballR,
                                      float firstJetSec)
{
    if (!GLandscape)
        return 0;
    Ensure();
    Wreck w;
    w.id = ++_objectIds;
    w.born = _clock;
    w.x = x;
    w.z = z;
    w.groundY = GLandscape->SurfaceY(x, z);
    w.ux = std::sin(dirRad);
    w.uz = std::cos(dirRad);
    switch (cls)
    {
        case WreckClass::Truck:
        case WreckClass::Tanker:
            w.halfL = 3.6f;
            w.halfW = 1.25f;
            w.heightM = 3.0f;
            break;
        case WreckClass::Armour:
            w.halfL = 3.4f;
            w.halfW = 1.6f;
            w.heightM = 2.3f;
            break;
        default:
            w.halfL = 2.1f;
            w.halfW = 0.9f;
            w.heightM = 1.6f;
            break;
    }
    w.diamM = std::sqrt(16.0f * w.halfL * w.halfW / 3.14159265f);
    WreckConfig cfg;
    cfg.cls = cls;
    cfg.fuelBurning = fuel || cls == WreckClass::Tanker;
    cfg.ammo = ammo;
    cfg.fuelCargoL = cls == WreckClass::Tanker ? 5000.0f : 0.0f;
    const std::uint32_t seed = PointSeed(x, z) ^ static_cast<std::uint32_t>(w.id * 2654435761ULL);
    w.rng = ObjRand(seed ^ 0x7f4a7c15U);
    w.tl = WreckTimelineOf(cfg, seed);
    if (firstJetSec >= 0.0f && w.tl.jets > 0)
    {
        // (a test aid for filming: the jets' schedule moved so the first flares at firstJetSec)
        const float shift = firstJetSec - w.tl.jetAt[0];
        for (int i = 0; i < w.tl.jets; ++i)
            w.tl.jetAt[i] = std::max(w.tl.jetAt[i] + shift, 0.0f);
    }
    if (static_cast<int>(_wrecks.size()) >= kWrecksMax)
        _wrecks.erase(_wrecks.begin());
    _wrecks.push_back(w);
    Wreck& nw = _wrecks.back();
    if (ballR < 0.0f)
    {
        // its class's own fireball (WreckFireballKg; armour the stock's 150 kg at most)
        const float kg = WreckFireballKg(cls, lighting::kMaxFuelKg, nw.rng.U(), fuel ? 60.0f : 0.0f, cfg.fuelCargoL);
        ballR = kg >= lighting::kMinFuelKg
                    ? lighting::DeflagrationFromFuel(kg, cls == WreckClass::Tanker ? kTankerBallMaxKg
                                                                                   : lighting::kMaxFuelKg)
                          .radius
                    : 0.0f;
    }
    LOG_INFO(Script,
             "UDFIRE;event=wreck;id={};cls={};test=1;ammo={};x={:.1f};z={:.1f};ball={:.1f};fuelFlame={:.1f}/{:.0f}s;"
             "body={:.1f}@{:.0f}s;flameEnd={:.0f};smokeEnd={:.0f};pool={:.0f}m2/{:.1f}m/{:.0f}s;jets={};blasts={};"
             "m1h={:.3f}",
             nw.id, WreckClassName(cls), nw.tl.ammo ? 1 : 0, x, z, ballR, nw.tl.fuelFlameM, nw.tl.fuelSec,
             nw.tl.bodyFlameM, nw.tl.bodyPeakSec, nw.tl.flameEndSec, nw.tl.smokeEndSec, nw.tl.poolAreaM2,
             nw.tl.poolFlameM, nw.tl.poolSec, nw.tl.jets, nw.tl.blasts, _grid ? _grid->GetParams().moisture.m1h : 0.0f);
    nw.ballR = ballR;
    WreckFireball(nw, ballR);
    return nw.id;
}

// (followups) At most this many wreck looks are kept (WreckLook).
constexpr std::size_t kWreckLooksMax = 256;

bool FireWorld::WreckLookOf(const Object* obj, float& charK, float& glow) const
{
    const auto it = _wreckLooks.find(obj);
    if (it == _wreckLooks.end() || it->second.obj.GetLink() != obj)
        return false;
    charK = it->second.charK;
    glow = it->second.glow;
    return true;
}

void FireWorld::UpdateWrecks(float dt)
{
    // (followups) a look whose vehicle is gone goes
    for (auto it = _wreckLooks.begin(); it != _wreckLooks.end();)
        it = it->second.obj.GetLink() != it->first ? _wreckLooks.erase(it) : std::next(it);
    if (_wrecks.empty() || !_grid)
        return;
    const WreckDay day = WreckDayOf(_grid->GetParams().moisture.m1h); // (rain wets it: Simulate's moisture)
    for (std::size_t n = 0; n < _wrecks.size();)
    {
        Wreck& w = _wrecks[n];
        float fadeK = 1.0f;
        if (w.hadObject)
        {
            Object* o = w.obj.GetLink();
            if (o && o->GetShape())
                WreckBox(w, o);
            else
            {
                w.gone += dt;
                fadeK = 1.0f - w.gone / kWreckGoneSec;
            }
        }
        w.age += static_cast<double>(dt) * _objTimeK;
        const float age = static_cast<float>(w.age);
        const WreckState s = WreckStateAt(w.tl, age, w.diamM);
        // (followups) its look for the renderer: its char and its shell's
        // glow while it burns; once its fire is over its char stays
        if (Object* o = w.hadObject ? w.obj.GetLink() : nullptr)
        {
            const bool over = s.done || fadeK <= 0.0f;
            if (!over && _wreckLooks.size() >= kWreckLooksMax && _wreckLooks.find(o) == _wreckLooks.end())
                for (auto it = _wreckLooks.begin(); it != _wreckLooks.end(); ++it)
                    if (it->second.glow <= 0.0f)
                    {
                        _wreckLooks.erase(it);
                        break;
                    }
            if (over || _wreckLooks.size() < kWreckLooksMax || _wreckLooks.count(o))
            {
                WreckLook& look = _wreckLooks[o];
                look.obj = o;
                look.charK = s.charK;
                look.glow = over ? 0.0f : s.shellGlow * std::max(fadeK, 0.0f);
            }
        }
        if (s.done || fadeK <= 0.0f)
        {
            LOG_INFO(Script, "UDFIRE;event=wreckdone;id={};cls={};age={:.0f};lit={}/{}/{}/{}/{}/{}/{};first={:.1f}",
                     w.id, WreckClassName(w.tl.cls), age, w.lit[0], w.lit[1], w.lit[2], w.lit[3], w.lit[4], w.lit[5],
                     w.lit[6], w.firstLitAge);
            _wrecks.erase(_wrecks.begin() + static_cast<std::ptrdiff_t>(n));
            continue;
        }
        const bool high = w.aboveM > 3.0f; // (an aircraft's still falling: nothing under it lights)
        // its body
        ObjectFire body = WreckBodyOf(w);
        WreckBodyFire(w.tl, s, day, body);
        body.flameM *= fadeK;
        body.heat *= fadeK;
        body.heatMW *= fadeK;
        body.smoke *= fadeK;
        body.glow *= fadeK;
        body.emberRate *= _objTimeK;
        body.emberDelaySec /= _objTimeK;
        if (body.heatMeanSec > 0.0f)
            body.heatMeanSec /= _objTimeK;
        if (high || fadeK < 1.0f)
            body.heatMeanSec = body.emberRate = 0.0f;
        body.flameRefM = std::max({w.tl.fuelFlameM, w.tl.bodyFlameM, 0.3f});
        body.sparkRate = kWreckSparksPerM * body.flameM * body.heat * (s.emberFuel ? 0.5f : 1.0f) * w.tl.emberK;
        if (body.flameM > 0.01f || body.smoke > 0.01f || body.glow > 0.01f)
            _objectFires.push_back(body);
        // its pool
        if (s.poolAreaM2 > 0.0f)
        {
            ObjectFire pool = WreckBodyOf(w);
            pool.id = (w.id << 3) | 1U;
            pool.y = w.groundY + 0.03f;
            WreckPoolFire(w.tl, s, day, w.halfL, w.halfW, pool);
            pool.flameM *= fadeK;
            pool.heat *= fadeK;
            pool.heatMW *= fadeK;
            pool.smoke *= fadeK;
            pool.absorb = kWreckAbsorb;
            pool.soot = 1.0f;
            pool.tempK = kWreckTempK;
            if (pool.heatMeanSec > 0.0f)
                pool.heatMeanSec /= _objTimeK;
            if (high || fadeK < 1.0f)
                pool.heatMeanSec = 0.0f;
            pool.sparkRate = 0.0f; // (burning fuel makes soot, not embers)
            pool.flameRefM = std::max(w.tl.poolFlameM, 0.3f);
            if (pool.flameM > 0.01f)
                _objectFires.push_back(pool);
        }
        // a propellant jet from a hatch (on its top, toward its front)
        if (s.jetM > 0.01f)
        {
            ObjectFire jet;
            jet.id = (w.id << 3) | 2U;
            jet.owner = kObjectOwnerWreck;
            jet.shape = ObjectFireShape::Jet;
            const float off = (s.jet % 2 == 0 ? 0.25f : -0.2f) * w.halfL;
            jet.x = w.x + w.ux * off;
            jet.z = w.z + w.uz * off;
            jet.groundY = w.groundY;
            jet.y = w.groundY + w.aboveM + w.heightM;
            const float lx = 0.12f * (w.rng.U() - 0.5f), lz = 0.12f * (w.rng.U() - 0.5f);
            const float jl = std::sqrt(lx * lx + 1.0f + lz * lz);
            jet.jetX = lx / jl;
            jet.jetY = 1.0f / jl;
            jet.jetZ = lz / jl;
            jet.flameM = s.jetM * fadeK;
            jet.flameRefM = w.tl.jetM[std::clamp(s.jet, 0, kWreckJetsMax - 1)];
            jet.heat = 1.0f;
            jet.heatMW = kWreckJetMW;
            jet.smoke = 0.3f * kWreckJetMW;
            jet.black = 0.5f;
            jet.absorb = 2.5f;
            jet.soot = 0.15f;
            jet.tempK = kWreckJetTempK;
            jet.sparkRate = kWreckJetSparks;
            jet.hurtM = 1.0f;
            _objectFires.push_back(jet);
        }
        // its fuel fireball, through the flame pass
        {
            const BallState b = FireballAt(w.ballR, static_cast<float>(_clock - w.born));
            if (b.on)
            {
                ObjectFire ball;
                ball.id = (w.id << 3) | 4U;
                ball.owner = kObjectOwnerWreck;
                ball.shape = ObjectFireShape::Ball;
                ball.x = w.x;
                ball.z = w.z;
                ball.groundY = w.groundY;
                ball.y = w.groundY + w.aboveM + b.riseM;
                ball.halfL = ball.halfW = b.radiusM;
                ball.flameM = b.radiusM;
                ball.flameRefM = w.ballR;
                ball.heat = b.heat;
                ball.absorb = kWreckAbsorb;
                ball.soot = b.soot;
                ball.tempK = kWreckBallTempK;
                _objectFires.push_back(ball);
            }
        }
        // a high-explosive blast: its fragments, and a flash
        if (w.blastsDone < w.tl.blasts && age >= w.tl.blastAt[w.blastsDone])
        {
            if (!high)
                WreckBlastDraws(day, w.x, w.z, w.rng,
                                [&](float x, float z, float chance, float delay, ObjectIgniteKind kind)
                                { return ObjectLight(x, z, chance, delay, kind, w.id << 3); });
            w.flashUntil = _clock + kWreckFlashSec;
            LOG_INFO(Script, "UDFIRE;event=wreckblast;id={};age={:.0f}", w.id, age);
            w.blastsDone++;
        }
        if (w.flashUntil > _clock)
        {
            const float k = static_cast<float>((w.flashUntil - _clock) / kWreckFlashSec);
            ObjectFire fl;
            fl.id = (w.id << 3) | 3U;
            fl.owner = kObjectOwnerWreck;
            fl.shape = ObjectFireShape::Jet;
            fl.x = w.x;
            fl.z = w.z;
            fl.groundY = w.groundY;
            fl.y = w.groundY + w.aboveM + 0.6f * w.heightM;
            fl.flameM = kWreckFlashM * std::sqrt(k);
            fl.flameRefM = kWreckFlashM;
            fl.heat = 1.0f;
            fl.heatMW = 4.0f * kWreckJetMW;
            fl.smoke = kWreckJetMW;
            fl.black = 0.7f;
            fl.absorb = 3.0f;
            fl.soot = 0.4f;
            fl.tempK = kWreckJetTempK;
            fl.sparkRate = 4.0f * kWreckJetSparks * k;
            _objectFires.push_back(fl);
        }
        ++n;
    }
}

std::string FireWorld::WrecksLine() const
{
    std::string out;
    char b[400];
    for (const Wreck& w : _wrecks)
    {
        const float age = static_cast<float>(w.age);
        const WreckState s = WreckStateAt(w.tl, age, w.diamM);
        std::snprintf(b, sizeof(b),
                      "%s%llu,%s,age=%.1f,x=%.1f,z=%.1f,flame=%.2f,mw=%.2f,pool=%.1f/%.0f,smoke=%.1f,black=%.2f,"
                      "ember=%.4f,jet=%.1f,glow=%.2f,lit=%d/%d/%d/%d/%d/%d/%d,first=%.1f,char=%.2f,shell=%.2f",
                      out.empty() ? "" : ";", static_cast<unsigned long long>(w.id), WreckClassName(w.tl.cls), age, w.x,
                      w.z, s.bodyFlameM, s.bodyMW + s.poolMW, s.poolFlameM, s.poolAreaM2, s.smoke, s.black, s.emberRate,
                      s.jetM, s.glow, w.lit[0], w.lit[1], w.lit[2], w.lit[3], w.lit[4], w.lit[5], w.lit[6],
                      w.firstLitAge, s.charK, s.shellGlow);
        out += b;
    }
    return out;
}

// --- script commands -------------------------------------------------------

bool ReadXZ(GameValuePar v, float& x, float& z, float* r)
{
    if (v.GetType() != GameArray)
        return false;
    const GameArrayType& a = v;
    if (a.Size() < 2 || a[0].GetType() != GameScalar || a[1].GetType() != GameScalar)
        return false;
    x = a[0];
    // [x, z] or a position [x, z, y] / [x, y, z]: Arma positions are [x, z(north), y(up)].
    z = a[1];
    if (r && a.Size() >= 3 && a[2].GetType() == GameScalar)
        *r = a[2];
    return true;
}

// fireSpreadIgnite [x, z] (a point), [x, z, r] (an area) or [[x0, z0], [x1,
// z1]] (a line; kMatureLineM or longer starts at its equilibrium rate).
GameValue FireSpreadIgnite(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f, r = 0.0f;
    if (oper1.GetType() == GameArray)
    {
        const GameArrayType& a = oper1;
        float x1 = 0.0f, z1 = 0.0f;
        if (a.Size() == 2 && ReadXZ(a[0], x, z, nullptr) && ReadXZ(a[1], x1, z1, nullptr))
        {
            const int n = FireWorld::Get().Ensure().IgniteLine(x, z, x1, z1);
            LOG_INFO(Script, "UDFIRE;event=ignite_line;x0={:.1f};z0={:.1f};x1={:.1f};z1={:.1f};cells={}", x, z, x1, z1,
                     n);
            return GameValue(static_cast<float>(n));
        }
    }
    if (!ReadXZ(oper1, x, z, &r))
        return GameValue(0.0f);
    FireGrid& g = FireWorld::Get().Ensure();
    const int n = r > 0.0f ? g.IgniteArea(x, z, r) : (g.Ignite(x, z) ? 1 : 0);
    LOG_INFO(Script, "UDFIRE;event=ignite;x={:.1f};z={:.1f};r={:.1f};cells={}", x, z, r, n);
    return GameValue(static_cast<float>(n));
}

// fireSpreadSet [name, value]: "m1h" (fine fuel moisture, fraction), "mlive",
// "wind" ([x, z] m/s: a 10 m wind instead of the weather's; [] gives it
// back), "flames" and "smoke" (puffs a second), "buildup" (per minute),
// "wander" (the wind's swing, deg).
GameValue FireSpreadSet(const GameState* state, GameValuePar oper1)
{
    if (oper1.GetType() != GameArray)
        return GameValue(false);
    const GameArrayType& a = oper1;
    if (a.Size() < 2 || a[0].GetType() != GameString)
        return GameValue(false);
    const RString name = a[0];
    FireWorld& w = FireWorld::Get();
    w.Ensure();
    if (!strcmpi(name, "wind"))
    {
        float x = 0.0f, z = 0.0f;
        if (ReadXZ(a[1], x, z, nullptr))
            w.SetWindOverride(true, x, z);
        else
            w.SetWindOverride(false, 0.0f, 0.0f);
        return GameValue(true);
    }
    if (a[1].GetType() != GameScalar)
        return GameValue(false);
    const float v = a[1];
    Moisture& m = w.MoistureRef();
    if (!strcmpi(name, "m1h"))
    {
        m.m1h = v;
        m.m10h = v + 0.01f;
        m.m100h = v + 0.02f;
    }
    else if (!strcmpi(name, "mlive"))
        m.mLive = v;
    else if (!strcmpi(name, "flames"))
        w.SetBudgets(v, EnvFloat("POSEIDON_XP_FIRESPREAD_SMOKE", 16.0f));
    else if (!strcmpi(name, "smoke"))
        w.SetBudgets(EnvFloat("POSEIDON_XP_FIRESPREAD_FLAMES", 60.0f), v);
    else if (!strcmpi(name, "buildup"))
    {
        // The build-up's rate (per minute; 0: none), FireSpread.hpp kBuildUpPerMin.
        w.Grid()->SetBuildUpPerMin(v);
        return GameValue(true);
    }
    else if (!strcmpi(name, "objtime"))
    {
        // (buildings builder) a test aid: the object fires' clocks this many
        // times faster (1 real time; POSEIDON_XP_FIRESPREAD_OBJTIME)
        w.SetObjTime(v);
        return GameValue(true);
    }
    else if (!strcmpi(name, "plantmax"))
    {
        // (end-state agent) a test aid: the plant table's largest size
        w.SetPlantMax(static_cast<int>(v));
        return GameValue(true);
    }
    else if (!strcmpi(name, "bseed"))
    {
        // (buildings builder) the buildings' draws' seed (repeated trials);
        // (followups) fixed from now on, a negative one hands it back to the
        // game (a new one each game, one now)
        s_bldSeedFixed = v >= 0.0f;
        if (s_bldSeedFixed)
        {
            s_bldRunSeed = static_cast<std::uint32_t>(v);
            LOG_INFO(Script, "UDFIRE;event=bseed;seed={};fixed=1", s_bldRunSeed);
        }
        else
            RollBuildingRunSeed();
        return GameValue(true);
    }
    else if (!strcmpi(name, "keeppass"))
    {
        // (smoketrim) a test aid: the world pass's set-up kept (1, the
        // default) or done again (0) at each return from the screen pass
        s_keepPass = v > 0.5f;
        LOG_INFO(Script, "UDFIRE;event=keeppass;on={}", s_keepPass ? 1 : 0);
        return GameValue(true);
    }
    else if (!strcmpi(name, "wreckglow"))
    {
        // (followups) a test aid: the wrecks' hot shell glow's light this
        // many times its own (WreckGlowTestScale; 1 its own)
        s_wreckGlowScale = std::max(v, 0.0f);
        LOG_INFO(Script, "UDFIRE;event=wreckglow;scale={:.3f}", s_wreckGlowScale);
        return GameValue(true);
    }
    else if (!strcmpi(name, "wander"))
    {
        // The wind's wander (deg, standard deviation over minutes; 0: a fixed
        // wind), FireSpread.hpp kWindWanderDeg.
        w.SetWander(v);
        return GameValue(true);
    }
    else
        return GameValue(false);
    w.Grid()->SetMoisture(m);
    return GameValue(true);
}

// (buildings builder) fireSpreadBuilding [[x, z], "state" | "ignite" | "igniteroof"]:
// the building nearest (x, z) (within 30 m): its kind, box, fire and look
// now as a string; "ignite" sets it alight now (its first flame inside:
// room first; "igniteroof" in its roof), a test aid.
GameValue FireSpreadBuildingCmd(const GameState* state, GameValuePar oper1)
{
    if (oper1.GetType() != GameArray)
        return GameValue("none");
    const GameArrayType& a = oper1;
    float x = 0.0f, z = 0.0f;
    if (a.Size() < 1 || !ReadXZ(a[0], x, z, nullptr))
        return GameValue("none");
    RString mode = a.Size() > 1 && a[1].GetType() == GameString ? RString(a[1]) : RString("state");
    const bool ignite = !strcmpi(mode, "ignite") || !strcmpi(mode, "igniteroof");
    const std::string r = FireWorld::Get().BuildingCmd(x, z, ignite, !strcmpi(mode, "igniteroof"));
    return GameValue(RString(r.c_str()));
}
// fireSpreadBuildingTime [[x, z], "tb" | "flash" | "spread" | "collapse" | "floor" | "down"]:
// the building nearest (x, z): its fire's clock or a moment of its timeline (s), a test aid for the scenes.
GameValue FireSpreadBuildingTimeCmd(const GameState* state, GameValuePar oper1)
{
    if (oper1.GetType() != GameArray)
        return GameValue(-1.0f);
    const GameArrayType& a = oper1;
    float x = 0.0f, z = 0.0f;
    if (a.Size() < 2 || !ReadXZ(a[0], x, z, nullptr) || a[1].GetType() != GameString)
        return GameValue(-1.0f);
    const RString what = a[1];
    return GameValue(FireWorld::Get().BuildingTime(x, z, what));
}
// fireSpreadBuildings [x, z, r]: registers the buildings within r of (x, z)
// (each logged: UDFIRE;event=building) and counts them by kind.
GameValue FireSpreadBuildingsCmd(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f, r = 0.0f;
    if (!ReadXZ(oper1, x, z, &r))
        return GameValue("none");
    const std::string s = FireWorld::Get().BuildingsScan(x, z, std::max(r, 1.0f));
    return GameValue(RString(s.c_str()));
}

GameValue FireSpreadStats(const GameState* state)
{
    return GameValue(RString(FireWorld::Get().StatsLine().c_str()));
}

GameValue FireSpreadClear(const GameState* state)
{
    FireWorld::Get().Clear();
    return GameValue(true);
}

// fireSpreadFuel [x, z]: the Anderson fuel model there, 100 added under a
// canopy, 1000 more when a conifer (the kind that torches) stands there (debug).
GameValue FireSpreadFuel(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f;
    if (!ReadXZ(oper1, x, z, nullptr))
        return GameValue(-1.0f);
    const Fuel f = FireWorld::Get().FuelProbe(x, z);
    return GameValue(static_cast<float>(f.model + (f.canopy ? 100 : 0) +
                                        (f.canopy && f.conifer && f.treeHeightM > 0.0f ? 1000 : 0)));
}

// fireSpreadSight [[x, y, z], [x, y, z]] (ASL, engine order x, up, z): smoke transmittance (debug).
GameValue FireSpreadSightCmd(const GameState* state, GameValuePar oper1)
{
    if (oper1.GetType() != GameArray)
        return GameValue(1.0f);
    const GameArrayType& a = oper1;
    if (a.Size() < 2 || a[0].GetType() != GameArray || a[1].GetType() != GameArray)
        return GameValue(1.0f);
    const GameArrayType& p = a[0];
    const GameArrayType& q = a[1];
    if (p.Size() < 3 || q.Size() < 3)
        return GameValue(1.0f);
    const Vector3 from(static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2]));
    const Vector3 to(static_cast<float>(q[0]), static_cast<float>(q[1]), static_cast<float>(q[2]));
    return GameValue(SmokeSight(from, to));
}

// fireSpreadCell [x, z] (debug, the flame measurements): the cell there as
// "state=..;model=..;tree=..;shrub=..;flameMax=..;flame=..;torch=..;torchMax=..;torchAge=.." (m, s).
GameValue FireSpreadCell(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f;
    FireGrid* g = FireWorld::Get().Grid();
    BurningView v;
    CellState st = CellState::Unknown;
    if (!g || !ReadXZ(oper1, x, z, nullptr) || !g->ViewAt(x, z, v, &st))
        return GameValue("state=none");
    char buf[256];
    std::snprintf(
        buf, sizeof(buf),
        "state=%d;model=%d;tree=%.2f;shrub=%.2f;flameMax=%.2f;flame=%.2f;torch=%.2f;torchMax=%.2f;torchAge=%.1f",
        static_cast<int>(st), static_cast<int>(v.model), v.treeH, v.shrubH, v.flameMaxM, v.flameM, v.torchM,
        v.torchMaxM, v.torchAge);
    return GameValue(RString(buf));
}

// fireSpreadTree [x, z] (debug): the height (m) of the tallest tree standing
// in the 8 m cell round the point, as the fire's fuel sees it (0: none).
GameValue FireSpreadTree(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f;
    if (!ReadXZ(oper1, x, z, nullptr))
        return GameValue(0.0f);
    const float cell = 8.0f;
    const Fuel f =
        FireWorld::Get().FuelProbe((std::floor(x / cell) + 0.5f) * cell, (std::floor(z / cell) + 0.5f) * cell);
    return GameValue(f.treeHeightM);
}

// fireSpreadPlants [x, z, r] (debug): every tree, small tree and bush within
// r metres (default 30, at most kPlantsMax of them), as
// "kind:shape:height:x:z:radius:model;..." with kind tree, small or bush,
// height and radius (its shape's vertical and horizontal extent, scaled) in
// metres, and the fuel model of the 8 m cell it stands in (100 added under a
// canopy, as fireSpreadFuel). Each is logged too (UDFIRE;event=plant), the
// list being longer than a script's string can carry.
GameValue FireSpreadPlants(const GameState* state, GameValuePar oper1)
{
    constexpr int kPlantsMax = 240;
    float x = 0.0f, z = 0.0f, r = 30.0f;
    if (!ReadXZ(oper1, x, z, &r) || !GLandscape)
        return GameValue(RString(""));
    std::string out;
    int n = 0;
    const int gx0 = static_cast<int>(std::floor((x - r) * InvLandGrid)),
              gx1 = static_cast<int>(std::floor((x + r) * InvLandGrid));
    const int gz0 = static_cast<int>(std::floor((z - r) * InvLandGrid)),
              gz1 = static_cast<int>(std::floor((z + r) * InvLandGrid));
    for (int cz = gz0; cz <= gz1; ++cz)
        for (int cx = gx0; cx <= gx1; ++cx)
        {
            if (!InRange(cz, cx))
                continue;
            const ObjectList& list = GLandscape->GetObjects(cz, cx);
            for (int i = 0; i < list.Size() && n < kPlantsMax; ++i)
            {
                Object* obj = list[i];
                if (!obj || !obj->GetShape())
                    continue;
                const LODShape* shape = obj->GetShape();
                const MapType mt = shape->GetMapType();
                if (mt != MapTree && mt != MapSmallTree && mt != MapBush)
                    continue;
                const Vector3 pos = obj->Position();
                if ((pos.X() - x) * (pos.X() - x) + (pos.Z() - z) * (pos.Z() - z) > r * r)
                    continue;
                const float h = (shape->Max().Y() - shape->Min().Y()) * obj->Scale();
                const float rad = 0.25f *
                                  ((shape->Max().X() - shape->Min().X()) + (shape->Max().Z() - shape->Min().Z())) *
                                  obj->Scale();
                const float cell = 8.0f;
                const Fuel f = FireWorld::Get().FuelProbe((std::floor(pos.X() / cell) + 0.5f) * cell,
                                                          (std::floor(pos.Z() / cell) + 0.5f) * cell);
                char buf[256];
                std::snprintf(buf, sizeof(buf), "%s:%s:%.2f:%.1f:%.1f:%.2f:%d;",
                              mt == MapTree ? "tree" : (mt == MapSmallTree ? "small" : "bush"),
                              (const char*)shape->GetName(), h, pos.X(), pos.Z(), rad, f.model + (f.canopy ? 100 : 0));
                LOG_INFO(Script, "UDFIRE;event=plant;{}", buf);
                out += buf;
                ++n;
            }
        }
    return GameValue(RString(out.c_str()));
}

// (fire-objects, a test aid) fireSpreadWreck [x, z, class, dirDeg, fuel,
// ammo, ballR, firstJetSec]: a wreck's fire with no vehicle at (x, z)
// ("car", "truck", "tanker" or "armour"; facing dirDeg; its fuel burning (1,
// the default) or not (0); armour with ammunition (1) or not; its fireball's
// radius (m; -1 its class's own, the default; 0 none); armour's first
// propellant jet moved to firstJetSec, a filming aid (-1, the default: its
// own schedule)). Its id (0: the fire is off).
GameValue FireSpreadWreck(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f;
    if (!ReadXZ(oper1, x, z, nullptr))
        return GameValue(0.0f);
    const GameArrayType& a = oper1;
    WreckClass cls = WreckClass::Car;
    if (a.Size() >= 3 && a[2].GetType() == GameString)
    {
        const RString c = a[2];
        if (!strcmpi(c, "truck"))
            cls = WreckClass::Truck;
        else if (!strcmpi(c, "tanker"))
            cls = WreckClass::Tanker;
        else if (!strcmpi(c, "armour") || !strcmpi(c, "armor"))
            cls = WreckClass::Armour;
    }
    auto num = [&](int i, float def)
    { return a.Size() > i && a[i].GetType() == GameScalar ? static_cast<float>(a[i]) : def; };
    const float dir = num(3, 0.0f) * 0.01745329f;
    const bool fuel = num(4, 1.0f) > 0.5f, ammo = num(5, 0.0f) > 0.5f;
    const float ballR = num(6, -1.0f), firstJet = num(7, -1.0f);
    return GameValue(static_cast<float>(FireWorld::Get().AddTestWreck(x, z, cls, dir, fuel, ammo, ballR, firstJet)));
}

// fireSpreadWrecks: the burning wrecks, ';' between them (WrecksLine).
GameValue FireSpreadWrecks(const GameState* state)
{
    return GameValue(RString(FireWorld::Get().WrecksLine().c_str()));
}

// fireSpreadState [x, z]: "none", "unburnt", "burning" or "burnt".
GameValue FireSpreadState(const GameState* state, GameValuePar oper1)
{
    float x = 0.0f, z = 0.0f;
    FireGrid* g = FireWorld::Get().Grid();
    if (!g || !ReadXZ(oper1, x, z, nullptr))
        return GameValue("none");
    switch (g->StateAt(x, z))
    {
        case CellState::Unburnt:
            return GameValue("unburnt");
        case CellState::Burning:
            return GameValue("burning");
        case CellState::Burnt:
            return GameValue("burnt");
        default:
            return GameValue("none");
    }
}
} // namespace

// ---- Buildings (buildings builder): the pure laws (FireSpreadWorld.hpp) ----
BuildingKind BuildingKindOf(int mapType, const char* lowerName)
{
    const std::string n = lowerName ? lowerName : "";
    auto has = [&](const char* s) { return n.find(s) != std::string::npos; };
    // (Abel's: dumruina a ruin, garaz a concrete army garage, repair_center
    // a concrete workshop)
    if (mapType == 3 || has("bunk") || has("panel") || has("beton") || has("concrete") || has("hangar") ||
        has("silo") || has("ruin") || has("garaz") || has("repair"))
        return BuildingKind::Never;
    if (mapType == 6)
        return has("newplot") || has("statek_plot") || has("ohrad") || has("provizor") || has("zboren")
                   ? BuildingKind::Fence
                   : BuildingKind::Never;
    if (mapType == 4 || has("benzin") || has("pumpa") || has("fuel"))
        return BuildingKind::FuelStation;
    if (mapType == 1 || has("kostel") || has("church"))
        return BuildingKind::Church;
    if (mapType == 5 || has("nemocnic") || has("hospital"))
        return BuildingKind::Masonry;
    if (has("stodol") || has("seno") || has("senik") || has("barn") || has("stoh"))
        return BuildingKind::Barn;
    if (has("kulna") || has("bouda") || has("kurnik") || has("drevnik") || has("shed") || has("chata") ||
        has("plech") || has("dreven"))
        return BuildingKind::Shed;
    return BuildingKind::StoneHouse;
}

bool BuildingWallsStand(const char* lowerName)
{
    const std::string n = lowerName ? lowerName : "";
    return n.find("statek_kulna") != std::string::npos || n.find("stodol") != std::string::npos;
}

std::uint32_t BuildingRunSeedFor(std::uint32_t generation, bool deterministic, std::uint64_t entropy)
{
    // splitmix64 over the roll's number, with the clock's bits outside a
    // deterministic world
    std::uint64_t h = 0x9e3779b97f4a7c15ULL * (static_cast<std::uint64_t>(generation) + 1ULL);
    h ^= deterministic ? 0x51ed2701d3a7b6c5ULL : entropy * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return static_cast<std::uint32_t>(h ^ (h >> 32));
}

float BuildingDrawFor(std::uint32_t buildingSeed, int k, std::uint32_t runSeed)
{
    return Hash01(buildingSeed * 2654435761U + static_cast<std::uint32_t>(k) * 40503U + runSeed * 2246822519U + 17U);
}

float BuildingPassChance(BuildingKind kind, bool vegAgainst, float pig, float wind10)
{
    if (pig < kBuildingPigMin || kind == BuildingKind::Never)
        return 0.0f;
    float typical = vegAgainst ? 0.075f : 0.02f, worst = vegAgainst ? 0.30f : 0.10f;
    if (kind == BuildingKind::Shed || kind == BuildingKind::Fence)
    {
        typical = 0.15f;
        worst = 0.65f;
    }
    else if (kind == BuildingKind::Barn)
    {
        typical = 0.15f;
        worst = 0.75f;
    }
    else if (kind == BuildingKind::Masonry)
    {
        typical *= 0.5f;
        worst *= 0.5f;
    }
    return std::clamp(DayBlend(typical, worst, FireDayIndex(pig, wind10)), 0.0f, kBuildingChanceMax);
}

float BuildingHeatChance(BuildingKind kind, float gapM, float dayIndex)
{
    if (kind == BuildingKind::Never || gapM >= kHeatWoodM)
        return 0.0f;
    if (kind == BuildingKind::Shed || kind == BuildingKind::Barn || kind == BuildingKind::Fence)
        return 1.0f - Ss(kHeatNearM, kHeatWoodM, gapM);
    const float k = 1.0f - Ss(kHeatNearM, kHeatWoodM, gapM);
    return std::min(DayBlend(kHeatEavesTypical, kHeatEavesWorst, dayIndex), 0.9f) * k *
           (kind == BuildingKind::Masonry ? 0.5f : 1.0f);
}

float BuildingDelaySec(BuildingKind kind, float u)
{
    u = std::clamp(u, 0.0f, 1.0f);
    switch (kind)
    {
        case BuildingKind::Shed:
        case BuildingKind::Barn:
            return 30.0f + 210.0f * u;
        case BuildingKind::Fence: // its foot in the flames
            return 10.0f + 50.0f * u;
        case BuildingKind::FuelStation:
            return 60.0f + 240.0f * u;
        case BuildingKind::Never:
            return 0.0f;
        default: // 5 to 30 min, log-uniform (median about 12 min)
            return 300.0f * std::pow(6.0f, u);
    }
}

BuildingTimeline BuildingTimelineOf(BuildingKind kind, bool roofFirst, const float* u)
{
    auto U = [&](int i, float a, float b) { return a + (b - a) * std::clamp(u ? u[i] : 0.5f, 0.0f, 1.0f); };
    BuildingTimeline t;
    t.kind = kind;
    t.roofFirst = roofFirst;
    constexpr float kHour = 3600.0f, kDay = 86400.0f;
    switch (kind)
    {
        case BuildingKind::Never:
            return t;
        case BuildingKind::Fence:
            t.flashSec = t.spreadSec = U(0, 60.0f, 180.0f);
            t.collapseSec = t.floorSec = U(2, 600.0f, 1800.0f);
            t.downSec = t.collapseSec + U(4, 120.0f, 300.0f);
            t.glowSec = t.smokeSec = U(5, 1200.0f, 2400.0f);
            t.peakM = U(7, 0.8f, 1.5f);
            t.peakMW = 0.3f + 0.3f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            return t;
        case BuildingKind::Shed:
            t.flashSec = t.spreadSec = U(0, 120.0f, 240.0f);
            t.collapseSec = t.floorSec = U(2, 480.0f, 900.0f);
            t.downSec = U(4, 1800.0f, 2700.0f);
            t.glowSec = U(5, 2.0f * kHour, 4.0f * kHour);
            t.smokeSec = t.glowSec;
            t.peakM = U(7, 3.0f, 5.0f);
            t.peakMW = 3.0f + 3.0f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            return t;
        case BuildingKind::Barn:
            t.flashSec = t.spreadSec = U(0, 300.0f, 600.0f);
            t.collapseSec = t.floorSec = U(2, 900.0f, 1800.0f);
            t.downSec = U(4, kHour, 2.0f * kHour);
            t.glowSec = U(5, 6.0f * kHour, 12.0f * kHour);
            t.smokeSec = U(6, 2.0f * kDay, 4.0f * kDay); // the hay smoulders for days
            t.peakM = U(7, 10.0f, 15.0f);
            t.peakMW = 50.0f + 30.0f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            return t;
        case BuildingKind::FuelStation:
            t.flashSec = U(0, 60.0f, 300.0f);
            t.spreadSec = t.flashSec + U(1, 600.0f, 1800.0f); // the pumps' pools burnt out
            t.collapseSec = t.floorSec = std::max(U(2, 1200.0f, 2400.0f), t.spreadSec);
            t.downSec = std::max(U(4, 1800.0f, 3600.0f), t.collapseSec + 300.0f);
            t.glowSec = U(5, 2.0f * kHour, 4.0f * kHour);
            t.smokeSec = t.glowSec;
            t.peakM = U(7, 3.0f, 6.0f);
            t.peakMW = 8.0f + 7.0f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            return t;
        case BuildingKind::Church:
            t.roofFirst = true;
            t.flashSec = U(0, 900.0f, 1800.0f);                                  // the roof alight
            t.spreadSec = std::max(U(1, 1800.0f, 3600.0f), t.flashSec + 300.0f); // at its peak
            t.collapseSec = std::max(U(2, kHour, 2.0f * kHour), t.spreadSec + 600.0f);
            t.floorSec = t.collapseSec + U(3, 600.0f, 1800.0f);
            t.downSec = std::max(U(4, 2.0f * kHour, 4.0f * kHour), t.floorSec + 1200.0f);
            t.glowSec = U(5, 6.0f * kHour, 12.0f * kHour);
            t.smokeSec = U(6, kDay, 2.0f * kDay);
            t.peakM = U(7, 5.0f, 12.0f);
            t.windowM = U(8, 1.0f, 3.0f);
            t.roofM = 0.5f * t.peakM;
            t.peakMW = 30.0f + 30.0f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            return t;
        default: // a stone house, a masonry block
            if (!roofFirst)
            {
                t.flashSec = U(0, 600.0f, 1200.0f);
                t.spreadSec = std::max(U(1, 1200.0f, 2400.0f), t.flashSec + 300.0f);
                t.collapseSec = std::max(U(2, 1800.0f, kHour), t.spreadSec + 300.0f);
            }
            else
            {
                t.flashSec = U(0, 600.0f, 900.0f);
                t.collapseSec = U(2, 1500.0f, 2700.0f);
                t.spreadSec = t.collapseSec + U(1, 300.0f, 1200.0f);
            }
            t.floorSec = t.collapseSec + U(3, 600.0f, 1800.0f);
            t.downSec = std::max(U(4, 1.5f * kHour, 3.0f * kHour), t.floorSec + 1200.0f);
            t.glowSec = U(5, 6.0f * kHour, 12.0f * kHour);
            t.smokeSec = U(6, kDay, 2.0f * kDay);
            t.peakM = U(7, 3.0f, 8.0f);
            t.windowM = U(8, 1.0f, 3.0f);
            t.roofM = U(9, 2.0f, 5.0f);
            t.peakMW = 20.0f + 20.0f * std::clamp(u ? u[7] : 0.5f, 0.0f, 1.0f);
            if (kind == BuildingKind::Masonry)
            {
                for (float* f : {&t.flashSec, &t.spreadSec, &t.collapseSec, &t.floorSec, &t.downSec})
                    *f *= kMasonrySlow;
                t.peakM *= 0.6f;
                t.roofM *= 0.6f;
                t.peakMW *= 0.6f;
            }
            return t;
    }
}

BuildingPhase BuildingPhaseAt(const BuildingTimeline& tl, float t)
{
    BuildingPhase p;
    if (tl.kind == BuildingKind::Never || t < 0.0f || tl.downSec <= 0.0f)
        return p;
    const bool body = tl.kind == BuildingKind::Shed || tl.kind == BuildingKind::Barn ||
                      tl.kind == BuildingKind::FuelStation || tl.kind == BuildingKind::Fence;
    if (t >= tl.downSec + std::max(tl.glowSec, tl.smokeSec))
    {
        p.collapsed = true;
        p.over = true;
        return p;
    }
    p.collapsed = t >= tl.collapseSec;
    // Burnt down: the rubble glows and smokes thinly (a barn's hay white and longer).
    if (t >= tl.downSec)
    {
        const float a = t - tl.downSec;
        const float late = tl.kind == BuildingKind::Barn ? kHaySmoke : (body ? kHeapSmoke : kShellSmoke);
        p.glow = 1.0f - Ss(0.0f, tl.glowSec, a);
        p.smoke = late * (1.0f - Ss(0.0f, tl.smokeSec, a));
        p.black = tl.kind == BuildingKind::Barn ? 0.0f : 0.12f;
        p.heatMW = 0.02f * tl.peakMW * p.glow;
        return p;
    }
    if (body)
    {
        float b = 0.0f;
        if (t < tl.flashSec)
            b = std::pow(t / std::max(tl.flashSec, 1.0f), 1.5f);
        else if (t < tl.collapseSec)
            b = tl.kind == BuildingKind::FuelStation && t >= tl.spreadSec ? 0.6f : 1.0f;
        else
            b = (tl.kind == BuildingKind::FuelStation ? 0.6f : 0.4f) * (1.0f - Ss(tl.collapseSec, tl.downSec, t));
        p.bodyM = tl.peakM * b;
        p.heatMW = tl.peakMW * std::pow(b, 2.5f);
        p.smoke = 0.25f + 0.75f * b;
        p.black = tl.kind == BuildingKind::FuelStation ? 1.0f
                  : tl.kind == BuildingKind::Barn      ? 0.7f
                  : tl.kind == BuildingKind::Fence     ? 0.3f
                                                       : 0.85f;
        p.glow = p.collapsed ? 0.6f + 0.4f * Ss(tl.collapseSec, tl.downSec, t) : 0.0f;
        p.flaming = b > 0.02f;
        return p;
    }
    const bool church = tl.kind == BuildingKind::Church;
    const float drop = tl.floorSec + 0.5f * (tl.downSec - tl.floorSec); // the flames sunk inside the shell
    // Before its first flashover (or its roof alight): grey smoke, thickening and darkening.
    if (t < tl.flashSec)
    {
        const float k = Ss(0.0f, tl.flashSec, t);
        p.smoke = 0.08f + 0.3f * k;
        p.black = 0.15f + 0.5f * k;
        p.heatMW = tl.peakMW * 0.02f * (1.0f + 3.0f * t / std::max(tl.flashSec, 1.0f));
        return p;
    }
    p.flaming = true;
    // Its windows.
    if (!tl.roofFirst)
        p.windows = std::clamp((t - tl.flashSec) / std::max(tl.spreadSec - tl.flashSec, 1.0f), 0.0f, 1.0f);
    else if (t >= tl.collapseSec)
        p.windows = std::clamp((t - tl.collapseSec) / std::max(tl.spreadSec - tl.collapseSec, 60.0f), 0.0f, 1.0f);
    p.windowM = tl.windowM;
    // Above the wall tops: through the roof, then out of the roofless shell.
    float top = 0.0f, heat = 0.0f;
    if (church)
    {
        if (t < tl.collapseSec)
        {
            top = tl.peakM * Ss(tl.flashSec, tl.spreadSec, t);
            heat = 0.1f + 0.9f * Ss(tl.flashSec, tl.spreadSec, t);
        }
        else
        {
            top = 0.6f * tl.peakM * (1.0f - Ss(tl.floorSec, drop, t));
            heat = 0.6f * (1.0f - 0.95f * Ss(tl.floorSec, tl.downSec, t));
        }
    }
    else
    {
        if (t < tl.collapseSec)
        {
            if (tl.roofFirst)
            {
                top = tl.roofM * Ss(tl.flashSec, tl.flashSec + 120.0f, t) *
                      (1.0f + 0.5f * Ss(tl.flashSec, tl.collapseSec, t));
                heat = 0.15f + 0.45f * Ss(tl.flashSec, tl.collapseSec, t);
            }
            else
            {
                top = Ss(tl.collapseSec - 180.0f, tl.collapseSec, t); // breaking through at the last
                heat = t < tl.spreadSec ? 0.1f + 0.25f * Ss(tl.flashSec, tl.spreadSec, t)
                                        : 0.35f + 0.15f * Ss(tl.spreadSec, tl.collapseSec, t);
            }
        }
        else
        {
            // the roof falls in: a burst to the peak within a minute
            const float burst = Ss(tl.collapseSec, tl.collapseSec + 60.0f, t);
            const float before = tl.roofFirst ? 1.5f * tl.roofM : 1.0f;
            top = (before + (tl.peakM - before) * burst) * (1.0f - Ss(tl.floorSec, drop, t));
            heat = (0.5f + 0.5f * burst) * (1.0f - 0.95f * Ss(tl.floorSec, tl.downSec, t));
        }
    }
    p.topM = top;
    p.innerM = t >= tl.floorSec ? 2.5f * (1.0f - Ss(tl.floorSec, tl.downSec, t)) : 0.0f;
    p.heatMW = tl.peakMW * heat;
    // Its smoke: black once a room has flashed over, greying as it burns down.
    if (t < tl.collapseSec)
        p.smoke = 0.4f + 0.4f * Ss(tl.flashSec, tl.collapseSec, t);
    else if (t < tl.floorSec)
        p.smoke = 1.0f;
    else
        p.smoke = 1.0f - 0.65f * Ss(tl.floorSec, tl.downSec, t);
    p.black = 1.0f - 0.55f * Ss(tl.floorSec, tl.downSec, t);
    p.glow = t >= tl.floorSec ? 0.4f + 0.6f * Ss(tl.floorSec, tl.downSec, t) : (p.collapsed ? 0.4f : 0.0f);
    return p;
}

float BuildingWindowFlameM(const BuildingTimeline& tl, float windowShare, float t)
{
    if (tl.windowM <= 0.0f || t >= tl.downSec)
        return 0.0f;
    const float share = std::clamp(windowShare, 0.0f, 1.0f);
    const float start = !tl.roofFirst ? tl.flashSec + share * (tl.spreadSec - tl.flashSec)
                                      : tl.collapseSec + share * std::max(tl.spreadSec - tl.collapseSec, 60.0f);
    if (t < start)
        return 0.0f;
    float L = tl.windowM * Ss(start, start + kWindowGrowSec, t);
    if (t > tl.collapseSec)
        L *= 0.8f; // the rooms vent through the open roof
    if (t > tl.floorSec)
        L *= 1.0f - 0.8f * Ss(tl.floorSec, tl.floorSec + 0.5f * (tl.downSec - tl.floorSec), t);
    return L;
}

bool GetBurnMap(BurnMapView& out)
{
    if (!Enabled())
        return false;
    return FireWorld::Get().BurnMap(out);
}

// A cell's tongues read the front's age off a lattice this far apart (at
// most kAgeLatticeMax nodes a side: a square widened over its road).
constexpr float kAgeLatticeM = 2.0f;
constexpr int kAgeLatticeMax = 14;

bool GetFlameLightCells(FlameLightView& out)
{
    if (!Enabled())
        return false;
    return FireWorld::Get().LightCells(out);
}

void CellLightSliceAges(const BurningView& v, float cellM, float out[kFlameLightSlices])
{
    float fdx = v.frontX, fdz = v.frontZ;
    if (v.crossSec <= 0.0f || fdx * fdx + fdz * fdz < 0.25f)
    {
        fdx = 0.0f;
        fdz = 1.0f;
    }
    for (int k = 0; k < kFlameLightSlices; ++k)
    {
        const float d = ((static_cast<float>(k) + 0.5f) / static_cast<float>(kFlameLightSlices) - 0.5f) * cellM;
        out[k] = FrontAgeField(v, d * fdx, d * fdz, cellM);
    }
}

CellLight CellFlameLight(const BurningView& v, float trDraw, float cellM, float midflameMps, const float* sliceAge)
{
    CellLight out;
    if (v.model == 0 || v.flameMaxM <= 0.0f || v.age <= 0.0f)
        return out;
    const float tr = std::max(trDraw, 0.5f);
    const float Lc = std::max(0.05f, v.flameMaxM);
    const bool shrub = v.model >= 4 && v.model <= 7, litter = v.model >= 8;
    const float absorb = shrub ? kAbsorbShrub : (litter ? kAbsorbLitter : kAbsorbGrass);
    // the rows of tongues in a slice stack (Draw's grid: kRowSpacing of the
    // cell's flame apart)
    const float rowM = std::clamp(kRowSpacing * Lc, kRowSpacingMinM, kRowSpacingMaxM);
    const float rows = std::max(cellM / static_cast<float>(kFlameLightSlices) / rowM, 1.0f);
    const float glowSec = v.model >= 4 ? kEmberGlowWoodySec : kEmberGlowSec;
    float ages[kFlameLightSlices];
    if (sliceAge)
        std::copy(sliceAge, sliceAge + kFlameLightSlices, ages);
    else
        CellLightSliceAges(v, cellM, ages);
    float miss = 1.0f, sumA = 0.0f, sumBright = 0.0f, sumH = 0.0f, glow = 0.0f, ageSum = 0.0f;
    for (int k = 0; k < kFlameLightSlices; ++k)
        ageSum += std::max(ages[k], -1e6f);
    for (int k = 0; k < kFlameLightSlices; ++k)
    {
        const float a = ages[k];
        if (a <= 0.0f)
            continue;
        // the ash's glow: as the band leaves the point, fading behind it
        const float g = std::clamp((a - 0.5f * tr) / (0.5f * tr), 0.0f, 1.0f);
        glow += g * g * (3.0f - 2.0f * g) * std::exp(-std::max(a - tr, 0.0f) / glowSec);
        const LingerPoint lp = LingerFlame(a, tr);
        if (lp.cover <= 0.0f || lp.length <= 0.0f)
            continue;
        const float L = Lc * lp.length;
        const float h = L * std::cos(FlameTiltRad(midflameMps, std::max(L, 0.05f)));
        const float eps = 1.0f - std::exp(-absorb * kTongueThick * L);
        const float alpha = 1.0f - std::pow(std::max(1.0f - eps * lp.cover, 0.0f), rows);
        const float bright = kFlameLightDim + (1.0f - kFlameLightDim) * lp.heat;
        miss *= 1.0f - alpha;
        sumA += alpha;
        sumBright += alpha * h * bright;
        sumH += alpha * h;
    }
    out.ember = std::clamp(glow / static_cast<float>(kFlameLightSlices), 0.0f, 1.0f);
    if (sumA > 0.0f)
    {
        out.cd = kFlameLightNits * cellM * (sumBright / sumA) * (1.0f - miss);
        out.heightM = sumH / sumA;
    }
    // The burning bed under the band (the realism audit's fix 7: Draw's bed
    // streaks): a low layer kBedWidthShare of the flame tall across the
    // cell's front, its emissivity from the flaming zone's depth, its heat
    // BedHeat across the band, adding kBedLightGain of the flame body's
    // luminance. Its strongest slice counts.
    {
        const float bedW = std::max(kBedWidthShare * Lc, kBedWidthMinM);
        const float bedEps = 1.0f - std::exp(-absorb * std::max(bedW, std::min(v.rateMps * tr, kBedPathMaxM)));
        float bedHeat = 0.0f;
        for (int k = 0; k < kFlameLightSlices; ++k)
            if (ages[k] > 0.0f && ages[k] < tr)
                bedHeat = std::max(bedHeat, BedHeat(ages[k] / tr));
        if (bedHeat > 0.0f)
        {
            const float bedCd = kFlameLightNits * kBedLightGain * cellM * bedW * bedEps * bedHeat;
            if (out.cd <= 0.0f)
                out.heightM = 0.5f * bedW;
            out.cd += bedCd;
        }
    }
    // The flamelets and debris burning on behind the band (Draw's debris),
    // born from the band's end on, alive for their lognormal lives.
    const float am = ageSum / static_cast<float>(kFlameLightSlices);
    const float born = std::clamp((am - tr) / (0.5f * tr), 0.0f, 1.0f);
    if (born > 0.0f)
    {
        const float n = shrub    ? static_cast<float>(kShrubStems)
                        : litter ? 0.5f * (kTwigsMin + kTwigsMax)
                                 : kFlameletsPerCell * FlameletScale(v.model);
        const float median = shrub || litter ? 120.0f : kFlameletMedianSec;
        const float since = std::max(am - 1.25f * tr, 0.01f);
        const float alive = 0.5f * std::erfc(std::log(since / median) / (kFlameletSigma * 1.41421356f));
        out.cd += n * kFlameletLightCd * born * alive;
        if (out.heightM <= 0.0f)
            out.heightM = 0.3f;
    }
    return out;
}

float TorchLightCd(float torchM, float torchMaxM, float treeH)
{
    if (torchM <= 0.05f)
        return 0.0f;
    const float env = std::clamp(torchM / std::max(torchMaxM, 0.01f), 0.0f, 1.0f);
    return kTorchLightNits * torchM * kTorchCrownShare * std::max(treeH, 1.0f) * kTorchLightArea * (0.6f + 0.4f * env);
}

float FlareLightCd(float flameM, float widthM, float tiltRad)
{
    if (flameM <= 0.0f)
        return 0.0f;
    // its tongues cover about 0.8 of its body
    return kFlareLightNits * flameM * std::max(std::cos(tiltRad), 0.2f) * std::max(widthM, 0.5f) * 0.8f;
}

float LightNoise(float t, float rate, std::uint32_t seed)
{
    const float x = t * rate;
    const float f = std::floor(x);
    const float u = x - f;
    const auto i = static_cast<std::int64_t>(f);
    const float a = Hash01(static_cast<std::uint32_t>(i) * 747796405U + seed) * 2.0f - 1.0f;
    const float b = Hash01(static_cast<std::uint32_t>(i + 1) * 747796405U + seed) * 2.0f - 1.0f;
    const float s = u * u * (3.0f - 2.0f * u);
    return a + (b - a) * s;
}

float LightFlicker(float t, std::uint32_t seed)
{
    const float f1 = 1.5f + 1.0f * Hash01(seed * 747796405U + 1U), f2 = 2.2f + 0.8f * Hash01(seed * 747796405U + 2U);
    const float p1 = 6.2831853f * Hash01(seed * 2891336453U + 3U), p2 = 6.2831853f * Hash01(seed * 2891336453U + 4U);
    return 0.5f * std::sin(6.2831853f * f1 * t + p1) + 0.3f * std::sin(6.2831853f * f2 * t + p2) +
           0.2f * LightNoise(t, kLightFastRate, seed + 5U);
}

bool LightPick::Has(std::uint64_t id) const
{
    if (id == 0)
        return false;
    for (std::uint64_t i : ids)
        if (i == id)
            return true;
    return false;
}

void LightPick::Update(const LightCandidate* c, int n, float dt)
{
    auto scoreOf = [&](std::uint64_t id) -> float
    {
        for (int i = 0; i < n; ++i)
            if (c[i].id == id)
                return c[i].score;
        return -1.0f;
    };
    for (std::uint64_t& id : ids)
        if (id != 0 && scoreOf(id) <= 0.0f)
            id = 0;
    auto strongestFree = [&]() -> int
    {
        int best = -1;
        for (int i = 0; i < n; ++i)
        {
            if (c[i].id == 0 || c[i].score <= 0.0f || Has(c[i].id))
                continue;
            if (best < 0 || c[i].score > c[best].score)
                best = i;
        }
        return best;
    };
    for (std::uint64_t& id : ids)
    {
        if (id != 0)
            continue;
        const int b = strongestFree();
        if (b < 0)
            break;
        id = c[b].id;
    }
    const int b = strongestFree();
    int weakest = -1;
    for (int k = 0; k < kFireLightsMax; ++k)
    {
        if (ids[k] == 0)
        {
            weakest = -1;
            break;
        }
        if (weakest < 0 || scoreOf(ids[k]) < scoreOf(ids[weakest]))
            weakest = k;
    }
    if (b >= 0 && weakest >= 0 && c[b].score > kLightSwapRatio * scoreOf(ids[weakest]))
    {
        if (challenger == c[b].id)
            challengeSec += std::max(dt, 0.0f);
        else
        {
            challenger = c[b].id;
            challengeSec = 0.0f;
        }
        if (challengeSec >= kLightSwapSec)
        {
            ids[weakest] = c[b].id;
            challenger = 0;
            challengeSec = 0.0f;
        }
    }
    else
    {
        challenger = 0;
        challengeSec = 0.0f;
    }
}

TorchColumn TorchColumnOf(float treeH, float crownW, float tipRatio, float wind10)
{
    TorchColumn c;
    const float H = std::max(treeH, 1.0f);
    const float W = std::max(crownW, 0.5f);
    c.crownBaseM = kTorchFootShare * H;
    c.crownR = 0.5f * W;
    const float crownLen = H - c.crownBaseM;
    c.wideM = c.crownBaseM + kTorchWideAt * crownLen;
    c.tipM = std::max(tipRatio * H, H + 0.5f);
    // (a short flame, an olive's, necks halfway from the top to its tip)
    c.neckM = std::min(H + kTorchNeckRise * W, 0.5f * (H + c.tipM));
    c.neckR = 0.5f * kTorchNeckShare * W;
    c.axisM = c.crownBaseM + kTorchAxisAt * crownLen;
    const float zMid = std::max(c.axisM, 2.0f * kTorchWindZ0);
    c.windMps = std::max(wind10, 0.0f) * std::log(zMid / kTorchWindZ0) / std::log(10.0f / kTorchWindZ0);
    c.tilt = std::min(FlameTiltRad(c.windMps, std::max(0.5f, c.tipM - c.crownBaseM)), kTorchTiltMax);
    return c;
}

float TorchColumnOffset(const TorchColumn& c, float zM)
{
    const float span = std::max(c.tipM - c.axisM, 0.01f);
    const float u = std::clamp((zM - c.axisM) / span, 0.0f, 1.5f);
    return span * std::tan(c.tilt) * std::pow(u, kTorchBendPow);
}

float TorchColumnRadius(const TorchColumn& c, float zM)
{
    if (zM <= c.wideM)
        return c.crownR;
    if (zM <= c.neckM)
        return c.crownR + (c.neckR - c.crownR) * (zM - c.wideM) / std::max(c.neckM - c.wideM, 0.01f);
    if (zM >= c.tipM)
        return 0.0f;
    // tapering to a point: a little fuller than a cone at first
    const float u = (zM - c.neckM) / std::max(c.tipM - c.neckM, 0.01f);
    return c.neckR * std::pow(1.0f - u, 0.7f);
}

SurfaceGrid CellSurfaceTongues(const BurningView& v, float trDraw, float cellM, std::uint32_t cellKey, float gridX,
                               float gridZ, float dist, float lodBias, std::vector<SurfaceTongue>& out)
{
    out.clear();
    SurfaceGrid g;
    // Along the front (fdx, fdz: the front's direction across the cell; the
    // pinned grid direction when it lit all at once) in rows a apart, the
    // tongues b apart along each row (square to it).
    float fdx = v.frontX, fdz = v.frontZ;
    if (v.crossSec <= 0.0f || fdx * fdx + fdz * fdz < 0.25f)
    {
        fdx = gridX;
        fdz = gridZ;
    }
    const float sdx = -fdz, sdz = fdx; // along the front
    const float Lc = std::max(0.05f, v.flameMaxM);
    const float b = std::max(kTongueSpacing * Lc, kTongueSpacingMinM);
    const float a = std::clamp(kRowSpacing * Lc, kRowSpacingMinM, kRowSpacingMaxM);
    const float lodB = std::log2(std::max(1.0f, dist * kLodRad / b)) + lodBias;
    const float lodA = std::log2(std::max(1.0f, dist * kLodRad / a)) + lodBias;
    g.a = a;
    g.b = b;
    g.widen = std::min(kLodWidenMax, std::pow(kLodWiden, std::max(lodA, 0.0f) + std::max(lodB, 0.0f)));
    // the light each kept tongue carries (fix 08): its keep times this, over
    // the cell's tongues, is their number at any level of detail
    g.gain = 1.0f / (g.widen * FlameLodMeanKeep(lodA) * FlameLodMeanKeep(lodB));
    // The rows alight now: a point u of the way across flames from age
    // u crossSec through its band and tail (kTailEnd trDraw).
    // (the front's own field, off the lattice below, decides each tongue; the
    // planar one only which rows to look at, widened by a margin)
    const float aEnd = kTailEnd * trDraw;
    float u0 = 0.0f, u1 = 1.0f;
    if (v.crossSec > 0.0f)
    {
        const float slack = v.exact || v.grid ? 0.5f : 0.0f;
        u0 = (v.age - aEnd) / v.crossSec - slack;
        u1 = v.age / v.crossSec + slack;
    }
    else if (!v.exact && (v.age <= 0.0f || v.age >= aEnd))
        return g;
    const float half = 0.5f * cellM;
    // the square, wider on a side that borders a road cell (its verge burns on to the bare edge) or (buildings
    // builder) a building's bare cell (its ground burns on to the apron, in the quarter of that square on this side)
    const std::uint8_t runOn = static_cast<std::uint8_t>(v.roadSides | v.builtSides);
    const float xLo = -half - (runOn & 2 ? cellM : 0.0f), xHi = half + (runOn & 1 ? cellM : 0.0f);
    const float zLo = -half - (runOn & 8 ? cellM : 0.0f), zHi = half + (runOn & 4 ? cellM : 0.0f);
    const float reach =
        half * (std::fabs(fdx) + std::fabs(fdz)) + (runOn ? cellM : 0.0f); // its half extent along the front
    // The front's age with its edge noise (FrontAgeField) on a lattice over
    // the square about kAgeLatticeM apart, each node looked up once when
    // first needed; a tongue reads it bilinearly (the front and its noise are
    // smooth at that scale), its road's bare surface apart.
    const bool lattice = v.exact || v.grid || v.planar;
    const int nx = std::clamp(static_cast<int>(std::ceil((xHi - xLo) / kAgeLatticeM)), 1, kAgeLatticeMax - 1) + 1;
    const int nz = std::clamp(static_cast<int>(std::ceil((zHi - zLo) / kAgeLatticeM)), 1, kAgeLatticeMax - 1) + 1;
    const float hx = (xHi - xLo) / static_cast<float>(nx - 1), hz = (zHi - zLo) / static_cast<float>(nz - 1);
    float node[kAgeLatticeMax * kAgeLatticeMax];
    bool have[kAgeLatticeMax * kAgeLatticeMax] = {};
    auto nodeAge = [&](int i, int j)
    {
        const int k = j * nx + i;
        if (!have[k])
        {
            node[k] = FrontAgeField(v, xLo + static_cast<float>(i) * hx, zLo + static_cast<float>(j) * hz, cellM);
            have[k] = true;
        }
        return node[k];
    };
    auto fieldAt = [&](float ox, float oz)
    {
        if (!lattice)
            return FrontAgeField(v, ox, oz, cellM);
        const float u = std::clamp((ox - xLo) / hx, 0.0f, static_cast<float>(nx - 1));
        const float w = std::clamp((oz - zLo) / hz, 0.0f, static_cast<float>(nz - 1));
        const int i0 = std::min(static_cast<int>(u), nx - 2), j0 = std::min(static_cast<int>(w), nz - 2);
        const float fu = u - static_cast<float>(i0), fw = w - static_cast<float>(j0);
        const float a00 = nodeAge(i0, j0), a10 = nodeAge(i0 + 1, j0);
        const float a01 = nodeAge(i0, j0 + 1), a11 = nodeAge(i0 + 1, j0 + 1);
        return (a00 + (a10 - a00) * fu) * (1.0f - fw) + (a01 + (a11 - a01) * fu) * fw;
    };
    const int rows = static_cast<int>(std::ceil(reach / a));
    for (int i = -rows; i <= rows; ++i)
    {
        const float keepA = LodKeep(i + 4096, lodA);
        if (keepA <= 0.0f)
            continue;
        const float ra = (static_cast<float>(i) + 0.5f) * a;
        const float ur = std::clamp(ra / cellM + 0.5f, 0.0f, 1.0f);
        if (ur < u0 - a / cellM || ur > u1 + a / cellM)
            continue;
        // The row's chord through the cell's square (along the front).
        const float px = ra * fdx, pz = ra * fdz;
        float t0 = -1e9f, t1 = 1e9f;
        auto clip = [&](float p, float d, float lo, float hi)
        {
            if (std::fabs(d) < 1e-4f)
            {
                if (p < lo || p > hi)
                    t1 = t0 - 1.0f;
                return;
            }
            const float c0 = (lo - p) / d, c1 = (hi - p) / d;
            t0 = std::max(t0, std::min(c0, c1));
            t1 = std::min(t1, std::max(c0, c1));
        };
        clip(px, sdx, xLo, xHi);
        clip(pz, sdz, zLo, zHi);
        if (t1 <= t0)
            continue;
        if (lattice)
        {
            // The row runs along the front: the field at its ends and middle
            // says whether any of it can be alight (give or take a lattice
            // step at the cell's speed, the field bending between its nodes).
            const float m = std::min(kAgeLatticeM / std::max(v.rateMps, 0.05f), 30.0f) + 0.5f;
            float lo = 1e30f, hi = -1e30f;
            for (int q = 0; q < 3; ++q)
            {
                const float tt = t0 + (t1 - t0) * 0.5f * static_cast<float>(q);
                const float aa = fieldAt(px + tt * sdx, pz + tt * sdz);
                lo = std::min(lo, aa);
                hi = std::max(hi, aa);
            }
            if (hi < -m || lo > aEnd + m)
                continue;
        }
        // (every other row half a spacing along: the rows behind fill the gaps)
        const float stagger = (i & 1) != 0 ? kRowStagger : 0.0f;
        const int j0 = static_cast<int>(std::floor(t0 / b)) - 1, j1 = static_cast<int>(std::ceil(t1 / b));
        for (int j = j0; j <= j1; ++j)
        {
            const float keep = keepA * LodKeep(j + 4096, lodB);
            if (keep <= 0.0f)
                continue;
            const std::uint32_t key = cellKey * 2654435761U + static_cast<std::uint32_t>(i + 4096) * 40503U +
                                      static_cast<std::uint32_t>(j + 4096) * 2246822519U;
            // its foot: jittered in its grid square, fixed for its life
            const float h1 = Hash01(key + 1U), h2 = Hash01(key + 2U);
            const float along = (static_cast<float>(j) + 0.5f + stagger + kGridJitter * (2.0f * h1 - 1.0f)) * b;
            const float depth = ra + kGridJitter * (2.0f * h2 - 1.0f) * a;
            if (along < t0 || along > t1)
                continue;
            const float ox = depth * fdx + along * sdx, oz = depth * fdz + along * sdz;
            if (ox < xLo || ox > xHi || oz < zLo || oz > zHi)
                continue;
            // (buildings builder) past the square on a building's side: only the quarter on this side
            if (v.builtSides)
            {
                const bool roadSide = (ox > half && (v.roadSides & 1)) || (ox < -half && (v.roadSides & 2)) ||
                                      (oz > half && (v.roadSides & 4)) || (oz < -half && (v.roadSides & 8));
                if (!roadSide && !BuiltRunOn(v.builtSides, ox, oz, cellM))
                    continue;
            }
            // since the front reached it (none on a road's bare surface), and
            // its flame then (the tail's cover keeps a share of the points alight)
            const float pa = BareAt(v, ox, oz) ? -1e9f : fieldAt(ox, oz);
            const LingerPoint lp = LingerFlame(pa, trDraw);
            if (lp.cover <= 0.0f || (lp.cover < 1.0f && Hash01(key + 7U) >= lp.cover))
                continue;
            SurfaceTongue s;
            s.key = key;
            s.ox = ox;
            s.oz = oz;
            s.age = pa;
            s.lmax = Lc * (1.0f - kTongueSpread + 2.0f * kTongueSpread * Hash01(key + 4U));
            s.keep = keep;
            s.flame = lp;
            out.push_back(s);
        }
    }
    return g;
}

float BandResidenceSec(float trDraw, float rateMps, float flameMaxM)
{
    // (the rows CellSurfaceTongues stands its tongues in, kRowSpacing of the flame apart)
    const float rowM = std::clamp(kRowSpacing * std::max(0.05f, flameMaxM), kRowSpacingMinM, kRowSpacingMaxM);
    const float tr = std::max(trDraw, 0.5f);
    return std::clamp(kBandMinRows * rowM / std::max(rateMps, kBandMinRateMps), tr, std::max(tr, kBandMaxSec));
}

bool GetFlames(FlameView& out)
{
    if (!Enabled())
        return false;
    return FireWorld::Get().Flames(out);
}

PlantLook PlantLookOf(PlantKind kind, float h, float kwM, float flameM, const FlareShape* flare, bool torched,
                      float torchSec)
{
    PlantLook k;
    h = std::max(h, 0.3f);
    const bool thin = kind == PlantKind::Thicket || kind == PlantKind::Leafy || kind == PlantKind::Reed;
    BarkCharM(flameM, thin, k.barkWindM, k.barkLeeM);
    // The crown scorch of the surface fire, never under its flames' height.
    k.scorchTopM = std::max(CrownScorchM(kwM), 0.6f * flameM);
    k.clumpM = std::clamp(0.15f * h, 0.3f, 1.0f);
    if (torched && kind == PlantKind::Pine)
    {
        // The torch climbs the crown and burns every needle as it fills and
        // through its full flame; the trunk and branches go black with it.
        const float t0 = std::max(torchSec, 0.0f);
        k.scorchTopM = h + 1.0f;
        k.scorchRiseSec = t0 + kTorchRiseSec;
        k.burn0Sec = t0 + 0.5f;
        k.burn1Sec = t0 + kTorchSec - kTorchFallSec;
        k.scorchChar = 1.0f;
        k.goneShare = 1.0f;
        k.keptChar = 1.0f;
        k.frameTopM = h + 1.0f;
        k.glow = 1.0f;
        return k;
    }
    if (!flare)
        return k; // scorched only
    const FlareShape& s = *flare;
    switch (kind)
    {
        case PlantKind::Thicket:
            // The flame runs in at the windward base, then bursts through the
            // body: the leaves and fine twigs burn away over its rise and full
            // flame, a black skeleton of stems stays at nearly its height.
            k.scorchTopM = h + 0.5f;
            k.scorchRiseSec = s.peakSec;
            k.burn0Sec = 0.25f * s.peakSec;
            k.burn1Sec = s.peakSec + 0.8f * s.fullSec;
            k.scorchChar = 1.0f;
            k.goneShare = 1.0f - kThicketKeep;
            k.keptChar = 1.0f;
            k.frameTopM = h + 0.5f;
            k.strands = true;
            k.clumpM = std::clamp(0.3f * h, 0.5f, 1.0f);
            k.glow = 1.0f;
            break;
        case PlantKind::Leafy:
            // A weak flare on its windward lower half: half the leaves there
            // burn, the rest brown and hanging.
            k.scorchTopM = h + 0.5f;
            k.scorchRiseSec = s.peakSec + s.fullSec;
            k.burn0Sec = 0.5f;
            k.burn1Sec = s.peakSec + s.fullSec;
            k.sweepMax = kLeafySweep;
            k.scorchChar = 0.25f;
            k.goneShare = kLeafyGone;
            k.keptChar = 0.15f;
            k.frameTopM = 0.5f * h;
            k.glow = 0.6f;
            break;
        case PlantKind::Reed:
            // A fast flash: black stubble and a few charred canes.
            k.scorchTopM = h + 0.5f;
            k.scorchRiseSec = s.peakSec;
            k.burn0Sec = 0.3f;
            k.burn1Sec = s.peakSec + s.fullSec;
            k.scorchChar = 1.0f;
            k.goneShare = kReedGone;
            k.keptChar = 1.0f;
            k.keepBelowM = kReedStubbleM;
            k.frameTopM = h + 0.5f;
            k.strands = true;
            k.clumpM = 0.5f;
            k.glow = 1.0f;
            break;
        case PlantKind::Olive:
            // From the windward lower crown through it: most leaves burn, the
            // blackened frame keeps scorched brown ones.
            k.scorchTopM = h + 0.5f;
            k.scorchRiseSec = s.peakSec;
            k.burn0Sec = std::min(3.0f, 0.25f * s.peakSec);
            k.burn1Sec = s.peakSec + s.fullSec;
            k.scorchChar = 0.35f;
            k.goneShare = kOliveGone;
            k.keptChar = 0.12f;
            k.frameTopM = h + 0.5f;
            k.glow = 0.7f;
            break;
        default:
            break;
    }
    return k;
}

void PlantTable::Reset(int slotCount)
{
    slots = slotCount;
    used = 0;
    data.assign(static_cast<std::size_t>(slots) * kPlantSlotFloats, 0.0f);
    for (int s = 0; s < slots; ++s)
        Slot(s)[kPlantX] = kPlantEmpty;
}

bool PlantTable::Insert(float x, float z, int out[4], int reserve)
{
    for (int k = 0; k < 4; ++k)
        out[k] = -1;
    if (slots <= 0)
        return false;
    // (end-state agent) a refusal undoes the slots this foot took before it
    // (a half-taken foot made the table's owner rebuild it, dropping the
    // buildings' slots)
    auto undo = [this, out]()
    {
        for (int k = 0; k < 4; ++k)
            if (out[k] >= 0)
            {
                float* d = Slot(out[k]);
                std::fill(d, d + kPlantSlotFloats, 0.0f);
                d[kPlantX] = kPlantEmpty;
                --used;
                out[k] = -1;
            }
        return false;
    };
    // Its foot's bucket and, near an edge, the neighbour across it.
    const int bx = static_cast<int>(std::floor(x)), bz = static_cast<int>(std::floor(z));
    const float fx = x - static_cast<float>(bx), fz = z - static_cast<float>(bz);
    int xs[2] = {bx, bx}, zs[2] = {bz, bz};
    int nx = 1, nz = 1;
    if (fx < kPlantKeyNearM)
        xs[nx++] = bx - 1;
    else if (fx > 1.0f - kPlantKeyNearM)
        xs[nx++] = bx + 1;
    if (fz < kPlantKeyNearM)
        zs[nz++] = bz - 1;
    else if (fz > 1.0f - kPlantKeyNearM)
        zs[nz++] = bz + 1;
    const std::uint32_t mask = static_cast<std::uint32_t>(slots - 1);
    int n = 0;
    for (int i = 0; i < nx; ++i)
        for (int j = 0; j < nz; ++j)
        {
            if (kPlantLoadDiv * (used + 1 + std::max(reserve, 0)) > slots)
                return undo();
            int s = static_cast<int>(PlantBucketHash(xs[i], zs[j]) & mask);
            int found = -1;
            for (int p = 0; p < kPlantProbes; ++p)
            {
                if (Slot(s)[kPlantX] <= 0.5f * kPlantEmpty)
                {
                    found = s;
                    break;
                }
                s = static_cast<int>((static_cast<std::uint32_t>(s) + 1U) & mask);
            }
            if (found < 0)
                return undo();
            float* d = Slot(found);
            d[kPlantX] = x;
            d[kPlantZ] = z;
            d[kPlantContact] = kPlantNotYet;
            ++used;
            out[n++] = found;
        }
    return true;
}

int PlantTable::Find(float x, float z) const
{
    if (slots <= 0)
        return -1;
    const std::uint32_t mask = static_cast<std::uint32_t>(slots - 1);
    int s = static_cast<int>(PlantBucketHash(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(z))) & mask);
    for (int p = 0; p < kPlantProbes; ++p)
    {
        const float* d = Slot(s);
        if (d[kPlantX] <= 0.5f * kPlantEmpty)
            return -1;
        if (std::fabs(d[kPlantX] - x) < kPlantKeyNearM && std::fabs(d[kPlantZ] - z) < kPlantKeyNearM)
            return s;
        s = static_cast<int>((static_cast<std::uint32_t>(s) + 1U) & mask);
    }
    return -1;
}

namespace
{
// A table of `slots` slots holding the buildings' feet (those with slots)
// first, then the plants' (each kept `reserve` short), then `add`. With
// dropPlants a plant that does not fit is left out (its slots -1, counted in
// dropped); otherwise any foot that does not fit fails it. On success t and
// every slot array take the new table's; on failure nothing changes.
bool PlantTableRebuild(PlantTable& t, int slots, const std::vector<PlantFootRef>& buildings,
                       const std::vector<PlantFootRef>& plants, const PlantFootRef& add, bool addIsBuilding,
                       int reserve, bool dropPlants, int& dropped)
{
    PlantTable g;
    g.Reset(slots);
    using Four = std::array<int, 4>;
    std::vector<Four> bs(buildings.size(), Four{-1, -1, -1, -1}), ps(plants.size(), Four{-1, -1, -1, -1});
    Four mine{-1, -1, -1, -1};
    int lost = 0;
    for (std::size_t i = 0; i < buildings.size(); ++i)
        if (buildings[i].slots && buildings[i].slots[0] >= 0 && !g.Insert(buildings[i].x, buildings[i].z, bs[i].data()))
            return false;
    if (addIsBuilding && !g.Insert(add.x, add.z, mine.data()))
        return false;
    for (std::size_t i = 0; i < plants.size(); ++i)
    {
        if (!plants[i].slots || plants[i].slots[0] < 0)
            continue;
        if (!g.Insert(plants[i].x, plants[i].z, ps[i].data(), reserve))
        {
            if (!dropPlants)
                return false;
            ++lost;
        }
    }
    if (!addIsBuilding && !g.Insert(add.x, add.z, mine.data(), reserve))
        return false;
    t = std::move(g);
    for (std::size_t i = 0; i < buildings.size(); ++i)
        if (buildings[i].slots && buildings[i].slots[0] >= 0)
            std::copy(bs[i].begin(), bs[i].end(), buildings[i].slots);
    for (std::size_t i = 0; i < plants.size(); ++i)
        if (plants[i].slots && plants[i].slots[0] >= 0)
            std::copy(ps[i].begin(), ps[i].end(), plants[i].slots);
    std::copy(mine.begin(), mine.end(), add.slots);
    dropped = lost;
    return true;
}
} // namespace

bool PlantTableAdd(PlantTable& t, const PlantFootRef& add, bool isBuilding, int reserve, int maxSlots,
                   const std::vector<PlantFootRef>& buildings, const std::vector<PlantFootRef>& plants, bool& grew,
                   int& dropped)
{
    grew = false;
    dropped = 0;
    if (t.slots > 0 && t.Insert(add.x, add.z, add.slots, isBuilding ? 0 : reserve))
        return true;
    // Too full: a table twice the size (and so on) holds every foot again.
    const int start = std::max(t.slots, 1);
    int largest = t.slots;
    for (int n = start * 2; n <= maxSlots; n *= 2)
    {
        largest = n;
        if (PlantTableRebuild(t, n, buildings, plants, add, isBuilding, reserve, false, dropped))
        {
            grew = true;
            return true;
        }
    }
    if (!isBuilding)
        return false; // t and every slot as they were
    // A building at the largest table: the plants make room.
    if (largest <= 0 || !PlantTableRebuild(t, largest, buildings, plants, add, true, reserve, true, dropped))
        return false;
    grew = true;
    return true;
}

void NoteFlameDraw(int nearRibbons, int farRibbons, int splats)
{
    s_flameDraw.nearRibbons = nearRibbons;
    s_flameDraw.farRibbons = farRibbons;
    s_flameDraw.splats = splats;
}

bool GetSparks(SparkView& out)
{
    if (!Enabled())
        return false;
    return FireWorld::Get().Sparks(out);
}

namespace
{
// The shape Shape::Draw is drawing belongs to a tree, a bush or a forest
// block (Begin/EndVegetationDraw); never set while the fire is off.
bool s_drawingVegetation = false;
// (followups) or to a wreck the fire burns (its char and its shell's glow;
// DrawingWreck), the marks of the shapes it is drawn inside kept on a stack
// (Begin/End pair round each shape, a vehicle's proxies inside its own).
struct WreckDrawMark
{
    bool on = false;
    float charK = 0.0f, glow = 0.0f;
};
WreckDrawMark s_wreckDraw;
std::vector<WreckDrawMark> s_wreckDrawStack;
// the wrecks whose proxies are being drawn (Begin/EndWreckProxies)
std::vector<WreckDrawMark> s_wreckProxyStack;
} // namespace

// A tree, a bush or a forest block (its shape's map type).
static bool IsVegetation(const Object* obj)
{
    const LODShape* shape = obj ? obj->GetShape() : nullptr;
    if (!shape)
        return false;
    const MapType mt = shape->GetMapType();
    return mt == MapTree || mt == MapSmallTree || mt == MapBush || mt == MapForestBorder || mt == MapForestTriangle ||
           mt == MapForestSquare;
}

bool DrawingVegetation()
{
    return s_drawingVegetation;
}

bool BeginVegetationDraw(IAnimator* source)
{
    const bool previous = s_drawingVegetation;
    if (!Enabled())
        return previous;
    bool vegetation = false;
    // Map objects without a type draw as Static; men, vehicles and weapons
    // never char, so only those are looked at.
    const render::DrawObjectKind kind = source ? source->GetDrawObjectKind() : render::DrawObjectKind::Unknown;
    if (kind == render::DrawObjectKind::Static)
        vegetation = IsVegetation(dynamic_cast<const Object*>(source));
    s_drawingVegetation = vegetation;
    // (followups) a vehicle the fire burns: its char and its shell's glow
    s_wreckDrawStack.push_back(s_wreckDraw);
    s_wreckDraw = WreckDrawMark();
    if ((kind == render::DrawObjectKind::LandVehicle || kind == render::DrawObjectKind::Aircraft) &&
        FireWorld::Get().HasWreckLooks())
    {
        WreckDrawMark m;
        if (const Object* o = dynamic_cast<const Object*>(source))
            m.on = FireWorld::Get().WreckLookOf(o, m.charK, m.glow);
        if (m.on)
            s_wreckDraw = m;
    }
    // a burning wreck's proxy (its camouflage branches, its load) burns with
    // it; its crew and weapons do not
    if (!s_wreckDraw.on && !s_wreckProxyStack.empty() && kind != render::DrawObjectKind::Man &&
        kind != render::DrawObjectKind::Weapon)
    {
        s_wreckDraw = s_wreckProxyStack.back();
        s_drawingVegetation = false;
    }
    return previous;
}

bool BeginWreckProxies(const Object* parent)
{
    if (!Enabled() || !parent || !FireWorld::Get().HasWreckLooks())
        return false;
    WreckDrawMark m;
    m.on = FireWorld::Get().WreckLookOf(parent, m.charK, m.glow);
    if (!m.on)
        return false;
    s_wreckProxyStack.push_back(m);
    return true;
}

void EndWreckProxies(bool pushed)
{
    if (pushed && !s_wreckProxyStack.empty())
        s_wreckProxyStack.pop_back();
}

void EndVegetationDraw(bool previous)
{
    if (Enabled())
    {
        s_drawingVegetation = previous;
        if (!s_wreckDrawStack.empty())
        {
            s_wreckDraw = s_wreckDrawStack.back();
            s_wreckDrawStack.pop_back();
        }
    }
}

float WreckGlowTestScale() { return s_wreckGlowScale; }
bool KeepWorldPassSetup()
{
    return Enabled() && s_keepPass;
}

bool DrawingWreck(float& charK, float& glow)
{
    if (!s_wreckDraw.on)
        return false;
    charK = s_wreckDraw.charK;
    glow = s_wreckDraw.glow;
    return true;
}

bool TreeLeavesGone(const Object* obj)
{
    // EngineGL33_LightingShadersSurface.cpp burnObjectAt and kBurnLeafGone:
    // the leaves go past 0.86, give or take 0.04 from clump to clump, so a
    // crown's shadow goes only once every clump has.
    constexpr float kReach = 4.5f;
    constexpr float kRing = 0.85f;
    constexpr float kGone = 0.90f;
    if (!Enabled() || !IsVegetation(obj))
        return false;
    // A tree's or a bush's own fire decides (PlantLeavesGone); only what it
    // does not know (a forest block) reads the burn map.
    bool known = false;
    const bool gone = FireWorld::Get().PlantLeavesGone(obj, known);
    if (known)
        return gone;
    BurnMapView v;
    if (!GetBurnMap(v) || !v.texels || v.size <= 0 || v.texelM <= 0.0f)
        return false;
    const float span = static_cast<float>(v.size) * v.texelM;
    // The map at world x, z as the shader samples it: bilinear between texel
    // centres, clamped at the edge, 0 outside the window.
    auto at = [&](float x, float z) -> float
    {
        if (x < v.x0 || z < v.z0 || x > v.x0 + span || z > v.z0 + span)
            return 0.0f;
        const float fx = (x - v.x0) / v.texelM - 0.5f, fz = (z - v.z0) / v.texelM - 0.5f;
        const int ix = static_cast<int>(std::floor(fx)), iz = static_cast<int>(std::floor(fz));
        const float tx = fx - static_cast<float>(ix), tz = fz - static_cast<float>(iz);
        auto t = [&](int c, int r)
        {
            c = std::clamp(c, 0, v.size - 1);
            r = std::clamp(r, 0, v.size - 1);
            return static_cast<float>(v.texels[static_cast<size_t>(r) * static_cast<size_t>(v.size) + c]) / 255.0f;
        };
        return (t(ix, iz) * (1.0f - tx) + t(ix + 1, iz) * tx) * (1.0f - tz) +
               (t(ix, iz + 1) * (1.0f - tx) + t(ix + 1, iz + 1) * tx) * tz;
    };
    const Vector3 p = obj->Position();
    const float ring = std::max(std::max(at(p.X() + kReach, p.Z()), at(p.X() - kReach, p.Z())),
                                std::max(at(p.X(), p.Z() + kReach), at(p.X(), p.Z() - kReach)));
    return std::max(at(p.X(), p.Z()), kRing * ring) > kGone;
}

void SimulateWorld(float deltaT)
{
    if (!Enabled())
        return;
    FireWorld& w = FireWorld::Get();
    // (end-state agent) a shell to restore (a load, a remote machine's fire)
    // needs the world's burnt look even with no fire here
    if (!w.Grid() && w.ShellsPending() && GLandscape)
        w.Ensure();
    if (!w.Grid())
        return;
    w.Simulate(deltaT);
}

void PlumeWind(float& x, float& z)
{
    x = 0.0f;
    z = 0.0f;
    if (!Enabled())
    {
        if (GLandscape)
        {
            const Vector3 w = GLandscape->GetWind();
            x = w.X();
            z = w.Z();
        }
        return;
    }
    FireWorld::Get().Wind(x, z);
}

bool PlumeFireLight(Vector3Par p, float rgb[3])
{
    rgb[0] = rgb[1] = rgb[2] = 0.0f;
    if (!Enabled())
        return false;
    return FireWorld::Get().PlumeLightAt(p, rgb);
}

float PlumeHeatMW()
{
    return Enabled() ? FireWorld::Get().PlumeMW() : 0.0f;
}

bool GetPlumeShadow(PlumeShadowView& out)
{
    out = PlumeShadowView();
    if (!Enabled())
        return false;
    return FireWorld::Get().PlumeShadow(out);
}

float SmokeSight(Vector3Par from, Vector3Par to)
{
    if (!Enabled())
        return 1.0f;
    FireWorld& w = FireWorld::Get();
    if (!w.Active())
        return 1.0f;
    return w.Sight(from, to);
}

void NotifyExplosion(Vector3Par pos, float indirectHit, float indirectHitRange)
{
    if (!Enabled() || indirectHit < 8.0f || !GLandscape)
        return;
    // (fire-objects) a wreck's own blast: its fire (NotifyWreck) decides what it lights
    if (FireWorld::Get().WreckBlast())
        return;
    // Only on (or just over) the ground.
    if (pos.Y() - GLandscape->SurfaceY(pos.X(), pos.Z()) > 2.0f)
        return;
    FireWorld& w = FireWorld::Get();
    const float m1 = w.MoistureRef().m1h;
    // Chance a burst lights the grass: none when damp, likely when tinder dry,
    // more for a bigger burst (a rough stand-in for ignition probability).
    const float pig = std::pow(std::clamp((0.20f - m1) / 0.17f, 0.0f, 1.0f), 1.5f);
    const float size = std::clamp(indirectHit * std::max(indirectHitRange, 1.0f) / 200.0f, 0.1f, 1.0f);
    if (GFxRandGen.RandomValue() < pig * size)
        w.Ensure().Ignite(pos.X(), pos.Z());
}

void NotifyBuildingHeat(Vector3Par pos, float radiusM)
{
    if (!Enabled() || !GLandscape || radiusM <= 0.0f)
        return;
    FireWorld::Get().BuildingHeat(pos.X(), pos.Z(), radiusM);
}

void NotifyBurning(Vector3Par pos, float deltaT, float strength)
{
    if (!Enabled() || !GLandscape || strength <= 0.05f)
        return;
    FireWorld& w = FireWorld::Get();
    const float m1 = w.MoistureRef().m1h;
    const float pig = std::pow(std::clamp((0.20f - m1) / 0.17f, 0.0f, 1.0f), 1.5f);
    // A burning wreck drops burning fuel and embers round it: on average one
    // ignition of dry grass within about 15 s.
    if (GFxRandGen.RandomValue() < 0.07f * pig * deltaT)
    {
        const float a = GFxRandGen.RandomValue() * 6.2832f, r = 2.0f + 4.0f * GFxRandGen.RandomValue();
        w.Ensure().Ignite(pos.X() + r * std::cos(a), pos.Z() + r * std::sin(a));
    }
}

// ---- Wrecks (fire-objects): the stock wreck's hooks (Smokes.cpp) ----
float WreckFireballFuelKg(const Object* vehicle, float stockKg)
{
    if (!Enabled() || !vehicle)
        return stockKg;
    const WreckRead rd = ReadWreck(vehicle);
    if (!rd.ok)
        return stockKg;
    const Vector3 p = vehicle->Position();
    const float u = static_cast<float>(PointSeed(p.X(), p.Z()) & 0xffffU) / 65535.0f;
    return WreckFireballKg(rd.cfg.cls, stockKg, u, rd.fuelL, rd.loadL);
}

float WreckFireballMaxKg(const Object* vehicle)
{
    if (!Enabled() || !vehicle)
        return lighting::kMaxFuelKg;
    const WreckRead rd = ReadWreck(vehicle);
    return rd.ok && rd.cfg.cls == WreckClass::Tanker ? kTankerBallMaxKg : lighting::kMaxFuelKg;
}

bool NotifyWreck(const Object* vehicle, float fireballRadiusM)
{
    if (!Enabled() || !vehicle)
        return false;
    return FireWorld::Get().AddWreck(vehicle, fireballRadiusM);
}

void SetWreckBlast(bool on)
{
    if (Enabled())
        FireWorld::Get().SetWreckBlast(on);
}

bool InWreckBlast()
{
    return Enabled() && FireWorld::Get().WreckBlast();
}

// (end-state agent) FireShell.hpp
void NotifyGutted(Object* obj)
{
    if (Enabled())
        FireWorld::Get().QueueShell(obj, true);
}

void NotifyUngutted(Object* obj)
{
    if (Enabled())
        FireWorld::Get().QueueShell(obj, false);
}

ShellMode ShellModeOf(const LODShape* shape)
{
    return Enabled() && shape ? FireWorld::Get().ShellModeOfShape(shape) : ShellMode::Walls;
}

std::uint32_t ShellSeedOf(const Object* obj)
{
    if (!obj)
        return 0;
    const Vector3 p = obj->Position();
    const std::uint32_t at = static_cast<std::uint32_t>(static_cast<int>(std::floor(p.X() * 4.0f))) * 73856093U ^
                             static_cast<std::uint32_t>(static_cast<int>(std::floor(p.Z() * 4.0f))) * 19349663U;
    return at * 2654435761U ^ (s_bldRunSeed * 2246822519U + 0x5EEDU);
}

int ShellStageNow(const Object* obj)
{
    return Enabled() && obj ? FireWorld::Get().ShellStageFor(obj) : 1;
}

// fireSpreadShellRay [[x, y, z], [x, y, z]] (ASL, engine order x, up, z; TEST
// AID): what stops the line: "fire=<hits>;view=<hits>;visible=<0..1>;fireObj=..;viewObj=..".
GameValue FireSpreadShellRayCmd(const GameState* state, GameValuePar oper1)
{
    if (oper1.GetType() != GameArray)
        return GameValue("none");
    const GameArrayType& a = oper1;
    if (a.Size() < 2 || a[0].GetType() != GameArray || a[1].GetType() != GameArray)
        return GameValue("none");
    const GameArrayType& p = a[0];
    const GameArrayType& q = a[1];
    if (p.Size() < 3 || q.Size() < 3)
        return GameValue("none");
    const Vector3 from(static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2]));
    const Vector3 to(static_cast<float>(q[0]), static_cast<float>(q[1]), static_cast<float>(q[2]));
    return GameValue(RString(FireWorld::Get().ShellRay(from, to).c_str()));
}

void RegisterScriptCommands()
{
    if (!Enabled())
        return;
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadIgnite", FireSpreadIgnite, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "fireSpreadSet", FireSpreadSet, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadState", FireSpreadState, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadCell", FireSpreadCell, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadTree", FireSpreadTree, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadPlants", FireSpreadPlants, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadFuel", FireSpreadFuel, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadSight", FireSpreadSightCmd, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadBuilding", FireSpreadBuildingCmd, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadBuildings", FireSpreadBuildingsCmd, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadBuildingTime", FireSpreadBuildingTimeCmd, GameArray));
    GGameState.NewNularOp(GameNular(GameString, "fireSpreadStats", FireSpreadStats));
    GGameState.NewFunction(GameFunction(GameScalar, "fireSpreadWreck", FireSpreadWreck, GameArray));
    GGameState.NewNularOp(GameNular(GameString, "fireSpreadWrecks", FireSpreadWrecks));
    GGameState.NewNularOp(GameNular(GameBool, "fireSpreadClear", FireSpreadClear));
    GGameState.NewFunction(GameFunction(GameString, "fireSpreadShellRay", FireSpreadShellRayCmd, GameArray));
}

} // namespace Poseidon::firespread
