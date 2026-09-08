// The issue #57 name bank and the five-slot assembler.
//
// The bank is 872 strings. Per-pool counts cannot police a transcription: a
// transposition that duplicates one entry while dropping another keeps every
// count correct. So this suite checks CONTENT three ways - an order-sensitive
// checksum per list against the committed fixture, the head and tail of every
// list, and a within-list uniqueness lint - on top of the counts.
//
// The expected values are generated from
// tests/fixtures/legend-names/issue57-names.json by
// tools/legend-names/gen_legend_names.py; that script also re-verifies the
// compiled tables against the fixture with --check.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/LegendNames.hpp>
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <set>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// BEGIN GENERATED BANK CHECKSUMS
// Per-pool FNV-1a 64 checksums of the issue #57 fixture, in order, plus the
// head and tail of every list. Regenerate with tools/legend-names/gen_legend_names.py.
struct ExpectedPool
{
    const char* region;
    int nFirst, nLast;
    unsigned long long firstSum, lastSum;
    const char* firstHead;
    const char* firstTail;
    const char* lastHead;
    const char* lastTail;
};
const ExpectedPool kExpectedPools[] = {
    {"west_africa", 10, 10, 0xB7218A15F0F490DCULL, 0xAA8318125713FD03ULL, "Adebayo", "Babacar", "Okafor", "Toure"},
    {"east_africa", 10, 10, 0x8AA6C0F684378D98ULL, 0x4CC2623EDF168563ULL, "Jabari", "Ismail", "Mwangi", "Nur"},
    {"southern_africa", 10, 10, 0x51CD17A3BD16F1B9ULL, 0xC5DB987EEF75B7C8ULL, "Thabo", "Mandla", "Mokoena", "Sithole"},
    {"north_africa", 10, 10, 0x4FD4FCFA536013F2ULL, 0xFC14F4066B19E01AULL, "Yassine", "Samir", "El Mansouri",
     "Trabelsi"},
    {"levant", 12, 12, 0xD4C83F0D2A298867ULL, 0xC1AB550A8867E5D4ULL, "Laith", "Faris", "Haddad", "Saleh"},
    {"palestine", 20, 20, 0xF9152F68853A6496ULL, 0xA8150E486C74B42CULL, "Yazan", "Marwan", "Barghouti", "Mansour"},
    {"lebanon", 20, 20, 0x4FE4F94679F0EAA0ULL, 0x2D36259567B099C3ULL, "Jad", "Michel", "Khoury", "Dagher"},
    {"ireland", 20, 20, 0x5326503909108CECULL, 0xDC1A8ABF70EA0A0FULL, "Cian", "Lorcan", "Murphy", "Kavanagh"},
    {"iraq", 10, 10, 0xF5DA114B26B61ADAULL, 0xC896D636B37C6C90ULL, "Haydar", "Karrar", "Al-Khafaji", "Al-Tamimi"},
    {"arabian_peninsula", 10, 10, 0xBF62D17CA7C6F07DULL, 0x93194E309324A4B0ULL, "Fahad", "Muadh", "Al-Qahtani",
     "Al-Qadhi"},
    {"iran", 10, 10, 0xB3331FC792FABAFBULL, 0xBD4C0723B2DAC013ULL, "Arash", "Omid", "Farhadi", "Kazemi"},
    {"kurdish", 10, 10, 0x80F5D86CA0D58E04ULL, 0xFA6E14BC907D42A9ULL, "Baran", "Sherko", "Karaman", "Salih"},
    {"turkey", 10, 10, 0x90BEB254CF526D5CULL, 0xACDF98C089A528F1ULL, "Emre", "Kaan", "Yilmaz", "Kurt"},
    {"south_asia", 10, 10, 0xF8AAB9A3EB5A94CBULL, 0x8E659FDF7BB3FCFAULL, "Arjun", "Imran", "Mehta", "Qureshi"},
    {"himalayan", 10, 10, 0x847831A22A21ED17ULL, 0x1691E272767512BCULL, "Prabhat", "Tshering", "Gurung", "Bhutia"},
    {"china", 10, 10, 0xA98902FA8F74BE28ULL, 0x46B4BB9E2FFCFF2DULL, "Wei", "Cheng", "Zhang", "Sun"},
    {"korea", 10, 10, 0xCF1059D211672016ULL, 0x1E07C6D621CA8A1CULL, "Minho", "Sungmin", "Kim", "Han"},
    {"japan", 10, 10, 0xB9E97E315FAD3072ULL, 0x1EDBA9E6E1BA533DULL, "Haruto", "Akira", "Nakamura", "Watanabe"},
    {"mainland_southeast_asia", 10, 10, 0x0F4327E451DF3026ULL, 0x42F9EBE16AD41A87ULL, "Minh", "Chanthou", "Nguyen",
     "Seng"},
    {"maritime_southeast_asia", 10, 10, 0x4CC8BF6AD27839A3ULL, 0xB059632DE5EF2A43ULL, "Arif", "Isagani", "Santoso",
     "Reyes"},
    {"central_asia", 10, 10, 0x052F7BAC04160899ULL, 0xD0A5E2AADA72D51FULL, "Temur", "Bakhrom", "Karimov", "Mirzaev"},
    {"mongolia", 10, 10, 0xB38D2F1A1922CED8ULL, 0x1A0A4EE2CF3681BFULL, "Baatar", "Batbold", "Erdene", "Sukhbaatar"},
    {"caucasus", 10, 10, 0x1595785A2856518BULL, 0xBA6207F7B712DB91ULL, "Levan", "Hayk", "Beridze", "Aslanov"},
    {"balkans", 10, 10, 0xD674150FE33FA8CCULL, 0x527A6775B93A74A2ULL, "Milan", "Andrei", "Jovanovic", "Ionescu"},
    {"eastern_europe", 10, 10, 0x1B8F72D59410A2E6ULL, 0x8C20127F4E0ED0B0ULL, "Dimitri", "Yaroslav", "Petrov", "Melnyk"},
    {"latin_america", 10, 10, 0x3A6BC9C9C74A57F7ULL, 0x977E7DDB6DE7C1AAULL, "Diego", "Santiago", "Quispe", "Castillo"},
    {"caribbean", 10, 10, 0x8C47E271C380EA11ULL, 0x50C4FC0FEDCE1541ULL, "Jean-Baptiste", "Kofi", "Pierre", "Augustin"},
    {"indigenous_americas", 10, 10, 0x557213DE306A05D3ULL, 0x4882648A2505399BULL, "Balam", "Tasunka", "Cocom",
     "Yellowbird"},
    {"polynesia", 10, 10, 0x39EF002D5182C4F8ULL, 0xE0FBDFD82A030DF3ULL, "Tane", "Sio", "Raukawa", "Tupua"},
    {"melanesia", 10, 10, 0x8BB239E0DB259554ULL, 0xC39EBC9A59FC7317ULL, "Jone", "Sakiusa", "Nabua", "Matawalu"},
    {"western", 24, 24, 0x8929D9001B2D3818ULL, 0x39F441C3FA1D1D7FULL, "Tyler", "Jordan", "Miller", "Stone"},
    {"british", 20, 20, 0x595E6DD16CED74C5ULL, 0x45F0B406DC94F139ULL, "Oliver", "Nigel", "Harrington", "Fairfax"},
    {"israeli", 20, 20, 0x9D053D9350AE3A22ULL, 0x603BF3414A5ACB44ULL, "Noam", "Nir", "Cohen", "Ben-Ami"},
};
struct ExpectedBank
{
    int nPrefix, nDescriber, nTitle;
    unsigned long long prefixSum, describerSum, titleSum;
};
const ExpectedBank kExpectedBanks[2] = {
    {14, 13, 13, 0x4AE475CF2A7FE603ULL, 0x2F083CA907E27AA0ULL, 0x02B5A7D11D6992F8ULL},
    {10, 20, 10, 0x95828664A302AC05ULL, 0xCE487D0D82F367E2ULL, 0xC58AF0A59BA44BDEULL},
};
// END GENERATED BANK CHECKSUMS

