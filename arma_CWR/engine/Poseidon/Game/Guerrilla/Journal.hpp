#pragma once

// Guerrilla Mode field journal - the persistent campaign record behind the
// map screen's Notes / Plan pages.  Three small tables, all script-fed
// through the gmJournal* commands and all serialized with the campaign:
//
//   * diary entries   - a capped running log of campaign events
//                       ("Day 3 14:20  Outpost liberated"), newest last;
//   * objectives      - keyed rows with ACTIVE / DONE / FAILED / HIDDEN
//                       state, rendered on the Plan page next to the
//                       engine-computed standing goal and next steps;
//   * status lines    - keyed "label: text" rows the scripts publish for
//                       facts only they know (companions, gear unlocks),
//                       merged into the Situation block on the Notes page.
//
// The journal is pure data: no world access, no UI.  The page renderer
// lives in UI/Guerrilla/GuerrillaJournalPages.cpp and reads this plus the
// live native state (ZoneRegistry / AlertMachine / UndercoverSystem).  A
// revision counter lets the open map notice a change and repaint.
//
// Not gated on the ZoneRegistry being active (like StashRegistry): an empty
// journal costs nothing and is skipped by the save; the renderer is what
// keys off CfgGuerrillaZones.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
namespace Guerrilla
{

// objective states; the names mirror the stock objStatus vocabulary
enum JournalObjectiveState
{
    JOActive = 0,
    JODone = 1,
    JOFailed = 2,
    JOHidden = 3
};

struct JournalEntry
{
    RString stamp; // "Day 3 14:20" (or "" when logged without a clock)
    RString text;

    LSError Serialize(ParamArchive& ar);
};

struct JournalObjective
{
    RString id;   // script key, case-insensitive, unique
    RString text; // player-facing line
    int state = JOActive;

    LSError Serialize(ParamArchive& ar);
};

struct JournalStatusLine
{
    RString key;  // label shown on the page, also the upsert key
    RString text; // value text; an empty write removes the row

    LSError Serialize(ParamArchive& ar);
};

class Journal : public SerializeClass
{
  public:
    Journal() = default;

    // engine-wide instance (used by the World hooks, the map page renderer
    // and the script commands)
    static Journal& Instance();

    // hard ceiling on the diary; the oldest entry is dropped past it
    static constexpr int MaxEntries = 200;

    // lifecycle -----------------------------------------------------------
    void Clear();
    void InitMission(); // Clear; no config of its own

    // diary -----------------------------------------------------------------
    // appends (stamp may be empty); no-op on empty text
    void AddEntry(RString stamp, RString text);
    int EntryCount() const { return _entries.Size(); }
    const JournalEntry& Entry(int i) const { return _entries[i]; } // 0 = oldest

    // objectives ------------------------------------------------------------
    // upsert by id; an empty text keeps the existing text (state-only update)
    void SetObjective(RString id, RString text, int state);
    int ObjectiveCount() const { return _objectives.Size(); }
    const JournalObjective& Objective(int i) const { return _objectives[i]; }
    int FindObjective(const char* id) const;             // -1 when unknown
    static int ObjectiveStateFromName(const char* name); // -1 when unknown
    static const char* ObjectiveStateName(int state);

    // status lines ----------------------------------------------------------
    // upsert by key; an empty text removes the row
    void SetStatus(RString key, RString text);
    int StatusCount() const { return _status.Size(); }
    const JournalStatusLine& Status(int i) const { return _status[i]; }
    int FindStatus(const char* key) const; // -1 when unknown

    // queries ---------------------------------------------------------------
    bool IsEmpty() const { return _entries.Size() == 0 && _objectives.Size() == 0 && _status.Size() == 0; }
    // bumped on every mutation and on load; the map compares it to repaint
    unsigned Revision() const { return _revision; }

    // save/load (plain values, single pass)
    LSError Serialize(ParamArchive& ar) override;

  private:
    AutoArray<JournalEntry> _entries;
    AutoArray<JournalObjective> _objectives;
    AutoArray<JournalStatusLine> _status;
    unsigned _revision = 0;
};

// "Day N HH:MM" from the world clock and the script-owned campaign day
// counter (gmDayCount; day 1 when undefined).  Empty when no world is up,
// so the pure core and its unit tests never touch the clock.
RString JournalStampNow();

// The island's player-facing name: CfgWorlds >> <world> >> description
// ("Malden"), falling back to the world class name; empty with no world
// header.  Defined in JournalCommands.cpp (backs the gmIslandName nular).
RString IslandDisplayName();

} // namespace Guerrilla
} // namespace Poseidon
