#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/UI/Controls/UIControlsBase.hpp> // CHTMLContainer

#include <Poseidon/AI/AICenter.hpp>
#include <Poseidon/AI/AIGroup.hpp>
#include <Poseidon/AI/AIUnit.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp> // WeaponType, MaskSlot*
#include <Poseidon/World/World.hpp>
#include <Evaluator/express.hpp> // GameState / GameValue (VarGet)

#include <Poseidon/Foundation/Common/FltOpts.hpp>  // toInt
#include <Poseidon/Foundation/Enums/EnumNames.hpp> // GetEnumValue<TargetSide>
#include <Poseidon/Foundation/platform.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp> // GEngine (the pen face)
#include <Poseidon/IO/ParamFileExt.hpp>      // GetFontID

#include <climits> // INT_MAX (ManualTable)
#include <cmath>
#include <cstdarg> // va_list (Fmt)
#include <cstdio>
#include <cstring>

namespace Poseidon::Guerrilla
{

namespace
{

// ===========================================================================
// ink on paper
//
// The notepad keeps its stock look.  The briefing control's own format slots
// are the typography: Courier type in H1-H4 and P, Garamond in H5, the
// handwriting face in H6 (RscHTML in the game's resource config), black text
// and the stock link colour on the paper of the notepad model.  The journal
// adds only inks: the handwritten entries in blue-black, a red pen for alert
// and loss, pencil grey for stamps, labels and asides.  No background fill,
// no bars, no pips, no rules - a page is type and handwriting.
// ===========================================================================

inline PackedColor RGB(int r, int g, int b, int a = 255)
{
    return PackedColor(r, g, b, a);
}

const PackedColor kHandInk = RGB(14, 16, 52);    // fountain pen, blue-black
const PackedColor kRedInk = RGB(150, 22, 18);    // red pen: alert, loss, blown
const PackedColor kPencil = RGB(50, 46, 42);     // stamps, labels, asides
const PackedColor kFadedHand = RGB(84, 88, 122); // done items

// the stock RscHTML slots
constexpr HTMLFormat kTitle = HFH2;     // page title (Courier, 0.7 of H1)
constexpr HTMLFormat kSerif = HFH5;     // the campaign line under a title (Garamond)
constexpr HTMLFormat kHead = HFH3;      // section heads (Courier)
constexpr HTMLFormat kType = HFP;       // typed body
constexpr HTMLFormat kSmallType = HFH4; // small print: the blank spacer line
constexpr HTMLFormat kHand = HFH6;      // handwriting

// the hand is written in the journal's pen ("cwrpen": the handwriting face
// with a heavier stroke than the stock, thinned hand) and a little larger
// than the stock H6 (0.7 * 0.045), so it reads on the notepad at 800x600;
// relative to the typed body size so it follows the config and repaints
// idempotently
constexpr float kHandScale = 1.6f; // 0.036 at the stock P of 0.47 * 0.048

// ===========================================================================
// handbook - static in-universe text, typed.  Mini markup per line:
//   "#Heading"        section head        "!A|B|C"   table header
//   "|a|b|c"          table row; a cell starting with ~r is in red ink, ~w
//                     is bold type, ~g / ~y are plain type (kept so the
//                     text reads the same on paper as it did on the mock)
//   "- text"          bullet              "@standing" the live cover row
//   anything else     paragraph
// ===========================================================================

struct ManualTopic
{
    const char* anchor;
    const char* title;
    const char* subtitle;
    const char* lines[24]; // null-terminated
};

const ManualTopic kManual[] = {
    {"GM_MAN_MODE",
     "The campaign",
     "what this is",
     {"One fighter, one camp, an occupied island. There is no script to follow: take the island zone by zone, "
      "build a cell, and hold what you take against an occupier that grows stronger as you do.",
      "This journal is written as the campaign runs. Notes is the day's page; Plan carries the objectives and "
      "the next moves; Zones, Cell and Resistance are the ledgers; the Diary is the whole record.",
      "Open it whenever you like. It is rewritten every time, so the figures are current.", "#Pages",
      "|~wNOTES|the day's page: the threat, the cell, ground held, the latest entries",
      "|~wPLAN|objectives with progress, the done list, the next moves",
      "|~wZONES|every zone: state, support, capture, heat, garrison, last seen, range", "|~wCELL|roster, arms, supply",
      "|~wRESISTANCE|war level ladder, ground held, organisation", "|~wDIARY|the whole record, newest first", nullptr}},
    {"GM_MAN_ZONES",
     "Zones",
     "camp, bases, towns",
     {"Every place that matters is a zone with a flag on the map. Green is ours, red the occupier, yellow "
      "neutral, white contested.",
      "!TYPE|WHAT IT IS|HOW IT IS WON", "|~wCAMP|yours from the start; recruit, train and keep the record here|keep it",
      "|~wBASE|outpost, airfield or port with an occupier garrison|clear the garrison, hold the ground",
      "|~wTOWN|civilians with a support figure 0 to 100|support past 60, then fighters in the town",
      "A held base pays income and gets a holding squad. A risen town pays too and counts toward the war.",
      "Zones show on the ledger once they are within reach of ground you hold. Their meters only move while "
      "you are near (about 800 m), which is what last seen records.",
      nullptr}},
    {"GM_MAN_CAPTURE",
     "Taking a base",
     "the capture meter",
     {"A base carries a capture meter from 0 to 100. It climbs while your fighters stand inside the zone and no "
      "live occupier is inside it. More fighters climb it faster, up to a small crew.",
      "!IF|THEN", "|~yoccupier inside|the meter is contested and freezes; kill or drive them out",
      "|~yyou leave|the meter fades; alone, the defenders drive it down fast",
      "|~gmeter at 100|the base flips: income opens, a holding squad forms, regional heat spikes",
      "A patrol or a QRF that walks in contests the base like a garrison does. A capture is never safe until it "
      "is done.",
      nullptr}},
    {"GM_MAN_TOWNS",
     "Towns and support",
     "how a town rises",
     {"Each town has a support figure. It rises while your fighters stand in the town with no occupier "
      "present, and bleeds while only occupier troops are present, never below a floor.",
      "!SUPPORT|STATE|MEANS", "|~wunder 60|~wNEUTRAL|the town is being worked",
      "|~w60 and up|~gRISING|ready: fighters in the town while it is free of occupier troops and it flips",
      "|~wrisen|~gHELD|pays income, raises the war level, can be retaken if left exposed",
      "A disguised, undercover fighter counts for neither side. Bring fighters if you want a town to move.", nullptr}},
    {"GM_MAN_WAR",
     "War level, heat, alert",
     "what the occupier does back",
     {"The war level follows the share of the island you hold: 20, 40, 55, 70 and 85 percent step it up. Higher "
      "levels field better troops, heavier vehicles and sharper eyes for disguises.",
      "Heat is per zone, 0 to 100. Captures, blown cover and fights raise it; it decays while the zone is quiet. "
      "Past 30 the garrison is on edge, past 50 expect a sweep.",
      "!ALERT|MEANS|DO", "|~gGREEN|garrison calm|work", "|~yYELLOW|aware: they check your last known position|move",
      "|~rRED|combat: a quick reaction force is out toward your last known position|break contact, stay unseen",
      "Alert does not calm while they can see you.", nullptr}},
    {"GM_MAN_CELL",
     "The cell",
     "resources, manpower, recruiting",
     {"The cell lives on resources (R) and manpower (HR). Both come in from the zones you hold: bases and risen "
      "towns pay, the Camp does not. A panicked town pays less.",
      "!AT THE CAMP|COSTS|GIVES", "|~wRecruit fighter|1 HR|a rifleman in your group with the best pattern in issue",
      "|~wRecruit specialist|R|a medic, gunner or anti-tank man",
      "|~wTrain squad|R|skill for the whole group, up to a cap that grows with the war level",
      "Manpower is bodies: one HR is one recruit, and the pool has a ceiling. Resources buy specialists and "
      "training; they do nothing in the treasury.",
      nullptr}},
    {"GM_MAN_LOOT",
     "Arms and caches",
     "patterns, standard issue, the cache",
     {"Every occupier you kill drops what he carried into the cell's pool. Capture enough of one pattern and it "
      "becomes standard issue: recruits and companions can be kitted with it freely. The Cell page tracks the "
      "next pattern.",
      "The headquarters holds a cache and a garage. Weapons stored in the cache and vehicles locked in the "
      "garage stay where you left them, across saves and across an HQ move.",
      "Arms dealers and vehicle dealers trade in some towns for R; a dealer in an occupied town is a risk.", nullptr}},
    {"GM_MAN_COMPANIONS",
     "Companions",
     "named fighters",
     {"Named companions fight in your group, gain standing from kills and survival, and are promoted through the "
      "ranks. Each promotion raises their skill; the Cell page shows the progress to the next rank.",
      "Death is permanent. A fallen companion is gone for good and the diary records it.",
      "Companions are rebuilt beside you after a load from their saved record: rank, standing, kit.", nullptr}},
    {"GM_MAN_UNDERCOVER",
     "Undercover",
     "how the occupier reads you",
     {"To occupier eyes you begin as a civilian. Each occupier group judges you separately, from what it can see.",
      "!YOU ARE|SEEN AS|RANGE", "|~wUnarmed|~gCIVILIAN|any", "|~wRifle slung|~ySUSPECTED|under 20 m, or from behind",
      "|~wWeapon in hand|~ySUSPECTED, then BLOWN|any, with line of sight", "|~wFiring|~rBLOWN|every group in view",
      "@standing", "|~gCLEAN|no group has identified you", "|~ySUSPECTED|a group is checking you",
      "|~rBLOWN|identified; the journal counts the patrols who know you", "#Cover returns when",
      "- You walk away from every group that identified you.", "- The weapon is stowed and no witness is left.",
      "#Cover also fails when", "- A stolen occupier vehicle is seen close up.",
      "- A getaway is witnessed: that car is marked for those witnesses.", nullptr}},
    {"GM_MAN_SAVE",
     "Keeping the record",
     "saving and the diary",
     {"Save whenever you like: the whole campaign is kept. Zones, garrisons, alert, the cell, companions, "
      "patterns, caches, this journal.",
      "The diary is the running record of the campaign: captures, risings, quick reaction forces, blown cover, "
      "promotions, losses, patterns, saves and restores. It survives saving and loading.",
      "After a restore the diary notes it and everything resumes where it was.", nullptr}},
};

constexpr int kManualCount = sizeof(kManual) / sizeof(kManual[0]);

// ===========================================================================
// small formatting helpers
// ===========================================================================

inline const char* cstr(const RString& s)
{
    return (const char*)s;
}

RString Num(float v)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", toInt(v));
    return RString(buffer);
}

RString Fmt(const char* format, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return RString(buffer);
}

// "one" .. "twelve" for the prose, digits past that
RString Words(int n)
{
    static const char* words[] = {"no",    "one",   "two",  "three", "four",   "five",  "six",
                                  "seven", "eight", "nine", "ten",   "eleven", "twelve"};
    if (n >= 0 && n <= 12)
    {
        return RString(words[n]);
    }
    return Num((float)n);
}

// first letter up
RString Cap(const RString& s)
{
    if (s.GetLength() == 0)
    {
        return s;
    }
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s", cstr(s));
    if (buffer[0] >= 'a' && buffer[0] <= 'z')
    {
        buffer[0] = (char)(buffer[0] - 'a' + 'A');
    }
    return RString(buffer);
}

// "text." unless it already ends in a stop
RString Sentence(const RString& s)
{
    const int n = s.GetLength();
    if (n == 0)
    {
        return s;
    }
    const char last = s[n - 1];
    if (last == '.' || last == '!' || last == '?')
    {
        return s;
    }
    return s + RString(".");
}

const char* kRankShort[] = {"Pvt", "Cpl", "Sgt", "Lt", "Cpt", "Maj", "Col"};

RString RankShort(int rank)
{
    if (rank < 0 || rank > 6)
    {
        return RString();
    }
    return RString(kRankShort[rank]);
}

// zone type -> the word the journal uses for it
const char* TypeWord(const RString& type)
{
    if (stricmp(type, "CITY") == 0)
    {
        return "town";
    }
    if (stricmp(type, "CAMP") == 0)
    {
        return "camp";
    }
    if (stricmp(type, "AIRFIELD") == 0)
    {
        return "airfield";
    }
    if (stricmp(type, "SEAPORT") == 0)
    {
        return "port";
    }
    return "outpost";
}

RString Km(float meters)
{
    if (meters < 0)
    {
        return RString();
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f", meters / 1000.0f);
    return RString(buffer);
}

RString Range(const JournalZoneRow& z)
{
    if (z.distance < 0)
    {
        return RString();
    }
    return Km(z.distance) + RString(" km ") + z.bearing;
}

// "14:02" for today, "D2 21:10" otherwise; "" when never
RString Seen(int day, int minute, int today)
{
    if (day <= 0)
    {
        return RString();
    }
    if (day == today)
    {
        return Fmt("%02d:%02d", minute / 60, minute % 60);
    }
    return Fmt("D%d %02d:%02d", day, minute / 60, minute % 60);
}

// "Day 2 09:48" -> "D2 09:48" (or "09:48" when today); other stamps pass through
RString CompactStamp(const RString& stamp, int today)
{
    int day = 0;
    int hh = 0;
    int mm = 0;
    if (sscanf(cstr(stamp), "Day %d %d:%d", &day, &hh, &mm) == 3)
    {
        if (day == today)
        {
            return Fmt("%02d:%02d", hh, mm);
        }
        return Fmt("D%d %02d:%02d", day, hh, mm);
    }
    return stamp;
}

int StampDay(const RString& stamp)
{
    int day = 0;
    if (sscanf(cstr(stamp), "Day %d", &day) == 1)
    {
        return day;
    }
    return 0;
}

const char* HeatWord(float heat)
{
    if (heat >= 50)
    {
        return "on edge";
    }
    if (heat >= 30)
    {
        return "aware";
    }
    return "quiet";
}

const char* AlertName(int state)
{
    switch (state)
    {
        case 2:
            return "RED";
        case 1:
            return "YELLOW";
        default:
            return "GREEN";
    }
}

// zone state word for the ledger
struct ZoneState
{
    const char* word;
    int group; // 0 ours / 1 contested / 2 neutral / 3 occupied / 4 unscouted
};

ZoneState StateOf(const JournalZoneRow& z, float supportFlip)
{
    const bool town = stricmp(z.type, "CITY") == 0;
    if (!z.revealed)
    {
        return {"UNSCOUTED", 4};
    }
    if (z.holder == 0)
    {
        return {"HELD", 0};
    }
    if (!town && z.capture > 0)
    {
        return {"SECURING", 1};
    }
    if (town && z.holder != 1 && z.support >= supportFlip)
    {
        return {"RISING", 2};
    }
    if (z.holder == 1)
    {
        return {"OCCUPIED", 3};
    }
    return {"NEUTRAL", 2};
}

// ===========================================================================
// Sheet: a page emitter over the document model - type and handwriting
// ===========================================================================

class Sheet
{
  public:
    Sheet(CHTMLContainer* html, const char* name) : _html(html)
    {
        _section = html->FindSection(name);
        if (_section < 0)
        {
            _section = html->AddSection();
            html->AddName(_section, RString(name));
        }
        _pageW = html->GetPageWidth();
    }

    int Section() const { return _section; }
    float PageW() const { return _pageW; }

    // ---- primitives ------------------------------------------------------
    // one field; ink null = the control's stock text colour
    void Text(const RString& text, HTMLFormat f = kType, const PackedColor* ink = nullptr, const char* href = "",
              float tableW = 0, HTMLAlign align = HALeft, bool bold = false, bool bottom = false)
    {
        if (ink)
        {
            _html->SetFieldColor(*ink);
        }
        _html->AddText(_section, text, f, align, bottom, bold, RString(href), tableW * _pageW);
        _html->ClearFieldColor();
    }
    void Break(bool bottom = false) { _html->AddBreak(_section, bottom); }
    void Line(const RString& text, HTMLFormat f = kType, const PackedColor* ink = nullptr)
    {
        Text(text, f, ink);
        Break();
    }
    void Link(const RString& text, const char* href, HTMLFormat f = kType) { Text(text, f, nullptr, href); }
    // a fixed-width typed cell, truncated with "..." past its width
    void Cell(const RString& text, float w, HTMLFormat f = kType, const PackedColor* ink = nullptr,
              HTMLAlign align = HALeft, bool bold = false)
    {
        Text(Fit(text, w, f), f, ink, "", w, align, bold);
    }
    // a short blank line
    void Gap()
    {
        Text(RString(" "), kSmallType);
        Break();
    }
    void Hanging(float w) { _html->SetHanging(w * _pageW); }

    // ---- composites --------------------------------------------------------
    void Title(const RString& text) { Line(text, kTitle); }
    void Subtitle(const RString& text) { Line(text, kSerif, &kPencil); }
    void Head(const RString& text)
    {
        Gap();
        Line(text, kHead);
    }
    // handwritten line, blue-black unless told otherwise
    void Hand(const RString& text, const PackedColor* ink = &kHandInk)
    {
        Hanging(0.04f);
        Text(text, kHand, ink);
        Hanging(0);
        Break();
    }
    // a run of handwriting inside a paragraph: the sentences flow on, each
    // in its own ink (the trailing space joins the runs); the caller ends
    // the paragraph with Break().  A single Hand() line carries no trailing
    // space: on a wrap that falls at the end of the text it would open an
    // empty row
    void HandRun(const RString& text, const PackedColor* ink = &kHandInk)
    {
        Hanging(0.04f);
        Text(text + RString(" "), kHand, ink);
        Hanging(0);
    }
    // typed fact: "Label: value."
    void Note(const RString& label, const RString& value, const PackedColor* ink = nullptr)
    {
        Text(label + RString(": "), kType, &kPencil);
        Hanging(0.06f);
        Text(Sentence(value), kType, ink);
        Hanging(0);
        Break();
    }
    // a typed bullet with a hanging indent
    void Bullet(const RString& text)
    {
        Text(RString("- "), kType, &kPencil);
        Hanging(0.03f);
        Text(text, kType);
        Hanging(0);
        Break();
    }
    // a diary entry in the hand: "14:02 Airfield. text" (red pen for danger)
    void Entry(const JournalEntry& e, int today, bool forceDay, bool withZone)
    {
        const RString stamp = CompactStamp(e.stamp, forceDay ? -1 : today);
        if (stamp.GetLength() > 0)
        {
            Text(stamp + RString(" "), kHand, &kPencil);
        }
        RString body = e.text;
        if (withZone && e.zone.GetLength() > 0)
        {
            body = e.zone + RString(". ") + body;
        }
        Hanging(0.08f);
        Text(body, kHand, e.kind == JKDanger ? &kRedInk : &kHandInk);
        Hanging(0);
        Break();
    }
    // the footer: the journal's pages as a typed line pinned to the page foot
    void Footer(const char* current)
    {
        static const char* names[] = {"Notes", "Plan", "Zones", "Cell", "Resistance", "Diary", "Handbook"};
        static const char* anchors[] = {"#Main",       "#Plan",   "#GM_ZONES",    "#GM_CELL",
                                        "#GM_FACTION", "#GM_LOG", "#GM_MAN_INDEX"};
        Text(RString(" "), kSmallType, nullptr, "", 0, HALeft, false, true);
        Break(true);
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
            {
                Text(RString(" - "), kType, &kPencil, "", 0, HALeft, false, true);
            }
            if (strcmp(names[i], current) == 0)
            {
                Text(RString(names[i]), kType, &kPencil, "", 0, HALeft, false, true);
            }
            else
            {
                Text(RString(names[i]), kType, nullptr, anchors[i], 0, HALeft, false, true);
            }
        }
        Break(true);
        _html->FormatSection(_section);
    }

    // truncate to a cell width with "..." (parser-only containers measure by
    // character count, so the unit tests keep whole strings)
    RString Fit(const RString& text, float w, HTMLFormat f)
    {
        Font* font = _html->GetFormatFont(f, false);
        const float size = _html->GetFormatSize(f);
        const float maxW = w * _pageW * 0.97f;
        if (_html->GetTextWidth(size, font, text) <= maxW)
        {
            return text;
        }
        RString cut = text;
        while (cut.GetLength() > 1)
        {
            cut = cut.Substring(0, cut.GetLength() - 1);
            RString probe = cut + RString("...");
            if (_html->GetTextWidth(size, font, probe) <= maxW)
            {
                return probe;
            }
        }
        return cut;
    }

  private:
    CHTMLContainer* _html;
    int _section;
    float _pageW;
};

// ===========================================================================
// derived facts shared by several pages
// ===========================================================================

struct Derived
{
    const JournalZoneRow* redZone = nullptr;    // nearest RED zone
    const JournalZoneRow* yellowZone = nullptr; // nearest YELLOW zone
    int redCount = 0;
    const JournalZoneRow* hottest = nullptr;
    const JournalZoneRow* secondHottest = nullptr;
    AutoArray<const JournalZoneRow*> ready;    // towns past the line, not ours
    AutoArray<const JournalZoneRow*> securing; // bases with a meter running, not ours
    AutoArray<const JournalZoneRow*> targets;  // occupied bases with no meter running, nearest first
    int knownGarrison = 0;
    int garrisonZones = 0;
    int scouted = 0;
    int withPlayer = 0;
    int holding = 0;
    int wounded = 0;
    AutoArray<RString> holdingZones;
    int heldPct = 0;
};

bool Nearer(const JournalZoneRow* a, const JournalZoneRow* b)
{
    if (!b)
    {
        return true;
    }
    if (a->distance < 0)
    {
        return false;
    }
    return b->distance < 0 || a->distance < b->distance;
}

Derived Derive(const JournalPageInputs& in)
{
    Derived d;
    for (int i = 0; i < in.zones.Size(); i++)
    {
        const JournalZoneRow& z = in.zones[i];
        if (z.revealed)
        {
            d.scouted++;
        }
        if (z.alert == 2 && Nearer(&z, d.redZone))
        {
            d.redZone = &z;
        }
        if (z.alert == 2)
        {
            d.redCount++;
        }
        if (z.alert == 1 && Nearer(&z, d.yellowZone))
        {
            d.yellowZone = &z;
        }
        if (z.revealed && (!d.hottest || z.heat > d.hottest->heat))
        {
            d.secondHottest = d.hottest;
            d.hottest = &z;
        }
        else if (z.revealed && (!d.secondHottest || z.heat > d.secondHottest->heat))
        {
            d.secondHottest = &z;
        }
        const bool town = stricmp(z.type, "CITY") == 0;
        const bool camp = stricmp(z.type, "CAMP") == 0;
        if (z.revealed && town && z.holder != 0 && z.holder != 1 && z.support >= in.supportFlip)
        {
            d.ready.Add(&z);
        }
        if (z.revealed && !town && !camp && z.holder != 0 && z.capture > 0)
        {
            d.securing.Add(&z);
        }
        if (z.revealed && !town && !camp && z.holder == 1 && z.capture <= 0)
        {
            // insert nearest-first
            int at = d.targets.Size();
            for (int t = 0; t < d.targets.Size(); t++)
            {
                if (Nearer(&z, d.targets[t]))
                {
                    at = t;
                    break;
                }
            }
            d.targets.Insert(at, &z);
        }
        if (z.revealed && z.holder != 0 && z.garrison > 0)
        {
            d.knownGarrison += z.garrison;
            d.garrisonZones++;
        }
    }
    for (int i = 0; i < in.roster.Size(); i++)
    {
        const JournalRosterRow& r = in.roster[i];
        if (r.withPlayer)
        {
            d.withPlayer += r.count;
            if (r.wounded >= 25)
            {
                d.wounded++;
            }
        }
        else
        {
            d.holding += r.count;
            bool known = false;
            for (int k = 0; k < d.holdingZones.Size(); k++)
            {
                if (stricmp(d.holdingZones[k], r.zone) == 0)
                {
                    known = true;
                }
            }
            if (!known && r.zone.GetLength() > 0)
            {
                d.holdingZones.Add(r.zone);
            }
        }
    }
    const int total = in.militaryTotal + in.townsTotal;
    if (total > 0)
    {
        d.heldPct = ((in.militaryHeld + in.townsRisen) * 100) / total;
    }
    return d;
}

RString JoinNames(const AutoArray<RString>& names, int limit = 4)
{
    RString out;
    for (int i = 0; i < names.Size() && i < limit; i++)
    {
        if (i > 0)
        {
            out = out + RString(", ");
        }
        out = out + names[i];
    }
    if (names.Size() > limit)
    {
        out = out + RString(" ...");
    }
    return out;
}

// "Day 3, 14:20"; "" without a clock
RString DateLine(const JournalPageInputs& in)
{
    if (in.day <= 0)
    {
        return RString();
    }
    return Fmt("Day %d, %02d:%02d", in.day, in.minuteOfDay / 60, in.minuteOfDay % 60);
}

// "Malden. FIA against the Soviet Army."
RString CampaignLine(const JournalPageInputs& in)
{
    RString line;
    if (in.islandName.GetLength() > 0)
    {
        line = in.islandName + RString(". ");
    }
    if (in.resistanceName.GetLength() > 0 && in.occupierName.GetLength() > 0)
    {
        line = line + in.resistanceName + RString(" against the ") + in.occupierName + RString(".");
    }
    else if (line.GetLength() == 0)
    {
        line = "Guerrilla campaign.";
    }
    return line;
}

// a page's second line: the stat, then the date
RString Standing(const JournalPageInputs& in, const RString& stat)
{
    RString line = stat;
    const RString date = DateLine(in);
    if (date.GetLength() > 0)
    {
        line = line + (line.GetLength() > 0 ? RString(" ") : RString()) + date + RString(".");
    }
    return line;
}

// ===========================================================================
// pages
// ===========================================================================

// NOTES: the day's page, written in the hand
void BuildNotes(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "Main");
    const RString date = DateLine(in);
    s.Title(date.GetLength() > 0 ? date : RString("Notes"));
    s.Subtitle(CampaignLine(in));

