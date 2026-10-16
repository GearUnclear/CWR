#pragma once

// Guerrilla Mode campaign end state (issue #79 B3).
//
// THE WIN RULE: a Guerrilla campaign is won when every authored enemy
// military zone is resistance-owned.  "Enemy military zone" is every zone
// whose type is neither CAMP nor CITY and whose authored owner (the config
// owner: the OCCUPIER token or the occupier's literal side) is the
// campaign's occupier, plus every generated occupier base (OCCUPIER-FORCES.md
// 5.7).  Minor posts are left out (the owner, 2026-10-16): a zone whose
// countsForWin flag is false, and every CHECKPOINT, COASTAL, HELIPAD or RADAR
// zone (IsMinorPostType); they capture, source and alarm like any base.  The set (the "roster") is taken once, on the first
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
// that window resumes the clock on load.  Player death still drops into the
// stock end dialog (issue #79 D10).
//
// THE LOSE RULE (OCCUPIER-FORCES.md 9.2, D10, D13): checked every second
// after the win check, never once won, absent with campaignEnd = 0.  The
// campaign is LOST when the player has no HQ (GuerrillaBase::HasHq false:
// destroyed and not re-established), holds no base (any resistance-held
// non-CITY zone, the Camp included) and holds no town.  A campaign that never
// had an HQ and never held a zone is not lost (a mission with nothing to
// lose).  The lost latch is saved here only and never clears; after the same
// campaignEndDelay the service sets EMLoser once (still EMContinue only),
// which the stock flow turns into the defeat debriefing (the UD lose text,
// DisplayDebriefing), OutroLoose and the main menu.  Saves stay on disk.
//
// Script surface (CampaignEndCommands.cpp): gmCampaignWon, gmCampaignLost,
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
    // the zone's AUTHORED owner names the campaign's occupier.  False for a
    // generated base (ZoneRecord::generated): it is not authored.  The
    // occupier pool sizes R through this predicate, so a generated garrison
    // adds nothing to R or poolStart (the conservation stand-in of
    // OCCUPIER-FORCES.md 5.7 until the pool sizes generated bases itself)
    static bool IsAuthoredOccupier(const ZoneRecord& zone, const ZoneRegistry& registry);
    // a generated base whose config owner is the occupier (the OCCUPIER token)
    static bool IsGeneratedOccupier(const ZoneRecord& zone, const ZoneRegistry& registry);
    // the minor kinds of the base catalogue: CHECKPOINT, COASTAL, HELIPAD,
    // RADAR (case-insensitive)
    static bool IsMinorPostType(const char* type);
    // a military zone that counts toward the win: its countsForWin flag and
    // not a minor kind
    static bool CountsForWin(const ZoneRecord& zone);
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
    // the resistance holds any zone right now (a base, the Camp or a town)
    static bool HoldsAnything(const ZoneRegistry& registry);

    // one check: takes the roster when needed, then latches the win.  True
    // only on the tick that latches.  dayNow / minuteNow stamp the win.
    // Always false (and no roster is taken) while the rule is switched off.
    bool Evaluate(const ZoneRegistry& registry, int dayNow = 0, int minuteNow = 0);
    // the lose check (9.2), run after Evaluate on the same tick: latches when
    // there is no HQ (hasHq false), the resistance holds no zone at all, and
    // the campaign has had something to lose (hadHq, or a zone once held).
    // True only on the tick that latches; always false once won, once lost,
    // and while the rule is switched off.  Records the zones held this tick
    // (the "once held" counters of the debriefing) on every call.
    bool EvaluateLost(const ZoneRegistry& registry, bool hasHq, bool hadHq, int dayNow = 0, int minuteNow = 0);
    // add every zone the resistance holds now to the once-held set
    void NoteHeld(const ZoneRegistry& registry);
    // once-held military bases (not CAMP, not CITY) and towns (CITY), by
    // the registry's current types; zones the template lost are skipped
    int BasesOnceHeld(const ZoneRegistry& registry) const;
    int TownsOnceHeld(const ZoneRegistry& registry) const;
    int ZonesOnceHeld() const { return _onceHeld.Size(); }

    // the latch -------------------------------------------------------------
    bool Won() const { return _won; }
    int WonDay() const { return _wonDay; }
    int WonMinute() const { return _wonMinute; }
    // the roster zone whose capture completed the set: the first, in roster
    // order, that turned held on the check that latched.  Empty when the
    // latch came on the first check of the session (a fresh roster or a
    // load knows no "before")
    RString LastTaken() const { return _lastTaken; }
    // the lose latch (9.2); OccupierForces and CounterAttack freeze on it
    bool Lost() const { return _lost; }
    int LostDay() const { return _lostDay; }
    int LostMinute() const { return _lostMinute; }

    // the end request -------------------------------------------------------
    bool EndPending() const { return (_won || _lost) && !_endIssued; }
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
    // the defeat heading and body ("Malden is lost.")
    static RString LoseTitle(const char* island);
    static RString LoseBody(const char* islandPhrase, int day, int bases, int towns);
    // the live debriefing text: the win text for a won campaign, the lose
    // text for a lost one (the island's name and phrase from the world and
    // the registry); false while neither
    bool DebriefingText(RString& title, RString& body) const;

    // engine hook -----------------------------------------------------------
    // per-frame; no-op without an active ZoneRegistry.  Checks the win rule,
    // then the lose rule, at TickInterval, writes the journal line on the win
    // latch, and asks the world for END1 (won) or EMLoser (lost) when the end
    // clock runs out.
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
    // the lose latch's INFO line (D12: no other messaging)
    void OnLost();
    // once-held zones of one kind (towns: CITY; else military bases)
    int CountOnceHeld(const ZoneRegistry& registry, bool towns) const;
    // ask the world for END1 / EMLoser (only while it is still EMContinue)
    void RequestEnd();

    CampaignEndTuning _tuning;
    AutoArray<RosterRow> _roster;
    bool _rosterTaken = false;
    bool _won = false;
    int _wonDay = 0;
    int _wonMinute = 0;
    RString _lastTaken;
    bool _lost = false;
    int _lostDay = 0;
    int _lostMinute = 0;
    // every zone the resistance has held this campaign, by name (saved)
    AutoArray<RosterRow> _onceHeld;
    bool _endIssued = false;
    float _endClock = 0;
    float _accum = 0;
    // transient: heldLast was primed by a check this session (a load or a
    // fresh roster does not know who fell last)
    bool _heldPrimed = false;
};

} // namespace Guerrilla
} // namespace Poseidon
