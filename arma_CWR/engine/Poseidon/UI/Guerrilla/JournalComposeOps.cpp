#include <Poseidon/UI/Guerrilla/JournalComposeInternal.hpp>

#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp> // JournalPageInputs

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/platform.hpp>       // stricmp

#include <cstring>

// Guerrilla Mode journal, Compose part A: Contents, Dispatches, the
// Operations hub and its four pages (Objectives, Suggested actions,
// Supplies, Resistance strength).
//
// Everything here is pure: it reads the Journal, the gathered
// JournalPageInputs and the shared Derived facts, and writes pages, blocks
// and runs through Pen / NewPage / ListChain.  Nothing measures a pixel or
// sees the briefing control; Render owns the paper.
//
// The content is ported from the old page builders (BuildNotes, BuildPlan,
// BuildCell's supply block, BuildResistance) with the PR #61 pins kept:
// the objectives-open count counts only what the page lists, a securing
// base is never a target, red ink only for RED alert / blown cover / a
// failed objective / a danger entry.  The old Notes hand paragraphs are
// re-voiced, not copied: the threat, the top objective and the latest
// development become three typed rows on Dispatches, the cell / economy
// roll-up goes to Supplies, the ground held to Resistance strength.

namespace Poseidon::Guerrilla
{

using namespace JournalText;

namespace
{

// ===========================================================================
// shared helpers
// ===========================================================================

// the two engine objectives count as open only while they are on the page
// (PR #61: the count follows the page)
struct EngineObjectives
{
    bool basesDone = false;
    bool townsDone = false;
    int open = 0; // engine lines still open + JOActive journal objectives
};

EngineObjectives CountObjectives(const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    EngineObjectives eo;
    eo.basesDone = in.militaryTotal > 0 && in.militaryHeld >= in.militaryTotal;
    eo.townsDone = in.townsTotal > 0 && in.townsRisen >= in.townsTotal;
    eo.open = (eo.basesDone ? 0 : 1) + (eo.townsDone ? 0 : 1);
    for (int i = 0; i < ctx.journal.ObjectiveCount(); i++)
    {
        if (ctx.journal.Objective(i).state == JOActive)
        {
            eo.open++;
        }
    }
    return eo;
}

RString OpenLine(const EngineObjectives& eo)
{
    return Fmt("%d %s open.", eo.open, eo.open == 1 ? "objective" : "objectives");
}

// hand remarks are capped at ComposeLimits::HandWords.  The templates below
// all fit; script-authored objective text is one source Compose does not
// control, so it is clamped to the first HandWords - 1 words plus "..."
// (JournalText::ClampWords, shared with Pen::Entry for the diary entries)
RString ClampHand(const RString& text)
{
    return ClampWords(text, ComposeLimits::HandWords);
}

// the threat sentence (BuildNotes' opening line without the hottest-zone and
// cover sentences); red when a garrison is RED
RString ThreatLine(const Derived& d, JournalInk& ink)
{
    ink = InkStock;
    if (d.redZone)
    {
        RString p =
            d.redZone->name + RString(" went RED. A quick reaction force is out toward our last known position");
        const RString range = Range(*d.redZone);
        if (range.GetLength() > 0)
        {
            p = p + RString(", ") + range;
        }
        p = p + RString(".");
        if (d.redCount > 1)
        {
            p = p + Fmt(" %s more %s RED.", cstr(Cap(Words(d.redCount - 1))),
                        d.redCount - 1 == 1 ? "garrison is" : "garrisons are");
        }
        ink = InkRed;
        return p;
    }
    if (d.yellowZone)
    {
        RString p = d.yellowZone->name + RString(" is YELLOW: they are checking our last known position");
        const RString range = Range(*d.yellowZone);
        if (range.GetLength() > 0)
        {
            p = p + RString(", ") + range;
        }
        return p + RString(".");
    }
    return RString("The garrisons are quiet.");
}

// a hub / menu entry
struct MenuEntry
{
    const char* title;
    const char* href;
    const char* description; // <= ComposeLimits::ContentsDescriptionWords words
};

void Menu(Pen& pen, const MenuEntry* entries, int count)
{
    for (int i = 0; i < count; i++)
    {
        pen.Gap();
        pen.LinkRow(RString(entries[i].title), entries[i].href, RString(entries[i].description));
    }
}

} // namespace

// ===========================================================================
// CONTENTS ("Main", the Notes tab): the six sections as a static menu
// ===========================================================================

void ComposeContents(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    JournalPage& page = NewPage(doc, "Main", "Contents", "", "");
    page.aliases.Add(RString("GM_CONTENTS"));
    Pen pen(page);
    pen.Title("Resistance Dossier");
    RString subtitle = CampaignLine(in);
    const RString date = DateLine(in);
    if (date.GetLength() > 0)
    {
        subtitle = subtitle + RString(" ") + date + RString(".");
    }
    pen.Subtitle(subtitle);

    // a static menu: six entries, exempt from the five-per-page rule
    static const MenuEntry kContents[] = {
        {"Dispatches", "#GM_DISPATCH", "Threat, objective and the latest development."},
        {"Operations", "#Plan", "Objectives, moves, supplies and strength."},
        {"People", "#GM_PEOPLE", "The roster and the named."},
        {"Places", "#GM_PLACES", "Towns, bases, the headquarters."},
        {"Chronicles", "#GM_CHRONICLES", "The record of the campaign."},
        {"Reference", "#GM_REFERENCE", "Handbook topics."},
    };
    Menu(pen, kContents, 6);
}

// ===========================================================================
// DISPATCHES: three typed facts and one line in the hand
// ===========================================================================

void ComposeDispatch(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Journal& journal = ctx.journal;
    const Derived& d = ctx.d;
    JournalPage& page = NewPage(doc, "GM_DISPATCH", "Dispatches", "Main", "Contents");
    Pen pen(page);
    pen.Title("Dispatches");
    const RString date = Standing(in, RString());
    pen.Subtitle(date.GetLength() > 0 ? date : CampaignLine(in));
    pen.Gap();

    // ---- the threat
    {
        JournalInk ink = InkStock;
        const RString threat = ThreatLine(d, ink);
        pen.Note("Threat", threat, ink);
    }

    // ---- the top open objective: the engine's standing orders first, then
    // the first active journal objective
    {
        const EngineObjectives eo = CountObjectives(ctx);
        const JournalObjective* active = nullptr;
        for (int i = 0; i < journal.ObjectiveCount() && !active; i++)
        {
            if (journal.Objective(i).state == JOActive)
            {
                active = &journal.Objective(i);
            }
        }
        if (in.militaryTotal > 0 && !eo.basesDone)
        {
            pen.Note("Objective", Fmt("Hold every base. %d of %d.", in.militaryHeld, in.militaryTotal));
        }
        else if (in.townsTotal > 0 && !eo.townsDone)
        {
            pen.Note("Objective", Fmt("Raise every town. %d of %d.", in.townsRisen, in.townsTotal));
        }
        else if (active)
        {
            pen.Note("Objective", Sentence(active->text));
        }
        else
        {
            pen.Note("Objective", "Nothing open.", InkPencil);
        }
    }

    // ---- the latest development: the newest entry that is not plain, else
    // the newest entry
    {
        const JournalEntry* latest = nullptr;
        for (int i = journal.EntryCount() - 1; i >= 0 && !latest; i--)
        {
            if (journal.Entry(i).kind != JKPlain)
            {
                latest = &journal.Entry(i);
            }
        }
        if (!latest && journal.EntryCount() > 0)
        {
            latest = &journal.Entry(journal.EntryCount() - 1);
        }
        if (latest)
        {
            RString line;
            const RString stamp = CompactStamp(latest->stamp, in.day);
            if (stamp.GetLength() > 0)
            {
                line = stamp + RString(" ");
            }
            if (latest->zone.GetLength() > 0)
            {
                line = line + latest->zone + RString(". ");
            }
            line = line + latest->text;
            pen.Note("Latest", line, latest->kind == JKDanger ? InkRed : InkStock);
        }
        else
        {
            pen.Note("Latest", "Nothing written yet.", InkPencil);
        }
    }

    // ---- the cover line, first person, the page's single remark in the hand
    if (in.undercoverArmed)
    {
        pen.Gap();
        if (in.undercoverStatus == 2)
        {
            pen.Hand(Fmt("My cover is blown: %s %s my face.", cstr(Words(in.undercoverWitnesses)),
                         in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know"),
                     InkRed);
        }
        else if (in.undercoverStatus == 1)
        {
            pen.Hand("A patrol is checking me.");
        }
        else
        {
            pen.Hand("To the occupier I am still a civilian.");
        }
    }
}

// ===========================================================================
// OPERATIONS hub ("Plan", the Plan tab): four links.  UpdatePlan copies page
// 0 of this section into __PLAN, so the hub must stay one physical page
// ===========================================================================

void ComposeOperations(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    JournalPage& page = NewPage(doc, "Plan", "Operations", "Main", "Contents");
    page.aliases.Add(RString("GM_OPERATIONS"));
    Pen pen(page);
    pen.Title("Operations");
    pen.Subtitle(Standing(in, OpenLine(CountObjectives(ctx))));

    static const MenuEntry kOperations[] = {
        {"Objectives", "#GM_OBJECTIVES", "What we mean to do, and what is done."},
        {"Suggested actions", "#GM_ACTIONS", "The next moves, in order."},
        {"Supplies", "#GM_SUPPLY", "Treasury, manpower, caches, dealers."},
        {"Resistance strength", "#GM_FACTION", "War level, ground held, the occupier."},
    };
    Menu(pen, kOperations, 4);
}

// ===========================================================================
// OBJECTIVES: open, then failed in red, then done in the faded hand; five
// per page
// ===========================================================================

namespace
{
struct HandItem
{
    RString text;
    JournalInk ink = InkHand;
    int group = 0; // Objectives: 0 open / 1 failed / 2 done
};
} // namespace

void ComposeObjectives(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Journal& journal = ctx.journal;
    const EngineObjectives eo = CountObjectives(ctx);

    AutoArray<HandItem> items;
    auto Add = [&](const RString& text, JournalInk ink, int group)
    {
        HandItem item;
        item.text = ClampHand(text);
        item.ink = ink;
        item.group = group;
        items.Add(item);
    };
    // open
    if (!eo.basesDone)
    {
        Add(Fmt("Hold every base. %d of %d.", in.militaryHeld, in.militaryTotal), InkHand, 0);
    }
    if (!eo.townsDone)
    {
        Add(Fmt("Raise every town. %d of %d.", in.townsRisen, in.townsTotal), InkHand, 0);
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOActive)
        {
            Add(Sentence(o.text), InkHand, 0);
        }
    }
    // failed
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOFailed)
        {
            Add(RString("Failed: ") + Sentence(o.text), InkRed, 1);
        }
    }
    // done
    if (eo.basesDone)
    {
        Add("Every base held.", InkFaded, 2);
    }
    if (eo.townsDone)
    {
        Add("Every town risen.", InkFaded, 2);
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JODone)
        {
            Add(Sentence(o.text), InkFaded, 2);
        }
    }

    ListChain chain(doc, "GM_OBJECTIVES", "Objectives", "Plan", "Operations");
    {
        Pen pen(chain.First());
        pen.Title("Objectives");
        pen.Subtitle(Standing(in, OpenLine(eo)));
        if (items.Size() == 0)
        {
            pen.Gap();
            pen.Line("Nothing open.", VoiceType, InkPencil);
        }
    }
    static const char* kGroupHeads[] = {"Open", "Failed", "Done"};
    int lastGroup = -1;
    for (int i = 0; i < items.Size(); i++)
    {
        // a fresh Pen per item: PageFor may append a part and move the pages
        Pen pen(chain.PageFor(i));
        if (items[i].group != lastGroup)
        {
            // the group head lands on whichever page the group's first item does;
            // heads do not count toward the five
            pen.Head(kGroupHeads[items[i].group]);
            lastGroup = items[i].group;
        }
        pen.Hand(items[i].text, items[i].ink, true);
    }
}