    // ---- the threat first, in red when it matters
    {
        RString p;
        const PackedColor* ink = &kHandInk;
        if (d.redZone)
        {
            p = d.redZone->name + RString(" went RED. A quick reaction force is out toward our last known position");
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
            ink = &kRedInk;
        }
        else if (d.yellowZone)
        {
            p = d.yellowZone->name + RString(" is YELLOW: they are checking our last known position");
            const RString range = Range(*d.yellowZone);
            if (range.GetLength() > 0)
            {
                p = p + RString(", ") + range;
            }
            p = p + RString(".");
        }
        else
        {
            p = "The garrisons are quiet.";
        }
        s.HandRun(p, ink);
        if (d.hottest && d.hottest->heat >= 30)
        {
            s.HandRun(d.hottest->name + Fmt(" is %s, heat %d.", HeatWord(d.hottest->heat), toInt(d.hottest->heat)),
                      d.hottest->heat >= 50 ? &kRedInk : &kHandInk);
        }
        if (in.undercoverArmed)
        {
            if (in.undercoverStatus == 2)
            {
                s.HandRun(Fmt("My cover is blown: %s %s my face.", cstr(Words(in.undercoverWitnesses)),
                              in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know"),
                          &kRedInk);
            }
            else if (in.undercoverStatus == 1)
            {
                s.HandRun("A patrol is checking me.");
            }
            else
            {
                s.HandRun("To the occupier I am still a civilian.");
            }
        }
        s.Break();
    }
    s.Gap();

    // ---- the cell
    {
        int fighters = 0;
        for (int i = 0; i < in.roster.Size(); i++)
        {
            if (in.roster[i].withPlayer && !in.roster[i].isPlayer)
            {
                fighters += in.roster[i].count;
            }
        }
        RString p;
        if (fighters > 0)
        {
            p = Fmt("We are %s: me and %s %s", cstr(Words(fighters + 1)), cstr(Words(fighters)),
                    fighters == 1 ? "fighter" : "fighters");
            if (d.wounded > 0)
            {
                p = p + Fmt(", %s wounded", cstr(Words(d.wounded)));
            }
            p = p + RString(".");
        }
        else
        {
            p = "I am alone.";
        }
        if (d.holding > 0)
        {
            p = p + Fmt(" Another %s hold %s.", cstr(Words(d.holding)), cstr(JoinNames(d.holdingZones)));
        }
        if (in.economyKnown)
        {
            p = p + Fmt(" Treasury %d R, manpower %d HR", toInt(in.resources), toInt(in.manpower));
            if (in.manpowerCap > 0)
            {
                p = p + Fmt(", pool %d", toInt(in.manpowerCap));
            }
            p = p + RString(".");
        }
        if (in.hqEstablished)
        {
            const RString hq = in.hqZone.GetLength() > 0 ? in.hqZone : RString("an unknown zone");
            p = p + RString(in.hqIndoors ? " Headquarters in a house at " : " Headquarters at the edge of ") + hq;
            if (in.garageCount > 0)
            {
                p = p + Fmt(", %d %s garaged", in.garageCount, in.garageCount == 1 ? "vehicle" : "vehicles");
            }
            p = p + RString(".");
        }
        else
        {
            p = p + RString(" No headquarters yet.");
        }
        // status lines the managers publish (the companions roster) ride in
        // the same paragraph; the arms lines live on the Cell page
        for (int i = 0; i < journal.StatusCount(); i++)
        {
            const JournalStatusLine& st = journal.Status(i);
            if (stricmp(st.key, "Unlocked gear") == 0 || stricmp(st.key, "Standard issue") == 0 ||
                stricmp(st.key, "Next pattern") == 0)
            {
                continue;
            }
            p = p + RString(" ") + st.key + RString(": ") + Sentence(st.text);
        }
        s.Hand(p);
    }

    // ---- the ground
    if (in.militaryTotal + in.townsTotal > 0)
    {
        s.Gap();
        RString p = Fmt("We hold %d of %d %s and %d of %d %s. War level %d of %d.", in.militaryHeld, in.militaryTotal,
                        in.militaryTotal == 1 ? "base" : "bases", in.townsRisen, in.townsTotal,
                        in.townsTotal == 1 ? "town" : "towns", in.warLevel, in.warLevelMax);
        for (int i = 0; i < d.ready.Size() && i < 2; i++)
        {
            p = p + Fmt(" %s is ready to rise, support %d.", cstr(d.ready[i]->name), toInt(d.ready[i]->support));
        }
        for (int i = 0; i < d.securing.Size() && i < 2; i++)
        {
            p = p + Fmt(" %s is %d%% secured.", cstr(d.securing[i]->name), toInt(d.securing[i]->capture));
        }
        s.Hand(p);
    }

    // ---- the latest entries
    s.Head("Latest");
    if (journal.EntryCount() == 0)
    {
        s.Line("Nothing written yet.", kType, &kPencil);
    }
    const int recent = 2;
    for (int i = journal.EntryCount() - 1, n = 0; i >= 0 && n < recent; i--, n++)
    {
        const JournalEntry& e = journal.Entry(i);
        s.Entry(e, in.day, StampDay(e.stamp) != in.day, true);
    }
    s.Footer("Notes");
}

// PLAN: objectives and the next moves, in the hand
void BuildPlan(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "Plan");
    s.Title("Plan");
    // the two engine objectives count as open only while they are on the page
    const bool basesDone = in.militaryTotal > 0 && in.militaryHeld >= in.militaryTotal;
    const bool townsDone = in.townsTotal > 0 && in.townsRisen >= in.townsTotal;
    int open = (basesDone ? 0 : 1) + (townsDone ? 0 : 1);
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        if (journal.Objective(i).state == JOActive)
        {
            open++;
        }
    }
    s.Subtitle(Standing(in, Fmt("%d %s open.", open, open == 1 ? "objective" : "objectives")));

    s.Head("Objectives");
    if (!basesDone)
    {
        s.Hand(Fmt("Hold every base. %d of %d.", in.militaryHeld, in.militaryTotal));
    }
    if (!townsDone)
    {
        s.Hand(Fmt("Raise every town. %d of %d.", in.townsRisen, in.townsTotal));
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOActive)
        {
            s.Hand(Sentence(o.text));
        }
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOFailed)
        {
            s.Hand(RString("Failed: ") + Sentence(o.text), &kRedInk);
        }
    }
    bool anyDone = basesDone || townsDone;
    for (int i = 0; i < journal.ObjectiveCount() && !anyDone; i++)
    {
        anyDone = journal.Objective(i).state == JODone;
    }
    if (anyDone)
    {
        s.Head("Done");
        if (basesDone)
        {
            s.Hand("Every base held.", &kFadedHand);
        }
        if (townsDone)
        {
            s.Hand("Every town risen.", &kFadedHand);
        }
        for (int i = 0; i < journal.ObjectiveCount(); i++)
        {
            const JournalObjective& o = journal.Objective(i);
            if (o.state == JODone)
            {
                s.Hand(Sentence(o.text), &kFadedHand);
            }
        }
    }

    // ---- next moves, the urgent ones in red, in priority order
    s.Head("Next");
    int moves = 0;
    auto Move = [&](bool urgent, const RString& text, const RString& range)
    {
        RString line = text;
        if (range.GetLength() > 0)
        {
            line = line + RString(" ") + range + RString(".");
        }
        s.Hand(line, urgent ? &kRedInk : &kHandInk);
        moves++;
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
    if (moves == 0)
    {
        Move(false, "Scout the island. Zones show once they are within reach of ground we hold.", RString());
    }
    if (!in.hqEstablished)
    {
        // standing advice, not a tactical move: it never displaces the scout line
        Move(false, "Set up a headquarters. Any town, or the Camp. It gives us a cache and a garage.", RString());
    }
    s.Footer("Plan");
}

