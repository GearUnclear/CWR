#include <Poseidon/UI/Guerrilla/JournalComposeInternal.hpp>

#include <Poseidon/Game/Guerrilla/FactionHistory.hpp>      // kHistoryBioPageWords
#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp> // JournalPageInputs
#include <Poseidon/UI/Guerrilla/JournalManual.hpp>         // the handbook chapters

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/platform.hpp>       // stricmp

#include <climits> // INT_MAX (the handbook table's ~ markers)
#include <cstring>

// Guerrilla Mode journal, Compose part B: People index and the character
// dossiers, Roster, Places index and the zone pages, Chronicles hub, the
// generated history, Record, Reference index and the handbook chapters.
//
// Everything here is pure: (Journal, JournalPageInputs, Derived) in, pages of
// blocks out through Pen / NewPage / ListChain.  No pixel is measured and no
// CHTMLContainer is seen; Render (JournalRender.hpp) lays the document.
//
// Content is ported from the retired page builders (BuildZones, BuildCell,
// BuildDiary, ManualTable, BuildHandbook in GuerrillaJournalPages.cpp), with
// the design's changes of shape:
//
//   - dynamic lists paginate at five entries (ComposeLimits::ListPerPage) into
//     "<base>_2", "<base>_3" ...; group heads and day heads do not count
//   - roster names are wrapping runs, never a fixed-width cell, so a five-slot
//     name is never cut to "..."
//   - the zone record keeps its 10-line cap (ComposeLimits::ZoneRecordCap) as
//     a deliberate deviation from the five rule: it is a filtered excerpt of
//     the record with a link overflow to GM_RECORD, not a paginated list
//   - the Reference index is a static ten-row menu, exempt from the five rule
//
// Reference lifetime: JournalDocument::pages is an AutoArray, so a
// JournalPage& (and a Pen bound to it) is valid only until the next NewPage /
// ListChain::PageFor on the same document.  Every list below binds a fresh
// Pen per item for that reason.

