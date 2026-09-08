#pragma once

// Guerrilla Mode journal: the Compose stage's internal toolkit, shared by
// JournalCompose.cpp (entry point, Pen, NewPage, ListChain),
// JournalComposeOps.cpp (Contents, Dispatches, Operations and its four
// pages) and JournalComposePeoplePlaces.cpp (People, Roster, Places,
// Chronicles, Record, Reference).  Nothing here measures pixels or sees a
// CHTMLContainer; Render (JournalRender.hpp) turns the document into fields.
//
// Reference lifetime: JournalDocument::pages is an AutoArray, so a
// JournalPage& (and any Pen bound to it) is valid only until the next
// NewPage / ListChain::PageFor on the same document.  Bind a fresh Pen per
// item when writing through a ListChain.

#include <Poseidon/UI/Guerrilla/JournalCompose.hpp>
#include <Poseidon/UI/Guerrilla/JournalText.hpp>
#include <Poseidon/Game/Guerrilla/Journal.hpp>

namespace Poseidon::Guerrilla
{
struct ComposeContext
{
    const Journal& journal;
    const JournalPageInputs& in;
    const JournalText::Derived& d;
};

// page-level emitter (replaces Sheet); every composite ends the block.
// Link and Cell are primitives: they add a run to the open block, which the
// caller closes with EndBlock (or the next composite / Gap flushes it).
class Pen
{
  public:
    explicit Pen(JournalPage& page);
    void Title(const RString& text);    // VoiceTitle, keepWithNext
    void Subtitle(const RString& text); // VoiceSerif, pencil, keepWithNext
    void Head(const RString& text);     // VoiceHead with a leadGap (one block), keepWithNext
    void Line(const RString& text, JournalVoice v = VoiceType, JournalInk ink = InkStock);
    void Note(const RString& label, const RString& value,
              JournalInk ink = InkStock); // "Label: " pencil + Sentence(value), hanging 0.06
    void Bullet(const RString& text);
    void Link(const RString& text, const char* href, JournalVoice v = VoiceType);
    void LinkRow(const RString& text, const char* href,
                 const RString& description); // Contents / hub entry: link + serif one-liner
    void Hand(const RString& text, JournalInk ink = InkHand, bool annotation = false); // hanging 0.04
    void Entry(const JournalEntry& e, int today, bool forceDay,
               bool withZone); // annotation row, red on JKDanger; body clamped to HandWords less the stamp
    void Cell(const RString& text, float w, JournalInk ink = InkStock, JournalAlign a = AlignLeft, bool bold = false);
    void Gap();                                      // BlockGap
    void Portrait(const RString& src, bool present); // BlockPortrait
    // open-ended block assembly
    void Run(const JournalRun& run);
    void EndBlock(bool annotation = false);

  private:
    JournalPage& _page;
    JournalBlock _open;
    bool _dirty = false;
};

JournalPage& NewPage(JournalDocument& doc, const char* name, const char* title, const char* parentName,
                     const char* parentTitle);
// five-per-page pager: PageFor(i) returns base for i < 5, "<base>_2" for 5..9, ... creating parts on demand
class ListChain
{
  public:
    ListChain(JournalDocument& doc, const char* base, const char* title, const char* parentName,
              const char* parentTitle, int perPage = ComposeLimits::ListPerPage);
    JournalPage& PageFor(int itemIndex);
    JournalPage& First();
    int Parts() const;

  private:
    JournalDocument& _doc;
    RString _base;
    RString _title;
    RString _parentName;
    RString _parentTitle;
    int _perPage;
};

// T2 (JournalComposeOps.cpp)
void ComposeContents(JournalDocument&, const ComposeContext&);   // "Main" [GM_CONTENTS]
void ComposeDispatch(JournalDocument&, const ComposeContext&);   // GM_DISPATCH
void ComposeOperations(JournalDocument&, const ComposeContext&); // "Plan" [GM_OPERATIONS]
void ComposeObjectives(JournalDocument&, const ComposeContext&); // GM_OBJECTIVES(_n)
void ComposeActions(JournalDocument&, const ComposeContext&);    // GM_ACTIONS(_n)
void ComposeSupply(JournalDocument&, const ComposeContext&);     // GM_SUPPLY
void ComposeStrength(JournalDocument&, const ComposeContext&);   // GM_FACTION
// T3 (JournalComposePeoplePlaces.cpp)
void ComposePeople(JournalDocument&, const ComposeContext&);     // GM_PEOPLE [GM_CELL]
void ComposeRoster(JournalDocument&, const ComposeContext&);     // GM_ROSTER(_n)
void ComposePlaces(JournalDocument&, const ComposeContext&);     // GM_PLACES(_n) [GM_ZONES] + GM_ZONE_<i>
void ComposeChronicles(JournalDocument&, const ComposeContext&); // GM_CHRONICLES
void ComposeRecord(JournalDocument&, const ComposeContext&);     // GM_RECORD(_n) [GM_LOG]
void ComposeReference(JournalDocument&, const ComposeContext&);  // GM_REFERENCE [GM_MAN_INDEX] + GM_MAN_*
} // namespace Poseidon::Guerrilla