RString ZoneAnchor(int index)
{
    return Fmt("GM_ZONE_%d", index);
}

// the zone's one-line brief for the index
RString ZoneBrief(const JournalZoneRow& z, float supportFlip)
{
    const bool town = stricmp(z.type, "CITY") == 0;
    if (!z.revealed)
    {
        return RString();
    }
    const ZoneState st = StateOf(z, supportFlip);
    switch (st.group)
    {
        case 0: // ours: heat when it matters
            return z.heat >= 30 ? Fmt("heat %d", toInt(z.heat)) : RString();
        case 1: // contested: the capture meter and what is left of the garrison
            return z.garrison > 0 ? Fmt("%d%% secured, garrison %d", toInt(z.capture), z.garrison)
                                  : Fmt("%d%% secured", toInt(z.capture));
        case 2: // neutral: support against the line
        {
            if (!town)
            {
                return RString();
            }
            RString brief = strcmp(st.word, "RISING") == 0
                                ? Fmt("ready to rise, support %d", toInt(z.support))
                                : Fmt("support %d, line %d", toInt(z.support), toInt(supportFlip));
            if (z.garrison > 0)
            {
                brief = brief + Fmt(", %d occupiers in town", z.garrison);
            }
            return brief;
        }
        case 3: // occupied: the garrison
            return z.garrison > 0 ? Fmt("garrison %d", z.garrison) : RString();
        default:
            return RString();
    }
}

