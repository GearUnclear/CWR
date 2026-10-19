#pragma once

// Offline audit of the plan-15 faction-descriptor resolution pass (issue #54 A5).
//
// ZoneRegistry::ResolveFactionClasses degrades an unresolvable classname rather
// than failing: a tier substitutes its nearest resolved neighbour, a vehicle rung
// is dropped, a role slot blanks to the tier rifleman. That is the right runtime
// behaviour and the wrong shipping behaviour, because a faction pack whose pbos
// are not mounted still "works" while every spawn wears the wrong body.
//
// The audit is a diff, not a second implementation: load the same descriptor
// twice, once with no probe (the authored record) and once with the real one (the
// resolved record), then list every classname-valued key the pass changed. Only
// the comparison lives here, so it stays testable with hand-built records and
// cannot drift from what the resolution pass actually does.

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <string>
#include <vector>

namespace Poseidon::Guerrilla
{

enum class LintOutcome
{
    Ok,          //!< the authored class resolved
    Substituted, //!< the pass swapped in another class
    Dropped      //!< the pass removed the entry (or blanked it to a query-time default)
};

const char* ToString(LintOutcome outcome);

struct LintFinding
{
    RString key;        //!< "tiers[0]", "civVehicles[2]", "officer"
    RString value;      //!< what the descriptor authored
    RString substitute; //!< what the pass left behind; empty when dropped
    LintOutcome outcome = LintOutcome::Ok;
};

// Every classname-valued key of one faction, in a stable order, with what the
// resolution pass did to it. `raw` must be the record loaded with probe = nullptr
// and `resolved` the same record loaded with the probe; passing them the other way
// round reports the substitutions backwards.
//
// The array rules mirror ResolveFactionClasses: tiers / civTiers / the role
// ladders keep their length (or clear wholesale when nothing resolved), while
// vehicles[] and civVehicles[] keep the resolved entries as an ordered
// subsequence of the authored ones, so a missing entry is a drop.
std::vector<LintFinding> DiffFactionRecord(const FactionRecord& raw, const FactionRecord& resolved);

// True when the faction can field nothing at all: the resolution pass emptied
// tiers[], which is the rung every spawn path bottoms out on.
bool FactionIsSterile(const FactionRecord& resolved);

// A ClassProbe that answers through `inner` and records every query. Lets the
// audit report which classnames the loaded package actually failed to provide,
// rather than inferring it from the substitutions alone.
class RecordingClassProbe final : public ClassProbe
{
  public:
    struct Query
    {
        RString bank;
        RString className;
        bool exists = false;
    };

    explicit RecordingClassProbe(const ClassProbe& inner) : _inner(inner) {}

    bool Exists(const char* bank, const char* className) const override;
    // The shape-aware gate (issue #46 seam 4b), recorded under CfgVehicles
    // like a plain existence miss so Misses() names a shapeless class the
    // same way it names an absent one.
    bool Spawnable(const char* className) const override;

    const std::vector<Query>& Queries() const { return _queries; }
    // Distinct "<bank>/<class>" that came back false, in first-seen order.
    std::vector<RString> Misses() const;
    void Clear() { _queries.clear(); }

  private:
    const ClassProbe& _inner;
    mutable std::vector<Query> _queries;
};

// ---------------------------------------------------------------------------
// per-template base audit (issue #76, `PoseidonTools guerrilla lint --bases`)
// ---------------------------------------------------------------------------
//
// The detector (BaseDetect) finds the AIRFIELD / SEAPORT / OUTPOST hubs a world
// models; this half compares its candidates with what one template authors.
// Pure: the command parses description.ext and reads the .wrp, these functions
// only read ParamEntry trees and candidate lists.

// Every class under cfgGuerrillaZones >> Zones, read the way
// ZoneRegistry::LoadZones reads it: name defaults to the class name, type to
// OUTPOST, position[] is getPos order {easting, northing, elevation}. A zone
// without a usable position[] is kept at (0, 0). Null in, nothing out.
void ReadTemplateZones(const ParamEntry* cfgGuerrillaZones, std::vector<DetectAuthoredZone>& out);

// CfgGuerrillaZones >> seedBases as the runtime resolves it: a present key is
// on when nonzero; an absent key defaults to 1 when the template authors no
// AIRFIELD and no SEAPORT zone, else 0.
struct SeedBasesSetting
{
    bool seed = false;
    bool explicitKey = false; // seedBases is present in the template
    int authoredAirfields = 0;
    int authoredSeaports = 0;
};
SeedBasesSetting ResolveSeedBases(const ParamEntry* cfgGuerrillaZones, const std::vector<DetectAuthoredZone>& authored);

// playRegion[] = {minX, minZ, maxX, maxZ}. False (and the outputs untouched)
// when the key is absent, has fewer than four values, or is empty (min >= max).
bool ReadPlayRegion(const ParamEntry* cfgGuerrillaZones, float& minX, float& minZ, float& maxX, float& maxZ);

// The theatre as the runtime resolves it: the authored playRegion[] when it
// is valid, else the bounding box of the authored zones padded by `pad` (5 km)
// on every side. False only with neither (no valid key and no zones).
// `isDefault` (optional) says which one it was.
bool ResolvePlayRegion(const ParamEntry* cfgGuerrillaZones, const std::vector<DetectAuthoredZone>& authored,
                       float& minX, float& minZ, float& maxX, float& maxZ, bool* isDefault = nullptr,
                       float pad = 5000.0f);

// One audit row per candidate.
//   COVERED  an authored zone models it: coveredBy names it, coverDist is the
//            centre-to-centre distance (-1 when the name is not authored)
//   MISSING  the template does not author it: for AIRFIELD / SEAPORT, seeded
//            says whether the runtime seed adds it; an OUTPOST is never seeded
//   SKIPPED  outside playRegion[] or below the seeding confidence
struct BaseLintRow
{
    std::string verdict; // COVERED | MISSING | SKIPPED
    std::string type;
    std::string name;
    float x = 0;
    float z = 0;
    float confidence = 0;
    std::string status; // BaseStatusName of the candidate
    std::string coveredBy;
    std::string coveredByType;
    float coverDist = -1;
    bool seeded = false;
    std::string line; // the formatted report row, without the trailing newline
};
std::vector<BaseLintRow> LintTemplateBases(const BaseDetectResult& detected,
                                           const std::vector<DetectAuthoredZone>& authored,
                                           const SeedBasesSetting& seedBases);

// The per-template header row: "BASES <label> seedBases=<0|1> (<why>) ...".
std::string FormatSeedBasesRow(const char* label, const SeedBasesSetting& seedBases);

} // namespace Poseidon::Guerrilla