namespace Poseidon::Guerrilla
{

using namespace JournalText;

namespace
{

// the Reference index's hanging indent: a wrapped chapter subtitle sits under
// the chapter title, not back under the row number.  The roster and record
// rows hang at 0.04 because their text starts at the left margin; an index row
// opens with the number cell (0.06) and its spacer (0.02), so the same rule
// puts the tail at 0.08 (page-width fraction, Sheet::Hanging semantics)
const float kIndexHanging = 0.06f + 0.02f;

// a pencil run in the typed voice added to the open block (the caller ends
// the block); the composite this replaces is Sheet::Text(text, kType, &kPencil)
void PencilRun(Pen& pen, const RString& text)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceType;
    run.ink = InkPencil;
    pen.Run(run);
}

// a stock typed run added to the open block
void TypeRun(Pen& pen, const RString& text, JournalInk ink = InkStock, float hanging = 0, bool bold = false)
{
    JournalRun run;
    run.text = text;
    run.voice = VoiceType;
    run.ink = ink;
    run.hanging = hanging;
    run.bold = bold;
    pen.Run(run);
}

// "N under arms[, M wounded]." (the old Cell stat, shared by People and the
// roster's first page)
RString UnderArms(const Derived& d)
{
    RString stat = Fmt("%d under arms", d.withPlayer + d.holding);
    if (d.wounded > 0)
    {
        stat = stat + Fmt(", %d wounded", d.wounded);
    }
    return stat + RString(".");
}

// the Arms block that closes the roster: standard issue / next pattern from
// the journal's status lines, then the cache and garage facts
void ArmsBlock(Pen& pen, const Journal& journal, const JournalPageInputs& in)
{
    pen.Head("Arms");
    bool anyArms = false;
    for (int i = 0; i < journal.StatusCount(); i++)
    {
        const JournalStatusLine& st = journal.Status(i);
        if (stricmp(st.key, "Unlocked gear") == 0 || stricmp(st.key, "Standard issue") == 0)
        {
            pen.Note("Standard issue", st.text);
            anyArms = true;
        }
        else if (stricmp(st.key, "Next pattern") == 0)
        {
            pen.Note("Next pattern", st.text);
            anyArms = true;
        }
    }
    if (!anyArms)
    {
        pen.Note("Standard issue", "the faction's basic rifle", InkPencil);
    }
    if (in.hqEstablished)
    {
        pen.Note("Cache", in.hqZone + RString(", at the headquarters"));
        pen.Note("Garage", in.garage.Size() > 0 ? JoinNames(in.garage, 4)
                           : in.garageCount > 0 ? Fmt("%d vehicles", in.garageCount)
                                                : RString("empty"));
    }
    else if (in.stashCount > 0)
    {
        pen.Note("Caches", Num((float)in.stashCount));
    }
}

// the handbook's "|a|b|c" (or "!a|b|c" header) row: fixed cells for the
// first one or two columns, the last column a wrapping run.  These cells are
// the one place authored text may live in a cellWidth run (Render fits them
// with "..."); their widths are the retired ManualTable's
void ManualTableRow(Pen& pen, const char* line, bool header)
{
    AutoArray<RString> cells;
    const char* p = line + 1;
    while (*p)
    {
        const char* q = strchr(p, '|');
        if (!q)
        {
            cells.Add(RString(p));
            break;
        }
        cells.Add(RString(p, (int)(q - p)));
        p = q + 1;
    }
    const int n = cells.Size();
    float widths[3] = {0.27f, 0.73f, 0};
    if (n >= 3)
    {
        widths[0] = 0.27f;
        widths[1] = 0.36f;
        widths[2] = 0.37f;
    }
    for (int i = 0; i < n && i < 3; i++)
    {
        RString text = cells[i];
        if (header)
        {
            pen.Cell(text, widths[i], InkPencil);
            continue;
        }
        JournalInk ink = i == 0 ? InkStock : InkPencil;
        bool bold = false;
        if (text.GetLength() >= 2 && text[0] == '~')
        {
            switch (text[1])
            {
                case 'r':
                    ink = InkRed;
                    break;
                case 'w':
                    ink = InkStock;
                    bold = true;
                    break;
                default:
                    ink = InkStock;
                    break;
            }
            text = text.Substring(2, INT_MAX);
        }
        if (i == n - 1 || i == 2)
        {
            // The last cell is the only wrapping run of the row, so it carries
            // the hanging indent: without one its tail wraps back to the left
            // margin and reads as a new row in the FIRST column ("under 20 m,
            // or" / "from behind").  The indent is the sum of the columns
            // ahead of it, which is where the run's own text starts, the same
            // Sheet::Hanging semantics the Reference index rows use
            float hanging = 0;
            for (int c = 0; c < i && c < 3; c++)
            {
                hanging += widths[c];
            }
            TypeRun(pen, text, ink, hanging, bold);
        }
        else
        {
            pen.Cell(text, widths[i], ink, AlignLeft, bold);
        }
    }
    pen.EndBlock();
}

// ---------------------------------------------------------------------------
// the named (Change 2): dossier anchors, captions and the portrait source
// ---------------------------------------------------------------------------

// Beside a portrait box the dossier has half a page left for prose
// (JournalRender.hpp kPortraitMaxHeightFraction), so the biography is budgeted
// to the low half of the 35-55 band.  The GENERATOR owns that budget: the
// registry asks FactionHistory for a variant that already fits
// (kHistoryBioPageWords), because nothing downstream can carry the overflow -
// "Full record" lists the character's journal entries by charId and never the
// biography, so a clamped word is a word the player cannot read anywhere.
// The clamp below is what stands between a save written before that budget
// existed (biographies are persisted, never regenerated) and a dossier that
// runs off the page.  It is NOT gated on the photograph existing: Render
// reserves the identical box for the "Photograph unavailable" treatment
// (AddImage applies the same split units on its no-texture branch), so gating
// on presence would leave the heavier of the two pages unclamped.
const int kBioWordsBesidePortrait = 45;
static_assert(kBioWordsBesidePortrait == kHistoryBioPageWords,
              "the composed clamp and the generator's page budget must be one number");

RString WhoAnchor(const RString& id)
{
    return RString("GM_WHO_") + id;
}

RString RecordAnchor(const RString& id)
{
    return WhoAnchor(id) + RString("_REC");
}

// The history's event pages are GM_HIST_EV<k>, NOT GM_HISTORY_<k>: Render names
// a continuation page "<base>_<part>" and reuses sections by name, so a
// GM_HISTORY_2 event page and the history hub's own second page would silently
// merge into one section instead of colliding visibly.
RString HistoryEventAnchor(int k)
{
    return Fmt("GM_HIST_EV%d", k);
}

// "<role>, <status>." as the People row's description and the dossier's caption
RString CharacterCaption(const JournalCharacterView& ch)
{
    RString caption = ch.status;
    if (ch.role.GetLength() > 0)
    {
        caption = ch.status.GetLength() > 0 ? ch.role + RString(", ") + ch.status : ch.role;
    }
    return Sentence(Cap(caption));
}

// Render draws block.portraitSrc verbatim and Gather probed
// "<portraitDir>\<key>.paa" for existence, so both are built from the same
// injected directory: a Change 4 move of the folder can never leave a dossier
// that reports a photograph pointing at a texture that does not resolve.  The
// leading backslash is what makes AddImage skip the briefing-relative search.
// portraitDir is empty in every unit test, so the box is reserved and the
// "Photograph unavailable" treatment is drawn instead.
RString PortraitSrc(const JournalPageInputs& in, const JournalCharacterView& ch)
{
    if (in.portraitDir.GetLength() == 0 || ch.portraitKey.GetLength() == 0)
    {
        return RString();
    }
    return RString("\\") + in.portraitDir + RString("\\") + ch.portraitKey + RString(".paa");
}

// What each beat IS, as the hub row's description.  The beat kinds are fixed
// by the generator's three tables, so the gloss is compiled here rather than
// carried on the view.  It is deliberately NOT the place: every shipped event
// title ends in the place name, so a place description would repeat the last
// word of its own link and the hub would read as a mis-wired table.  The beat
// PAGE still carries the place as its subtitle, which is where it says
// something the title has not already said.
const char* HistoryBeatGloss(int k)
{
    static const char* const kGloss[3] = {"The ancient grievance.", "The broken settlement.", "The stand remembered."};
    return (k >= 0 && k < 3) ? kGloss[k] : "";
}

// a hub row for a history beat, with the beat's gloss as its description
void HistoryLinkRow(Pen& pen, const RString& title, const RString& href, const RString& description)
{
    if (description.GetLength() > 0)
    {
        pen.LinkRow(title, cstr(href), description);
        return;
    }
    pen.Link(title, cstr(href));
    pen.EndBlock();
}

// the "@standing" line of the Undercover chapter: the live cover state rides
// in a section head; the list below it stays pure reference
void StandingHead(Pen& pen, const JournalPageInputs& in)
{
    if (!in.undercoverArmed)
    {
        pen.Head("Your standing");
        return;
    }
    const char* word = in.undercoverStatus == 2 ? "BLOWN" : in.undercoverStatus == 1 ? "SUSPECTED" : "CLEAN";
    RString head = Fmt("Your standing: now %s", word);
    if (in.undercoverStatus == 2)
    {
        head = head + Fmt(", %d %s your face", in.undercoverWitnesses,
                          in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know");
    }
    pen.Head(head);
}

} // namespace

// ===========================================================================
// PEOPLE: the index of the named
// ===========================================================================

void ComposePeople(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const int perPage = ComposeLimits::ListPerPage;
    // The index IS part 1 of its own chain, so the first character row lands
    // after the roster entry instead of on a page of its own.
    ListChain chain(doc, "GM_PEOPLE", "People", "Main", "Contents", perPage);
    {
        JournalPage& page = chain.First();
        page.aliases.Add(RString("GM_CELL"));
        Pen pen(page);
        pen.Title("People");
        pen.Subtitle(UnderArms(ctx.d));
        pen.LinkRow("The roster", "#GM_ROSTER", "Every fighter, what they carry.");
    }

    // Companions, Enemy Legends and Memorials are THREE LISTS ON ONE RUNNING
    // INDEX, not three chains.  ListChain has no chain-local cursor (PageFor is
    // itemIndex / perPage + 1, and Parts() counts the document's pages by base
    // name), so three chains over "GM_PEOPLE" would each restart at item 0 and
    // pile fifteen rows onto page 1.  Flattening is also the convention the
    // roster and the record already follow: a group head rides with its first
    // row, repeats at the top of a continuation page, and never consumes one of
    // the five.  Defeated Legends stay under Enemy Legends (their dossier is
    // permanent); a fallen companion moves to Memorials.
    struct Group
    {
        const char* head;
        int kind;
        bool alive;
    };
    static const Group kGroups[] = {{"Companions", 0, true}, {"Enemy Legends", 1, true}, {"Memorials", 0, false}};
    int item = 0;
    for (int g = 0; g < 3; g++)
    {
        int headedPart = 0;
        for (int c = 0; c < in.characters.Size(); c++)
        {
            const JournalCharacterView& ch = in.characters[c];
            if (ch.kind != kGroups[g].kind)
            {
                continue;
            }
            // the alive split applies to companions only: an enemy Legend is
            // listed whether it is at large or defeated
            if (kGroups[g].kind == 0 && ch.alive != kGroups[g].alive)
            {
                continue;
            }
            JournalPage& page = chain.PageFor(item);
            Pen pen(page);
            const int part = item / perPage + 1;
            if (part != headedPart)
            {
                pen.Head(kGroups[g].head);
                headedPart = part;
            }
            pen.LinkRow(ch.displayName, cstr(RString("#") + WhoAnchor(ch.id)), CharacterCaption(ch));
            item++;
        }
    }

    // the dossiers hang off this page, so they are composed here rather than
    // from ComposeJournal: one call order, one file
    ComposeWho(doc, ctx);
}

// ===========================================================================
// WHO: one dossier per named character, and that character's own record
// ===========================================================================

void ComposeWho(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Journal& journal = ctx.journal;
    for (int c = 0; c < in.characters.Size(); c++)
    {
        const JournalCharacterView& ch = in.characters[c];
        if (ch.id.GetLength() == 0)
        {
            continue;
        }
        const RString anchor = WhoAnchor(ch.id);
        const RString record = RecordAnchor(ch.id);
        {
            JournalPage& page = NewPage(doc, cstr(anchor), cstr(ch.displayName), "GM_PEOPLE", "People");
            Pen pen(page);
            // the box is reserved whether or not the photograph exists, so the
            // page has the same shape either way and the unavailable treatment
            // reads as a blank frame in the file rather than a missing block
            pen.Portrait(PortraitSrc(in, ch), ch.portraitPresent);
            pen.Title(ch.displayName);
            const RString caption = CharacterCaption(ch);
            if (ch.defeated || !ch.alive)
            {
                // Pen::Subtitle is pencil by contract; a fallen companion and a
                // defeated Legend take the red pen, so the caption is a serif
                // line of its own
                pen.Line(caption, VoiceSerif, InkRed);
            }
            else
            {
                pen.Subtitle(caption);
            }
            // Where an enemy Legend stands, in pencil under the caption: the
            // one line that makes him a place on the map rather than a name in
            // a list, and the same zone his marker and his objective name.  It
            // is read off ch.zone, which Gather already fills from the row, so
            // nothing about the location is assembled twice.  Companions are
            // deliberately excluded: their zone is a last-seen reading that goes
            // stale the moment they move, and their status line already says
            // whether they are with the player.
            if (ch.kind == 1 && ch.zone.GetLength() > 0)
            {
                pen.Line(RString("near ") + ch.zone, VoiceType, InkPencil);
            }
            if (ch.bio.GetLength() > 0)
            {
                // 35-55 words: a Serif block, never a hand block (the hand cap
                // is 25 words)
                pen.Line(ClampWords(ch.bio, kBioWordsBesidePortrait), VoiceSerif);
            }
            if (ch.deedLatest.GetLength() > 0)
            {
                // one annotation row: exempt from the two-hand-blocks rule, still
                // inside the 25-word hand cap
                pen.Hand(ClampWords(ch.deedLatest, ComposeLimits::HandWords), InkHand, true);
            }
            pen.Link("Full record", cstr(RString("#") + record));
            pen.EndBlock();
        }

        // The character's own record: the journal lines carrying this id,
        // newest first, five to a page.  Filtering is by charId ONLY - a line
        // is never matched by finding the base name in its text, because two
        // fighters can share a first name and an earned name is not the name
        // the older lines were written under.
        ListChain chain(doc, cstr(record), cstr(ch.displayName), cstr(anchor), cstr(ch.displayName),
                        ComposeLimits::ListPerPage);
        int count = 0;
        for (int e = 0; e < journal.EntryCount(); e++)
        {
            if (stricmp(journal.Entry(e).charId, ch.id) == 0)
            {
                count++;
            }
        }
        {
            Pen pen(chain.First());
            pen.Title(ch.displayName);
            pen.Subtitle(Fmt("%d %s.", count, count == 1 ? "entry" : "entries"));
            if (count == 0)
            {
                pen.Line("Nothing written about this one yet.", VoiceType, InkPencil);
            }
        }
        int k = 0;
        for (int e = journal.EntryCount() - 1; e >= 0; e--)
        {
            const JournalEntry& entry = journal.Entry(e);
            if (stricmp(entry.charId, ch.id) != 0)
            {
                continue;
            }
            JournalPage& page = chain.PageFor(k);
            Pen pen(page);
            pen.Entry(entry, in.day, StampDay(entry.stamp) != in.day, true);
            k++;
        }
    }
}

// ===========================================================================
// ROSTER: every fighter in typed rows, five to a page, then the Arms block
// ===========================================================================

void ComposeRoster(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const int perPage = ComposeLimits::ListPerPage;
    ListChain chain(doc, "GM_ROSTER", "The roster", "GM_PEOPLE", "People", perPage);
    {
        Pen pen(chain.First());
        pen.Title("The roster");
        pen.Subtitle(UnderArms(ctx.d));
        if (in.roster.Size() == 0)
        {
            pen.Line("No fighters recorded.", VoiceType, InkPencil);
        }
    }

    // per fighter two blocks: "<name> <Rank|xN>" (the name a wrapping run,
    // bold for the player; the rank or count in pencil) and a hanging
    // "<role>, <arms>[, WIA n%]." line with the WIA part in red.  A group
    // label ("With me" / "Holding <zone>") precedes the first fighter of a
    // group and is repeated at the top of a continuation page; labels do not
    // count toward the five
    RString lastGroup;
    int lastPart = 0;
    for (int i = 0; i < in.roster.Size(); i++)
    {
        const JournalRosterRow& r = in.roster[i];
        JournalPage& page = chain.PageFor(i);
        Pen pen(page);
        const int part = i / perPage + 1;
        if (part != lastPart)
        {
            lastGroup = RString();
            lastPart = part;
        }
        const RString group = r.withPlayer ? RString("With me") : RString("Holding ") + r.zone;
        if (stricmp(group, lastGroup) != 0)
        {
            pen.Line(group, VoiceType, InkPencil);
            lastGroup = group;
        }
        // row 1: name, rank or count
        TypeRun(pen, r.name, InkStock, 0, r.isPlayer);
        TypeRun(pen, RString(" "));
        PencilRun(pen, r.count > 1 ? Fmt("x%d", r.count) : RankShort(r.rank));
        pen.EndBlock();
        // row 2: role, arms, condition
        RString arms = r.primary;
        if (r.launcher.GetLength() > 0)
        {
            arms = arms + RString(" + ") + r.launcher;
        }
        RString detail = r.role + RString(", ") + arms;
        if (r.wounded >= 25)
        {
            // the red run carries the full stop; the stock run ends in the
            // separator (fields lay adjacent, so the run carries its own space)
            TypeRun(pen, detail + RString(", "), InkStock, 0.04f);
            TypeRun(pen, Fmt("WIA %d%%.", r.wounded), InkRed, 0.04f);
        }
        else
        {
            TypeRun(pen, detail + RString("."), InkStock, 0.04f);
        }
        pen.EndBlock();
    }

    // the Arms block closes the roster on its last page (page 1 when the
    // roster fits under five, or is empty)
    {
        JournalPage& last = in.roster.Size() > 0 ? chain.PageFor(in.roster.Size() - 1) : chain.First();
        Pen pen(last);
        ArmsBlock(pen, ctx.journal, in);
    }
}

// ===========================================================================
// PLACES: the zone index, five rows to a page, then one page per zone
// ===========================================================================

void ComposePlaces(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Journal& journal = ctx.journal;
    const int perPage = ComposeLimits::ListPerPage;

    // ---- index: one typed row per zone, grouped, nearest first
    ListChain chain(doc, "GM_PLACES", "Places", "Main", "Contents", perPage);
    {
        JournalPage& first = chain.First();
        first.aliases.Add(RString("GM_ZONES"));
        Pen pen(first);
        pen.Title("Places");
        pen.Subtitle(Fmt("%d of %d scouted.", ctx.d.scouted, in.zones.Size()));
    }
    static const char* groupNames[] = {"Ours", "Contested", "Neutral", "Occupied", "Unscouted"};
    int row = 0;
    for (int g = 0; g < 5; g++)
    {
        AutoArray<int> rows;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            if (StateOf(in.zones[i], in.supportFlip).group != g)
            {
                continue;
            }
            int at = rows.Size();
            for (int r = 0; r < rows.Size(); r++)
            {
                if (Nearer(&in.zones[i], &in.zones[rows[r]]))
                {
                    at = r;
                    break;
                }
            }
            rows.Insert(at, i);
        }
        if (rows.Size() == 0)
        {
            continue;
        }
        for (int r = 0; r < rows.Size(); r++)
        {
            const int zi = rows[r];
            const JournalZoneRow& z = in.zones[zi];
            JournalPage& page = chain.PageFor(row++);
            Pen pen(page);
            if (r == 0)
            {
                // the group head lands with its first row, on whichever page
                // that is; heads do not count toward the five
                pen.Head(groupNames[g]);
            }
            pen.Link(z.name, cstr(RString("#") + ZoneAnchor(zi)));
            // the tail: kind, brief, alert, range
            AutoArray<RString> parts;
            if (stricmp(z.name, TypeWord(z.type)) != 0)
            {
                parts.Add(RString(TypeWord(z.type)));
            }
            const RString brief = ZoneBrief(z, in.supportFlip);
            if (brief.GetLength() > 0)
            {
                parts.Add(brief);
            }
            if (z.revealed && z.alert > 0)
            {
                parts.Add(RString(AlertName(z.alert)));
            }
            const RString range = Range(z);
            if (range.GetLength() > 0)
            {
                parts.Add(range);
            }
            if (parts.Size() > 0)
            {
                TypeRun(pen, RString(" - ") + JoinNames(parts, 8), z.alert == 2 ? InkRed : InkStock, 0.06f);
            }
            pen.EndBlock();
        }
    }