void BuildZones(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    // ---- index: one typed line per zone, grouped, nearest first
    {
        Sheet s(html, "GM_ZONES");
        s.Title("Zones");
        s.Subtitle(Fmt("%d of %d scouted.", d.scouted, in.zones.Size()));
        static const char* groupNames[] = {"Ours", "Contested", "Neutral", "Occupied", "Unscouted"};
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
            s.Head(groupNames[g]);
            for (int r = 0; r < rows.Size(); r++)
            {
                const JournalZoneRow& z = in.zones[rows[r]];
                s.Link(z.name, cstr(RString("#") + ZoneAnchor(rows[r])));
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
                    s.Hanging(0.06f);
                    s.Text(RString(" - ") + JoinNames(parts, 8), kType, z.alert == 2 ? &kRedInk : nullptr);
                    s.Hanging(0);
                }
                s.Break();
            }
        }
        s.Footer("Zones");
    }

    // ---- one page per zone
    for (int i = 0; i < in.zones.Size(); i++)
    {
        const JournalZoneRow& z = in.zones[i];
        const ZoneState st = StateOf(z, in.supportFlip);
        const bool town = stricmp(z.type, "CITY") == 0;
        const bool camp = stricmp(z.type, "CAMP") == 0;
        Sheet s(html, ZoneAnchor(i));
        const char* stateWord = st.group == 0                    ? "Our"
                                : st.group == 1                  ? "Contested"
                                : st.group == 3                  ? "Occupied"
                                : st.group == 4                  ? "Unscouted"
                                : strcmp(st.word, "RISING") == 0 ? "Rising"
                                                                 : "Neutral";
        s.Title(z.name);
        s.Subtitle(Standing(in, Fmt("%s %s.", stateWord, TypeWord(z.type))));
        if (!z.revealed)
        {
            s.Line("Not scouted yet. It shows on the map; move within reach of ground we hold to read it.", kType,
                   &kPencil);
            const RString range = Range(z);
            if (range.GetLength() > 0)
            {
                s.Note("Distance", range);
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
            s.Note("State", state, z.holder == 1 ? &kRedInk : nullptr);
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
            s.Note("Alert", alert, z.alert == 2 ? &kRedInk : nullptr);
            if (town && z.holder != 0)
            {
                s.Note("Support", Fmt("%d, %s", toInt(z.support),
                                      z.support >= in.supportFlip ? "past the line"
                                                                  : cstr(Fmt("line %d", toInt(in.supportFlip)))));
            }
            else if (town)
            {
                s.Note("Support", Fmt("%d, risen", toInt(z.support)));
            }
            if (!town && !camp && z.holder != 0)
            {
                s.Note("Capture", z.capture > 0 ? Fmt("%d%% secured", toInt(z.capture)) : RString("not started"));
            }
            s.Note("Heat", Fmt("%d, %s", toInt(z.heat), HeatWord(z.heat)), z.heat >= 50 ? &kRedInk : nullptr);
            const RString range = Range(z);
            if (range.GetLength() > 0)
            {
                s.Note("Distance", range);
            }
            const RString seen = Seen(z.seenDay, z.seenMinute, in.day);
            s.Note("Last seen", seen.GetLength() > 0 ? seen : RString("never up close"));
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
                s.Note("Here", JoinNames(here, 8));
            }
        }
        // the zone's own record: the latest lines here, the rest in the Diary
        s.Head("Record");
        const int recordCap = 10;
        int lines = 0;
        int earlier = 0;
        for (int e = journal.EntryCount() - 1; e >= 0; e--)
        {
            const JournalEntry& entry = journal.Entry(e);
            if (stricmp(entry.zone, z.name) != 0)
            {
                continue;
            }
            if (lines >= recordCap)
            {
                earlier++;
                continue;
            }
            s.Entry(entry, in.day, StampDay(entry.stamp) != in.day, false);
            lines++;
        }
        if (lines == 0)
        {
            s.Line("Nothing written about this place yet.", kType, &kPencil);
        }
        else if (earlier > 0)
        {
            s.Text(Fmt("%d earlier %s in the ", earlier, earlier == 1 ? "line" : "lines"), kType, &kPencil);
            s.Link("Diary", "#GM_LOG");
            s.Text(RString("."), kType, &kPencil);
            s.Break();
        }
        s.Footer("Zones");
    }
}