// The identical loop gen_legend_names.py runs over the fixture: every string
// in order, each followed by a newline. Order-sensitive, so swapping two
// entries inside one pool changes it.
unsigned long long Fnv1aList(const char* const* items, int n)
{
    unsigned long long h = 0xCBF29CE484222325ULL;
    for (int i = 0; i < n; i++)
    {
        for (const unsigned char* p = (const unsigned char*)items[i]; *p != 0; p++)
        {
            h = (h ^ (unsigned long long)*p) * 0x100000001B3ULL;
        }
        h = (h ^ (unsigned long long)'\n') * 0x100000001B3ULL;
    }
    return h;
}

// Every lint a shipped table string must pass. Player-facing: it can reach a
// marker label and the notepad.
void LintTableString(const char* s, const char* where)
{
    INFO(where << ": " << (s ? s : "<null>"));
    REQUIRE(s != nullptr);
    const int len = (int)strlen(s);
    REQUIRE(len > 0);
    REQUIRE(s[0] != '@');
    REQUIRE(s[0] != '$');
    REQUIRE(s[0] != ' ');
    REQUIRE(s[len - 1] != ' ');
    REQUIRE(strstr(s, "--") == nullptr);
    REQUIRE(strstr(s, "  ") == nullptr);
    for (int i = 0; i < len; i++)
    {
        const unsigned char c = (unsigned char)s[i];
        REQUIRE(c < 0x80); // ASCII only, which also excludes the em dash bytes
        REQUIRE(c != '\n');
        REQUIRE(c != '\r');
        REQUIRE(c != '\t');
        REQUIRE(c != '"');
    }
}

