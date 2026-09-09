#pragma once

// Guerrilla Mode journal: the prose and formatting helpers shared by the
// Compose stage (JournalCompose*.cpp) and the Gather half
// (GuerrillaJournalPages.cpp).  Pure functions over JournalPageInputs and
// plain values; nothing here touches the document model, a singleton or the
// world.  Moved verbatim out of GuerrillaJournalPages.cpp so the two stages
// share one copy.

#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>

namespace Poseidon::Guerrilla::JournalText
{
inline const char* cstr(const RString& s)
{
    return (const char*)s;
}
RString Num(float v);
RString Fmt(const char* format, ...);
RString Words(int n);
RString Cap(const RString& s);
RString Sentence(const RString& s);
RString RankShort(int rank);
const char* TypeWord(const RString& type);
RString Km(float meters);
RString Range(const JournalZoneRow& z);
RString Seen(int day, int minute, int today);
RString CompactStamp(const RString& stamp, int today);
int StampDay(const RString& stamp);
const char* HeatWord(float heat);
const char* AlertName(int state);
struct ZoneState
{
    const char* word;
    int group; // 0 ours / 1 contested / 2 neutral / 3 occupied / 4 unscouted
};
ZoneState StateOf(const JournalZoneRow& z, float supportFlip);

// derived facts shared by several pages
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
bool Nearer(const JournalZoneRow* a, const JournalZoneRow* b);
Derived Derive(const JournalPageInputs& in);
RString JoinNames(const AutoArray<RString>& names, int limit = 4);
RString DateLine(const JournalPageInputs& in);
RString CampaignLine(const JournalPageInputs& in);
RString Standing(const JournalPageInputs& in, const RString& stat);
RString ZoneAnchor(int index); // "GM_ZONE_%d"
RString ZoneBrief(const JournalZoneRow& z, float supportFlip);
int WordCount(const RString& s); // whitespace-separated tokens
// `text` when it is within maxWords, else its first maxWords - 1 words plus
// "..." (the hand caps: script-authored objectives and entries)
RString ClampWords(const RString& text, int maxWords);
} // namespace Poseidon::Guerrilla::JournalText
