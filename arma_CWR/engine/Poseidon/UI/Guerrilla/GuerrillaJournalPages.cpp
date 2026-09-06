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
#include <Poseidon/Graphics/Core/Engine.hpp> // GEngine (fonts, aspect)
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
// theme: one colour law for every page
//   red    = alert / loss / wounded      yellow = caution (alert YELLOW, heat)
//   green  = ours / gain / go            tan    = neutral meter fill, heads
//   white  = names and values            muted / dim = labels, secondary
// ===========================================================================

inline PackedColor RGB(int r, int g, int b, int a = 255)
{
    return PackedColor(r, g, b, a);
}

const PackedColor kBg = RGB(18, 19, 13, 236);
const PackedColor kText = RGB(217, 215, 196);
const PackedColor kWhite = RGB(239, 238, 224);
const PackedColor kMuted = RGB(141, 140, 122);
const PackedColor kDim = RGB(95, 94, 80);
const PackedColor kTan = RGB(201, 185, 138);
const PackedColor kYellow = RGB(226, 177, 58);
const PackedColor kRed = RGB(210, 69, 47);
const PackedColor kGreen = RGB(127, 185, 68);
const PackedColor kTrack = RGB(38, 39, 25);
const PackedColor kRule = RGB(58, 59, 44);
const PackedColor kLink = kYellow;

// format slots after ApplyGuerrillaJournalTheme
constexpr HTMLFormat kTitle = HFH1;   // page title (display face)
constexpr HTMLFormat kStrip = HFH2;   // alert strip (display face, mid)
constexpr HTMLFormat kSection = HFH3; // section heads (display face, small caps)
constexpr HTMLFormat kMono = HFH4;    // figures, stamps, codes
constexpr HTMLFormat kSmall = HFH5;   // secondary body
constexpr HTMLFormat kSmallMono = HFH6;
constexpr HTMLFormat kBody = HFP;

// grid, as fractions of the page width
constexpr float kLabelW = 0.27f; // row label
constexpr float kDigitW = 0.11f; // right-aligned figure
constexpr float kUnitW = 0.08f;  // unit / denominator
constexpr float kGapW = 0.02f;
constexpr float kTailW = 0.18f; // bar tail (zone / target name)
constexpr float kValueX = kLabelW + kDigitW + kUnitW + kGapW;

// ===========================================================================
// handbook - static in-universe text.  Mini markup per line:
//   "#Heading"        section head        "!A|B|C"   table header (mono)
//   "|a|b|c"          table row; a cell starting with ~g / ~y / ~r / ~w is
//                     mono, coloured green / yellow / red / white
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
      "Everything in this journal is written as the campaign runs. Situation is the board; Plan carries the "
      "objectives and the next moves; Zones, Cell and Resistance are the ledgers; the Diary is the record.",
      "Open the journal whenever you like. It is rebuilt every time, so the figures are current.", "#Pages",
      "|~wSITUATION|the board: alert, strength, territory, threat, latest entries",
      "|~wPLAN|objectives with progress, the done list, tagged next moves",
      "|~wZONES|every zone: state, support, capture, heat, garrison, last seen, range",
      "|~wCELL|roster, fallen, arms, supply", "|~wRESISTANCE|war level ladder, ground held, organisation",
      "|~wDIARY|the whole record, newest first", nullptr}},
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
      "you are near (about 800 m), which is what SEEN records.",
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

RString Num(float v)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", toInt(v));
    return RString(buffer);
}