void BuildCell(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "GM_CELL");
    s.Title("Cell");
    RString stat = Fmt("%d under arms", d.withPlayer + d.holding);
    if (d.wounded > 0)
    {
        stat = stat + Fmt(", %d wounded", d.wounded);
    }
    s.Subtitle(stat + RString("."));

    // ---- roster, typed
    s.Head("Roster");
    const float cName = 0.27f, cRank = 0.09f, cRole = 0.20f, cArms = 0.30f, cCond = 0.14f;
    if (in.roster.Size() == 0)
    {
        s.Line("No fighters recorded.", kType, &kPencil);
    }
    RString lastGroup;
    for (int i = 0; i < in.roster.Size(); i++)
    {
        const JournalRosterRow& r = in.roster[i];
        const RString group = r.withPlayer ? RString("With me") : RString("Holding ") + r.zone;
        if (stricmp(group, lastGroup) != 0)
        {
            s.Line(group, kType, &kPencil);
            lastGroup = group;
        }
        s.Cell(r.name, cName, kType, nullptr, HALeft, r.isPlayer);
        s.Cell(r.count > 1 ? Fmt("x%d", r.count) : RankShort(r.rank), cRank, kType, &kPencil);
        s.Cell(r.role, cRole, kType, &kPencil);
        RString arms = r.primary;
        if (r.launcher.GetLength() > 0)
        {
            arms = arms + RString(" + ") + r.launcher;
        }
        s.Cell(arms, cArms, kType);
        if (r.wounded >= 25)
        {
            s.Cell(Fmt("WIA %d%%", r.wounded), cCond, kType, &kRedInk, HARight);
        }
        s.Break();
    }

    // ---- arms
    s.Head("Arms");
    bool anyArms = false;
    for (int i = 0; i < journal.StatusCount(); i++)
    {
        const JournalStatusLine& st = journal.Status(i);
        if (stricmp(st.key, "Unlocked gear") == 0 || stricmp(st.key, "Standard issue") == 0)
        {
            s.Note("Standard issue", st.text);
            anyArms = true;
        }
        else if (stricmp(st.key, "Next pattern") == 0)
        {
            s.Note("Next pattern", st.text);
            anyArms = true;
        }
    }
    if (!anyArms)
    {
        s.Note("Standard issue", "the faction's basic rifle", &kPencil);
    }
    if (in.hqEstablished)
    {
        s.Note("Cache", in.hqZone + RString(", at the headquarters"));
        s.Note("Garage", in.garage.Size() > 0 ? JoinNames(in.garage, 4)
                         : in.garageCount > 0 ? Fmt("%d vehicles", in.garageCount)
                                              : RString("empty"));
    }
    else if (in.stashCount > 0)
    {
        s.Note("Caches", Num((float)in.stashCount));
    }

    // ---- supply
    s.Head("Supply");
    if (in.economyKnown)
    {
        s.Note("Treasury", Fmt("%d R", toInt(in.resources)));
        s.Note("Manpower", in.manpowerCap > 0 ? Fmt("%d HR, pool %d", toInt(in.manpower), toInt(in.manpowerCap))
                                              : Fmt("%d HR", toInt(in.manpower)));
    }
    if (in.incomeKnown)
    {
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
        s.Note(label, income);
    }
    if (in.marketActive)
    {
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
                // tag the town's state when it is not ours
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
            s.Note(label, list.GetLength() > 0 ? list : RString("none known"),
                   list.GetLength() > 0 ? nullptr : &kPencil);
        };
        Dealers("Arms dealers", in.weaponDealerTowns);
        Dealers("Vehicle dealers", in.vehicleDealerTowns);
    }
    s.Footer("Cell");
}