void LintUniqueList(const char* const* items, int n, const char* where)
{
    std::set<std::string> seen;
    for (int i = 0; i < n; i++)
    {
        LintTableString(items[i], where);
        INFO(where << " duplicate: " << items[i]);
        REQUIRE(seen.insert(std::string(items[i])).second);
    }
}

// Same lints on an assembled display name, minus the quote rule: the
// describer treatment adds ASCII double quotes on purpose.
void LintDisplayName(const RString& name)
{
    const char* s = name.Data();
    INFO("assembled: " << s);
    const int len = (int)strlen(s);
    REQUIRE(len > 0);
    REQUIRE(len <= 96);
    REQUIRE(s[0] != '@');
    REQUIRE(s[0] != '$');
    REQUIRE(s[0] != ' ');
    REQUIRE(s[len - 1] != ' ');
    REQUIRE(strstr(s, "  ") == nullptr);
    REQUIRE(strstr(s, "--") == nullptr);
    for (int i = 0; i < len; i++)
    {
        const unsigned char c = (unsigned char)s[i];
        REQUIRE(c < 0x80);
        REQUIRE(c >= 0x20);
    }
}

const char* Longest(const char* const* items, int n)
{
    const char* best = "";
    for (int i = 0; i < n; i++)
    {
        if (strlen(items[i]) > strlen(best))
        {
            best = items[i];
        }
    }
    return best;
}

NameParts Parts(const char* prefix, const char* first, const char* describer, const char* last, const char* title)
{
    NameParts parts;
    parts.prefix = RString(prefix);
    parts.first = RString(first);
    parts.describer = RString(describer);
    parts.last = RString(last);
    parts.title = RString(title);
    return parts;
}

} // namespace