RString Fmt(const char* format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return RString(buffer);
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

// zone type -> ledger code
RString TypeCode(const RString& type)
{
    if (stricmp(type, "CITY") == 0)
    {
        return "TOWN";
    }
    if (stricmp(type, "CAMP") == 0)
    {
        return "CAMP";
    }
    if (stricmp(type, "AIRFIELD") == 0)
    {
        return "AIR";
    }
    if (stricmp(type, "SEAPORT") == 0)
    {
        return "PORT";
    }
    return "BASE";
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
    if (sscanf((const char*)stamp, "Day %d %d:%d", &day, &hh, &mm) == 3)
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
    if (sscanf((const char*)stamp, "Day %d", &day) == 1)
    {
        return day;
    }
    return 0;
}

RString Upper(const RString& s)
{
    RString out = s;
    out.Upper();
    return out;
}

PackedColor KindColor(int kind)
{
    switch (kind)
    {
        case JKGood:
            return kGreen;
        case JKWarn:
            return kYellow;
        case JKDanger:
            return kRed;
        default:
            return kDim;
    }
}

PackedColor HeatColor(float heat)
{
    if (heat >= 50)
    {
        return kRed;
    }
    if (heat >= 30)
    {
        return kYellow;
    }
    return kMuted;
}

PackedColor AlertColor(int alert)
{
    switch (alert)
    {
        case 2:
            return kRed;
        case 1:
            return kYellow;
        default:
            return kGreen;
    }
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

// zone state word + colour for the ledger
struct ZoneState
{
    const char* word;
    PackedColor color;
    int group; // 0 ours / 1 contested / 2 neutral / 3 occupied / 4 unscouted
};

ZoneState StateOf(const JournalZoneRow& z, float supportFlip)
{
    const bool town = stricmp(z.type, "CITY") == 0;
    if (!z.revealed)
    {
        return {"UNSCOUTED", kDim, 4};
    }
    if (z.holder == 0)
    {
        return {"HELD", kGreen, 0};
    }
    if (!town && z.capture > 0)
    {
        return {"SECURING", kGreen, 1};
    }
    if (town && z.holder != 1 && z.support >= supportFlip)
    {
        return {"RISING", kGreen, 2};
    }
    if (z.holder == 1)
    {
        return {"OCCUPIED", kRed, 3};
    }
    return {"NEUTRAL", kMuted, 2};
}

// ===========================================================================
// Sheet: a page emitter over the document model on the theme grid
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
        _lineH = html->GetPHeight();
        // a square pip needs the screen aspect; 16:9 when no engine is up
        _aspect = 0.5625f;
        if (GEngine && GEngine->Width2D() > 0)
        {
            _aspect = (float)GEngine->Height2D() / (float)GEngine->Width2D();
        }
    }

    int Section() const { return _section; }
    float PageW() const { return _pageW; }

    // ---- primitives ------------------------------------------------------
    void Text(const RString& text, HTMLFormat f = kBody, const PackedColor* color = nullptr, float tableW = 0,
              HTMLAlign align = HALeft, const char* href = "", bool bottom = false)
    {
        if (color)
        {
            _html->SetFieldColor(*color);
        }
        _html->AddText(_section, text, f, align, bottom, false, RString(href), tableW * _pageW);
        _html->ClearFieldColor();
    }
    void Cell(const RString& text, float w, HTMLFormat f = kBody, const PackedColor* color = nullptr,
              HTMLAlign align = HALeft)
    {
        Text(Fit(text, w, f), f, color, w, align);
    }
    void Break(bool bottom = false) { _html->AddBreak(_section, bottom); }
    void Gap(float lines = 0.5f)
    {
        // an empty bar of the wanted height makes a spacer row
        _html->AddBar(_section, 0, 1, lines * _lineH * 480.0f, RGB(0, 0, 0, 0), RGB(0, 0, 0, 0));
        Break();
    }
    void Rule(PackedColor color, float thickness = 0.08f, float w = 1.0f)
    {
        _html->AddBar(_section, 1.0f, w * _pageW * 640.0f, thickness * _lineH * 480.0f, color, RGB(0, 0, 0, 0));
        Break();
    }
    void Bar(float fill, float w, PackedColor color, float tick = -1, float h = 0.45f)
    {
        // a tick splits the bar in two adjacent segments with a white sliver
        if (tick > 0 && tick < 1)
        {
            const float sliver = 0.006f;
            const float left = w * tick - sliver * 0.5f;
            const float right = w - left - sliver;
            float f1 = tick > 0 ? fill / tick : 0;
            float f2 = (fill - tick) / (1 - tick);
            saturate(f1, 0.0f, 1.0f);
            saturate(f2, 0.0f, 1.0f);
            _html->AddBar(_section, f1, left * _pageW * 640.0f, h * _lineH * 480.0f, color, kTrack);
            _html->AddBar(_section, 1.0f, sliver * _pageW * 640.0f, (h + 0.25f) * _lineH * 480.0f, kWhite,
                          RGB(0, 0, 0, 0));
            _html->AddBar(_section, f2, right * _pageW * 640.0f, h * _lineH * 480.0f, color, kTrack);
            return;
        }
        _html->AddBar(_section, fill, w * _pageW * 640.0f, h * _lineH * 480.0f, color, kTrack);
    }
    void Pip(PackedColor color, float cellW = 0.04f)
    {
        const float h = 0.5f * _lineH * 480.0f;
        const float w = h * (640.0f / 480.0f) * _aspect;
        _html->AddBar(_section, 1.0f, w, h, color, RGB(0, 0, 0, 0), HALeft, cellW * _pageW);
    }
    void Spacer(float w) { Text(RString(""), kBody, nullptr, w); }

    // ---- composites --------------------------------------------------------
    void Title(const RString& text)
    {
        Text(Upper(text), kTitle, &kWhite);
        Break();
    }
    void Eyebrow(const RString& text)
    {
        Text(text, kSmallMono, &kDim);
        Break();
    }
    void Subtitle(const RString& text)
    {
        Text(text, kMono, &kMuted);
        Break();
        Rule(kTan, 0.1f);
    }
    void Head(const RString& text, bool dim = false)
    {
        Gap(0.35f);
        Text(Upper(text), kSection, dim ? &kDim : &kTan);
        Break();
        Rule(kRule, 0.05f);
    }
    void Para(const RString& text, HTMLFormat f = kBody, const PackedColor* color = nullptr)
    {
        Text(text, f, color);
        Break();
    }
    void Bullet(const RString& text)
    {
        Text(RString("-"), kBody, &kTan, 0.035f);
        _html->SetHanging(0.035f * _pageW);
        Text(text, kBody);
        _html->SetHanging(0);
        Break();
    }
    // label | digits | unit | value
    void KV(const RString& label, const RString& digits, const RString& unit, const RString& value,
            const PackedColor* digitColor = nullptr, const PackedColor* valueColor = nullptr)
    {
        Cell(label, kLabelW, kBody, &kMuted);
        Cell(digits, kDigitW, kMono, digitColor ? digitColor : &kWhite, HARight);
        Cell(RString(" ") + unit, kUnitW, kMono, &kMuted);
        Spacer(kGapW);
        _html->SetHanging(kValueX * _pageW);
        Text(value, kBody, valueColor ? valueColor : &kWhite);
        _html->SetHanging(0);
        Break();
    }
    // label | digits | unit | bar | tail
    void KVBar(const RString& label, const RString& digits, const RString& unit, float fill, PackedColor color,
               const RString& tail = RString(), float tick = -1, const PackedColor* digitColor = nullptr,
               const PackedColor* labelColor = nullptr)
    {
        Cell(label, kLabelW, kBody, labelColor ? labelColor : &kMuted);
        Cell(digits, kDigitW, kMono, digitColor ? digitColor : &kWhite, HARight);
        Cell(RString(" ") + unit, kUnitW, kMono, &kMuted);
        Spacer(kGapW);
        Bar(fill, 1.0f - kValueX - kTailW - kGapW, color, tick);
        Spacer(kGapW);
        Cell(tail, kTailW - kGapW, kBody, &kWhite);
        Break();
    }
    // the bottom-pinned nav row
    void Nav(const char* current, const char* backText = nullptr, const char* backHref = nullptr)
    {
        static const char* names[] = {"Situation", "Plan", "Zones", "Cell", "Resistance", "Diary", "Handbook"};
        static const char* anchors[] = {"#Main",       "#Plan",   "#GM_ZONES",    "#GM_CELL",
                                        "#GM_FACTION", "#GM_LOG", "#GM_MAN_INDEX"};
        Gap(0.5f);
        if (backText && backHref)
        {
            // a secondary link pinned just above the nav row
            Text(RString(backText), kSmallMono, nullptr, 0, HALeft, backHref, true);
            Break(true);
        }
        _html->AddBar(_section, 1.0f, _pageW * 640.0f, 0.05f * _lineH * 480.0f, kRule, RGB(0, 0, 0, 0));
        Break(true);
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
            {
                Text(RString("  |  "), kMono, &kDim, 0, HALeft, "", true);
            }
            if (strcmp(names[i], current) == 0)
            {
                Text(RString(names[i]), kMono, &kMuted, 0, HALeft, "", true);
            }
            else
            {
                Text(RString(names[i]), kMono, nullptr, 0, HALeft, anchors[i], true);
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
    float _lineH;
    float _aspect;
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
    AutoArray<const JournalZoneRow*> targets;  // occupied bases, nearest first
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
        if (z.revealed && !town && !camp && z.holder == 1)
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

// page chrome: eyebrow (optional), title, subtitle, alert strip
void Masthead(Sheet& s, const JournalPageInputs& in, const Derived& d, const RString& title, const RString& stat,
              const char* eyebrow = nullptr)
{
    if (eyebrow)
    {
        s.Eyebrow(RString(eyebrow));
    }
    s.Title(title);
    RString sub;
    if (in.resistanceName.GetLength() > 0 && in.occupierName.GetLength() > 0)
    {
        sub = in.resistanceName + RString(" vs. ") + in.occupierName;
    }
    else if (in.islandName.GetLength() > 0)
    {
        sub = in.islandName + RString(" campaign");
    }
    else
    {
        sub = "Guerrilla campaign";
    }
    if (stat.GetLength() > 0)
    {
        sub = sub + RString(" - ") + stat;
    }
    if (in.day > 0)
    {
        sub = sub + Fmt(" - Day %d %02d:%02d", in.day, in.minuteOfDay / 60, in.minuteOfDay % 60);
    }
    s.Subtitle(sub);
    if (d.redZone)
    {
        s.Cell(RString("ALERT RED - ") + Upper(d.redZone->name), 0.55f, kStrip, &kRed);
        RString right = Range(*d.redZone);
        right = right.GetLength() > 0 ? right + RString(" - QRF OUT") : RString("QRF OUT");
        if (d.redCount > 1)
        {
            right = right + Fmt(" (+%d)", d.redCount - 1);
        }
        s.Cell(right, 0.45f, kMono, &kRed, HARight);
        s.Break();
        s.Rule(kRed, 0.1f);
    }
}

// the diary line as a row: stamp | zone tag | text
void DiaryLine(Sheet& s, const JournalEntry& e, int today, bool forceDay, bool withTag = true)
{
    RString stamp = CompactStamp(e.stamp, forceDay ? -1 : today);
    s.Cell(stamp, 0.15f, kMono, &kMuted);
    PackedColor tagColor = KindColor(e.kind);
    if (withTag)
    {
        s.Cell(Upper(e.zone), 0.16f, kMono, &tagColor);
    }
    else
    {
        s.Pip(tagColor, 0.03f);
    }
    s.Text(e.text, kBody, &kText);
    s.Break();
}

// ===========================================================================
// pages
// ===========================================================================

void BuildSituation(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "Main");
    Masthead(s, in, d, "Situation", in.islandName);

    // ---- strength
    s.Head("Strength");
    if (in.economyKnown)
    {
        s.KV("Treasury", Num(in.resources), "R", "");
        RString pool = in.manpowerCap > 0 ? Fmt("pool %d", toInt(in.manpowerCap)) : RString();
        s.KV("Manpower", Num(in.manpower), "HR", pool, nullptr, &kMuted);
    }
    {
        // who is with you: you, companions by name, the rest by count
        RString who;
        int riflemen = 0;
        for (int i = 0; i < in.roster.Size(); i++)
        {
            const JournalRosterRow& r = in.roster[i];
            if (!r.withPlayer)
            {
                continue;
            }
            if (r.isPlayer)
            {
                who = "you";
            }
            else
            {
                riflemen += r.count;
            }
        }
        if (riflemen > 0)
        {
            who = who + Fmt("%s%d %s", who.GetLength() ? ", " : "", riflemen, riflemen == 1 ? "fighter" : "fighters");
        }
        if (d.wounded > 0)
        {
            who = who + Fmt(" - %d WIA", d.wounded);
        }
        s.KV("Fighters with you", Num((float)d.withPlayer), "", who);
    }
    if (d.holding > 0)
    {
        s.KV("Holding", Num((float)d.holding), "",
             JoinNames(d.holdingZones) + RString(d.holdingZones.Size() == 1 ? " squad" : " squads"));
    }
    if (in.hqEstablished)
    {
        RString hq = in.hqZone.GetLength() > 0 ? in.hqZone : RString("unknown zone");
        hq = hq + RString(in.hqIndoors ? " - house" : " - camp");
        if (in.garageCount > 0)
        {
            hq = hq + Fmt(" - %d garaged", in.garageCount);
        }
        s.KV("Headquarters", "", "", hq);
    }
    else
    {
        s.KV("Headquarters", "", "", "none - set one up in a town or at the Camp", nullptr, &kMuted);
    }
    // script status lines the managers publish (companions roster text);
    // the arms lines live on the Cell page
    for (int i = 0; i < journal.StatusCount(); i++)
    {
        const JournalStatusLine& st = journal.Status(i);
        if (stricmp(st.key, "Unlocked gear") == 0 || stricmp(st.key, "Standard issue") == 0 ||
            stricmp(st.key, "Next pattern") == 0)
        {
            continue;
        }
        s.KV(st.key, "", "", st.text);
    }

    // ---- territory
    s.Head("Territory");
    s.KVBar("Bases held", Num((float)in.militaryHeld), Fmt("/ %d", in.militaryTotal),
            in.militaryTotal > 0 ? (float)in.militaryHeld / in.militaryTotal : 0, kGreen);
    s.KVBar("Towns risen", Num((float)in.townsRisen), Fmt("/ %d", in.townsTotal),
            in.townsTotal > 0 ? (float)in.townsRisen / in.townsTotal : 0, kGreen);
    s.KVBar("War level", Num((float)in.warLevel), Fmt("/ %d", in.warLevelMax),
            in.warLevelMax > 0 ? (float)in.warLevel / in.warLevelMax : 0, kTan);
    for (int i = 0; i < d.ready.Size() && i < 2; i++)
    {
        s.KVBar(i == 0 ? "Ready to rise" : "", Num(d.ready[i]->support), "SUP", d.ready[i]->support / 100.0f, kTan,
                d.ready[i]->name, in.supportFlip / 100.0f);
    }
    for (int i = 0; i < d.securing.Size() && i < 2; i++)
    {
        s.KVBar(i == 0 ? "Securing" : "", Num(d.securing[i]->capture), "%", d.securing[i]->capture / 100.0f, kTan,
                d.securing[i]->name);
    }

    // ---- threat
    s.Head("Threat");
    if (d.redZone)
    {
        s.KV("QRF", Num((float)d.redCount), "out",
             RString("from ") + d.redZone->name + RString(" - on last known position"), &kRed);
    }
    if (d.yellowZone)
    {
        s.Cell("Garrisons", kLabelW, kBody, &kMuted);
        s.Spacer(kDigitW + kUnitW + kGapW);
        s.Pip(kYellow);
        s.Text("YELLOW ", kMono, &kYellow);
        s.Text(d.yellowZone->name, kBody, &kWhite);
        s.Break();
    }
    else if (!d.redZone)
    {
        s.KV("Garrisons", "", "", "all GREEN", nullptr, &kGreen);
    }
    if (d.hottest && d.hottest->heat >= 30)
    {
        PackedColor c = HeatColor(d.hottest->heat);
        s.KVBar("Heat, highest", Num(d.hottest->heat), "", d.hottest->heat / 100.0f, c, d.hottest->name, -1, &c);
    }
    if (in.undercoverArmed)
    {
        s.Cell("Cover", kLabelW, kBody, &kMuted);
        s.Spacer(kDigitW + kUnitW + kGapW);
        if (in.undercoverStatus == 2)
        {
            s.Text("BLOWN", kMono, &kRed);
            s.Text(Fmt(" - known to %d %s", in.undercoverWitnesses, in.undercoverWitnesses == 1 ? "patrol" : "patrols"),
                   kBody, &kMuted);
        }
        else if (in.undercoverStatus == 1)
        {
            s.Text("SUSPECTED", kMono, &kYellow);
            s.Text(" - a group is checking you", kBody, &kMuted);
        }
        else
        {
            s.Text("CLEAN", kMono, &kGreen);
            s.Text(" - you pass as a civilian", kBody, &kMuted);
        }
        s.Break();
    }

    // ---- latest entries
    s.Head("Latest entries");
    if (journal.EntryCount() == 0)
    {
        s.Para("Nothing recorded yet.", kBody, &kMuted);
    }
    const int recent = 3;
    for (int i = journal.EntryCount() - 1, n = 0; i >= 0 && n < recent; i--, n++)
    {
        const JournalEntry& e = journal.Entry(i);
        DiaryLine(s, e, in.day, StampDay(e.stamp) != in.day);
    }
    s.Nav("Situation");
}

// one objective row: pip | text | (digits | unit | bar)
void ObjectiveRow(Sheet& s, bool done, const RString& text, const RString& digits = RString(),
                  const RString& unit = RString(), float fill = -1)
{
    s.Pip(done ? kGreen : kTan, 0.035f);
    if (fill < 0)
    {
        s.Text(text, kBody, done ? &kDim : &kWhite);
        s.Break();
        return;
    }
    s.Cell(text, kLabelW - 0.035f, kBody, done ? &kDim : &kWhite);
    s.Cell(digits, kDigitW, kMono, &kWhite, HARight);
    s.Cell(RString(" ") + unit, kUnitW, kMono, &kMuted);
    s.Spacer(kGapW);
    s.Bar(fill, 1.0f - kValueX - kTailW - kGapW, kGreen);
    s.Break();
}

void BuildPlan(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "Plan");
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
    Masthead(s, in, d, "Plan", Fmt("%d %s open", open, open == 1 ? "objective" : "objectives"));

    s.Head("Objectives");
    if (!basesDone)
    {
        ObjectiveRow(s, false, "Hold every base", Num((float)in.militaryHeld), Fmt("/ %d", in.militaryTotal),
                     in.militaryTotal > 0 ? (float)in.militaryHeld / in.militaryTotal : 0);
    }
    if (!townsDone)
    {
        ObjectiveRow(s, false, "Raise every town", Num((float)in.townsRisen), Fmt("/ %d", in.townsTotal),
                     in.townsTotal > 0 ? (float)in.townsRisen / in.townsTotal : 0);
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOActive)
        {
            ObjectiveRow(s, false, o.text);
        }
    }
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        if (o.state == JOFailed)
        {
            s.Pip(kRed, 0.035f);
            s.Text(o.text, kBody, &kRed);
            s.Break();
        }
    }
    bool anyDone = basesDone || townsDone;
    for (int i = 0; i < journal.ObjectiveCount() && !anyDone; i++)
    {
        anyDone = journal.Objective(i).state == JODone;
    }
    if (anyDone)
    {
        s.Head("Done", true);
        if (basesDone)
        {
            ObjectiveRow(s, true, "Every base held");
        }
        if (townsDone)
        {
            ObjectiveRow(s, true, "Every town risen");
        }
        for (int i = 0; i < journal.ObjectiveCount(); i++)
        {
            const JournalObjective& o = journal.Objective(i);
            if (o.state == JODone)
            {
                ObjectiveRow(s, true, o.text);
            }
        }
    }

    // ---- next moves: URGENT (red) / READY (green) / ROUTINE (muted)
    s.Head("Next moves");
    int moves = 0;
    auto Move = [&](int tier, const RString& range, const RString& head, const RString& tail)
    {
        const char* tag = tier == 0 ? "URGENT" : tier == 1 ? "READY" : "ROUTINE";
        const PackedColor& tagColor = tier == 0 ? kRed : tier == 1 ? kGreen : kMuted;
        const PackedColor& headColor = tier == 2 ? kWhite : tagColor;
        s.Cell(RString(tag), kLabelW, kSmallMono, &tagColor);
        s.Text(head + RString(" "), kBody, &headColor);
        s.Text(tail, kBody, &kMuted);
        s.Break();
        s.Cell(range.GetLength() > 0 ? range : RString("-"), kLabelW, kSmallMono, &kDim);
        s.Break();
        moves++;
    };
    if (d.redZone)
    {
        Move(0, Range(*d.redZone), "Break contact.", d.redZone->name + RString(" RED, QRF out."));
    }
    if (in.undercoverArmed && in.undercoverStatus == 2)
    {
        Move(0, "", "Go dark.",
             Fmt("%d %s your face. Stow the weapon; lose or drop the witnesses.", in.undercoverWitnesses,
                 in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know"));
    }
    if (d.hottest && d.hottest->heat >= 50)
    {
        if (d.hottest->holder == 0)
        {
            Move(0, Range(*d.hottest), d.hottest->name + Fmt(" heat %d.", toInt(d.hottest->heat)),
                 "On our own holding. Reinforce or pull the squad.");
        }
        else
        {
            Move(2, Range(*d.hottest), RString("Lie low near ") + d.hottest->name + RString("."),
                 Fmt("Heat %d: the garrison is on edge.", toInt(d.hottest->heat)));
        }
    }
    for (int i = 0; i < d.ready.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.ready[i];
        RString tail = Fmt("Support %d, line %d. ", toInt(z.support), toInt(in.supportFlip));
        tail = tail + (z.garrison > 0 ? Fmt("%d occupiers in town: clear or wait them out.", z.garrison)
                                      : RString("Fighters into the town while no occupier is present."));
        Move(1, Range(z), RString("Raise ") + z.name + RString("."), tail);
    }
    for (int i = 0; i < d.securing.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.securing[i];
        Move(1, Range(z), RString("Finish securing ") + z.name + RString("."),
             z.garrison > 0
                 ? Fmt("%d%% secured. Fighters inside; garrison %d still on the field.", toInt(z.capture), z.garrison)
                 : Fmt("%d%% secured. Fighters inside, keep the garrison out.", toInt(z.capture)));
    }
    for (int i = 0; i < d.targets.Size() && i < 3; i++)
    {
        const JournalZoneRow& z = *d.targets[i];
        Move(2, Range(z), RString("Target ") + z.name + RString("."),
             Fmt("Garrison %d, alert %s.", z.garrison, AlertName(z.alert)));
    }
    if (in.economyKnown && in.manpower >= 1)
    {
        Move(2, "", "Recruit at the Camp.", Fmt("%d HR in reserve, 1 HR a fighter.", toInt(in.manpower)));
    }
    if (moves == 0)
    {
        Move(2, "", "Scout the island.", "Zones show once they are within reach of ground you hold.");
    }
    if (!in.hqEstablished)
    {
        // standing advice, not a tactical move: it never displaces the scout line
        Move(2, "", "Set up a headquarters.", "Any town, or the Camp. It gives you a cache and a garage.");
    }
    s.Nav("Plan");
}

RString ZoneAnchor(int index)
{
    return Fmt("GM_ZONE_%d", index);
}

// the zone's meter sentence for the index and its page
RString ZoneMeter(const JournalZoneRow& z, float supportFlip)
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
        case 1: // contested: the capture meter
            return Fmt("%d%% secured", toInt(z.capture));
        case 2: // neutral: support against the line
            return town ? Fmt("support %d - line %d", toInt(z.support), toInt(supportFlip)) : RString();
        case 3: // occupied: the garrison
            return z.garrison > 0 ? Fmt("garrison %d", z.garrison) : RString();
        default:
            return RString();
    }
}