    // ---- one page per zone
    for (int i = 0; i < in.zones.Size(); i++)
    {
        const JournalZoneRow& z = in.zones[i];
        const ZoneState st = StateOf(z, in.supportFlip);
        const bool town = stricmp(z.type, "CITY") == 0;
        const bool camp = stricmp(z.type, "CAMP") == 0;
        JournalPage& page = NewPage(doc, ZoneAnchor(i), z.name, "GM_PLACES", "Places");
        Pen pen(page);
        const char* stateWord = st.group == 0                    ? "Our"
                                : st.group == 1                  ? "Contested"
                                : st.group == 3                  ? "Occupied"
                                : st.group == 4                  ? "Unscouted"
                                : strcmp(st.word, "RISING") == 0 ? "Rising"
                                                                 : "Neutral";
        pen.Title(z.name);
        pen.Subtitle(Standing(in, Fmt("%s %s.", stateWord, TypeWord(z.type))));
        if (!z.revealed)
        {
            pen.Line("Not scouted yet. It shows on the map; move within reach of ground we hold to read it.", VoiceType,
                     InkPencil);
            const RString range = Range(z);
            if (range.GetLength() > 0)
            {
                pen.Note("Distance", range);
            }
        }
        else
        {
            RString state = st.word;
            if (z.holder == 1 && z.garrison > 0)
            {
                state = state + Fmt(", garrison %d", z.garrison);
            }
            else if (z.holder != 0 && z.garrison > 0)
            {
                state = state + Fmt(", %d occupiers in town", z.garrison);
            }
            pen.Note("State", state, z.holder == 1 ? InkRed : InkStock);
            RString alert = AlertName(z.alert);
            if (z.alert == 2)
            {
                alert = alert + RString(", a quick reaction force is out");
            }
            else if (z.alert == 1)
            {
                alert = alert + RString(", they are checking our last known position");
            }
            else
            {
                alert = alert + RString(", calm");
            }
            pen.Note("Alert", alert, z.alert == 2 ? InkRed : InkStock);
            if (town && z.holder != 0)
            {
                pen.Note("Support", Fmt("%d, %s", toInt(z.support),
                                        z.support >= in.supportFlip ? "past the line"
                                                                    : cstr(Fmt("line %d", toInt(in.supportFlip)))));
            }
            else if (town)
            {
                pen.Note("Support", Fmt("%d, risen", toInt(z.support)));
            }
            if (!town && !camp && z.holder != 0)
            {
                pen.Note("Capture", z.capture > 0 ? Fmt("%d%% secured", toInt(z.capture)) : RString("not started"));
            }
            pen.Note("Heat", Fmt("%d, %s", toInt(z.heat), HeatWord(z.heat)), z.heat >= 50 ? InkRed : InkStock);
            const RString range = Range(z);
            if (range.GetLength() > 0)
            {
                pen.Note("Distance", range);
            }
            const RString seen = Seen(z.seenDay, z.seenMinute, in.day);
            pen.Note("Last seen", seen.GetLength() > 0 ? seen : RString("never up close"));
            // what the cell has here
            AutoArray<RString> here;
            if (in.hqEstablished && stricmp(in.hqZone, z.name) == 0)
            {
                here.Add(RString("headquarters"));
                if (in.garageCount > 0)
                {
                    here.Add(Fmt("%d garaged", in.garageCount));
                }
            }
            for (int r = 0; r < in.roster.Size(); r++)
            {
                if (!in.roster[r].withPlayer && stricmp(in.roster[r].zone, z.name) == 0)
                {
                    here.Add(Fmt("holding squad of %d", in.roster[r].count));
                }
            }
            for (int t = 0; t < in.weaponDealerTowns.Size(); t++)
            {
                if (stricmp(in.weaponDealerTowns[t], z.name) == 0)
                {
                    here.Add(RString("arms dealer"));
                }
            }
            for (int t = 0; t < in.vehicleDealerTowns.Size(); t++)
            {
                if (stricmp(in.vehicleDealerTowns[t], z.name) == 0)
                {
                    here.Add(RString("vehicle dealer"));
                }
            }
            if (here.Size() > 0)
            {
                pen.Note("Here", JoinNames(here, 8));
            }
        }
        // the zone's own record: the latest lines here, the rest in the
        // record.  The 10-line cap (ComposeLimits::ZoneRecordCap) is a
        // deliberate deviation from the five-per-page rule: this is a
        // filtered excerpt with a link overflow, not a paginated list
        pen.Head("Record");
        int lines = 0;
        int earlier = 0;
        for (int e = journal.EntryCount() - 1; e >= 0; e--)
        {
            const JournalEntry& entry = journal.Entry(e);
            if (stricmp(entry.zone, z.name) != 0)
            {
                continue;
            }
            if (lines >= ComposeLimits::ZoneRecordCap)
            {
                earlier++;
                continue;
            }
            pen.Entry(entry, in.day, StampDay(entry.stamp) != in.day, false);
            lines++;
        }
        if (lines == 0)
        {
            pen.Line("Nothing written about this place yet.", VoiceType, InkPencil);
        }
        else if (earlier > 0)
        {
            PencilRun(pen, Fmt("%d earlier %s in the ", earlier, earlier == 1 ? "line" : "lines"));
            pen.Link("record", "#GM_RECORD");
            PencilRun(pen, RString("."));
            pen.EndBlock();
        }
    }
}

