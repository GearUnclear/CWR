#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>

#include <Poseidon/Game/Guerrilla/Journal.hpp> // victory line, JournalClockNow, IslandDisplayName
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/AI/AICenter.hpp> // EndMode (EMContinue / EMEnd1)
#include <Poseidon/World/World.hpp> // GWorld end mode

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <cstdio>

namespace Poseidon::Guerrilla
{

// Defined in CampaignEndCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureZoneRegistryCommandsLinked.
void EnsureCampaignEndCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
CampaignEnd& CampaignEnd::Instance()
{
    EnsureCampaignEndCommandsLinked();
    static CampaignEnd instance;
    return instance;
}
#pragma clang diagnostic pop

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void CampaignEnd::Clear()
{
    _tuning = CampaignEndTuning();
    _roster.Clear();
    _rosterTaken = false;
    _won = false;
    _wonDay = 0;
    _wonMinute = 0;
    _lastTaken = RString();
    _endIssued = false;
    _endClock = 0;
    _accum = 0;
    _heldPrimed = false;
}

void CampaignEnd::InitMission()
{
    Clear();
    LoadFromConfig();
}

void CampaignEnd::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
}

void CampaignEnd::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = CampaignEndTuning();
    if (!zonesCfg)
    {
        return;
    }
    _tuning.enabled = zonesCfg->ReadValue("campaignEnd", _tuning.enabled ? 1.0f : 0.0f) != 0.0f;
    _tuning.endDelay = zonesCfg->ReadValue("campaignEndDelay", _tuning.endDelay);
    if (_tuning.endDelay < 0)
    {
        _tuning.endDelay = 0;
    }
}

// ---------------------------------------------------------------------------
// the rule
// ---------------------------------------------------------------------------

bool CampaignEnd::IsMilitaryType(const char* type)
{
    if (!type || !*type)
    {
        return false;
    }
    return stricmp(type, "CAMP") != 0 && stricmp(type, "CITY") != 0;
}

bool CampaignEnd::IsAuthoredOccupier(const ZoneRecord& zone, const ZoneRegistry& registry)
{
    // the AUTHORED owner, not the live one: a campaign derived from a save
    // written before this service existed still knows which bases were the
    // enemy's, even if some have fallen since
    if (stricmp(zone.ownerConfig, "OCCUPIER") == 0)
    {
        return true;
    }
    const RString occupier = registry.OccupierSide();
    return occupier.GetLength() > 0 && stricmp(zone.ownerConfig, occupier) == 0;
}

void CampaignEnd::TakeRoster(const ZoneRegistry& registry)
{
    if (_rosterTaken || !registry.IsActive())
    {
        return;
    }
    _roster.Clear();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !IsMilitaryType(z->type) || !IsAuthoredOccupier(*z, registry))
        {
            continue;
        }
        RosterRow row;
        row.name = z->name;
        _roster.Add(row);
    }
    _rosterTaken = true;
    _heldPrimed = false;
    LOG_INFO(Core, "CampaignEnd: {} enemy military zone(s) to take{}", _roster.Size(),
             _roster.Size() == 0 ? " - this campaign can never be won" : "");
}

RString CampaignEnd::RosterName(int index) const
{
    if (index < 0 || index >= _roster.Size())
    {
        return RString();
    }
    return _roster[index].name;
}

static bool HeldByResistance(const ZoneRegistry& registry, const RString& name, bool& present)
{
    present = false;
    const int index = registry.FindZoneIndex(name);
    if (index < 0)
    {
        return false;
    }
    const ZoneRecord* z = registry.GetZone(index);
    if (!z)
    {
        return false;
    }
    present = true;
    return stricmp(z->owner, registry.ResistanceSide()) == 0;
}

int CampaignEnd::RosterPresent(const ZoneRegistry& registry) const
{
    int count = 0;
    for (int i = 0; i < _roster.Size(); i++)
    {
        bool present = false;
        HeldByResistance(registry, _roster[i].name, present);
        count += present ? 1 : 0;
    }
    return count;
}