void BuildZones(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    // ---- index: one line per zone, grouped, nearest first
    {
        Sheet s(html, "GM_ZONES");
        Masthead(s, in, d, "Zones", Fmt("%d of %d scouted", d.scouted, in.zones.Size()));
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
                s.Pip(z.revealed ? AlertColor(z.alert) : kTrack, 0.04f);
                s.Text(z.name, kBody, nullptr, 0.30f, HALeft, RString("#") + ZoneAnchor(rows[r]));
                s.Cell(TypeCode(z.type), 0.10f, kSmallMono, &kDim);
                s.Cell(ZoneMeter(z, in.supportFlip), 0.34f, kSmall, &kMuted);
                s.Cell(Km(z.distance).GetLength() ? Km(z.distance) + RString(" km ") + z.bearing : RString(), 0.22f,
                       kMono, &kMuted, HARight);
                s.Break();
            }
        }
        s.Gap(0.4f);
        s.Pip(kGreen, 0.03f);
        s.Text("GREEN calm  ", kSmallMono, &kMuted);
        s.Pip(kYellow, 0.03f);
        s.Text("YELLOW aware  ", kSmallMono, &kMuted);
        s.Pip(kRed, 0.03f);
        s.Text("RED combat, QRF out  ", kSmallMono, &kMuted);
        s.Pip(kTrack, 0.03f);
        s.Text("unscouted", kSmallMono, &kMuted);
        s.Break();
        s.Nav("Zones");
    }

    // ---- one page per zone
    for (int i = 0; i < in.zones.Size(); i++)
    {
        const JournalZoneRow& z = in.zones[i];
        const ZoneState st = StateOf(z, in.supportFlip);
        const bool town = stricmp(z.type, "CITY") == 0;
        const bool camp = stricmp(z.type, "CAMP") == 0;
        Sheet s(html, ZoneAnchor(i));
        RString kind = town                               ? "town"
                       : camp                             ? "camp"
                       : stricmp(z.type, "AIRFIELD") == 0 ? "airfield"
                       : stricmp(z.type, "SEAPORT") == 0  ? "port"
                                                          : "outpost";
        RString stateWord = st.group == 0                    ? "our"
                            : st.group == 1                  ? "contested"
                            : st.group == 3                  ? "occupied"
                            : st.group == 4                  ? "unscouted"
                            : strcmp(st.word, "RISING") == 0 ? "rising"
                                                             : "neutral";
        Masthead(s, in, d, z.name, stateWord + RString(" ") + kind);
        if (!z.revealed)
        {
            s.Para("Not scouted yet. It shows on the map; move within reach of ground we hold to read it.", kBody,
                   &kMuted);
            RString range = Range(z);
            if (range.GetLength() > 0)
            {
                s.Gap(0.4f);
                s.KV("Distance", "", "", range);
            }
        }
        else
        {
            s.Cell("State", kLabelW, kBody, &kMuted);
            s.Spacer(kDigitW + kUnitW + kGapW);
            s.Text(RString(st.word), kMono, &st.color);
            if (z.holder == 1 && z.garrison > 0)
            {
                s.Text(Fmt(" - garrison %d", z.garrison), kBody, &kMuted);
            }
            else if (z.holder != 0 && z.garrison > 0)
            {
                s.Text(Fmt(" - %d occupiers in town", z.garrison), kBody, &kMuted);
            }
            s.Break();
            s.Cell("Alert", kLabelW, kBody, &kMuted);
            s.Spacer(kDigitW + kUnitW + kGapW);
            s.Pip(AlertColor(z.alert));
            PackedColor ac = AlertColor(z.alert);
            s.Text(RString(AlertName(z.alert)), kMono, &ac);
            if (z.alert == 2)
            {
                s.Text(" - a quick reaction force is out", kBody, &kMuted);
            }
            else if (z.alert == 1)
            {
                s.Text(" - they are checking your last known position", kBody, &kMuted);
            }
            s.Break();
            if (town && z.holder != 0)
            {
                s.KVBar("Support", Num(z.support), "SUP", z.support / 100.0f, kTan,
                        z.support >= in.supportFlip ? RString("past the line") : Fmt("line %d", toInt(in.supportFlip)),
                        in.supportFlip / 100.0f);
            }
            else if (town)
            {
                s.KVBar("Support", Num(z.support), "SUP", z.support / 100.0f, kTan, "risen");
            }
            if (!town && !camp && z.holder != 0)
            {
                s.KVBar("Capture", Num(z.capture), "%", z.capture / 100.0f, kTan,
                        z.capture > 0 ? RString("securing") : RString("not started"));
            }
            PackedColor hc = HeatColor(z.heat);
            s.KVBar("Heat", Num(z.heat), "", z.heat / 100.0f, hc,
                    z.heat >= 50   ? RString("on edge")
                    : z.heat >= 30 ? RString("aware")
                                   : RString("quiet"),
                    -1, &hc);
            RString range = Range(z);
            if (range.GetLength() > 0)
            {
                s.KV("Distance", "", "", range);
            }
            s.KV("Last seen", "", "",
                 Seen(z.seenDay, z.seenMinute, in.day).GetLength() ? Seen(z.seenDay, z.seenMinute, in.day)
                                                                   : RString("never up close"));
            // what the cell has here
            RString here;
            if (in.hqEstablished && stricmp(in.hqZone, z.name) == 0)
            {
                here = "headquarters";
                if (in.garageCount > 0)
                {
                    here = here + Fmt(", %d garaged", in.garageCount);
                }
            }
            for (int r = 0; r < in.roster.Size(); r++)
            {
                if (!in.roster[r].withPlayer && stricmp(in.roster[r].zone, z.name) == 0)
                {
                    here = here + Fmt("%sholding squad of %d", here.GetLength() ? " - " : "", in.roster[r].count);
                }
            }
            for (int t = 0; t < in.weaponDealerTowns.Size(); t++)
            {
                if (stricmp(in.weaponDealerTowns[t], z.name) == 0)
                {
                    here = here + RString(here.GetLength() ? " - arms dealer" : "arms dealer");
                }
            }
            for (int t = 0; t < in.vehicleDealerTowns.Size(); t++)
            {
                if (stricmp(in.vehicleDealerTowns[t], z.name) == 0)
                {
                    here = here + RString(here.GetLength() ? " - vehicle dealer" : "vehicle dealer");
                }
            }
            if (here.GetLength() > 0)
            {
                s.KV("Here", "", "", here);
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
            DiaryLine(s, entry, in.day, StampDay(entry.stamp) != in.day, false);
            lines++;
        }
        if (lines == 0)
        {
            s.Para("Nothing written about this place yet.", kBody, &kMuted);
        }
        else if (earlier > 0)
        {
            s.Text(Fmt("%d earlier %s in the ", earlier, earlier == 1 ? "line" : "lines"), kSmall, &kMuted);
            s.Text("Diary", kSmall, nullptr, 0, HALeft, "#GM_LOG");
            s.Break();
        }
        s.Nav("Zones", "< Zones", "#GM_ZONES");
    }
}