TEST_CASE("Legend names - the compiled tables carry the whole issue #57 bank", "[game][guerrilla][legends][names]")
{
    REQUIRE(NamePoolCount() == 33);
    REQUIRE(NamePoolCount() == (int)(sizeof(kExpectedPools) / sizeof(kExpectedPools[0])));

    int totalFirst = 0;
    int totalLast = 0;
    for (int i = 0; i < NamePoolCount(); i++)
    {
        const NamePool& pool = NamePoolAt(i);
        const ExpectedPool& want = kExpectedPools[i];
        INFO("pool " << i << " " << pool.region << " (expected " << want.region << ")");
        REQUIRE(strcmp(pool.region, want.region) == 0);
        REQUIRE(pool.nFirst == want.nFirst);
        REQUIRE(pool.nLast == want.nLast);
        // the transposition guard: order-sensitive checksums against the fixture
        REQUIRE(Fnv1aList(pool.first, pool.nFirst) == want.firstSum);
        REQUIRE(Fnv1aList(pool.last, pool.nLast) == want.lastSum);
        // plus the cheap human-readable spot check on each end of each list
        REQUIRE(strcmp(pool.first[0], want.firstHead) == 0);
        REQUIRE(strcmp(pool.first[pool.nFirst - 1], want.firstTail) == 0);
        REQUIRE(strcmp(pool.last[0], want.lastHead) == 0);
        REQUIRE(strcmp(pool.last[pool.nLast - 1], want.lastTail) == 0);
        totalFirst += pool.nFirst;
        totalLast += pool.nLast;
    }
    REQUIRE(totalFirst == 396);
    REQUIRE(totalLast == 396);

    // the per-pool shape the plan spells out, checked independently of the
    // generated block so a regenerated fixture cannot quietly reshape the bank
    REQUIRE(NamePoolAt(FindNamePool("levant")).nFirst == 12);
    REQUIRE(NamePoolAt(FindNamePool("levant")).nLast == 12);
    const char* twenties[] = {"palestine", "lebanon", "ireland", "british", "israeli"};
    for (const char* region : twenties)
    {
        INFO(region);
        REQUIRE(NamePoolAt(FindNamePool(region)).nFirst == 20);
        REQUIRE(NamePoolAt(FindNamePool(region)).nLast == 20);
    }
    REQUIRE(NamePoolAt(FindNamePool("western")).nFirst == 24);
    REQUIRE(NamePoolAt(FindNamePool("western")).nLast == 24);
    for (int i = 0; i < NamePoolCount(); i++)
    {
        const NamePool& pool = NamePoolAt(i);
        const bool sized = pool.nFirst == 10 || pool.nFirst == 12 || pool.nFirst == 20 || pool.nFirst == 24;
        INFO(pool.region << " nFirst " << pool.nFirst);
        REQUIRE(sized);
        REQUIRE(pool.nFirst == pool.nLast);
    }

    const NicknameBank& friendly = Bank(ToneFriendly);
    REQUIRE(friendly.nPrefix == 14);
    REQUIRE(friendly.nDescriber == 13);
    REQUIRE(friendly.nTitle == 13);
    REQUIRE(Fnv1aList(friendly.prefix, friendly.nPrefix) == kExpectedBanks[0].prefixSum);
    REQUIRE(Fnv1aList(friendly.describer, friendly.nDescriber) == kExpectedBanks[0].describerSum);
    REQUIRE(Fnv1aList(friendly.title, friendly.nTitle) == kExpectedBanks[0].titleSum);

    const NicknameBank& hostile = Bank(ToneHostile);
    REQUIRE(hostile.nPrefix == 10);
    REQUIRE(hostile.nDescriber == 20);
    REQUIRE(hostile.nTitle == 10);
    REQUIRE(Fnv1aList(hostile.prefix, hostile.nPrefix) == kExpectedBanks[1].prefixSum);
    REQUIRE(Fnv1aList(hostile.describer, hostile.nDescriber) == kExpectedBanks[1].describerSum);
    REQUIRE(Fnv1aList(hostile.title, hostile.nTitle) == kExpectedBanks[1].titleSum);
}

TEST_CASE("Legend names - every table string is ASCII, sigil-free and unique in its list",
          "[game][guerrilla][legends][names]")
{
    std::set<std::string> regions;
    for (int i = 0; i < NamePoolCount(); i++)
    {
        const NamePool& pool = NamePoolAt(i);
        INFO(pool.region);
        // region tokens: ^[a-z][a-z_]*$ and unique
        const int len = (int)strlen(pool.region);
        REQUIRE(len > 0);
        REQUIRE(pool.region[0] >= 'a');
        REQUIRE(pool.region[0] <= 'z');
        for (int c = 0; c < len; c++)
        {
            const char ch = pool.region[c];
            REQUIRE(((ch >= 'a' && ch <= 'z') || ch == '_'));
        }
        REQUIRE(regions.insert(std::string(pool.region)).second);
        LintUniqueList(pool.first, pool.nFirst, pool.region);
        LintUniqueList(pool.last, pool.nLast, pool.region);
    }
    for (int t = 0; t < 2; t++)
    {
        const NicknameBank& bank = Bank(t == 0 ? ToneFriendly : ToneHostile);
        LintUniqueList(bank.prefix, bank.nPrefix, "prefix");
        LintUniqueList(bank.describer, bank.nDescriber, "describer");
        LintUniqueList(bank.title, bank.nTitle, "title");
    }
}

