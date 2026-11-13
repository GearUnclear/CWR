#pragma once

// Generated occupier bases, step 2: the outpost layout (OCCUPIER-FORCES.md
// 5.7, ISLAND-PACKS.md 11).  Every generated OUTPOST (BaseGenerate.hpp, a
// stored GeneratedBaseRow) gets one layout of stock statics read from the
// config class CfgGuerrillaBaseLayouts (guerrilla-mode/config/
// guerrilla-hq.hpp): rows of {class, dx, dz, dir[, jitter]} in base-local
// metres, +z towards the base's road (the stored heading), +x to the right
// when facing it.  World position = centre + Rot(heading) * (dx, dz), world
// direction = heading + dir.  Shaped like TownFlags: the props are created
// ONCE per campaign, behind the loading screen at new game (InitMission,
// before any garrison spawns), kept as LLink rows and re-created only when a
// link comes back null (a deleted prop); the objects ride the world's
// building serializer, so a load re-binds the saved rows (SerializeRef) and
// never creates a second set.
//
// Placement rules (tmp/of-generated-bases-engine.md 2 and 5):
//   * a yard (yard, 25 m) at the centre stays prop-free: births, the hold
//     posture and capture presence start there; a row authored inside it is
//     refused;
//   * nothing within roadMargin (6 m) of a road link, measured from the
//     prop's footprint (GRoadNet::IsOnRoad with the footprint radius);
//   * a solid landscape object (a wall, a fence, a gate, a building) stays
//     lane (4 m) clear of the footprint, a tree only clear of it (step 3);
//   * one layout per generated kind (the zone type); a "$key" alternate is
//     the occupier faction descriptor's value of key (the faction style);
//   * dry ground, no landscape object in the footprint; a refused spot tries
//     its left-right mirror, then both pushed outward by ShiftStep and twice
//     that, then the row is dropped (logged);
//   * class alternates "A|B": the first the loaded package can spawn wins
//     (the plan-15 Spawnable probe); none = the row is skipped with one
//     WARN, never fatal; a layout must still read as a base without it;
//   * the AI pathing cache (Landscape::OperationalCache) of the 3 x 3 cells
//     around each created prop is invalidated, the Dammage.cpp idiom, so a
//     prop created after a field was cached is not invisible to pathing.
// Seams for the rest of the campaign: ExcludesLz (OccupierAir's LZ probe
// skips any point within LzClear of a prop, since the probe reads terrain
// fixed at map load) and NudgeClear (GarrisonCache moves a garrison spawn
// point of a generated base out of every prop's box).
//
// NOTE: no comment in this file may END in a backslash (see IslandScaffold.cpp).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

#include <string>
#include <vector>

class ParamArchive;

namespace Poseidon
{
class EntityAI;
class ParamEntry;

namespace Guerrilla
{
struct GeneratedBaseRow;

// one authored layout row (base-local)
struct BasePropRow
{
    // "Class" or "ClassA|ClassB" (the first spawnable wins); an alternate
    // "$key" names the occupier faction's descriptor key (the faction style:
    // LoBo's brown towers for the IDF and the Arab armies), skipped when the
    // descriptor has no such key
    RString cls;
    float dx = 0;
    float dz = 0;
    float dir = 0;       // degrees, relative to the base heading
    bool jitter = false; // tents, crates, sandbags: up to jitterDeg of seeded turn
};

struct BaseLayout
{
    RString name; // the config class ("Outpost")
    RString type; // the zone type it serves ("OUTPOST")
    float yard = 25.0f;
    float roadMargin = 6.0f;
    float jitterDeg = 5.0f;
    float lane = 4.0f; // m between a prop and a solid map object (the design's 4 m lanes)
    int minProps = 8;  // fewer placed props log a WARN (never fatal)
    AutoArray<BasePropRow> rows;
};

// a model's footprint in model space (Shape Min / Max on x and z)
struct BasePropBox
{
    float minX = -1, maxX = 1, minZ = -1, maxZ = 1;
    // the 2D radius around the model origin that holds the whole footprint
    float Radius() const;
};

// one placed (or planned) prop
struct BasePropState
{
    RString base; // the generated base's zone name
    int row = -1; // the layout row
    RString cls;  // the resolved class
    float x = 0, z = 0;
    float dir = 0; // degrees clockwise from north
    BasePropBox box;
    // OLink semantics: a deleted prop reads back as null -> re-created
    LLink<EntityAI> prop;
    int made = 0;           // creations this session (transient)
    RString appliedTexture; // transient, the flag texture last pushed
    // test bookkeeping (the fake world's handle); never saved
    int token = 0;