void BuildCell(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "GM_CELL");
    RString stat = Fmt("%d under arms", d.withPlayer + d.holding);
    if (d.wounded > 0)
    {
        stat = stat + Fmt(" - %d WIA", d.wounded);
    }
    Masthead(s, in, d, "Cell", stat);

    // ---- roster
    s.Head("Roster");
    const float cName = 0.27f, cRank = 0.10f, cRole = 0.21f, cArms = 0.30f, cCond = 0.12f;
    if (in.roster.Size() == 0)
    {
        s.Para("No fighters recorded.", kBody, &kMuted);
    }
    RString lastGroup;
    for (int i = 0; i < in.roster.Size(); i++)
    {
        const JournalRosterRow& r = in.roster[i];
        RString group = r.withPlayer ? RString("WITH YOU") : RString("HOLDING - ") + Upper(r.zone);
        if (stricmp(group, lastGroup) != 0)
        {
            s.Text(group, kSmallMono, &kDim);
            s.Break();
            lastGroup = group;
        }
        s.Cell(r.name, cName, kBody, &kWhite);
        s.Cell(r.count > 1 ? Fmt("x%d", r.count) : RankShort(r.rank), cRank, kMono, &kMuted);
        s.Cell(r.role, cRole, kBody, &kMuted);
        RString arms = r.primary;
        if (r.launcher.GetLength() > 0)
        {
            arms = arms + RString(" + ") + r.launcher;
        }
        s.Cell(arms, cArms, kMono, &kText);
        if (r.wounded >= 25)
        {
            s.Cell(Fmt("WIA %d%%", r.wounded), cCond, kMono, &kRed, HARight);
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
            s.KV("Standard issue", "", "", st.text);
            anyArms = true;
        }
        else if (stricmp(st.key, "Next pattern") == 0)
        {
            s.KV("Next pattern", "", "", st.text);
            anyArms = true;
        }
    }
    if (!anyArms)
    {
        s.KV("Standard issue", "", "", "the faction's basic rifle", nullptr, &kMuted);
    }
    if (in.hqEstablished)
    {
        s.KV("Cache", Num((float)(in.stashCount > 0 ? in.stashCount : 1)), "", in.hqZone + RString(" - HQ"));
        s.KV("Garage", Num((float)in.garageCount), "",
             in.garage.Size() > 0 ? JoinNames(in.garage, 4) : RString(in.garageCount > 0 ? "" : "empty"));
    }
    else if (in.stashCount > 0)
    {
        s.KV("Caches", Num((float)in.stashCount), "", "");
    }

    // ---- supply
    s.Head("Supply");
    if (in.economyKnown)
    {
        s.KV("Treasury", Num(in.resources), "R", "");
        s.KV("Manpower", Num(in.manpower), "HR", in.manpowerCap > 0 ? Fmt("pool %d", toInt(in.manpowerCap)) : RString(),
             nullptr, &kMuted);
    }
    if (in.incomeKnown)
    {
        RString label = in.econTickSeconds > 0 ? Fmt("Income / %d min", toInt(in.econTickSeconds / 60.0f))
                                               : RString("Income / tick");
        AutoArray<RString> payers;
        for (int i = 0; i < in.zones.Size(); i++)
        {
            if (in.zones[i].holder == 0 && stricmp(in.zones[i].type, "CAMP") != 0)
            {
                payers.Add(in.zones[i].name);
            }
        }
        s.KV(label, Fmt("+%d", toInt(in.incomeR)), "R", JoinNames(payers, 4), nullptr, &kMuted);
        s.KV("", Fmt("+%d", toInt(in.incomeHR)), "HR", "");
    }
    if (in.marketActive)
    {
        auto DealerLine = [&](const char* label, const AutoArray<RString>& towns)
        {
            s.Cell(RString(label), kLabelW, kBody, &kMuted);
            s.Spacer(kDigitW + kUnitW + kGapW);
            if (towns.Size() == 0)
            {
                s.Text("none known", kBody, &kMuted);
            }
            for (int t = 0; t < towns.Size() && t < 4; t++)
            {
                if (t > 0)
                {
                    s.Text(" - ", kBody, &kMuted);
                }
                s.Text(towns[t], kBody, &kWhite);
                // tag the town's state when it is not ours
                for (int i = 0; i < in.zones.Size(); i++)
                {
                    if (stricmp(in.zones[i].name, towns[t]) != 0)
                    {
                        continue;
                    }
                    ZoneState st = StateOf(in.zones[i], in.supportFlip);
                    if (st.group != 0)
                    {
                        s.Text(RString(" ") + RString(st.word), kSmallMono, &st.color);
                    }
                }
            }
            s.Break();
        };
        DealerLine("Arms dealers", in.weaponDealerTowns);
        DealerLine("Vehicle dealers", in.vehicleDealerTowns);
    }
    s.Nav("Cell");
}

