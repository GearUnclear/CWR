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

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

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

    const std::vector<Query>& Queries() const { return _queries; }
    // Distinct "<bank>/<class>" that came back false, in first-seen order.
    std::vector<RString> Misses() const;
    void Clear() { _queries.clear(); }

  private:
    const ClassProbe& _inner;
    mutable std::vector<Query> _queries;
};

} // namespace Poseidon::Guerrilla