int CampaignEnd::RosterHeld(const ZoneRegistry& registry) const
{
    int count = 0;
    for (int i = 0; i < _roster.Size(); i++)
    {
        bool present = false;
        count += HeldByResistance(registry, _roster[i].name, present) ? 1 : 0;
    }
    return count;
}

bool CampaignEnd::AllHeld(const ZoneRegistry& registry) const
{
    int present = 0;
    for (int i = 0; i < _roster.Size(); i++)
    {
        bool here = false;
        const bool held = HeldByResistance(registry, _roster[i].name, here);
        if (!here)
        {
            continue;
        }
        if (!held)
        {
            return false;
        }
        present++;
    }
    return present > 0;
}

bool CampaignEnd::Evaluate(const ZoneRegistry& registry, int dayNow, int minuteNow)
{
    if (!_tuning.enabled || !registry.IsActive())
    {
        return false;
    }
    TakeRoster(registry);
    if (_won)
    {
        return false;
    }
    // which roster zone turned held since the previous check: the first in
    // roster order names the line, and only when a previous check this
    // session saw the set (a fresh roster or a load knows no "before")
    RString newlyHeld;
    for (int i = 0; i < _roster.Size(); i++)
    {
        bool present = false;
        const bool held = HeldByResistance(registry, _roster[i].name, present);
        if (_heldPrimed && held && !_roster[i].heldLast && newlyHeld.GetLength() == 0)
        {
            newlyHeld = _roster[i].name;
        }
        _roster[i].heldLast = held;
    }
    _heldPrimed = true;
    if (!AllHeld(registry))
    {
        return false;
    }
    _won = true;
    _wonDay = dayNow;
    _wonMinute = minuteNow;
    _lastTaken = newlyHeld;
    _endIssued = false;
    _endClock = _tuning.endDelay;
    return true;
}

// ---------------------------------------------------------------------------
// the end request
// ---------------------------------------------------------------------------

void CampaignEnd::SetEndDelay(float seconds)
{
    _tuning.endDelay = seconds < 0 ? 0 : seconds;
    if (EndPending())
    {
        _endClock = _tuning.endDelay;
    }
}

bool CampaignEnd::AdvanceEnd(float deltaT)
{
    if (!EndPending())
    {
        return false;
    }
    _endClock -= deltaT;
    if (_endClock > 0)
    {
        return false;
    }
    _endClock = 0;
    _endIssued = true;
    return true;
}

// ---------------------------------------------------------------------------
// text
// ---------------------------------------------------------------------------

RString CampaignEnd::VictoryLine(const char* islandPhrase, const char* lastZone)
{
    const char* where = islandPhrase && *islandPhrase ? islandPhrase : "on the island";
    char line[384];
    if (lastZone && *lastZone)
    {
        snprintf(line, sizeof(line), "%s is ours. It was the last base they held %s.", lastZone, where);
    }
    else
    {
        snprintf(line, sizeof(line), "We took their last base. They hold nothing %s now.", where);
    }
    return RString(line);
}

RString CampaignEnd::DebriefingTitle(const char* island)
{
    char line[256];
    snprintf(line, sizeof(line), "%s is free.", island && *island ? island : "The island");
    return RString(line);
}

RString CampaignEnd::DebriefingBody(const char* islandPhrase, int held, int day)
{
    const char* where = islandPhrase && *islandPhrase ? islandPhrase : "on the island";
    char line[384];
    if (day > 0)
    {
        snprintf(line, sizeof(line),
                 "Every occupier base %s is in resistance hands, %d taken in all. The campaign was won on Day %d.",
                 where, held, day);
    }
    else
    {
        snprintf(line, sizeof(line), "Every occupier base %s is in resistance hands, %d taken in all.", where, held);
    }
    return RString(line);
}

