#pragma once

#include <CLI/CLI.hpp>

#include <memory>
#include <string>

namespace Poseidon
{
class ParamEntry;
namespace Guerrilla
{
struct BaseDetectInputs;
struct HeightField;
} // namespace Guerrilla
} // namespace Poseidon

namespace PoseidonTools
{

// `PoseidonTools guerrilla scaffold` (issue #54 C2): turn any world that has a
// CfgWorlds entry with a Names block and a road net into a playable Guerrilla
// Mode template - description.ext + mission.sqm + init.sqs - in one command,
// with no hand-sampled elevations and no hand-listed addOns[]. Since issue #76
// it also runs the base detector (Game/Guerrilla/BaseDetect) and writes the
// AIRFIELD / SEAPORT / OUTPOST candidates it finds into the zone block.
//
// This file owns the I/O half: the package mount, the .wrp read, the config
// probes and the file writes. The placement rules and the template text are
// pure and live in engine/Poseidon/Game/Guerrilla/IslandScaffold.{hpp,cpp},
// where PoseidonTests exercises them against synthetic terrain.
class GuerrillaScaffoldCommand
{
  public:
    static void Setup(CLI::App& parent);
};

// The world half of the scaffold, shared with `guerrilla lint --bases`: finds
// CfgWorlds >> <world> in the mounted package (PackageMount must be up) and
// loads its .wrp, then hands the detector its inputs.
class ScaffoldWorld
{
  public:
    ScaffoldWorld();
    ~ScaffoldWorld();
    ScaffoldWorld(const ScaffoldWorld&) = delete;
    ScaffoldWorld& operator=(const ScaffoldWorld&) = delete;

    // `dataDir` is the mounted package root (for the loose Worlds\<name>.wrp
    // fallback). Returns false with `error` set (several lines when the world
    // is unknown: the worlds the package declares are listed).
    bool Load(const std::string& world, const std::string& dataDir, std::string& error);

    const Poseidon::ParamEntry& Entry() const;
    const char* WorldClass() const;
    const char* WorldDisplay() const; // CfgWorlds description, may be a $STR key
    const char* WrpShown() const;     // the path the terrain was read from
    const Poseidon::Guerrilla::HeightField& Height() const;
    float WorldSize() const;
    // one human-readable line: format, grid, cell size, object count
    std::string Summary() const;

    // Fills worldClass, worldDisplay, ils (ReadIlsEntries), objects (every
    // .wrp object whose model ClassifyBaseModel weighs nonzero or
    // ModelIsBuilding accepts; radius 0 = unknown), height, worldSize and towns
    // (the Names entries ZoneRegistry::NamesEntryIsTown accepts). authored and
    // the region are the caller's.
    void FillBaseInputs(Poseidon::Guerrilla::BaseDetectInputs& in) const;

    // the raw object table, for the scaffold's road / building split
    int ObjectCount() const;
    const char* ObjectModel(int i) const;
    float ObjectX(int i) const;
    float ObjectZ(int i) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace PoseidonTools
