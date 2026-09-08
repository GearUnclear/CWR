#pragma once

// The issue #57 name bank as compiled tables, plus the five-slot assembler.
//
// TWO ORTHOGONAL AXES (spec.md section 3, last bullet): the REGIONAL POOL
// supplies first + last names and is chosen by the faction descriptor's
// `namePool` key; the NICKNAME TONE BANK supplies prefix/describer/title and
// is chosen by campaign ALLEGIANCE (resistance = friendly, occupier =
// hostile). A resistance IDF campaign draws israeli names with the FRIENDLY
// bank. The two source lists (international_good and western_evil) are
// flattened into one kNamePools[] union plus two banks.
//
// The tables are GENERATED from tests/fixtures/legend-names/issue57-names.json
// by tools/legend-names/gen_legend_names.py; edit the fixture and rerun the
// script rather than hand-editing LegendNames.cpp, and run the script with
// --check to prove the compiled tables still match the fixture.
//
// This translation unit is PURE: no config, no world, no singleton, no state.
// Every draw is a pure function of (key, channel) through LegendSeed.hpp, so
// the same campaign seed always yields the same name.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>

namespace Poseidon::Guerrilla
{

enum NicknameTone
{
    ToneFriendly = 0,
    ToneHostile = 1
};

enum NameSlot
{
    SlotPrefix = 0,
    SlotFirst = 1,
    SlotDescriber = 2,
    SlotLast = 3,
    SlotTitle = 4,
    NNameSlots = 5
};

// One regional pool: the personal names, never the nickname words.
struct NamePool
{
    const char* region;
    const char* const* first;
    int nFirst;
    const char* const* last;
    int nLast;
};

// One tone bank: the nickname words, never personal names.
struct NicknameBank
{
    const char* const* prefix;
    int nPrefix;
    const char* const* describer;
    int nDescriber;
    const char* const* title;
    int nTitle;
};

int NamePoolCount(); // 33
const NamePool& NamePoolAt(int i);
int FindNamePool(const char* region); // exact, case-insensitive; -1 unknown
const NicknameBank& Bank(NicknameTone tone);

struct NameParts
{
    RString prefix, first, describer, last, title;
};

// "Iron Rami \"The Fox\" Haddad The Brave"
//  * non-empty slots joined by single spaces in slot order
//  * a describer beginning "The " is wrapped in ASCII double quotes, capital kept
//  * a title is emitted VERBATIM, capital kept: issue #57's own worked
//    convention writes "The Brave" (plan amendment [medium] "title article
//    rule vs issue #57"). The assembled string is written onto the body as
//    AIUnitInfo::_name and persisted, so changing this later is a save
//    migration, not a formatting tweak.
//  * leading-sigil guard: a leading slot that would make the string start with
//    '@' or '$' is dropped and the assembly retried (Localize blanks such a
//    string on a marker label, Stringtable.cpp:567-593)
RString AssembleDisplayName(const NameParts& parts);

RString PickFirst(int pool, unsigned long long key, unsigned channel);
RString PickLast(int pool, unsigned long long key, unsigned channel);
// slot is SlotPrefix, SlotDescriber or SlotTitle; the personal-name slots and
// anything out of range return "". Every award draw names its OWN channel:
// Roll(key, CH_AWARD1, 3) picks the first slot and CH_AWARD1WORD its word,
// Roll(key, CH_AWARD2, 2) the second and CH_AWARD2WORD its word. Never pass
// CH_AWARD1 | CH_AWARD2: that is a bitwise or and collapses both awards onto
// one channel (plan amendment [medium] "award channels").
RString PickSlotWord(NicknameTone tone, int slot, unsigned long long key, unsigned channel);

// namePool key -> pool index; side fallback WEST western / EAST eastern_europe
// / GUER levant; ONE LOG_WARN per (faction, token); never returns -1.
int ResolveNamePool(const char* namePoolValue, const char* side, const char* factionForLog);

} // namespace Poseidon::Guerrilla