void BuildResistance(CHTMLContainer* html, const Journal& /*journal*/, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "GM_FACTION");
    Masthead(s, in, d, "Resistance", Fmt("war level %d", in.warLevel));

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
    RString tail = nextAt > 0 ? Fmt("WL %d at %d", in.warLevel + 1, nextAt) : RString("top of the ladder");
    {
        // the ladder as tick marks on the bar, labels right-aligned under each tick
        const float barW = 1.0f - kValueX - kTailW - kGapW;
        s.Cell("Island held", kLabelW, kBody, &kMuted);
        s.Cell(Num((float)d.heldPct), kDigitW, kMono, &kWhite, HARight);
        s.Cell(" %", kUnitW, kMono, &kMuted);
        s.Spacer(kGapW);
        float at = 0;
        for (int i = 0; i < 5; i++)
        {
            const float to = ladder[i] / 100.0f;
            float fill = (d.heldPct / 100.0f - at) / (to - at);
            saturate(fill, 0.0f, 1.0f);
            s.Bar(fill, barW * (to - at) - 0.004f, kGreen);
            s.Bar(1.0f, 0.004f, kMuted, -1, 0.7f);
            at = to;
        }
        float fill = (d.heldPct / 100.0f - at) / (1.0f - at);
        saturate(fill, 0.0f, 1.0f);
        s.Bar(fill, barW * (1.0f - at), kGreen);
        s.Spacer(kGapW);
        s.Cell(tail, kTailW - kGapW, kBody, &kWhite);
        s.Break();
        s.Spacer(kValueX);
        at = 0;
        for (int i = 0; i < 5; i++)
        {
            const float to = ladder[i] / 100.0f;
            s.Cell(Fmt("%d", ladder[i]), barW * (to - at), kSmallMono, &kDim, HARight);
            at = to;
        }
        s.Break();
    }
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
        s.KV("Occupier steps up at", "", "", tiers + RString(" - better troops, heavier vehicles, sharper eyes"),
             nullptr, &kMuted);
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
        html->SetHanging(kValueX * s.PageW());
        s.Cell("Towns", kLabelW, kBody, &kMuted);
        s.Spacer(kDigitW + kUnitW + kGapW);
        s.Text(Fmt("%d risen", risen), kBody, &kGreen);
        s.Text(Fmt(" - %d rising", rising), kBody, rising > 0 ? &kGreen : &kMuted);
        s.Text(Fmt(" - %d neutral - %d occupied", neutral, occupied), kBody, &kMuted);
        s.Text(Fmt(" - %d unscouted", unscouted), kBody, &kMuted);
        s.Break();
        s.Cell("Bases", kLabelW, kBody, &kMuted);
        s.Spacer(kDigitW + kUnitW + kGapW);
        s.Text(Fmt("%d held", held), kBody, &kGreen);
        s.Text(Fmt(" - %d contested", contested), kBody, contested > 0 ? &kGreen : &kMuted);
        s.Text(Fmt(" - %d occupied", occBases), kBody, &kMuted);
        s.Break();
        html->SetHanging(0);
    }
    {
        RString garrisons;
        int shown = 0;
        for (int i = 0; i < in.zones.Size() && shown < 4; i++)
        {
            const JournalZoneRow& z = in.zones[i];
            if (z.revealed && z.holder != 0 && z.garrison > 0)
            {
                garrisons = garrisons + (shown ? RString(", ") : RString("known: ")) + z.name + Fmt(" %d", z.garrison);
                shown++;
            }
        }
        s.KV("Occupier under arms", Num((float)d.knownGarrison), "", garrisons, nullptr, &kMuted);
        RString ours = Fmt("%d with you, %d holding", d.withPlayer, d.holding);
        if (in.economyKnown)
        {
            ours = ours + Fmt("; %d HR in reserve", toInt(in.manpower));
        }
        s.KV("Ours under arms", Num((float)(d.withPlayer + d.holding)), "", ours, nullptr, &kMuted);
        // heat across the ground we hold, and what the cell owns there
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
            PackedColor c = HeatColor(mean);
            s.KVBar("Heat on our ground", Num(mean), "mean", mean / 100.0f, c, Fmt("%d zones", heldZones), -1, &c);
        }
    }

    // ---- organisation (faction-management stubs read script globals)
    s.Head("Organisation");
    RString cell = in.hqEstablished ? RString("yours - HQ ") + in.hqZone : RString("yours - no HQ yet");
    s.KV("Cells", Num((float)(1 + in.faction.alliedCells)), "", cell);
    {
        RString holdings;
        if (in.stashCount > 0)
        {
            holdings = holdings + Fmt("%s%d %s", holdings.GetLength() ? " - " : "", in.stashCount,
                                      in.stashCount == 1 ? "cache" : "caches");
        }
        if (in.garageCount > 0)
        {
            holdings = holdings + Fmt("%s%d garaged", holdings.GetLength() ? " - " : "", in.garageCount);
        }
        if (holdings.GetLength() > 0)
        {
            s.KV("Holdings", "", "", holdings);
        }
    }
    if (in.faction.doctrine.GetLength() > 0)
    {
        s.KV("Doctrine", "", "", in.faction.doctrine);
    }
    if (in.faction.outsideSupport.GetLength() > 0)
    {
        s.KV("Outside support", "", "", in.faction.outsideSupport);
    }
    if (in.faction.alliedCells == 0 && in.faction.outsideSupport.GetLength() == 0)
    {
        s.Para("No allied cells. No outside contact.", kBody, &kDim);
    }
    s.Nav("Resistance");
}

