#pragma once

// The seeded, local, template-driven campaign history.
//
// No runtime AI, no network, no config reads: a pure function from
// HistoryInputs to HistoryRecord, unit-testable with no world. Three beats -
// an ancient grievance, a broken settlement, a remembered catastrophe or
// stand - told in plain dossier prose, past tense, with the shipped
// factions as the present inheritors of a quarrel much older than either.
//
// Bare faction display names modify a plural group noun, for example
// "Soviet Army troops" or "FIA fighters". Full names and acronyms therefore read
// naturally without an article, possessive, or invented founding date. Which
// nouns a slot may take depends on the slot and the faction: a slot whose verb
// is about remembering people ("remember the dead", "keep the names") takes a
// people noun, never "units" or "forces"; "fighters" and "members" belong to an
// irregular roster, "troops" and "soldiers" to a regular army; a noun that
// repeats the name's own last word ("Frontier Force forces") is never added,
// and a name that is already a plural group ("Israel Defense Forces") stands
// bare. Within those rules each faction's two slots take distinct labels.
//
// The record is persisted VERBATIM as resolved prose plus indices plus a
// version, so reopening the journal or loading a save can never reroll it.
// HistoryRecord derives from SerializeClass because ParamArchive has no
// generic single-struct by-name overload (ParamArchive.hpp lists bool / int /
// unsigned char / float / Time / TimeSec / RString / Vector3 / Matrix4 /
// Color / SerializeClass& only); LegendRow and LegendDeed deliberately do NOT
// derive from it, because array items reach Serialize through
// SerializeArrayItem, which calls value.Serialize(arItem) directly.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>
#include <Poseidon/Game/Guerrilla/WorldNames.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon::Guerrilla
{

// Bumping this changes nothing for a campaign already under way: the record is
// persisted as resolved prose and is never regenerated on load.
constexpr int kHistoryVersion = 3;
constexpr int kHistoryEvents = 3;    // beats: ancient grievance, broken settlement, stand
constexpr int kHistoryOpenSlots = 4; // opening prose slots A B C D
constexpr int kHistoryPhrases = 4;   // faction constructions the history itself spends

// Word budgets, all inclusive.
constexpr int kHistoryOpeningMinWords = 80;
constexpr int kHistoryOpeningMaxWords = 100;
constexpr int kHistoryEventMinWords = 30;
constexpr int kHistoryEventMaxWords = 60;
constexpr int kHistoryBioMinWords = 20;
constexpr int kHistoryBioMaxWords = 55;
// What a dossier page can actually show beside the portrait box. The journal's
// Compose stage clamps the biography to this many words
// (JournalComposePeoplePlaces.cpp kBioWordsBesidePortrait, which static_asserts
// against this constant), so the generator is asked for a variant that already
// fits and the clamp stays a guard for saves written before it did.
constexpr int kHistoryBioPageWords = 45;

struct HistoryInputs
{
    RString resistanceName, occupierName, islandName; // "" island -> "this country"
    // the descriptors' `irregular` keys (WorldNames FactionIsIrregular)
    bool resistanceIrregular = false, occupierIrregular = false;
    // the template's locative phrase for the island ("in the Sinai"); "" reads
    // "on <island word>", or "in this country" when there is no island word
    RString islandPhrase;
    AutoArray<PlaceName> settlements, features;
    AutoArray<RString> zoneNames; // ZoneRegistry fallback when the world has no Names block
    AutoArray<bool> zoneIsTown;   // parallel to zoneNames (CITY zones); a missing entry reads false
    unsigned seed = 0;
};

// What kind of place an event happened at, persisted beside its name so a
// biography generated later can say "from near <hill>" rather than "from
// <hill>". Unknown is a record written before the kind existed: it renders as
// it always did.
enum HistoryPlaceKind
{
    HPKUnknown = 0,
    HPKSettlement = 1,
    HPKOther = 2 // a map feature, a zone name or the generic table
};

struct HistoryRecord : public SerializeClass
{
    int version = 0; // 0 = absent
    unsigned seed = 0;
    int eventIndex[kHistoryEvents] = {0, 0, 0};
    int openingIndex[kHistoryOpenSlots] = {0, 0, 0, 0};
    // The faction constructions this history spent, in insertion order:
    // opening slot B, opening slot D, event one, event two. Persisted so a
    // biography generated later, at row creation, can take the one construction
    // the history did not use.
    // -1 is a bare name that is already a plural group ("Israel Defense Forces").
    int phraseIndex[kHistoryPhrases] = {0, 0, 0, 0};
    RString openingPage1, openingPage2; // two BLOCKS; Render decides pages
    RString eventTitle[kHistoryEvents];
    RString eventText[kHistoryEvents];
    RString eventPlace[kHistoryEvents];
    int eventPlaceKind[kHistoryEvents] = {HPKUnknown, HPKUnknown, HPKUnknown};

    bool Present() const { return version != 0; }
    // The construction index none of phraseIndex[] holds; 0 if a corrupt record
    // somehow spends them all.
    int BioPhraseIndex() const;
    LSError Serialize(ParamArchive& ar) override;
};

// Test seam: -1 in any slot means "roll / search normally". Forcing A and B
// only still runs the C x D budget search, which is how the table lint proves
// every (A, B) pair can be completed inside the opening budget.
struct HistoryForcedIndices
{
    int opening[kHistoryOpenSlots] = {-1, -1, -1, -1};
    int events[kHistoryEvents] = {-1, -1, -1};
    int phrases[kHistoryPhrases] = {-1, -1, -1, -1};
};

// Pure: byte-identical output for equal inputs, on every platform.
HistoryRecord GenerateHistory(const HistoryInputs& in);
HistoryRecord GenerateHistoryForced(const HistoryInputs& in, const HistoryForcedIndices& forced);

// Strictly PRE-campaign background, hung on one of the three shared history
// events so every dossier and the Chronicles read as one past. It never
// mentions current rank, XP, kill totals, captured zones or other live deeds.
// Earlier violence and service can be part of a person's past. Gender-neutral
// pronouns keep long display names from being repeated.
//
// maxWords is the UPPER half of the word budget the variant walk selects
// against; the caller passes the budget of the page the text has to fit
// (kHistoryBioPageWords for the journal dossier), so the composed page shows
// the whole biography instead of an ellipsis. The default keeps the wider band
// for a caller with no page of its own.
RString GenerateBio(const HistoryRecord& history, int eventIndex, const RString& displayName,
                    const RString& factionName, bool hostile, unsigned long long key,
                    int maxWords = kHistoryBioMaxWords);
RString GenerateBioForced(const HistoryRecord& history, int eventIndex, const RString& displayName,
                          const RString& factionName, bool hostile, unsigned long long key, int forcedVariant,
                          int maxWords = kHistoryBioMaxWords);

// GenerateBio for a caller drawing several biographies in one campaign: the
// variants whose bit is set in excludedVariants are skipped (draw without
// replacement), the chosen variant comes back through outVariant. When every
// variant is excluded the walk ignores the mask rather than return nothing.
// With excludedVariants == 0 the result is exactly GenerateBio's.
RString GenerateBioDistinct(const HistoryRecord& history, int eventIndex, const RString& displayName,
                            const RString& factionName, bool hostile, unsigned long long key, int maxWords,
                            unsigned excludedVariants, int* outVariant);

// The variant a persisted biography was rendered from, recovered by rendering
// every variant of its cell with the same name and comparing; -1 when none
// matches (a biography from older templates, or a different name).
int HistoryBioVariantOf(const HistoryRecord& history, int eventIndex, const RString& displayName,
                        const RString& factionName, bool hostile, const RString& bio);

// "Lebanon (80's)" -> "Lebanon"; "Lebanon80" -> "Lebanon"; "Malden" -> "Malden";
// "" or a still-numeric result -> "this country".
RString HistoryIslandWord(const RString& display);

// The island as a place, preposition first: the authored phrase when there is
// one ("in the Sinai"), else "on <island word>" (every stock world is an
// island), else "in this country". Used ONLY in opening slot A, as {IslandIn};
// {Island} there is the same phrase without its preposition ("the Sinai").
RString HistoryIslandPhrase(const RString& islandDisplay, const RString& authoredPhrase);

// The faction construction for one insertion point: the name plus the label
// at phraseIndex, or the bare name for an index of -1. Exposed for the lints.
RString HistoryFactionGroup(const RString& factionName, int phraseIndex);

// The labels insertion point `slot` (0 opening B, 1 opening D, 2 event one,
// 3 event two) may give a faction of that name and kind, as a bit per label
// index; 0 means the name stands bare.
unsigned HistoryAllowedPhrases(int slot, const RString& factionName, bool irregular);

// Whitespace-separated token count; the one word counter the budgets use.
int HistoryWordCount(const char* text);

// Every template string in this translation unit - openings, event titles,
// event texts, biographies, faction constructions and the generic place table -
// so the lints can walk them without friend access.
int HistoryTemplateStringCount();
const char* HistoryTemplateString(int i);

// Table shape, for the exhaustive budget lint.
int HistoryVariantCount();       // 6, per opening slot / per beat / per bio cell
int HistoryFactionPhraseCount(); // 6
const char* HistoryFactionPhrase(int i);

} // namespace Poseidon::Guerrilla
