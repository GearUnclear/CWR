#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>

#include <Poseidon/Game/Guerrilla/Journal.hpp> // victory line, JournalClockNow, IslandDisplayName
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp> // HasHq / HqEver (the lose rule)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/AI/AICenter.hpp> // EndMode (EMContinue / EMEnd1 / EMLoser)
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
    _lost = false;
    _lostDay = 0;
    _lostMinute = 0;
    _onceHeld.Clear();
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

bool CampaignEnd::HoldsAnything(const ZoneRegistry& registry)
{
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (z && stricmp(z->owner, resistance) == 0)
        {
            return true;
        }
    }
    return false;
}

void CampaignEnd::NoteHeld(const ZoneRegistry& registry)
{
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) != 0)
        {
            continue;
        }
        bool known = false;
        for (int k = 0; k < _onceHeld.Size() && !known; k++)
        {
            known = stricmp(_onceHeld[k].name, z->name) == 0;
        }
        if (!known)
        {
            RosterRow row;
            row.name = z->name;
            _onceHeld.Add(row);
        }
    }
}

int CampaignEnd::CountOnceHeld(const ZoneRegistry& registry, bool towns) const
{
    int count = 0;
    for (int i = 0; i < _onceHeld.Size(); i++)
    {
        const int index = registry.FindZoneIndex(_onceHeld[i].name);
        const ZoneRecord* z = index >= 0 ? registry.GetZone(index) : nullptr;
        if (!z)
        {
            continue;
        }
        const bool town = stricmp(z->type, "CITY") == 0;
        if (towns ? town : CampaignEnd::IsMilitaryType(z->type))
        {
            count++;
        }
    }
    return count;
}

int CampaignEnd::BasesOnceHeld(const ZoneRegistry& registry) const
{
    return CountOnceHeld(registry, false);
}

int CampaignEnd::TownsOnceHeld(const ZoneRegistry& registry) const
{
    return CountOnceHeld(registry, true);
}

bool CampaignEnd::EvaluateLost(const ZoneRegistry& registry, bool hasHq, bool hadHq, int dayNow, int minuteNow)
{
    if (!_tuning.enabled || !registry.IsActive() || _won || _lost)
    {
        return false;
    }
    TakeRoster(registry);
    NoteHeld(registry);
    if (hasHq || HoldsAnything(registry))
    {
        return false;
    }
    // nothing to lose yet: no HQ ever and no zone ever held (a mission whose
    // opening holds nothing is not a defeat on its first second)
    if (!hadHq && _onceHeld.Size() == 0)
    {
        return false;
    }
    _lost = true;
    _lostDay = dayNow;
    _lostMinute = minuteNow;
    _endIssued = false;
    _endClock = _tuning.endDelay;
    return true;
}

bool CampaignEnd::Evaluate(const ZoneRegistry& registry, int dayNow, int minuteNow)
{
    if (!_tuning.enabled || !registry.IsActive())
    {
        return false;
    }
    TakeRoster(registry);
    if (_won || _lost)
    {
        return false;
    }
    NoteHeld(registry);
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
        snprintf(line, sizeof(line), "%s is ours, the last base they held. No occupier flag flies %s now.", lastZone,
                 where);
    }
    else
    {
        snprintf(line, sizeof(line), "Their last base is ours. No occupier flag flies %s now.", where);
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

RString CampaignEnd::LoseTitle(const char* island)
{
    char line[256];
    snprintf(line, sizeof(line), "%s is lost.", island && *island ? island : "The island");
    return RString(line);
}

RString CampaignEnd::LoseBody(const char* islandPhrase, int day, int bases, int towns)
{
    const char* where = islandPhrase && *islandPhrase ? islandPhrase : "on the island";
    char held[160];
    if (bases <= 0 && towns <= 0)
    {
        snprintf(held, sizeof(held), "It never took a base or a town from them.");
    }
    else
    {
        snprintf(held, sizeof(held), "Before the end it had taken %d %s and %d %s.", bases,
                 bases == 1 ? "base" : "bases", towns, towns == 1 ? "town" : "towns");
    }
    char line[448];
    if (day > 0)
    {
        snprintf(line, sizeof(line),
                 "The headquarters is gone and the resistance holds no base and no town %s. It held out until "
                 "Day %d. %s",
                 where, day, held);
    }
    else
    {
        snprintf(line, sizeof(line), "The headquarters is gone and the resistance holds no base and no town %s. %s",
                 where, held);
    }
    return RString(line);
}

bool CampaignEnd::DebriefingText(RString& title, RString& body) const
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (_won)
    {
        title = DebriefingTitle(IslandDisplayName());
        body = DebriefingBody(IslandPhrase(registry), RosterHeld(registry), _wonDay);
        return true;
    }
    if (_lost)
    {
        title = LoseTitle(IslandDisplayName());
        body = LoseBody(IslandPhrase(registry), _lostDay, BasesOnceHeld(registry), TownsOnceHeld(registry));
        return true;
    }
    return false;
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

void CampaignEnd::OnLost()
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    LOG_INFO(Core,
             "CampaignEnd: campaign LOST on day {} (no headquarters, no base, no town; {} base(s) and {} town(s) once "
             "held); end in {:.1f}s",
             _lostDay, BasesOnceHeld(registry), TownsOnceHeld(registry), _endClock);
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
    // a lost campaign ends as EMLoser, never EMEnd2 (that would play
    // OutroWin and count as a completion)
    GWorld->SetEndMode(_won ? EMEnd1 : EMLoser);
    LOG_INFO(Core, "CampaignEnd: mission end requested ({})", _won ? "END1" : "LOSER");
}

void CampaignEnd::Simulate(float deltaT)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return;
    }
    if (!_won && !_lost)
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
        else
        {
            const GuerrillaBase& base = GuerrillaBase::Instance();
            if (EvaluateLost(registry, base.HasHq(), base.HqEver(), day, minute))
            {
                OnLost();
            }
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
    // the lose latch (OCCUPIER-FORCES.md 9.2, 11): absent in older saves =
    // not lost, nothing once held (refilled from the live owners next tick)
    PARAM_CHECK(ar.Serialize("lost", _lost, 1, false))
    PARAM_CHECK(ar.Serialize("lostDay", _lostDay, 1, 0))
    PARAM_CHECK(ar.Serialize("lostMinute", _lostMinute, 1, 0))
    PARAM_CHECK(ar.Serialize("OnceHeld", _onceHeld, 1))
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
