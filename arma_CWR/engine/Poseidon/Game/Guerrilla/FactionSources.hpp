#pragma once

// The ONE faction table every Guerrilla consumer reads (issue #54 A1, the
// packaging half of issue #26).
//
// CfgGuerrillaFactions used to be an either/or lookup: the island template's
// description.ext block if it had one, else the global config (Pars). That
// welded factions to islands - a faction pack shipping its blocks in an
// addon's config.cpp appeared on zero islands, because every shipped template
// carries a block of its own and so never fell back to Pars.
//
// FactionSources builds the UNION instead:
//
//     Pars >> CfgGuerrillaFactions  U  <island description.ext> >> CfgGuerrillaFactions
//
// with the island winning on a class-name collision (whole-class replacement,
// never a per-key merge: an island author who redeclares IDF gets exactly the
// IDF they wrote). Order is island classes first in their config order, then
// the global-only classes in theirs. Inheritance inside a source block
// (`class Pack : Base {}`) is flattened into the copy, so consumers that walk
// GetEntry(i) see every key the class resolves.
//
// The three consumers - GuerrillaNewGame::RefreshFactionsForIsland (menu),
// ZoneRegistry::LoadFromConfig (mission), OutfitSelect's player-body seam -
// all go through Build so they cannot drift; a fourth must do the same.
//
// The merged table is an owned ParamFile: Factions() points into it and stays
// valid until the next Build/Clear. Sources are only READ during Build and
// need not outlive it.

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <vector>

namespace Poseidon::Guerrilla
{

class FactionSources
{
  public:
    enum class Origin
    {
        Island, // the island template's own description.ext block
        Global, // Pars (an addon's config, a mod's bin/config.cpp, bin/config-extra.cpp)
    };

    struct Record
    {
        RString className;
        Origin origin = Origin::Global;
        // Global class shadowed by an island class of the same name (the
        // collision rule); false for Island records that replaced nothing
        bool overrodeGlobal = false;
        // ParamEntry::GetOwner of the source class - the addon (CfgPatches)
        // that shipped it; empty for base-game/description.ext content
        RString owner;
    };

    // globalCfg / islandCfg may each be null. With both null, Factions()
    // is null too - consumers keep their "no descriptor" behaviour.
    void Build(const ParamEntry* globalCfg, const ParamEntry* islandCfg);
    void Clear();

    // The merged CfgGuerrillaFactions class, or null.
    const ParamEntry* Factions() const { return _factions; }
    // One record per merged class, in Factions() entry order.
    const std::vector<Record>& Records() const { return _records; }
    const Record* FindRecord(const char* className) const;

  private:
    void CopyClass(ParamClass& dst, const ParamEntry& srcClass, const ParamEntry& srcTable);

    ParamFile _merged;
    const ParamEntry* _factions = nullptr;
    std::vector<Record> _records;
};

// The engine's two sources: the current mission's description.ext
// (ExtParsMission) as the island block and Pars as the global one.
void BuildFactionSourcesFromEngine(FactionSources& out);

} // namespace Poseidon::Guerrilla
