#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>

#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

namespace Poseidon::Guerrilla
{

namespace
{

// One walk serves both directions; `match` decides which chain member wins.
// The hop budget is the cycle guard: a self-referential chain simply runs out
// of hops and reports "no twin", which every caller already handles.
template <class Match> RString WalkTwins(const ParamEntry* factionsCfg, RString faction, const Match& match)
{
    if (!factionsCfg)
    {
        return RString();
    }
    RString current = faction;
    for (int hop = 0; hop < kMaxTwinHops && current.GetLength() > 0; hop++)
    {
        const ParamEntry* cls = factionsCfg->FindEntry(current);
        if (!cls || !cls->IsClass())
        {
            return RString(); // a sideTwin pointing at nothing ends the chain
        }
        if (match(cls->ReadValue("side", current)))
        {
            return current;
        }
        current = cls->ReadValue("sideTwin", RString());
    }
    return RString();
}

} // namespace

RString FactionSideOf(const ParamEntry* factionsCfg, RString faction)
{
    if (!factionsCfg || faction.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* cls = factionsCfg->FindEntry(faction);
    if (!cls || !cls->IsClass())
    {
        return RString();
    }
    return cls->ReadValue("side", faction);
}

RString TwinOnSide(const ParamEntry* factionsCfg, RString faction, const char* wantSide)
{
    if (!wantSide || !*wantSide)
    {
        return RString();
    }
    return WalkTwins(factionsCfg, faction, [wantSide](const RString& side) { return stricmp(side, wantSide) == 0; });
}

RString TwinOffSide(const ParamEntry* factionsCfg, RString faction, const char* avoidSide)
{
    if (!avoidSide || !*avoidSide)
    {
        return RString();
    }
    return WalkTwins(factionsCfg, faction, [avoidSide](const RString& side) { return stricmp(side, avoidSide) != 0; });
}

RString FirstFreeWarSide(const char* avoidSide)
{
    static const char* const kWarSides[] = {"GUER", "WEST", "EAST"};
    for (const char* side : kWarSides)
    {
        if (!avoidSide || stricmp(side, avoidSide) != 0)
        {
            return RString(side);
        }
    }
    return RString();
}

} // namespace Poseidon::Guerrilla