    LSError Serialize(ParamArchive& ar);
};

// The world seam: the engine reads GLandscape / GRoadNet / Pars and creates
// entities; unit tests inject a fake.
struct BasePropWorld
{
    virtual ~BasePropWorld() = default;
    virtual bool Spawnable(const char* cls) const = 0;
    virtual bool Box(const char* cls, BasePropBox& out) const = 0;
    // a road link within clearance of (x, z)
    virtual bool NearRoad(float x, float z, float clearance) const = 0;
    virtual bool Wet(float x, float z) const = 0;
    // a landscape object (not one of the layout's own props) overlaps the
    // circle of radius r around (x, z)
    // a solid one (a building, a wall, a fence or a gate: anything but a
    // tree) must also stay lane metres clear, so a prop never closes a gap
    // a soldier or a car walks through
    virtual bool Blocked(float x, float z, float r, float lane) const = 0;
    // the occupier faction's descriptor value of key (a "$key" alternate:
    // the faction-style props, basePropTower, basePropHut, ...); "" = none
    virtual RString FactionClass(const char* /*key*/) const { return RString(); }
    // the prop object of s still stands (a destroyed one still stands)
    virtual bool Exists(const BasePropState& s) const = 0;
    // create s's prop and bind it (s.prop, or s.token in a fake)
    virtual bool Create(BasePropState& s) const = 0;
    // per tick: the flag texture of a flag row
    virtual void Refresh(BasePropState& /*s*/, const char* /*owner*/) const {}
};

class BaseProps : public SerializeClass
{
  public:
    BaseProps() = default;
    static BaseProps& Instance();

    static constexpr float TickInterval = 5.0f; // s between re-create passes
    static constexpr float LzClear = 60.0f;     // m, OccupierAir's LZ exclusion
    static constexpr float NudgeMargin = 1.5f;  // m outside a prop's box
    static constexpr float ShiftStep = 4.0f;    // m, the outward push of a refused row

    // lifecycle ---------------------------------------------------------------
    void Clear();       // drops the rows and links, never objects
    void InitMission(); // Clear, read the layouts, create the props of a new game
    bool IsActive() const { return _rows.Size() > 0 || _planned.Size() > 0; }

    // layouts -----------------------------------------------------------------
    // CfgGuerrillaBaseLayouts; null = no layouts (no props, one INFO line)
    void LoadLayouts(const ParamEntry* cfg);
    static bool ParseLayout(const ParamEntry& cls, BaseLayout& out, std::vector<std::string>* notes);
    const BaseLayout* FindLayout(const char* zoneType) const;
    int NLayouts() const { return (int)_layouts.size(); }

    // pure helpers (unit-tested) ----------------------------------------------
    // base-local (dx, dz) to world, heading in degrees clockwise from north
    static void LocalToWorld(float cx, float cz, float headingDeg, float dx, float dz, float& x, float& z);
    // the seeded turn of a jitter row, -jitterDeg..+jitterDeg
    static float JitterDeg(int layoutSeed, int row, float jitterDeg);
    // the first alternate of "A|B|C" the world can spawn ("" none)
    static RString ResolveClass(const char* alternates, const BasePropWorld& world);
    // world point inside a prop's box grown by margin
    static bool InsideBox(const BasePropState& s, float x, float z, float margin);
    // the layout planned for one base: placements in row order
    static int PlanLayout(const BaseLayout& layout, const GeneratedBaseRow& base, const BasePropWorld& world,
                          AutoArray<BasePropState>& out, std::vector<std::string>* notes);
    // any point of pts within r of (x, z), 2D
    static bool NearAny(const AutoArray<BasePropState>& pts, float x, float z, float r);

    // the service step: plan every unplanned base with a layout (once per
    // campaign), create every planned prop whose object is gone.  Returns
    // the props created.
    int Sync(const GeneratedBaseRow* const* bases, int n, const BasePropWorld& world);

    // queries -----------------------------------------------------------------
    int NProps() const { return _rows.Size(); }
    const BasePropState* PropAt(int i) const { return i >= 0 && i < _rows.Size() ? &_rows[i] : nullptr; }
    int NPropsOf(const char* base) const;
    bool IsPlanned(const char* base) const;
    // OccupierAir: an LZ candidate within LzClear of any prop is refused
    bool ExcludesLz(float x, float z) const { return NearAny(_rows, x, z, LzClear); }
    // GarrisonCache: push a spawn point out of every prop box (+NudgeMargin);
    // true when it moved
    bool NudgeClear(float& x, float& z) const;

    // engine ------------------------------------------------------------------
    void Simulate(float deltaT);
    LSError Serialize(ParamArchive& ar) override;

    // test seam: the world Simulate / InitMission read instead of the engine's
    void SetWorldOverride(const BasePropWorld* world) { _worldOverride = world; }

  private:
    // the engine world's creation pass over the registry's generated rows
    int SyncRegistry(const BasePropWorld& world);

    std::vector<BaseLayout> _layouts;
    bool _layoutsLoaded = false;
    AutoArray<BasePropState> _rows;
    AutoArray<RString> _planned; // bases whose layout has been planned (saved)
    float _accum = 0;
    const BasePropWorld* _worldOverride = nullptr;
    // deserialized, parked between the passes
    AutoArray<BasePropState> _pendingRows;
    AutoArray<RString> _pendingPlanned;
};

} // namespace Guerrilla
} // namespace Poseidon