// ===========================================================================
// CHRONICLES: the hub over the record (and, from Change 2, the history)
// ===========================================================================

void ComposeChronicles(JournalDocument& doc, const ComposeContext& ctx)
{
    const Journal& journal = ctx.journal;
    JournalPage& page = NewPage(doc, "GM_CHRONICLES", "Chronicles", "Main", "Contents");
    Pen pen(page);
    pen.Title("Chronicles");
    pen.Subtitle(Fmt("%d %s.", journal.EntryCount(), journal.EntryCount() == 1 ? "entry" : "entries"));
    pen.LinkRow("The record", "#GM_RECORD", "Every line, newest first.");
    // the History link exists only once the historians have written (Change
    // 2 fills GM_HISTORY); no placeholder line before that
    if (ctx.in.history.present)
    {
        pen.Gap();
        pen.LinkRow("History", "#GM_HISTORY", "How the struggle began.");
    }
    // the history hangs off this page, so it is composed here rather than from
    // ComposeJournal: one call order, one file
    ComposeHistory(doc, ctx);
}

// ===========================================================================
// HISTORY: the campaign's generated past, one hub and one page per beat
// ===========================================================================

void ComposeHistory(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalHistoryView& history = ctx.in.history;
    if (!history.present)
    {
        // nothing was ever written for this campaign (a save from before the
        // registry existed): no hub, no beats, and ComposeChronicles omits the
        // link, so there is no dead end to click into
        return;
    }
    {
        JournalPage& page = NewPage(doc, "GM_HISTORY", "History", "GM_CHRONICLES", "Chronicles");
        Pen pen(page);
        pen.Title("The long quarrel");
        pen.Subtitle("As the historians tell it.");
        // two blocks, not two pages: Render moves the second whole onto
        // GM_HISTORY_2 when the paper runs out
        if (history.opening1.GetLength() > 0)
        {
            pen.Line(history.opening1, VoiceSerif);
        }
        if (history.opening2.GetLength() > 0)
        {
            pen.Line(history.opening2, VoiceSerif);
        }
        for (int k = 0; k < 3; k++)
        {
            if (history.events[k].title.GetLength() == 0)
            {
                continue;
            }
            HistoryLinkRow(pen, history.events[k].title, RString("#") + HistoryEventAnchor(k),
                           RString(HistoryBeatGloss(k)));
        }
    }
    for (int k = 0; k < 3; k++)
    {
        const JournalHistoryView::Event& event = history.events[k];
        if (event.title.GetLength() == 0)
        {
            continue;
        }
        const RString anchor = HistoryEventAnchor(k);
        JournalPage& page = NewPage(doc, cstr(anchor), cstr(event.title), "GM_CHRONICLES", "Chronicles");
        Pen pen(page);
        pen.Title(event.title);
        if (event.place.GetLength() > 0)
        {
            pen.Subtitle(Sentence(event.place));
        }
        if (event.text.GetLength() > 0)
        {
            pen.Line(event.text, VoiceSerif);
        }
    }
}