void BuildDiary(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in, const Derived& d)
{
    Sheet s(html, "GM_LOG");
    Masthead(s, in, d, "Diary", Fmt("%d entries", journal.EntryCount()));
    if (journal.EntryCount() == 0)
    {
        s.Para("Nothing recorded yet.", kBody, &kMuted);
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
        DiaryLine(s, e, day, false);
    }
    s.Nav("Diary");
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
            s.Cell(text, widths[i], kSmallMono, &kMuted);
            continue;
        }
        const PackedColor* color = i == 0 ? &kWhite : &kMuted;
        HTMLFormat f = kBody;
        if (text.GetLength() >= 2 && text[0] == '~')
        {
            switch (text[1])
            {
                case 'g':
                    color = &kGreen;
                    break;
                case 'y':
                    color = &kYellow;
                    break;
                case 'r':
                    color = &kRed;
                    break;
                default:
                    color = &kWhite;
                    break;
            }
            f = kMono;
            text = text.Substring(2, INT_MAX);
        }
        if (i == n - 1 || i == 2)
        {
            // the last cell wraps under itself
            s.Text(text, f, color);
        }
        else
        {
            s.Cell(text, widths[i], f, color);
        }
    }
    s.Break();
    if (header)
    {
        s.Rule(kRule, 0.05f);
    }
}

