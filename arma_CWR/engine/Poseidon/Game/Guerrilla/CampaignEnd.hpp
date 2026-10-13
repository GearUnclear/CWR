#pragma once

// Guerrilla Mode campaign end state (issue #79 B3).
//
// THE WIN RULE: a Guerrilla campaign is won when every authored enemy
// military zone is resistance-owned.  "Enemy military zone" is every zone
// whose type is neither CAMP nor CITY and whose authored owner (the config
// owner: the OCCUPIER token or the occupier's literal side) is the
// campaign's occupier.  The set (the "roster") is taken once, on the first
// tick with an active ZoneRegistry, and saved with the campaign; a template
// that later gains or renames a zone does not move an existing campaign's
// goal.  A campaign whose roster is empty can never win.  Town support is
// not part of the rule.
//
// THE LATCH: the first tick that finds every roster zone held latches the
// win, writes one victory line to the field journal and starts the end
// clock.  The latch is saved and never clears, so a zone that changes hands
// afterwards cannot undo the win or repeat the journal line.  When the
// clock (campaignEndDelay, default 10 s of simulation time) runs out the
// service sets the world's end mode to END1 once, which the stock mission
// display turns into the debriefing (with the UD win text added by
// DisplayDebriefing when the briefing carries no Debriefing:End1 section),
// then the empty OutroWin, then the main menu.  A campaign saved inside
// that window resumes the clock on load.  The lose path is untouched:
// player death still drops into the stock end dialog (issue #79 D10).
//
// Script surface (CampaignEndCommands.cpp): gmCampaignWon,
// gmCampaignEnd and the gmCampaignEndDelay knob.  Config (CfgGuerrillaZones):
// campaignEndDelay (seconds) and campaignEnd = 0 to switch the rule off.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;
struct ZoneRecord;

struct CampaignEndTuning
{
    // config key campaignEnd (0/1): 0 switches the win rule off for a
    // mission that is not a campaign (test fixtures and reference missions
    // that capture their only base): no roster, no latch, no end
    bool enabled = true;
    // seconds of simulation time between the win latch and the end request
    // (config key campaignEndDelay; negative values read as 0)
    float endDelay = 10.0f;
};

class CampaignEnd : public SerializeClass
{
  public:
    CampaignEnd() = default;

    // engine-wide instance (World hooks, script commands, the debriefing)
    static CampaignEnd& Instance();

    // seconds between win checks while the campaign is still open
    static constexpr float TickInterval = 1.0f;

    // lifecycle -------------------------------------------------------------
    void Clear();
    void InitMission(); // Clear + read campaignEndDelay from CfgGuerrillaZones
    // read the tuning from CfgGuerrillaZones; null resets to defaults
    void LoadFromParams(const ParamEntry* zonesCfg);
    const CampaignEndTuning& Tuning() const { return _tuning; }

    // the rule (pure, unit-tested) ------------------------------------------
    // every type that is not CAMP and not CITY (case-insensitive)
    static bool IsMilitaryType(const char* type);
    // the zone's AUTHORED owner names the campaign's occupier
    static bool IsAuthoredOccupier(const ZoneRecord& zone, const ZoneRegistry& registry);
    // take the roster from the registry; a no-op once taken (also after a load)
    void TakeRoster(const ZoneRegistry& registry);
    bool HasRoster() const { return _rosterTaken; }
    int RosterSize() const { return _roster.Size(); }
    RString RosterName(int index) const;
    // roster zones present in the registry / resistance-owned right now
    int RosterPresent(const ZoneRegistry& registry) const;
    int RosterHeld(const ZoneRegistry& registry) const;
    // the predicate: a non-empty roster, at least one of its zones present in
    // the registry, and every present one held by the resistance (a saved
    // zone the template no longer has is skipped, not counted against)
    bool AllHeld(const ZoneRegistry& registry) const;

    // one check: takes the roster when needed, then latches the win.  True
    // only on the tick that latches.  dayNow / minuteNow stamp the win.
    // Always false (and no roster is taken) while the rule is switched off.
    bool Evaluate(const ZoneRegistry& registry, int dayNow = 0, int minuteNow = 0);

    // the latch -------------------------------------------------------------
    bool Won() const { return _won; }
    int WonDay() const { return _wonDay; }
    int WonMinute() const { return _wonMinute; }
    // the roster zone whose capture completed the set: the first, in roster
    // order, that turned held on the check that latched.  Empty when the
    // latch came on the first check of the session (a fresh roster or a
    // load knows no "before")
    RString LastTaken() const { return _lastTaken; }

    // the end request -------------------------------------------------------
    bool EndPending() const { return _won && !_endIssued; }
    bool EndIssued() const { return _endIssued; }
    float EndClock() const { return _endClock; }
    // runtime override of endDelay (script knob gmCampaignEndDelay); a
    // pending clock restarts from the new value
    void SetEndDelay(float seconds);
    // advance a pending end clock; true exactly once, when it runs out
    bool AdvanceEnd(float deltaT);

    // text (pure) -----------------------------------------------------------
    // the journal's victory line.  islandPhrase is the locative ("on Malden",
    // "in the Sinai"); lastZone may be empty.
    static RString VictoryLine(const char* islandPhrase, const char* lastZone);
    // the debriefing's win heading and body ("Malden is free.")
    static RString DebriefingTitle(const char* island);
    static RString DebriefingBody(const char* islandPhrase, int held, int day);
    // the live debriefing text for a won campaign (the island's name and
    // phrase from the world and the registry); false while not won
    bool DebriefingText(RString& title, RString& body) const;

    // engine hook -----------------------------------------------------------
    // per-frame; no-op without an active ZoneRegistry.  Checks the rule at
    // TickInterval, writes the journal line on the latch, and asks the
    // world for END1 when the end clock runs out.
    void Simulate(float deltaT);

    // save gate: a campaign carries the block once its roster is taken
    bool HasState() const { return _rosterTaken; }
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct RosterRow
    {
        RString name;
        // transient: held on the previous check (picks LastTaken), never saved
        bool heldLast = false;
        LSError Serialize(ParamArchive& ar);
    };

    // CfgGuerrillaZones from the mission (ExtParsMission), then Pars
    void LoadFromConfig();
    // the latch's side effects in the live world (journal line, log)
    void OnWon();
    // ask the world for END1 (only while it is still EMContinue)
    void RequestEnd();

    CampaignEndTuning _tuning;
    AutoArray<RosterRow> _roster;
    bool _rosterTaken = false;
    bool _won = false;
    int _wonDay = 0;
    int _wonMinute = 0;
    RString _lastTaken;
    bool _endIssued = false;
    float _endClock = 0;
    float _accum = 0;
    // transient: heldLast was primed by a check this session (a load or a
    // fresh roster does not know who fell last)
    bool _heldPrimed = false;
};

} // namespace Guerrilla
} // namespace Poseidon