// ===========================================================================
// RECORD: every entry in the hand, newest first, by day, five to a page
// ===========================================================================

void ComposeRecord(JournalDocument& doc, const ComposeContext& ctx)
{
    const Journal& journal = ctx.journal;
    const int perPage = ComposeLimits::ListPerPage;
    ListChain chain(doc, "GM_RECORD", "The record", "GM_CHRONICLES", "Chronicles", perPage);
    {
        JournalPage& first = chain.First();
        first.aliases.Add(RString("GM_LOG"));
        Pen pen(first);
        pen.Title("The record");
        pen.Subtitle(Fmt("%d %s.", journal.EntryCount(), journal.EntryCount() == 1 ? "entry" : "entries"));
        if (journal.EntryCount() == 0)
        {
            pen.Line("Nothing written yet.", VoiceType, InkPencil);
        }
    }
    // a day head when the day changes and at the top of every continuation
    // page (heads do not count toward the five)
    int lastDay = -1;
    int lastPart = 0;
    int k = 0;
    for (int i = journal.EntryCount() - 1; i >= 0; i--, k++)
    {
        const JournalEntry& e = journal.Entry(i);
        const int day = StampDay(e.stamp);
        JournalPage& page = chain.PageFor(k);
        Pen pen(page);
        const int part = k / perPage + 1;
        if (part != lastPart)
        {
            lastDay = -1;
            lastPart = part;
        }
        if (day != lastDay)
        {
            pen.Head(day > 0 ? Fmt("Day %d", day) : RString("Undated"));
            lastDay = day;
        }
        pen.Entry(e, day, false, true);
    }
}