// the locative the campaign history uses ("on Malden", "in the Sinai")
static RString IslandPhrase(const ZoneRegistry& registry)
{
    if (registry.Tuning().islandPhrase.GetLength() > 0)
    {
        return registry.Tuning().islandPhrase;
    }
    const RString island = IslandDisplayName();
    if (island.GetLength() == 0)
    {
        return RString();
    }
    return RString("on ") + island;
}

bool CampaignEnd::DebriefingText(RString& title, RString& body) const
{
    if (!_won)
    {
        return false;
    }
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    title = DebriefingTitle(IslandDisplayName());
    body = DebriefingBody(IslandPhrase(registry), RosterHeld(registry), _wonDay);
    return true;
}

// ---------------------------------------------------------------------------
// engine hook
// ---------------------------------------------------------------------------

void CampaignEnd::OnWon()
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const RString line = VictoryLine(IslandPhrase(registry), _lastTaken);
    Journal::Instance().AddEntry(JournalStampNow(), line, _lastTaken, JKGood);
    LOG_INFO(Core,
             "CampaignEnd: campaign WON on day {} ({} of {} enemy military zones held, last '{}'); end in {:.1f}s",
             _wonDay, RosterHeld(registry), _roster.Size(), (const char*)_lastTaken, _endClock);
}

void CampaignEnd::RequestEnd()
{
    if (!GWorld)
    {
        return;
    }
    if (GWorld->GetEndMode() != EMContinue)
    {
        // the player died (or something else ended the mission) inside the
        // window: the stock end path already owns the screen
        LOG_INFO(Core, "CampaignEnd: end request skipped, the mission is already ending");
        return;
    }
    GWorld->SetEndMode(EMEnd1);
    LOG_INFO(Core, "CampaignEnd: mission end requested (END1)");
}

void CampaignEnd::Simulate(float deltaT)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return;
    }
    if (!_won)
    {
        _accum += deltaT;
        if (_accum < TickInterval)
        {
            return;
        }
        _accum = 0;
        int day = 0;
        int minute = 0;
        JournalClockNow(day, minute);
        if (Evaluate(registry, day, minute))
        {
            OnWon();
        }
    }
    if (AdvanceEnd(deltaT))
    {
        RequestEnd();
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError CampaignEnd::RosterRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    return LSOK;
}

LSError CampaignEnd::Serialize(ParamArchive& ar)
{
    // plain values only - one pass is enough, but the World serializer
    // calls us on both load passes; only touch state on the first
    if (ar.IsLoading() && ar.GetPass() != ParamArchive::PassFirst)
    {
        // the mission config was reparsed during the first pass (SetMission
        // at the end of World::Serialize): the tuning comes from it, the
        // pending end clock from the save
        LoadFromConfig();
        return LSOK;
    }
    PARAM_CHECK(ar.Serialize("rosterTaken", _rosterTaken, 1, false))
    // an empty roster writes no subclass and loads back empty, which with
    // rosterTaken set keeps a zero-zone campaign unwinnable
    PARAM_CHECK(ar.Serialize("Roster", _roster, 1))
    PARAM_CHECK(ar.Serialize("won", _won, 1, false))
    PARAM_CHECK(ar.Serialize("wonDay", _wonDay, 1, 0))
    PARAM_CHECK(ar.Serialize("wonMinute", _wonMinute, 1, 0))
    PARAM_CHECK(ar.Serialize("lastTaken", _lastTaken, 1, RString()))
    PARAM_CHECK(ar.Serialize("endIssued", _endIssued, 1, false))
    PARAM_CHECK(ar.Serialize("endClock", _endClock, 1, 0.0f))
    if (ar.IsLoading())
    {
        // the previous-check memory is session state: a loaded campaign
        // does not know which zone fell last before the save
        for (int i = 0; i < _roster.Size(); i++)
        {
            _roster[i].heldLast = false;
        }
        _heldPrimed = false;
        _accum = 0;
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