TEST_CASE("Legend names - region lookup is exact and case-insensitive", "[game][guerrilla][legends][names]")
{
    REQUIRE(FindNamePool("levant") >= 0);
    REQUIRE(FindNamePool("LEVANT") == FindNamePool("levant"));
    REQUIRE(FindNamePool("LeVaNt") == FindNamePool("levant"));
    REQUIRE(FindNamePool("levantine") == -1); // not a prefix match
    REQUIRE(FindNamePool("lev") == -1);
    REQUIRE(FindNamePool("") == -1);
    REQUIRE(FindNamePool(nullptr) == -1);
    REQUIRE(FindNamePool("west_africa") == 0); // config order, not alphabetical
    REQUIRE(NamePoolAt(-1).nFirst == 0);       // out of range is empty, never a crash
    REQUIRE(NamePoolAt(NamePoolCount()).nLast == 0);
}

TEST_CASE("Legend names - the five-slot assembler", "[game][guerrilla][legends][names]")
{
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "", "", ""))) == "Petra");
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "", "Kovacevic", ""))) == "Petra Kovacevic");
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "The Hawk", "Kovacevic", ""))) ==
            "Petra \"The Hawk\" Kovacevic");
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "The Hawk", "Kovacevic", "The Unbroken"))) ==
            "Petra \"The Hawk\" Kovacevic The Unbroken");
    // the practical maximum: a hostile five-slot boss
    REQUIRE(Str(AssembleDisplayName(Parts("Cold", "Erez", "The Land-Taker", "Dayan", "The Occupier"))) ==
            "Cold Erez \"The Land-Taker\" Dayan The Occupier");

    // Both branches below are unreachable for the shipped bank - every one of
    // the 46 describers and titles begins with "The " - so they pin the
    // behaviour for a future or modded word, not shipped coverage.
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "Hawkeye", "Kovacevic", ""))) == "Petra Hawkeye Kovacevic");
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra", "", "Kovacevic", "Unbroken"))) == "Petra Kovacevic Unbroken");

    // leading-sigil guard: Localize would blank a marker label starting with
    // '@' or '$', so the offending leading slot is dropped and the assembly
    // retried
    const RString atName = AssembleDisplayName(Parts("@Bad", "Petra", "", "Kovacevic", ""));
    REQUIRE(Str(atName) == "Petra Kovacevic");
    const RString dollarName = AssembleDisplayName(Parts("$Bad", "Petra", "", "Kovacevic", ""));
    REQUIRE(Str(dollarName) == "Petra Kovacevic");
    REQUIRE(Str(AssembleDisplayName(Parts("@A", "@B", "", "", ""))).empty());
    REQUIRE(Str(AssembleDisplayName(Parts("", "", "", "", ""))).empty());

    // whitespace never reaches the display name
    REQUIRE(Str(AssembleDisplayName(Parts("", "  Petra  ", "", "Kovacevic", ""))) == "Petra Kovacevic");
    REQUIRE(Str(AssembleDisplayName(Parts("", "Petra   Marie", "", "Kovacevic", ""))) == "Petra Marie Kovacevic");
}