// ===========================================================================
// REFERENCE: the handbook index (a static ten-row menu) and its chapters
// ===========================================================================

void ComposeReference(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const int count = GuerrillaManualTopicCount();

    // ---- index: ten rows, exempt from the five rule (a static menu)
    {
        JournalPage& page = NewPage(doc, "GM_REFERENCE", "Reference", "Main", "Contents");
        page.aliases.Add(RString("GM_MAN_INDEX"));
        Pen pen(page);
        pen.Title("Reference");
        pen.Subtitle("Notes from the old hands.");
        for (int t = 0; t < count; t++)
        {
            const ManualTopic& topic = GuerrillaManualTopic(t);
            pen.Cell(Fmt("%d", t + 1), 0.06f, InkPencil, AlignRight);
            pen.Cell(RString(" "), 0.02f);
            // a wrapped subtitle aligns under the chapter title, never back
            // under the row number (the number cell plus its spacer)
            JournalRun link;
            link.text = topic.title;
            link.href = RString("#") + RString(topic.anchor);
            link.hanging = kIndexHanging;
            pen.Run(link);
            JournalRun subtitle;
            subtitle.text = RString("  ") + RString(topic.subtitle);
            subtitle.ink = InkPencil;
            subtitle.hanging = kIndexHanging;
            pen.Run(subtitle);
            pen.EndBlock();
        }
    }

    // ---- one page per chapter
    for (int t = 0; t < count; t++)
    {
        const ManualTopic& topic = GuerrillaManualTopic(t);
        JournalPage& page = NewPage(doc, topic.anchor, topic.title, "GM_REFERENCE", "Reference");
        // chapter to chapter walks through the bottom-pinned footer: prev and
        // next run past the ends of this chapter's own page chain into the
        // neighbouring chapters, so the handbook reads straight through and
        // the body carries no second row of navigation under the prose
        if (t > 0)
        {
            page.prevChainName = RString(GuerrillaManualTopicAnchor(t - 1));
        }
        if (t + 1 < count)
        {
            page.nextChainName = RString(GuerrillaManualTopicAnchor(t + 1));
        }
        Pen pen(page);
        pen.Title(RString(topic.title));
        pen.Subtitle(Cap(RString(topic.subtitle)) + Fmt(". Handbook %d of %d.", t + 1, count));
        for (int l = 0; topic.lines[l]; l++)
        {
            const char* line = topic.lines[l];
            if (line[0] == '#')
            {
                pen.Head(RString(line + 1));
            }
            else if (line[0] == '!')
            {
                ManualTableRow(pen, line, true);
            }
            else if (line[0] == '|')
            {
                ManualTableRow(pen, line, false);
            }
            else if (line[0] == '-' && line[1] == ' ')
            {
                pen.Bullet(RString(line + 2));
            }
            else if (strcmp(line, "@standing") == 0)
            {
                StandingHead(pen, in);
            }
            else
            {
                pen.Line(RString(line));
                pen.Gap();
            }
        }
        // no in-body nav row: the footer carries Contents, the Reference
        // index (the chapter's parent) and prev / next through the chapters
    }
}

} // namespace Poseidon::Guerrilla
