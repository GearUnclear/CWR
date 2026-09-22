#include <Poseidon/UI/Guerrilla/JournalText.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/platform.hpp>       // stricmp

#include <cstdarg> // va_list (Fmt)
#include <cstdio>
#include <cstring>

namespace Poseidon::Guerrilla::JournalText
{

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

namespace
{
const char* kRankShort[] = {"Pvt", "Cpl", "Sgt", "Lt", "Cpt", "Maj", "Col"};
} // namespace

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
// derived facts shared by several pages
// ===========================================================================

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

RString JoinNames(const AutoArray<RString>& names, int limit)
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

// whitespace-separated tokens (the Compose word caps count these)
int WordCount(const RString& s)
{
    int count = 0;
    bool inWord = false;
    for (const char* p = cstr(s); p && *p; p++)
    {
        const bool space = *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r';
        if (!space && !inWord)
        {
            count++;
        }
        inWord = !space;
    }
    return count;
}

// the hand caps: text within the cap is kept whole; over it, the first
// maxWords - 1 words plus "..." (at least one word always survives)
RString ClampWords(const RString& text, int maxWords)
{
    if (maxWords < 2)
    {
        maxWords = 2;
    }
    if (WordCount(text) <= maxWords)
    {
        return text;
    }
    const char* s = cstr(text);
    int words = 0;
    bool inWord = false;
    int cut = 0;
    for (int i = 0; s[i]; i++)
    {
        const bool space = s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r';
        if (!space && !inWord)
        {
            if (words == maxWords - 1)
            {
                cut = i;
                break;
            }
            words++;
        }
        inWord = !space;
    }
    RString head = text.Substring(0, cut);
    while (head.GetLength() > 0 && head[head.GetLength() - 1] == ' ')
    {
        head = head.Substring(0, head.GetLength() - 1);
    }
    return head + RString("...");
}

} // namespace Poseidon::Guerrilla::JournalText