TEST_CASE("Legend names - every assembled name passes the player-text lints", "[game][guerrilla][legends][names]")
{
    int assembled = 0;
    for (int p = 0; p < NamePoolCount(); p++)
    {
        const NamePool& pool = NamePoolAt(p);
        const int firstIdx[2] = {0, pool.nFirst - 1};
        const int lastIdx[2] = {0, pool.nLast - 1};
        for (int fi = 0; fi < 2; fi++)
        {
            for (int li = 0; li < 2; li++)
            {
                for (int t = 0; t < 2; t++)
                {
                    const NicknameTone tone = t == 0 ? ToneFriendly : ToneHostile;
                    const NicknameBank& bank = Bank(tone);
                    const int slots[3] = {SlotPrefix, SlotDescriber, SlotTitle};
                    const int counts[3] = {bank.nPrefix, bank.nDescriber, bank.nTitle};
                    const char* const* words[3] = {bank.prefix, bank.describer, bank.title};
                    for (int s = 0; s < 3; s++)
                    {
                        for (int w = 0; w < counts[s]; w++)
                        {
                            NameParts parts;
                            parts.first = RString(pool.first[firstIdx[fi]]);
                            parts.last = RString(pool.last[lastIdx[li]]);
                            if (slots[s] == SlotPrefix)
                            {
                                parts.prefix = RString(words[s][w]);
                            }
                            else if (slots[s] == SlotDescriber)
                            {
                                parts.describer = RString(words[s][w]);
                            }
                            else
                            {
                                parts.title = RString(words[s][w]);
                            }
                            LintDisplayName(AssembleDisplayName(parts));
                            assembled++;
                        }
                    }
                }
            }
        }
    }
    REQUIRE(assembled > 5000);

    // and the full five-slot shape, which is what a Legend at Colonel wears
    for (int p = 0; p < NamePoolCount(); p++)
    {
        const NamePool& pool = NamePoolAt(p);
        for (int t = 0; t < 2; t++)
        {
            const NicknameTone tone = t == 0 ? ToneFriendly : ToneHostile;
            const NicknameBank& bank = Bank(tone);
            for (int w = 0; w < bank.nDescriber; w++)
            {
                NameParts parts;
                parts.prefix = RString(bank.prefix[w % bank.nPrefix]);
                parts.first = RString(pool.first[w % pool.nFirst]);
                parts.describer = RString(bank.describer[w]);
                parts.last = RString(pool.last[w % pool.nLast]);
                parts.title = RString(bank.title[w % bank.nTitle]);
                LintDisplayName(AssembleDisplayName(parts));
            }
        }
    }

    // The exact worst case, which the sampled matrix above cannot reach: the
    // longest entry of every list, in every pool, in both tones. This is the
    // number a page budget has to survive, so it is worth stating.
    int longest = 0;
    for (int p = 0; p < NamePoolCount(); p++)
    {
        const NamePool& pool = NamePoolAt(p);
        for (int t = 0; t < 2; t++)
        {
            const NicknameBank& bank = Bank(t == 0 ? ToneFriendly : ToneHostile);
            NameParts parts;
            parts.prefix = RString(Longest(bank.prefix, bank.nPrefix));
            parts.first = RString(Longest(pool.first, pool.nFirst));
            parts.describer = RString(Longest(bank.describer, bank.nDescriber));
            parts.last = RString(Longest(pool.last, pool.nLast));
            parts.title = RString(Longest(bank.title, bank.nTitle));
            const RString name = AssembleDisplayName(parts);
            LintDisplayName(name);
            longest = longest > name.GetLength() ? longest : name.GetLength();
        }
    }
    INFO("longest assemblable name " << longest);
    REQUIRE(longest >= 60); // the bank really does produce long names
    REQUIRE(longest <= 96);
}

TEST_CASE("Legend names - draws are deterministic and channel-separated", "[game][guerrilla][legends][names]")
{
    const int pool = FindNamePool("palestine"); // 20 entries: the sample below needs the headroom
    REQUIRE(pool >= 0);
    const unsigned long long key = HashKey("legend_petra_0", 12345u);

    const RString once = PickFirst(pool, key, CH_OPEN_A);
    for (int i = 0; i < 1000; i++)
    {
        REQUIRE(Str(PickFirst(pool, key, CH_OPEN_A)) == Str(once));
    }
    REQUIRE(Str(PickLast(pool, key, CH_LAST)) == Str(PickLast(pool, key, CH_LAST)));
    REQUIRE(Str(PickSlotWord(ToneHostile, SlotTitle, key, CH_BOSSWORD)) ==
            Str(PickSlotWord(ToneHostile, SlotTitle, key, CH_BOSSWORD)));

    int differ = 0;
    for (int i = 0; i < 200; i++)
    {
        char id[32];
        sprintf(id, "legend_%d", i);
        const unsigned long long rowKey = HashKey(id, 7u);
        if (Str(PickFirst(pool, rowKey, CH_BOSSFIRST)) != Str(PickFirst(pool, rowKey, CH_OPEN_A)))
        {
            differ++;
        }
    }
    REQUIRE(differ >= 180); // 90 % of the sample

    REQUIRE(Roll(key, CH_LAST, 0) == 0u);
    REQUIRE(Roll(key, CH_LAST, -3) == 0u);
    REQUIRE(Str(PickFirst(-1, key, CH_LAST)).empty());
    REQUIRE(Str(PickLast(NamePoolCount(), key, CH_LAST)).empty());
    // the personal-name slots are drawn from the regional pool, never the bank
    REQUIRE(Str(PickSlotWord(ToneFriendly, SlotFirst, key, CH_LAST)).empty());
    REQUIRE(Str(PickSlotWord(ToneFriendly, SlotLast, key, CH_LAST)).empty());
    REQUIRE(Str(PickSlotWord(ToneFriendly, NNameSlots, key, CH_LAST)).empty());

    // every draw a picked word makes must be inside its own table
    for (int i = 0; i < 200; i++)
    {
        char id[32];
        sprintf(id, "legend_%d", i);
        const unsigned long long rowKey = HashKey(id, 99u);
        const RString word = PickSlotWord(ToneFriendly, SlotPrefix, rowKey, CH_AWARD1WORD);
        const NicknameBank& bank = Bank(ToneFriendly);
        bool found = false;
        for (int w = 0; w < bank.nPrefix; w++)
        {
            found = found || strcmp(word, bank.prefix[w]) == 0;
        }
        REQUIRE(found);
    }
}