void BuildHandbook(CHTMLContainer* html, const Journal& /*journal*/, const JournalPageInputs& in, const Derived& d)
{
    // index
    {
        Sheet s(html, "GM_MAN_INDEX");
        Masthead(s, in, d, "Handbook", "notes from the old hands");
        for (int t = 0; t < kManualCount; t++)
        {
            s.Cell(Fmt("%d", t + 1), 0.06f, kMono, &kDim, HARight);
            s.Spacer(0.02f);
            s.Text(RString(kManual[t].title), kBody, nullptr, 0, HALeft, (RString("#") + RString(kManual[t].anchor)));
            s.Text(RString("  ") + RString(kManual[t].subtitle), kSmall, &kMuted);
            s.Break();
        }
        s.Nav("Handbook");
    }
    // chapters
    for (int t = 0; t < kManualCount; t++)
    {
        const ManualTopic& topic = kManual[t];
        Sheet s(html, topic.anchor);
        Masthead(s, in, d, topic.title, Fmt("Handbook %d / %d", t + 1, kManualCount));
        s.Para(RString(topic.subtitle), kSmall, &kMuted);
        s.Gap(0.3f);
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
                    RString head = Fmt("Your standing - now %s", word);
                    if (in.undercoverStatus == 2)
                    {
                        head = head + Fmt(" - %d %s your face", in.undercoverWitnesses,
                                          in.undercoverWitnesses == 1 ? "patrol knows" : "patrols know");
                    }
                    s.Head(head);
                }
            }
            else
            {
                s.Para(RString(line));
                s.Gap(0.3f);
            }
        }
        // chapter nav
        s.Gap(0.3f);
        if (t > 0)
        {
            s.Text(RString("< ") + RString(kManual[t - 1].title), kSmallMono, nullptr, 0, HALeft,
                   RString("#") + RString(kManual[t - 1].anchor));
        }
        if (t + 1 < kManualCount)
        {
            s.Text(t > 0 ? "   |   " : "", kSmallMono, &kDim);
            s.Text(RString(kManual[t + 1].title) + RString(" >"), kSmallMono, nullptr, 0, HALeft,
                   RString("#") + RString(kManual[t + 1].anchor));
        }
        s.Text("   |   ", kSmallMono, &kDim);
        s.Text("Index", kSmallMono, nullptr, 0, HALeft, "#GM_MAN_INDEX");
        s.Break();
        s.Nav("Handbook");
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