void BuildResistance(CHTMLContainer* html, const Journal& /*journal*/, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "GM_FACTION");
    s.Title("Resistance");
    s.Subtitle(Fmt("War level %d of %d.", in.warLevel, in.warLevelMax));

    // ---- war level ladder (escalation.sqs bands: 20 / 40 / 55 / 70 / 85 % held)
    s.Head("War level");
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
    s.Note("Island held", held);
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
        s.Line(RString("The occupier steps up at ") + tiers +
                   RString(": better troops, heavier vehicles, sharper eyes."),
               kType, &kPencil);
    }

    // ---- ground
    s.Head("Ground");
    {
        int risen = 0, rising = 0, neutral = 0, occupied = 0, unscouted = 0;
        int held = 0, contested = 0, occBases = 0;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            const JournalZoneRow& z = in.zones[i];
            const bool town = stricmp(z.type, "CITY") == 0;
            const bool camp = stricmp(z.type, "CAMP") == 0;
            ZoneState st = StateOf(z, in.supportFlip);
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
                    held++;
                else if (z.capture > 0)
                    contested++;
                else if (z.holder == 1)
                    occBases++;
            }
        }
        s.Note("Towns", Fmt("%d risen, %d rising, %d neutral, %d occupied, %d unscouted", risen, rising, neutral,
                            occupied, unscouted));
        s.Note("Bases", Fmt("%d held, %d contested, %d occupied", held, contested, occBases));
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
        s.Note("Occupier under arms", Fmt("%d known.", d.knownGarrison) + garrisons);
        RString ours = Fmt("%d. %d with me, %d holding", d.withPlayer + d.holding, d.withPlayer, d.holding);
        if (in.economyKnown)
        {
            ours = ours + Fmt(", %d HR in reserve", toInt(in.manpower));
        }
        s.Note("Ours under arms", ours);
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
            s.Note("Heat on our ground",
                   Fmt("%d mean over %d %s", toInt(mean), heldZones, heldZones == 1 ? "zone" : "zones"),
                   mean >= 50 ? &kRedInk : nullptr);
        }
    }

    // ---- organisation (faction-management stubs read script globals)
    s.Head("Organisation");
    s.Note("Cells", Fmt("%d, ours. ", 1 + in.faction.alliedCells) +
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
            s.Note("Holdings", JoinNames(holdings, 4));
        }
    }
    if (in.faction.doctrine.GetLength() > 0)
    {
        s.Note("Doctrine", in.faction.doctrine);
    }
    if (in.faction.outsideSupport.GetLength() > 0)
    {
        s.Note("Outside support", in.faction.outsideSupport);
    }
    if (in.faction.alliedCells == 0 && in.faction.outsideSupport.GetLength() == 0)
    {
        s.Line("No allied cells. No outside contact.", kType, &kPencil);
    }
    s.Footer("Resistance");
}

// DIARY: the whole record by day, newest first, in the hand
void BuildDiary(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& /*d*/)
{
    Sheet s(html, "GM_LOG");
    s.Title("Diary");
    s.Subtitle(Fmt("%d %s.", journal.EntryCount(), journal.EntryCount() == 1 ? "entry" : "entries"));
    if (journal.EntryCount() == 0)
    {
        s.Line("Nothing written yet.", kType, &kPencil);
    }
    int lastDay = -1;
    for (int i = journal.EntryCount() - 1; i >= 0; i--)
    {
        const JournalEntry& e = journal.Entry(i);
        const int day = StampDay(e.stamp);
        if (day != lastDay)
        {
            s.Head(day > 0 ? Fmt("Day %d", day) : RString("Undated"));
            lastDay = day;
        }
        s.Entry(e, day, false, true);
    }
    (void)in;
    s.Footer("Diary");
}

