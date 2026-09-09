#include <Poseidon/UI/Guerrilla/JournalComposeInternal.hpp>

#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp> // JournalPageInputs

#include <Poseidon/Foundation/platform.hpp> // stricmp

#include <cstring>

namespace Poseidon::Guerrilla
{

using namespace JournalText;

namespace
{

// part 1 = base, else "<base>_<part>" (the same rule as
// JournalRender.hpp's ContinuationName; kept local so Compose never pulls the
// control headers in)
RString PartName(const RString& base, int part)
{
    if (part <= 1)
    {
        return base;
    }
    return Fmt("%s_%d", cstr(base), part);
}

} // namespace

// ===========================================================================
// document model
// ===========================================================================

int JournalBlock::HandWords() const
{
    int words = 0;
    for (int i = 0; i < runs.Size(); i++)
    {
        if (runs[i].voice == VoiceHand)
        {
            words += WordCount(runs[i].text);
        }
    }
    return words;
}

int JournalDocument::FindPage(const char* name) const
{
    for (int i = 0; i < pages.Size(); i++)
    {
        if (stricmp(pages[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

int JournalDocument::PartCount(const char* baseName) const
{
    int count = 0;
    for (int i = 0; i < pages.Size(); i++)
    {
        if (stricmp(pages[i].baseName, baseName) == 0)
        {
            count++;
        }
    }
    return count;
}

// ===========================================================================
// Pen: a page emitter over the document model
// ===========================================================================

Pen::Pen(JournalPage& page) : _page(page) {}

void Pen::Run(const JournalRun& run)
{
    _open.runs.Add(run);
    _dirty = true;
}

// close the open block; Render ends every block in a break, so a block is
// the unit that moves whole onto a continuation page
void Pen::EndBlock(bool annotation)
{
    if (!_dirty)
    {
        return;
    }
    _open.kind = BlockText;
    _open.annotation = annotation;
    _page.blocks.Add(_open);
    _open = JournalBlock();
    _dirty = false;
}

// the title, the subtitle and a head introduce what follows them, so they
// are kept with the next block: Render never ends a physical page on one
void Pen::Title(const RString& text)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceTitle;
    Run(run);
    _open.keepWithNext = true;
    EndBlock();
}

void Pen::Subtitle(const RString& text)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceSerif;
    run.ink = InkPencil;
    Run(run);
    _open.keepWithNext = true;
    EndBlock();
}

// a section head with its short blank line folded into the same block, so
// the gap and the head can never be parted by a page cut
void Pen::Head(const RString& text)
{
    EndBlock();
    _open.leadGap = true;
    JournalRun run;
    run.text = text;
    run.voice = VoiceHead;
    Run(run);
    _open.keepWithNext = true;
    EndBlock();
}

void Pen::Line(const RString& text, JournalVoice v, JournalInk ink)
{
    JournalRun run;
    run.text = text;
    run.voice = v;
    run.ink = ink;
    Run(run);
    EndBlock();
}

// typed fact: "Label: value."
void Pen::Note(const RString& label, const RString& value, JournalInk ink)
{
    JournalRun head;
    head.text = label + RString(": ");
    head.voice = VoiceType;
    head.ink = InkPencil;
    Run(head);
    JournalRun body;
    body.text = Sentence(value);
    body.voice = VoiceType;
    body.ink = ink;
    body.hanging = 0.06f;
    Run(body);
    EndBlock();
}

// a typed bullet with a hanging indent
void Pen::Bullet(const RString& text)
{
    JournalRun dash;
    dash.text = "- ";
    dash.voice = VoiceType;
    dash.ink = InkPencil;
    Run(dash);
    JournalRun body;
    body.text = text;
    body.voice = VoiceType;
    body.hanging = 0.03f;
    Run(body);
    EndBlock();
}

void Pen::Link(const RString& text, const char* href, JournalVoice v)
{
    JournalRun run;
    run.text = text;
    run.voice = v;
    run.href = href;
    Run(run);
}

// a Contents / hub entry: the link, then its one-line serif description in
// pencil, as ONE block.  Render lays the two runs adjacently, so the
// description carries the two-space lead the handbook index used
void Pen::LinkRow(const RString& text, const char* href, const RString& description)
{
    Link(text, href, VoiceType);
    JournalRun desc;
    desc.text = RString("  ") + description;
    desc.voice = VoiceSerif;
    desc.ink = InkPencil;
    Run(desc);
    EndBlock();
}

// handwritten line, blue-black unless told otherwise
void Pen::Hand(const RString& text, JournalInk ink, bool annotation)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceHand;
    run.ink = ink;
    run.hanging = 0.04f;
    Run(run);
    EndBlock(annotation);
}

// a diary entry in the hand: "14:02 Airfield. text" (red pen for danger).
// Entry text is script-authored (gmJournalNote), the one input Compose does
// not control, so the body is clamped to the hand cap less the stamp's words:
// HandWords() sums every hand run in the block, stamp included
void Pen::Entry(const JournalEntry& e, int today, bool forceDay, bool withZone)
{
    const RString stamp = CompactStamp(e.stamp, forceDay ? -1 : today);
    int budget = ComposeLimits::HandWords;
    if (stamp.GetLength() > 0)
    {
        JournalRun when;
        when.text = stamp + RString(" ");
        when.voice = VoiceHand;
        when.ink = InkPencil;
        Run(when);
        budget -= WordCount(stamp);
    }
    RString text = e.text;
    if (withZone && e.zone.GetLength() > 0)
    {
        text = e.zone + RString(". ") + text;
    }
    JournalRun body;
    body.text = ClampWords(text, budget);
    body.voice = VoiceHand;
    body.ink = e.kind == JKDanger ? InkRed : InkHand;
    body.hanging = 0.08f;
    Run(body);
    EndBlock(true);
}

// a fixed-width typed cell (Render truncates with "..." past its width)
void Pen::Cell(const RString& text, float w, JournalInk ink, JournalAlign a, bool bold)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceType;
    run.ink = ink;
    run.cellWidth = w;
    run.align = a;
    run.bold = bold;
    Run(run);
}

// a short blank line: its own block, so it moves with the block after it
void Pen::Gap()
{
    EndBlock();
    JournalBlock gap;
    gap.kind = BlockGap;
    _page.blocks.Add(gap);
}

void Pen::Portrait(const RString& src, bool present)
{
    EndBlock();
    JournalBlock block;
    block.kind = BlockPortrait;
    block.portraitSrc = src;
    block.portraitPresent = present;
    _page.blocks.Add(block);
}

// ===========================================================================
// pages and the five-per-page pager
// ===========================================================================

JournalPage& NewPage(JournalDocument& doc, const char* name, const char* title, const char* parentName,
                     const char* parentTitle)
{
    JournalPage page;
    page.name = name;
    page.baseName = name;
    page.part = 1;
    page.title = title;
    page.parentName = parentName;
    page.parentTitle = parentTitle;
    const int at = doc.pages.Add(page);
    return doc.pages[at];
}

ListChain::ListChain(JournalDocument& doc, const char* base, const char* title, const char* parentName,
                     const char* parentTitle, int perPage)
    : _doc(doc), _base(base), _title(title), _parentName(parentName), _parentTitle(parentTitle),
      _perPage(perPage > 0 ? perPage : ComposeLimits::ListPerPage)
{
}

int ListChain::Parts() const
{
    return _doc.PartCount(_base);
}

// the page item i lands on, creating the parts up to it; a continuation
// part carries the same title and parent and a pencil "<Title>, continued."
JournalPage& ListChain::PageFor(int itemIndex)
{
    if (itemIndex < 0)
    {
        itemIndex = 0;
    }
    const int part = itemIndex / _perPage + 1;
    while (Parts() < part)
    {
        const int next = Parts() + 1;
        const RString name = PartName(_base, next);
        JournalPage& page = NewPage(_doc, name, _title, _parentName, _parentTitle);
        page.baseName = _base;
        page.part = next;
        if (next > 1)
        {
            Pen pen(page);
            pen.Subtitle(_title + RString(", continued."));
        }
    }
    const int index = _doc.FindPage(PartName(_base, part));
    return _doc.pages[index];
}

JournalPage& ListChain::First()
{
    return PageFor(0);
}

// ===========================================================================
// entry point
// ===========================================================================

JournalDocument ComposeJournal(const Journal& journal, const JournalPageInputs& in)
{
    JournalDocument doc;
    const Derived d = Derive(in);
    const ComposeContext ctx{journal, in, d};
    ComposeContents(doc, ctx);
    ComposeDispatch(doc, ctx);
    ComposeOperations(doc, ctx);
    ComposeObjectives(doc, ctx);
    ComposeActions(doc, ctx);
    ComposeSupply(doc, ctx);
    ComposeStrength(doc, ctx);
    ComposePeople(doc, ctx);
    ComposeRoster(doc, ctx);
    ComposePlaces(doc, ctx);
    ComposeChronicles(doc, ctx);
    ComposeRecord(doc, ctx);
    ComposeReference(doc, ctx);
    return doc;
}

} // namespace Poseidon::Guerrilla