void ApplyGuerrillaJournalTheme(CHTMLContainer* html)
{
    if (!html)
    {
        return;
    }
    html->SetBgColor(kBg);
    html->SetTextColor(kText);
    html->SetLinkColor(kLink);
    if (!GEngine)
    {
        return; // parser-only container: colours suffice
    }
    Font* title = GEngine->LoadFont(GetFontID("cwrtitle"));
    Font* body = GEngine->LoadFont(GetFontID("cwrbody"));
    Font* mono = GEngine->LoadFont(GetFontID("cwrmono"));
    const float p = html->GetPHeight();
    if (body)
    {
        html->SetFormatFont(kBody, body, body, p);
        html->SetFormatFont(kSmall, body, body, 0.85f * p);
    }
    if (title)
    {
        html->SetFormatFont(kTitle, title, title, 2.1f * p);
        html->SetFormatFont(kStrip, title, title, 1.35f * p);
        html->SetFormatFont(kSection, title, title, 1.0f * p);
    }
    if (mono)
    {
        html->SetFormatFont(kMono, mono, mono, 0.95f * p);
        html->SetFormatFont(kSmallMono, mono, mono, 0.82f * p);
    }
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
    const Derived d = Derive(in);
    BuildSituation(html, journal, in, d);
    BuildPlan(html, journal, in, d);
    BuildZones(html, journal, in, d);
    BuildCell(html, journal, in, d);
    BuildResistance(html, journal, in, d);
    BuildDiary(html, journal, in, d);
    BuildHandbook(html, journal, in, d);
}

} // namespace Poseidon::Guerrilla