void ManualTable(Sheet& s, const char* line, bool header)
{
    // split "|a|b|c" (or "!a|b|c") into cells
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
            s.Cell(text, widths[i], kType, &kPencil);
            continue;
        }
        const PackedColor* ink = i == 0 ? nullptr : &kPencil;
        bool bold = false;
        if (text.GetLength() >= 2 && text[0] == '~')
        {
            switch (text[1])
            {
                case 'r':
                    ink = &kRedInk;
                    break;
                case 'w':
                    ink = nullptr;
                    bold = true;
                    break;
                default:
                    ink = nullptr;
                    break;
            }
            text = text.Substring(2, INT_MAX);
        }
        if (i == n - 1 || i == 2)
        {
            // the last cell wraps under itself
            s.Text(text, kType, ink, "", 0, HALeft, bold);
        }
        else
        {
            s.Cell(text, widths[i], kType, ink, HALeft, bold);
        }
    }
    s.Break();
}

void BuildHandbook(CHTMLContainer* html, const Journal& /*journal*/, const JournalPageInputs& in, const Derived& /*d*/)
{
    // index
    {
        Sheet s(html, "GM_MAN_INDEX");
        s.Title("Handbook");
        s.Subtitle("Notes from the old hands.");
        for (int t = 0; t < kManualCount; t++)
        {
            s.Cell(Fmt("%d", t + 1), 0.06f, kType, &kPencil, HARight);
            s.Text(RString(" "), kType, nullptr, "", 0.02f);
            s.Link(RString(kManual[t].title), cstr(RString("#") + RString(kManual[t].anchor)));
            s.Text(RString("  ") + RString(kManual[t].subtitle), kType, &kPencil);
            s.Break();
        }
        s.Footer("Handbook");
    }
    // chapters
    for (int t = 0; t < kManualCount; t++)
    {
        const ManualTopic& topic = kManual[t];
        Sheet s(html, topic.anchor);
        s.Title(RString(topic.title));
        s.Subtitle(Cap(RString(topic.subtitle)) + Fmt(". Handbook %d of %d.", t + 1, kManualCount));
        for (int l = 0; topic.lines[l]; l++)
        {
            const char* line = topic.lines[l];
            if (line[0] == '#')
            {
                s.Head(RString(line + 1));
            }
            else if (line[0] == '!')
            {
                ManualTable(s, line, true);
            }
            else if (line[0] == '|')
            {
                ManualTable(s, line, false);
            }
            else if (line[0] == '-' && line[1] == ' ')
            {
                s.Bullet(RString(line + 2));
            }
            else if (strcmp(line, "@standing") == 0)
            {
                // the live cover state rides in the section head; the list
                // below stays pure reference
                if (!in.undercoverArmed)
                {
                    s.Head("Your standing");
                }
                else
                {
                    const char* word = in.undercoverStatus == 2   ? "BLOWN"
                                       : in.undercoverStatus == 1 ? "SUSPECTED"
                                                                  : "CLEAN";
                    RString head = Fmt("Your standing: now %s", word);
                    if (in.undercoverStatus == 2)
                    {
                        head = head + Fmt(", %d %s your face", in.undercoverWitnesses,
                                          in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know");
                    }
                    s.Head(head);
                }
            }
            else
            {
                s.Line(RString(line));
                s.Gap();
            }
        }
        // chapter nav
        if (t > 0)
        {
            s.Link(RString("< ") + RString(kManual[t - 1].title), cstr(RString("#") + RString(kManual[t - 1].anchor)));
            s.Text(RString("   "), kType);
        }
        if (t + 1 < kManualCount)
        {
            s.Link(RString(kManual[t + 1].title) + RString(" >"), cstr(RString("#") + RString(kManual[t + 1].anchor)));
            s.Text(RString("   "), kType);
        }
        s.Link("Index", "#GM_MAN_INDEX");
        s.Break();
        s.Footer("Handbook");
    }
}

// ---------------------------------------------------------------------------
// world-dependent helpers (all null-safe)
// ---------------------------------------------------------------------------

float ReadScalar(GameState* gstate, const char* name, float fallback, bool* found = nullptr)
{
    if (found)
    {
        *found = false;
    }
    if (!gstate)
    {
        return fallback;
    }
    GameValue value = gstate->VarGet(name);
    if (value.GetType() != GameScalar)
    {
        return fallback;
    }
    if (found)
    {
        *found = true;
    }
    return (float)value;
}

bool ReadBool(GameState* gstate, const char* name)
{
    if (!gstate)
    {
        return false;
    }
    GameValue value = gstate->VarGet(name);
    return value.GetType() == GameBool && (GameBoolType)value;
}

RString ReadString(GameState* gstate, const char* name)
{
    if (!gstate)
    {
        return RString();
    }
    GameValue value = gstate->VarGet(name);
    if (value.GetType() != GameString)
    {
        return RString();
    }
    return RString((GameStringType)value);
}

RString BearingOf(float dx, float dz)
{
    static const char* names[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    float deg = atan2f(dx, dz) * 180.0f / 3.14159265f;
    if (deg < 0)
    {
        deg += 360.0f;
    }
    int idx = (int)((deg + 22.5f) / 45.0f) % 8;
    return RString(names[idx]);
}

// the faction's displayName (optional descriptor key), else class name, else side
RString FactionDisplay(const ZoneRegistry& registry, const RString& className, const RString& side)
{
    if (className.GetLength() > 0)
    {
        RString dn = registry.FactionValue(className, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn;
        }
        return className;
    }
    if (side.GetLength() > 0)
    {
        RString dn = registry.FactionValue(side, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn;
        }
    }
    return side;
}

// primary / launcher display names of a person
void ArmsOf(Person* person, RString& primary, RString& launcher)
{
    if (!person)
    {
        return;
    }
    for (int i = 0; i < person->NWeaponSystems(); i++)
    {
        const WeaponType* weapon = person->GetWeaponSystem(i);
        if (!weapon || weapon->_scope < 2)
        {
            continue;
        }
        if ((weapon->_weaponType & MaskSlotPrimary) && primary.GetLength() == 0)
        {
            primary = weapon->GetDisplayName();
        }
        else if ((weapon->_weaponType & MaskSlotSecondary) && launcher.GetLength() == 0)
        {
            launcher = weapon->GetDisplayName();
        }
    }
}

RString NearestZoneName(const ZoneRegistry& registry, Vector3Par pos)
{
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        float dist = (z->pos - pos).SizeXZ();
        if (best < 0 || dist < bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }
    const ZoneRecord* z = best >= 0 ? registry.GetZone(best) : nullptr;
    return z ? z->name : RString();
}

} // namespace

// ===========================================================================
// public surface
// ===========================================================================

int GuerrillaManualTopicCount()
{
    return kManualCount;
}

const char* GuerrillaManualTopicTitle(int i)
{
    if (i < 0 || i >= kManualCount)
    {
        return "";
    }
    return kManual[i].title;
}

const char* GuerrillaManualTopicAnchor(int i)
{
    if (i < 0 || i >= kManualCount)
    {
        return "";
    }
    return kManual[i].anchor;
}

bool GuerrillaJournalActive()
{
    return ZoneRegistry::Instance().IsActive();
}

JournalPageInputs GatherGuerrillaJournalInputs()
{
    JournalPageInputs in;
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return in;
    }

    in.islandName = IslandDisplayName();
    in.resistanceName = FactionDisplay(registry, registry.ResistanceFaction(), registry.ResistanceSide());
    in.occupierName = FactionDisplay(registry, registry.OccupierFaction(), registry.OccupierSide());
    JournalClockNow(in.day, in.minuteOfDay);
    in.supportFlip = registry.Tuning().supportFlip;

    GameState* gstate = GWorld ? GWorld->GetGameState() : nullptr;
    bool foundR = false;
    bool foundHR = false;
    in.resources = ReadScalar(gstate, "gmresources", 0, &foundR);
    in.manpower = ReadScalar(gstate, "gmmanpower", 0, &foundHR);
    in.manpowerCap = ReadScalar(gstate, "gm_manpower_cap", 0);
    in.warLevel = toInt(ReadScalar(gstate, "gmwarlevel", 1));
    in.economyKnown = foundR && foundHR;
    bool foundIncome = false;
    in.incomeR = ReadScalar(gstate, "gmecor", 0, &foundIncome);
    in.incomeKnown = foundIncome;
    in.incomeHR = ReadScalar(gstate, "gmecohr", 0);
    in.econTickSeconds = ReadScalar(gstate, "gm_econ_tick", 0);
    in.undercoverArmed = ReadBool(gstate, "gmundercover");
    if (in.undercoverArmed)
    {
        in.undercoverStatus = UndercoverSystem::Instance().Status();
        in.undercoverWitnesses = UndercoverSystem::Instance().WitnessCount();
    }
    in.stashCount = StashRegistry::Instance().Count();
    in.faction.doctrine = ReadString(gstate, "gmdoctrine");
    in.faction.outsideSupport = ReadString(gstate, "gmoutsidesupport");
    in.faction.alliedCells = toInt(ReadScalar(gstate, "gmalliedcells", 0));

    // occupier tier ladder (war levels where the next tier arrives)
    if (const FactionRecord* occ = registry.FindFactionForSide(registry.OccupierSide()))
    {
        for (int i = 0; i < occ->tierThresholds.Size(); i++)
        {
            in.occupierTierThresholds.Add(occ->tierThresholds[i]);
        }
    }

    // headquarters (GuerrillaBase) and the dealer market
    {
        const GuerrillaBase& base = GuerrillaBase::Instance();
        in.hqEstablished = base.IsEstablished();
        in.hqZone = base.ZoneName();
        in.hqIndoors = base.IsIndoors();
        in.garageCount = base.GarageCount();
        for (int i = 0; i < base.GarageCount(); i++)
        {
            EntityAI* veh = base.GarageVehicle(i);
            if (veh && veh->GetType())
            {
                in.garage.Add(veh->GetType()->GetDisplayName());
            }
        }
        const Market& market = Market::Instance();
        in.marketActive = market.IsActive();
        for (int i = 0; i < market.DealerCount(); i++)
        {
            const DealerRecord* dealer = market.Dealer(i);
            if (!dealer)
            {
                continue;
            }
            (dealer->kind == DKWeapon ? in.weaponDealerTowns : in.vehicleDealerTowns).Add(dealer->zoneName);
        }
    }

    // the player and their group
    Vector3 playerPos = VZero;
    bool havePlayer = false;
    AIGroup* playerGroup = nullptr;
    if (GWorld)
    {
        Person* player = GWorld->PlayerOn();
        if (player)
        {
            playerPos = player->Position();
            havePlayer = true;
            AIUnit* unit = player->Brain();
            playerGroup = unit ? unit->GetGroup() : nullptr;
        }
    }
    if (playerGroup)
    {
        for (int id = 1; id <= MAX_UNITS_PER_GROUP; id++)
        {
            AIUnit* unit = playerGroup->UnitWithID(id);
            if (!unit || !unit->IsUnit())
            {
                continue;
            }
            Person* person = unit->GetPerson();
            if (!person)
            {
                continue;
            }
            JournalRosterRow row;
            row.isPlayer = unit->IsPlayer();
            row.name = row.isPlayer ? RString("You") : person->GetInfo()._name;
            row.rank = ClampRankIndex(person->GetRank());
            row.role = row.isPlayer ? RString("Leader") : RString(person->GetType()->GetDisplayName());
            ArmsOf(person, row.primary, row.launcher);
            row.wounded = toInt(person->GetTotalDammage() * 100.0f);
            row.withPlayer = true;
            // the player first
            if (row.isPlayer)
            {
                in.roster.Insert(0, row);
            }
            else
            {
                in.roster.Add(row);
            }
        }
    }
    // other resistance groups: holding squads, one row per group
    if (GWorld)
    {
        using Poseidon::Foundation::GetEnumValue;
        TargetSide side = GetEnumValue<TargetSide>((const char*)registry.ResistanceSide());
        AICenter* center = ((int)side >= 0 && side < TSideUnknown) ? GWorld->GetCenter(side) : nullptr;
        for (int g = 0; center && g < center->NGroups(); g++)
        {
            AIGroup* grp = center->GetGroup(g);
            if (!grp || grp == playerGroup)
            {
                continue;
            }
            JournalRosterRow row;
            row.withPlayer = false;
            row.count = 0;
            Vector3 where = VZero;
            for (int id = 1; id <= MAX_UNITS_PER_GROUP; id++)
            {
                AIUnit* unit = grp->UnitWithID(id);
                if (!unit || !unit->IsUnit())
                {
                    continue;
                }
                Person* person = unit->GetPerson();
                if (!person)
                {
                    continue;
                }
                if (row.count == 0)
                {
                    row.role = person->GetType()->GetDisplayName();
                    ArmsOf(person, row.primary, row.launcher);
                    where = person->Position();
                }
                row.count++;
            }
            if (row.count == 0)
            {
                continue;
            }
            row.zone = NearestZoneName(registry, where);
            row.name = row.zone.GetLength() > 0 ? row.zone + RString(" squad") : RString("Holding squad");
            in.roster.Add(row);
        }
    }

    // zones
    const RString resistance = registry.ResistanceSide();
    const RString occupier = registry.OccupierSide();
    const AlertMachine& alerts = AlertMachine::Instance();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        JournalZoneRow row;
        row.name = z->name;
        row.type = z->type;
        const bool ours = stricmp(z->owner, resistance) == 0;
        const bool theirs = stricmp(z->owner, occupier) == 0;
        row.holder = ours ? 0 : theirs ? 1 : 2;
        row.revealed = z->revealed;
        row.support = z->support;
        row.capture = z->capture;
        row.heat = z->heat;
        row.garrison = toInt(z->garrison);
        row.alert = alerts.GetZoneState(i);
        row.seenDay = z->seenDay;
        row.seenMinute = z->seenMinute;
        if (havePlayer)
        {
            const float dx = z->pos.X() - playerPos.X();
            const float dz = z->pos.Z() - playerPos.Z();
            row.distance = sqrtf(dx * dx + dz * dz);
            row.bearing = BearingOf(dx, dz);
        }
        const bool isCity = stricmp(z->type, "CITY") == 0;
        const bool isCamp = stricmp(z->type, "CAMP") == 0;
        if (isCity)
        {
            in.townsTotal++;
            if (ours)
            {
                in.townsRisen++;
            }
        }
        else if (!isCamp)
        {
            in.militaryTotal++;
            if (ours)
            {
                in.militaryHeld++;
            }
        }
        in.zones.Add(row);
    }
    return in;
}

void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in)
{
    if (!html)
    {
        return;
    }
    const float handSize = kHandScale * html->GetPHeight();
    html->SetFormatSize(kHand, handSize);
    if (GEngine)
    {
        // the pen face; the parser-only test container keeps the slot's face
        if (Font* pen = GEngine->LoadFont(GetFontID("cwrpen")))
        {
            html->SetFormatFont(kHand, pen, pen, handSize);
        }
    }
    const Derived d = Derive(in);
    BuildNotes(html, journal, in, d);
    BuildPlan(html, journal, in, d);
    BuildZones(html, journal, in, d);
    BuildCell(html, journal, in, d);
    BuildResistance(html, journal, in, d);
    BuildDiary(html, journal, in, d);
    BuildHandbook(html, journal, in, d);
}

} // namespace Poseidon::Guerrilla
