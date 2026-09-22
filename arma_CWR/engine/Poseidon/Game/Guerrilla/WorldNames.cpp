#include <Poseidon/Game/Guerrilla/WorldNames.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // NamesEntryIsTown, FactionValue

#include <Poseidon/Core/Global.hpp>                             // Glob.header.worldname
#include <Poseidon/IO/ParamFile/ParamFile.hpp>                  // ParamEntry
#include <Poseidon/IO/ParamFileExt.hpp>                         // Pars
#include <Poseidon/UI/Locale/Stringtable/CodepageTranscode.hpp> // DecodeLegacyTextToRString
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp>       // GLanguage

#include <Poseidon/Foundation/platform.hpp> // stricmp

namespace Poseidon::Guerrilla
{

namespace
{

// A class name as a place-name-safe display string: '_' becomes a space, runs of
// spaces collapse to one, and the ends are trimmed.  Only ever applied to a
// CLASS NAME fallback, never to an authored displayName.
RString SpacedClassName(const RString& className)
{
    const int len = className.GetLength();
    if (len == 0)
    {
        return className;
    }
    AutoArray<char> buffer;
    buffer.Realloc(len + 1);
    int out = 0;
    for (int i = 0; i < len; i++)
    {
        char c = className[i];
        if (c == '_')
        {
            c = ' ';
        }
        if (c == ' ' && (out == 0 || buffer[out - 1] == ' '))
        {
            continue; // leading space, or a run of them
        }
        buffer.Add(c);
        out++;
    }
    while (out > 0 && buffer[out - 1] == ' ')
    {
        out--; // trailing space
    }
    if (out == 0)
    {
        return className; // a name made of nothing but separators: leave it be
    }
    return RString(buffer.Data(), out);
}

// true when `name` already sits in `out` (case-insensitive on the display string)
bool AlreadyNamed(const AutoArray<PlaceName>& out, const RString& name)
{
    for (int i = 0; i < out.Size(); i++)
    {
        if (stricmp(out[i].name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

void Partition(const AutoArray<PlaceName>& all, bool settlement, AutoArray<PlaceName>& out)
{
    out.Clear();
    for (int i = 0; i < all.Size(); i++)
    {
        const PlaceName& row = all[i];
        if (row.settlement != settlement || row.name.GetLength() == 0)
        {
            continue;
        }
        if (AlreadyNamed(out, row.name))
        {
            continue;
        }
        out.Add(row);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// the Names block
// ---------------------------------------------------------------------------

void CollectPlaceNames(const ParamEntry* worldNames, AutoArray<PlaceName>& out)
{
    out.Clear();
    if (!worldNames)
    {
        return;
    }
    for (int i = 0; i < worldNames->GetEntryCount(); i++)
    {
        const ParamEntry& e = worldNames->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        PlaceName row;
        row.key = e.GetName();
        row.type = e.ReadValue("type", RString());
        // Names entries often ship name="" - fall back to the class key, the
        // same two-step ZoneRegistry::NamesEntryIsTown takes
        RString raw = e.ReadValue("name", RString(e.GetName()));
        if (raw.GetLength() == 0)
        {
            raw = e.GetName();
        }
        row.name = DecodeLegacyTextToRString(raw, GLanguage);
        if (row.name.GetLength() == 0)
        {
            row.name = e.GetName();
        }
        // position[] is {easting, northing[, LABEL SIZE]}.  Element [2] is the
        // OFP map label size, NOT an elevation (UIMap.cpp reads [0]/[1] only and
        // zeroes Y); discard it.
        const ParamEntry* position = e.FindEntry("position");
        if (position && position->IsArray() && position->GetSize() >= 2)
        {
            row.pos = Vector3((float)(*position)[0], 0.0f, (float)(*position)[1]);
        }
        // The classifier is asked for its BOOLEAN only: it returns false before
        // writing its out-params for a typed non-town entry or a short
        // position[], and those rows are exactly what FeatureNames must carry.
        RString classifiedName;
        Vector3 classifiedPos;
        row.settlement = ZoneRegistry::NamesEntryIsTown(e, classifiedName, classifiedPos);
        out.Add(row);
    }
}

const ParamEntry* WorldNamesEntry()
{
    RString world = Glob.header.worldname;
    if (world.GetLength() == 0)
    {
        return nullptr;
    }
    const ParamEntry* worlds = Pars.FindEntry("CfgWorlds");
    if (!worlds)
    {
        return nullptr;
    }
    const ParamEntry* entry = worlds->FindEntry(world);
    if (!entry)
    {
        return nullptr;
    }
    return entry->FindEntry("Names");
}

void CollectWorldPlaceNames(AutoArray<PlaceName>& out)
{
    CollectPlaceNames(WorldNamesEntry(), out);
}

void SettlementNames(const AutoArray<PlaceName>& all, AutoArray<PlaceName>& out)
{
    Partition(all, true, out);
}

void FeatureNames(const AutoArray<PlaceName>& all, AutoArray<PlaceName>& out)
{
    Partition(all, false, out);
}

// ---------------------------------------------------------------------------
// faction display names
// ---------------------------------------------------------------------------

RString FactionDisplayName(const ZoneRegistry& registry, const RString& className, const RString& side)
{
    if (className.GetLength() > 0)
    {
        RString dn = registry.FactionValue(className, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn; // authored: verbatim, underscores and all
        }
        return SpacedClassName(className);
    }
    if (side.GetLength() > 0)
    {
        RString dn = registry.FactionValue(side, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn;
        }
    }
    return side;
}

} // namespace Poseidon::Guerrilla
