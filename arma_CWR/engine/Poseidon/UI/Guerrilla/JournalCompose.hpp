#pragma once

// Guerrilla Mode journal: the Compose stage.
//
// Compose is pure: (const Journal&, const JournalPageInputs&) -> JournalDocument.
// It decides what every page says (voices, inks, links, list pagination at
// five entries, the 25-word hand caps) and never measures pixels: it never
// includes UIControlsBase.hpp and never sees a CHTMLContainer.  The Render
// stage (JournalRender.hpp) lays the document into the briefing control.
//
// A JournalPage is an ordered list of JournalBlocks; a block is a list of runs
// (fields) with an implied trailing break, so a block is the unit that moves
// whole onto a continuation page.  Voices map onto the control's format slots
// in Render (see the slot table in JournalRender.hpp).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon::Guerrilla
{
class Journal;
struct JournalPageInputs;

enum JournalVoice
{
    VoiceTitle,     // -> HFH3 garamond 1.45 P
    VoiceSerif,     // -> HFH5 garamond 1.1 P
    VoiceHead,      // -> HFH4 couriernewb 1.15 P
    VoiceType,      // -> HFP
    VoiceSmallType, // -> HFP, pencil ink by convention (a voice, not a slot)
    VoiceHand,      // -> HFH6 cwrpen 1.6 P
    VoiceSpacer     // -> HFP, text " " (never the head slot)
};
enum JournalInk
{
    InkStock,
    InkHand,
    InkRed,
    InkPencil,
    InkFaded
};
enum JournalAlign
{
    AlignLeft,
    AlignRight
};

// one field
struct JournalRun
{
    RString text;
    JournalVoice voice = VoiceType;
    JournalInk ink = InkStock;
    RString href;        // "#GM_PLACES" or ""
    float hanging = 0;   // page-width fraction (Sheet::Hanging semantics)
    float cellWidth = 0; // page-width fraction; > 0 = fixed cell (Fit applies in Render)
    bool bold = false;
    JournalAlign align = AlignLeft;
};

enum JournalBlockKind
{
    BlockText,
    BlockPortrait,
    BlockGap
};

// the unit that moves whole onto a continuation page; every block ends in a break
struct JournalBlock
{
    JournalBlockKind kind = BlockText;
    AutoArray<JournalRun> runs;
    bool annotation = false;   // one-line entry/objective/action/deed row: exempt from the 2-hand-blocks rule
    bool leadGap = false;      // BlockText: a short blank line is laid inside the block, ahead of its runs (a Head)
    bool keepWithNext = false; // a Title / Subtitle / Head never ends a physical page while a block follows it
    RString portraitSrc;       // BlockPortrait: "\\gmcore\\portraits\\<key>.paa" or "" (unavailable)
    bool portraitPresent = false;
    int HandWords() const; // words in VoiceHand runs (0 for other blocks)
};

struct JournalPage
{
    RString name;               // section name: base for part 1, "<base>_<part>" after
    RString baseName;           // "GM_RECORD"
    int part = 1;               // Compose's five-entry part; Render may insert height parts
    RString title;              // footer/parent label ("Places")
    AutoArray<RString> aliases; // legacy anchors, page 0 of the chain only
    RString parentName;         // "" on Contents
    RString parentTitle;        // "Contents" / "Operations" / ...
    AutoArray<JournalBlock> blocks;
};

struct JournalDocument
{
    AutoArray<JournalPage> pages;
    int FindPage(const char* name) const; // -1 when absent
    int PartCount(const char* baseName) const;
};

struct ComposeLimits
{
    static constexpr int ListPerPage = 5;
    static constexpr int HandWords = 25;
    static constexpr int HandBlocksPerPage = 2; // non-annotation hand blocks
    static constexpr int ZoneRecordCap = 10;    // deliberate deviation, see D1
    static constexpr int ContentsDescriptionWords = 8;
    static constexpr int NarrativeMinWords = 60, NarrativeMaxWords = 90; // Change 2 Serif
    static constexpr int PortraitMinWords = 35, PortraitMaxWords = 55;   // Change 2
};

JournalDocument ComposeJournal(const Journal& journal, const JournalPageInputs& in);
} // namespace Poseidon::Guerrilla