// ===========================================================================
// SUGGESTED ACTIONS: the next moves in priority order, the urgent ones in
// red; five per page
// ===========================================================================

void ComposeActions(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Derived& d = ctx.d;

    AutoArray<HandItem> moves;
    auto Move = [&](bool urgent, const RString& text, const RString& range)
    {
        RString line = text;
        if (range.GetLength() > 0)
        {
            line = line + RString(" ") + range + RString(".");
            // a long place name can push a template past the cap: drop the range first
            if (WordCount(line) > ComposeLimits::HandWords)
            {
                line = text;
            }
        }
        HandItem item;
        item.text = ClampHand(line);
        item.ink = urgent ? InkRed : InkHand;
        moves.Add(item);
    };
    if (d.redZone)
    {
        Move(true, RString("Break contact. ") + d.redZone->name + RString(" is RED, a QRF is out."), Range(*d.redZone));
    }
    if (in.undercoverArmed && in.undercoverStatus == 2)
    {
        Move(true,
             Fmt("Go dark. %s %s my face: stow the weapon, lose or drop the witnesses.",
                 cstr(Cap(Words(in.undercoverWitnesses))),
                 in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know"),
             RString());
    }
    if (d.hottest && d.hottest->heat >= 50)
    {
        if (d.hottest->holder == 0)
        {
            Move(true,
                 d.hottest->name +
                     Fmt(", heat %d, on our own ground. Reinforce or pull the squad.", toInt(d.hottest->heat)),
                 Range(*d.hottest));
        }
        else
        {
            Move(false,
                 RString("Lie low near ") + d.hottest->name +
                     Fmt(". Heat %d, the garrison is on edge.", toInt(d.hottest->heat)),
                 Range(*d.hottest));
        }
    }
    for (int i = 0; i < d.ready.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.ready[i];
        RString text =
            RString("Raise ") + z.name + Fmt(". Support %d, line %d. ", toInt(z.support), toInt(in.supportFlip));
        text = text + (z.garrison > 0 ? Fmt("%d occupiers in town: clear or wait them out.", z.garrison)
                                      : RString("Fighters into the town while no occupier is present."));
        Move(false, text, Range(z));
    }
    for (int i = 0; i < d.securing.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.securing[i];
        RString text = RString("Finish securing ") + z.name + Fmt(". %d%% secured. ", toInt(z.capture));
        text = text + (z.garrison > 0 ? Fmt("Fighters inside; garrison %d still on the field.", z.garrison)
                                      : RString("Fighters inside, keep the garrison out."));
        Move(false, text, Range(z));
    }
    // d.targets already excludes a base with a meter running (PR #61: a
    // securing base is not a target)
    for (int i = 0; i < d.targets.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.targets[i];
        Move(false, RString("Target ") + z.name + Fmt(". Garrison %d, alert %s.", z.garrison, AlertName(z.alert)),
             Range(z));
    }
    if (in.economyKnown && in.manpower >= 1)
    {
        Move(false, Fmt("Recruit at the Camp. %d HR in reserve, 1 HR a fighter.", toInt(in.manpower)), RString());
    }
    if (moves.Size() == 0)
    {
        Move(false, "Scout the island. Zones show once they are within reach of ground we hold.", RString());
    }
    if (!in.hqEstablished)
    {
        // standing advice, not a tactical move: it never displaces the scout line
        Move(false, "Set up a headquarters. Any town, or the Camp. It gives us a cache and a garage.", RString());
    }

    ListChain chain(doc, "GM_ACTIONS", "Suggested actions", "Plan", "Operations");
    {
        Pen pen(chain.First());
        pen.Title("Suggested actions");
        pen.Subtitle(Standing(in, Fmt("%d %s.", moves.Size(), moves.Size() == 1 ? "move" : "moves")));
        pen.Gap();
    }
    for (int i = 0; i < moves.Size(); i++)
    {
        // a fresh Pen per move: PageFor may append a part and move the pages
        Pen pen(chain.PageFor(i));
        pen.Hand(moves[i].text, moves[i].ink, true);
    }
}

// ===========================================================================
// SUPPLIES: treasury, manpower, income, holdings, dealers (typed)
// ===========================================================================

void ComposeSupply(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    JournalPage& page = NewPage(doc, "GM_SUPPLY", "Supplies", "Plan", "Operations");
    Pen pen(page);
    pen.Title("Supplies");
    pen.Subtitle(Standing(in, in.economyKnown
                                  ? Fmt("Treasury %d R, manpower %d HR.", toInt(in.resources), toInt(in.manpower))
                                  : RString()));
    bool anything = false;

    // ---- treasury
    if (in.economyKnown)
    {
        pen.Head("Treasury");
        pen.Note("Treasury", Fmt("%d R", toInt(in.resources)));
        pen.Note("Manpower", in.manpowerCap > 0 ? Fmt("%d HR, pool %d", toInt(in.manpower), toInt(in.manpowerCap))
                                                : Fmt("%d HR", toInt(in.manpower)));
        anything = true;
    }

    // ---- income
    if (in.incomeKnown)
    {
        pen.Head("Income");
        const RString label = in.econTickSeconds > 0 ? Fmt("Income every %d min", toInt(in.econTickSeconds / 60.0f))
                                                     : RString("Income a tick");
        AutoArray<RString> payers;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            if (in.zones[i].holder == 0 && stricmp(in.zones[i].type, "CAMP") != 0)
            {
                payers.Add(in.zones[i].name);
            }
        }
        RString income = Fmt("+%d R, +%d HR", toInt(in.incomeR), toInt(in.incomeHR));
        if (payers.Size() > 0)
        {
            income = income + RString(", from ") + JoinNames(payers, 4);
        }
        pen.Note(label, income);
        anything = true;
    }

    // ---- holdings: the caches and the garage
    if (in.hqEstablished || in.stashCount > 0)
    {
        pen.Head("Holdings");
        if (in.hqEstablished)
        {
            pen.Note("Cache", in.hqZone + RString(", at the headquarters"));
            pen.Note("Garage", in.garage.Size() > 0 ? JoinNames(in.garage, 4)
                               : in.garageCount > 0 ? Fmt("%d vehicles", in.garageCount)
                                                    : RString("empty"));
        }
        else
        {
            pen.Note("Caches", Num((float)in.stashCount));
        }
        anything = true;
    }

    // ---- dealers, each town tagged with its state when it is not ours
    if (in.marketActive)
    {
        pen.Head("Dealers");
        auto Dealers = [&](const char* label, const AutoArray<RString>& towns)
        {
            RString list;
            for (int t = 0; t < towns.Size() && t < 4; t++)
            {
                if (t > 0)
                {
                    list = list + RString(", ");
                }
                list = list + towns[t];
                for (int i = 0; i < in.zones.Size(); i++)
                {
                    if (stricmp(in.zones[i].name, towns[t]) != 0)
                    {
                        continue;
                    }
                    const ZoneState st = StateOf(in.zones[i], in.supportFlip);
                    if (st.group != 0)
                    {
                        RString word = st.word;
                        word.Lower();
                        list = list + RString(" (") + word + RString(")");
                    }
                }
            }
            pen.Note(RString(label), list.GetLength() > 0 ? list : RString("none known"),
                     list.GetLength() > 0 ? InkStock : InkPencil);
        };
        Dealers("Arms dealers", in.weaponDealerTowns);
        Dealers("Vehicle dealers", in.vehicleDealerTowns);
        anything = true;
    }

    if (!anything)
    {
        pen.Gap();
        pen.Line("Nothing recorded yet.", VoiceType, InkPencil);
    }
}