TEST_CASE("Legend seed - no two channels share a value", "[game][guerrilla][legends][names]")
{
    // Determinism guarantee 3: one channel per draw KIND, never reused, which
    // is what keeps a row's rolls from moving when another row appears. The
    // four award draws in particular need four separate channels - writing
    // CH_AWARD1 | CH_AWARD2 would be a bitwise or collapsing both awards onto
    // channel 41.
    const unsigned channels[] = {CH_OPEN_A,    CH_OPEN_B,   CH_OPEN_C,   CH_OPEN_D,  CH_EVENT0,     CH_EVENT1,
                                 CH_EVENT2,    CH_PLACE0,   CH_PLACE1,   CH_PLACE2,  CH_LAST,       CH_FACE,
                                 CH_BIOEVENT,  CH_BIOVAR,   CH_AWARD1,   CH_AWARD2,  CH_AWARD1WORD, CH_AWARD2WORD,
                                 CH_BOSSFIRST, CH_BOSSLAST, CH_BOSSSLOT, CH_BOSSWORD};
    const int count = (int)(sizeof(channels) / sizeof(channels[0]));
    std::set<unsigned> seen;
    for (int i = 0; i < count; i++)
    {
        INFO("channel index " << i << " value " << channels[i]);
        REQUIRE(seen.insert(channels[i]).second);
    }
    REQUIRE((unsigned)(CH_AWARD1 | CH_AWARD2) != (unsigned)CH_AWARD1);
}

TEST_CASE("Legend names - namePool resolution never fails", "[game][guerrilla][legends][names]")
{
    const int levant = FindNamePool("levant");
    const int western = FindNamePool("western");
    const int eastern = FindNamePool("eastern_europe");
    REQUIRE(levant >= 0);
    REQUIRE(western >= 0);
    REQUIRE(eastern >= 0);

    REQUIRE(ResolveNamePool("levant", "EAST", "X") == levant); // the key wins over the side
    REQUIRE(ResolveNamePool("israeli", "WEST", "IDF") == FindNamePool("israeli"));
    REQUIRE(ResolveNamePool("ISRAELI", "WEST", "IDF") == FindNamePool("israeli"));
    REQUIRE(ResolveNamePool("", "WEST", "X") == western);
    REQUIRE(ResolveNamePool("", "EAST", "X") == eastern);
    REQUIRE(ResolveNamePool("", "GUER", "X") == levant);
    REQUIRE(ResolveNamePool("", "guer", "X") == levant);
    REQUIRE(ResolveNamePool("nonsense", "GUER", "X") == levant);
    REQUIRE(ResolveNamePool("nonsense", "WEST", "X") == western);
    REQUIRE(ResolveNamePool("", "CIV", "X") == levant); // any unknown side
    REQUIRE(ResolveNamePool(nullptr, nullptr, nullptr) >= 0);
    REQUIRE(ResolveNamePool(nullptr, nullptr, nullptr) < NamePoolCount());
    REQUIRE(ResolveNamePool("", "", "") == levant);

    // the descriptor keys the plan assigns must all resolve to themselves
    const char* keys[] = {"western",      "eastern_europe", "balkans",   "israeli",
                          "north_africa", "lebanon",        "palestine", "levant"};
    for (const char* key : keys)
    {
        INFO(key);
        REQUIRE(ResolveNamePool(key, "WEST", "X") == FindNamePool(key));
    }
}