// ===========================================================================
// RESISTANCE STRENGTH ("GM_FACTION"): war level, ground, organisation
// ===========================================================================

void ComposeStrength(JournalDocument& doc, const ComposeContext& ctx)
{
    const JournalPageInputs& in = ctx.in;
    const Derived& d = ctx.d;
    JournalPage& page = NewPage(doc, "GM_FACTION", "Resistance strength", "Plan", "Operations");
    Pen pen(page);
    pen.Title("Resistance");
    pen.Subtitle(Fmt("War level %d of %d.", in.warLevel, in.warLevelMax));

    // ---- war level ladder (escalation.sqs bands: 20 / 40 / 55 / 70 / 85 % held)
    pen.Head("War level");
    static const int ladder[] = {20, 40, 55, 70, 85};
    int nextAt = -1;
    for (int i = 0; i < 5; i++)
    {
        if (d.heldPct < ladder[i])
        {
            nextAt = ladder[i];
            break;
        }
    }
    RString held = Fmt("%d%%. ", d.heldPct);
    held = held + (nextAt > 0 ? Fmt("Level %d at %d%%; the ladder runs 20, 40, 55, 70, 85", in.warLevel + 1, nextAt)
                              : RString("Top of the ladder"));
    pen.Note("Island held", held);
    if (in.occupierTierThresholds.Size() > 0)
    {
        RString tiers;
        for (int i = 0; i < in.occupierTierThresholds.Size(); i++)
        {
            if (i > 0)
            {
                tiers = tiers + RString(", ");
            }
            tiers = tiers + Fmt("WL %d", toInt(in.occupierTierThresholds[i]));
        }
        pen.Line(RString("The occupier steps up at ") + tiers +
                     RString(": better troops, heavier vehicles, sharper eyes."),
                 VoiceType, InkPencil);
    }

    // ---- ground
    pen.Head("Ground");
    {
        int risen = 0, rising = 0, neutral = 0, occupied = 0, unscouted = 0;
        int heldBases = 0, contested = 0, occBases = 0;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            const JournalZoneRow& z = in.zones[i];
            const bool town = stricmp(z.type, "CITY") == 0;
            const bool camp = stricmp(z.type, "CAMP") == 0;
            const ZoneState st = StateOf(z, in.supportFlip);
            if (town)
            {
                if (st.group == 4)
                    unscouted++;
                else if (z.holder == 0)
                    risen++;
                else if (strcmp(st.word, "RISING") == 0)
                    rising++;
                else if (z.holder == 1)
                    occupied++;
                else
                    neutral++;
            }
            else if (!camp)
            {
                if (z.holder == 0)
                    heldBases++;
                else if (z.capture > 0)
                    contested++;
                else if (z.holder == 1)
                    occBases++;
            }
        }
        pen.Note("Towns", Fmt("%d risen, %d rising, %d neutral, %d occupied, %d unscouted", risen, rising, neutral,
                              occupied, unscouted));
        pen.Note("Bases", Fmt("%d held, %d contested, %d occupied", heldBases, contested, occBases));
    }
    {
        RString garrisons;
        int shown = 0;
        for (int i = 0; i < in.zones.Size() && shown < 4; i++)
        {
            const JournalZoneRow& z = in.zones[i];
            if (z.revealed && z.holder != 0 && z.garrison > 0)
            {
                garrisons = garrisons + (shown ? RString(", ") : RString(" ")) + z.name + Fmt(" %d", z.garrison);
                shown++;
            }
        }
        pen.Note("Occupier under arms", Fmt("%d known.", d.knownGarrison) + garrisons);
        RString ours = Fmt("%d. %d with me, %d holding", d.withPlayer + d.holding, d.withPlayer, d.holding);
        if (in.economyKnown)
        {
            ours = ours + Fmt(", %d HR in reserve", toInt(in.manpower));
        }
        pen.Note("Ours under arms", ours);
        // heat across the ground we hold
        float heatSum = 0;
        int heldZones = 0;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            if (in.zones[i].holder == 0)
            {
                heatSum += in.zones[i].heat;
                heldZones++;
            }
        }
        if (heldZones > 0)
        {
            const float mean = heatSum / heldZones;
            pen.Note("Heat on our ground",
                     Fmt("%d mean over %d %s", toInt(mean), heldZones, heldZones == 1 ? "zone" : "zones"),
                     mean >= 50 ? InkRed : InkStock);
        }
    }

    // ---- organisation (faction-management stubs read script globals)
    pen.Head("Organisation");
    pen.Note("Cells",
             Fmt("%d, ours. ", 1 + in.faction.alliedCells) +
                 (in.hqEstablished ? RString("Headquarters at ") + in.hqZone : RString("No headquarters yet")));
    {
        AutoArray<RString> holdings;
        if (in.stashCount > 0)
        {
            holdings.Add(Fmt("%d %s", in.stashCount, in.stashCount == 1 ? "cache" : "caches"));
        }
        if (in.garageCount > 0)
        {
            holdings.Add(Fmt("%d garaged", in.garageCount));
        }
        if (holdings.Size() > 0)
        {
            pen.Note("Holdings", JoinNames(holdings, 4));
        }
    }
    if (in.faction.doctrine.GetLength() > 0)
    {
        pen.Note("Doctrine", in.faction.doctrine);
    }
    if (in.faction.outsideSupport.GetLength() > 0)
    {
        pen.Note("Outside support", in.faction.outsideSupport);
    }
    if (in.faction.alliedCells == 0 && in.faction.outsideSupport.GetLength() == 0)
    {
        pen.Line("No allied cells. No outside contact.", VoiceType, InkPencil);
    }
}

} // namespace Poseidon::Guerrilla
