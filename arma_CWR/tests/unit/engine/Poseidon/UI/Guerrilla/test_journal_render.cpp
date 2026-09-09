// Guerrilla journal, Render stage: the page budget (measure-then-repartition
// into "<base>_2", "<base>_3" continuation pages), the footer, legacy aliases
// attached after the format, the portrait box and the slot binding.
//
// Everything runs against JournalPageHtml, a parser-only CHTMLContainer whose
// page height is settable and whose text widths scale with the slot size, so
// the 1.45x title and the 1.6x hand wrap earlier than the typed body.  The
// container never hardcodes a slot size: Render sets them from P.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp> // NoTextures
#include <Poseidon/UI/Controls/UIControlsBase.hpp>    // CHTMLContainer (parser-only subclass)
#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>
#include <Poseidon/UI/Guerrilla/JournalCompose.hpp>
#include <Poseidon/UI/Guerrilla/JournalComposeInternal.hpp> // Pen (the keep-with-next flags as Compose sets them)
#include <Poseidon/UI/Guerrilla/JournalRender.hpp>
#include <Poseidon/UI/UITestEngine.hpp> // GetHtmlText

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// parser-only container with a settable page height and size-proportional
// text widths (half a unit per character at size 1)
class JournalPageHtml : public CHTMLContainer
{
  public:
    float pageHeight = 100000;

    void SelectSection(const char* name) { _currentSection = FindSection(name); }
    // the page read as on screen: fields joined by one space, runs collapsed
    std::string Text(const char* name)
    {
        SelectSection(name);
        if (_currentSection < 0)
        {
            return std::string();
        }
        std::string raw = UITestEngine::GetHtmlText(*this);
        std::string out;
        for (char c : raw)
        {
            if (c == ' ' && !out.empty() && out.back() == ' ')
            {
                continue;
            }
            out += c;
        }
        return out;
    }

    float GetPageWidth() const override { return 1000; }
    float GetPageHeight() const override { return pageHeight; }
    float GetTextWidth(float size, Font*, const char* text) const override
    {
        return size * (float)std::strlen(text) * 0.5f;
    }
};

// SplitSection (the safety net) decorates its pages with sipka_*.paa arrow
// links: keep the texture bank out of the parser-only test
struct NoTexturesGuard
{
    bool previous;
    NoTexturesGuard() : previous(Poseidon::NoTextures) { Poseidon::NoTextures = true; }
    ~NoTexturesGuard() { Poseidon::NoTextures = previous; }
};

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

JournalBlock TextBlock(const std::string& text, JournalVoice voice = VoiceType, float hanging = 0)
{
    JournalBlock block;
    JournalRun run;
    run.text = text.c_str();
    run.voice = voice;
    run.hanging = hanging;
    block.runs.Add(run);
    return block;
}

JournalBlock PortraitBlock(bool present)
{
    JournalBlock block;
    block.kind = BlockPortrait;
    block.portraitPresent = present;
    block.portraitSrc = present ? "\\gmcore\\portraits\\test.paa" : "";
    return block;
}

JournalPage MakePage(const char* name, const char* title, const char* parentName, const char* parentTitle)
{
    JournalPage page;
    page.name = name;
    page.baseName = name;
    page.title = title;
    page.parentName = parentName;
    page.parentTitle = parentTitle;
    return page;
}

// "block 00 xxxxxxxx..." padded to 40 characters (not BlockText: that is the
// JournalBlockKind enumerator)
std::string PaddedText(int i)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "block %02d ", i);
    std::string text = buffer;
    while (text.size() < 40)
    {
        text += 'x';
    }
    return text;
}

std::string Repeat(const char* word, int n)
{
    std::string out;
    for (int i = 0; i < n; i++)
    {
        out += word;
    }
    return out;
}

float RowSum(const HTMLSection& sec)
{
    float sum = 0;
    for (int r = 0; r < sec.rows.Size(); r++)
    {
        sum += sec.rows[r].height;
    }
    return sum;
}

// index of the first field carrying exactly `text` (-1 when absent)
int FieldIndex(const HTMLSection& sec, const char* text)
{
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (S(sec.fields[f].text) == text)
        {
            return f;
        }
    }
    return -1;
}

// the section holding a field with exactly `text` (-1 when absent)
int SectionOfText(const CHTMLContainer& html, const char* text)
{
    for (int s = 0; s < html.NSections(); s++)
    {
        if (FieldIndex(html.GetSection(s), text) >= 0)
        {
            return s;
        }
    }
    return -1;
}

// height of the row that starts at the field carrying exactly `text` (-1 when
// no row starts there)
float RowHeightOf(const HTMLSection& sec, const char* text)
{
    const int f = FieldIndex(sec, text);
    for (int r = 0; r < sec.rows.Size(); r++)
    {
        if (f >= 0 && sec.rows[r].firstField == f)
        {
            return sec.rows[r].height;
        }
    }
    return -1;
}

// true when the section's content (non-footer) fields are nothing but spacers
bool OnlySpacers(const HTMLSection& sec)
{
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        if (!fld.bottom && S(fld.text) != " " && fld.text.GetLength() > 0)
        {
            return false;
        }
    }
    return true;
}

// hrefs of the bottom-pinned (footer) fields, in order
std::vector<std::string> FooterHrefs(const HTMLSection& sec)
{
    std::vector<std::string> out;
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        if (fld.bottom && fld.href.GetLength() > 0)
        {
            out.push_back(S(fld.href));
        }
    }
    return out;
}

// text of the footer field carrying `href` ("" when absent)
std::string FooterText(const HTMLSection& sec, const char* href)
{
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        if (fld.bottom && S(fld.href) == href)
        {
            return S(fld.text);
        }
    }
    return std::string();
}

// every block's first field (the field after a break, or field 0) starts a row
bool EveryBlockStartsARow(const HTMLSection& sec)
{
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (f > 0 && !sec.fields[f - 1].nextline)
        {
            continue;
        }
        bool startsRow = false;
        for (int r = 0; r < sec.rows.Size() && !startsRow; r++)
        {
            startsRow = sec.rows[r].firstField == f;
        }
        if (!startsRow)
        {
            return false;
        }
    }
    return true;
}

int CountText(const std::string& text, const char* needle)
{
    int count = 0;
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + 1))
    {
        count++;
    }
    return count;
}

// sections carrying `name` among their names
int SectionsNamed(const CHTMLContainer& html, const char* name)
{
    int count = 0;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int n = 0; n < sec.names.Size(); n++)
        {
            if (stricmp(sec.names[n], name) == 0)
            {
                count++;
                break;
            }
        }
    }
    return count;
}

bool NamesContain(const HTMLSection& sec, const char* name)
{
    for (int n = 0; n < sec.names.Size(); n++)
    {
        if (stricmp(sec.names[n], name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool IsPencil(const PackedColor& c)
{
    return c.R8() == 50 && c.G8() == 46 && c.B8() == 42;
}

// the journal's inks, as JournalRender.cpp defines them
bool IsHandInk(const PackedColor& c)
{
    return c.R8() == 14 && c.G8() == 16 && c.B8() == 52;
}

bool IsRedInk(const PackedColor& c)
{
    return c.R8() == 150 && c.G8() == 22 && c.B8() == 18;
}

// Rec. 709 relative luminance, 0 .. 255
float Luminance(const PackedColor& c)
{
    return 0.2126f * (float)c.R8() + 0.7152f * (float)c.G8() + 0.0722f * (float)c.B8();
}

struct ImageAt
{
    int section;
    int field;
};

std::vector<ImageAt> Images(const CHTMLContainer& html)
{
    std::vector<ImageAt> out;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            if (sec.fields[f].format == HFImg)
            {
                out.push_back({s, f});
            }
        }
    }
    return out;
}

} // namespace

// ===========================================================================
// 1. the page budget and the continuation chain
// ===========================================================================

TEST_CASE("Journal render - blocks that overrun the budget move whole onto _2, _3 continuation pages",
          "[game][guerrilla][journal][render]")
{
    JournalDocument doc;
    {
        // a second-level page (parent Operations), so the footer carries the
        // parent link as well as Contents
        JournalPage page = MakePage("X", "Twelve blocks", "Plan", "Operations");
        for (int i = 0; i < 12; i++)
        {
            page.blocks.Add(TextBlock(PaddedText(i)));
        }
        doc.pages.Add(page);
    }
    JournalPageHtml html;
    const float p = html.GetPHeight();
    // budget = 10 - 3.5 = 6.5 P; the footer block takes 2 P of rows plus the
    // 2 P bar reserve pinned under them; two one-row blocks fit
    html.pageHeight = 10 * p;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    // twelve blocks, two to a page: X, X_2 .. X_6, never X/<n>
    REQUIRE(html.FindSection("X") >= 0);
    for (int part = 2; part <= 6; part++)
    {
        CHECK(html.FindSection(ContinuationName("X", part)) >= 0);
    }
    CHECK(html.FindSection("X_7") < 0);
    CHECK(html.FindSection("X/0") < 0);
    CHECK(html.FindSection("X/1") < 0);
    CHECK(html.NSections() == 6);
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        CHECK(RowSum(sec) <= html.GetPageHeight());
        CHECK(EveryBlockStartsARow(sec));
        for (int n = 0; n < sec.names.Size(); n++)
        {
            CHECK(S(sec.names[n]).find('/') == std::string::npos);
        }
    }

    // whole blocks, in order
    const std::string first = html.Text("X");
    CHECK(first.find("block 00") != std::string::npos);
    CHECK(first.find("block 01") != std::string::npos);
    CHECK(first.find("block 02") == std::string::npos);
    const std::string second = html.Text("X_2");
    CHECK(second.find("block 02") != std::string::npos);
    CHECK(second.find("block 03") != std::string::npos);
    CHECK(second.find("block 01") == std::string::npos);
    const std::string last = html.Text("X_6");
    CHECK(last.find("block 10") != std::string::npos);
    CHECK(last.find("block 11") != std::string::npos);

    // footers: Contents, the parent, prev only after page 1, next only before
    // the last; the current page is never listed
    {
        const HTMLSection& sec = html.GetSection(html.FindSection("X"));
        const std::vector<std::string> want = {"#Main", "#Plan", "#X_2"};
        CHECK(FooterHrefs(sec) == want);
        CHECK(FooterText(sec, "#Main") == "Contents");
        CHECK(FooterText(sec, "#Plan") == "Operations");
        CHECK(FooterText(sec, "#X_2") == "next");
        CHECK(first.find("prev") == std::string::npos);
    }
    {
        const HTMLSection& sec = html.GetSection(html.FindSection("X_2"));
        const std::vector<std::string> want = {"#Main", "#Plan", "#X", "#X_3"};
        CHECK(FooterHrefs(sec) == want);
        CHECK(FooterText(sec, "#X") == "prev");
        CHECK(FooterText(sec, "#X_3") == "next");
    }
    {
        const HTMLSection& sec = html.GetSection(html.FindSection("X_4"));
        const std::vector<std::string> want = {"#Main", "#Plan", "#X_3", "#X_5"};
        CHECK(FooterHrefs(sec) == want);
    }
    {
        const HTMLSection& sec = html.GetSection(html.FindSection("X_6"));
        const std::vector<std::string> want = {"#Main", "#Plan", "#X_5"};
        CHECK(FooterHrefs(sec) == want);
        CHECK(last.find("next") == std::string::npos);
        CHECK(FooterText(sec, "#Main") == "Contents");
        // the two footer rows and the bar reserve under them are bottom-pinned,
        // the content rows are not
        int bottomRows = 0;
        for (int r = 0; r < sec.rows.Size(); r++)
        {
            if (sec.rows[r].bottom)
            {
                bottomRows++;
            }
        }
        CHECK(bottomRows == 2 + kBottomBarReserveRows);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const HTMLField& fld = sec.fields[f];
            if (fld.href.GetLength() > 0 || S(fld.text) == " - ")
            {
                CHECK(fld.bottom);
            }
            if (S(fld.text).find("block ") == 0)
            {
                CHECK_FALSE(fld.bottom);
            }
        }
    }
    CHECK_FALSE(html.HasFieldColor());
}

// ===========================================================================
// 1b. the bar reserve: the footer's links clear the map screen's group bar
// ===========================================================================

TEST_CASE("Journal render - the footer's link row is lifted clear of the map screen's group bar",
          "[game][guerrilla][journal][render]")
{
    // The geometry the reserve is sized against.  A bottom-pinned block ends
    // flush with the page bottom, and at the stock notepad metrics
    // (triBriefingMetrics 0.0411, 0.2364, 0.3395, 0.6903 at scale 1, P 0.0226,
    // identical at both capture lanes) that bottom is 0.0267 INSIDE the group
    // bar, so a footer with no reserve is drawn under the squad icons.
    {
        RenderMetrics stock;
        stock.pageH = 0.6903f;
        stock.sizeP = 0.0226f;
        const float intrusion = kMapNotepadPageBottom - kMapGroupBarTop;
        CHECK(intrusion > 0);
        // the reserve lifts the link row's bottom edge above the bar ...
        const float linkRowBottom = kMapNotepadPageBottom - stock.BarReserve();
        CHECK(linkRowBottom < kMapGroupBarTop);
        // ... with clear air, not by a rounding error
        CHECK(kMapGroupBarTop - linkRowBottom >= 0.5f * stock.sizeP);
        // and it is charged: the footer block is its two rows plus the reserve
        CHECK(stock.FooterHeight() == Catch::Approx(2.0f * stock.sizeP + stock.BarReserve()));
        CHECK(stock.BarReserve() == Catch::Approx(kBottomBarReserveRows * stock.sizeP));
    }

    // and the reserve is really emitted: every page ends in that many blank
    // pinned rows, below the row that carries the links
    JournalDocument doc;
    {
        JournalPage page = MakePage("X", "Two blocks", "Plan", "Operations");
        page.blocks.Add(TextBlock(PaddedText(0)));
        page.blocks.Add(TextBlock(PaddedText(1)));
        doc.pages.Add(page);
    }
    JournalPageHtml html;
    const float p = html.GetPHeight();
    html.pageHeight = 10 * p;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    REQUIRE(html.FindSection("X") >= 0);
    CHECK(html.FindSection("X_2") < 0);
    const HTMLSection& sec = html.GetSection(html.FindSection("X"));
    REQUIRE(sec.rows.Size() > kBottomBarReserveRows + 1);
    const int linkRow = sec.rows.Size() - 1 - kBottomBarReserveRows;
    // the links live on the last row above the reserve
    bool linkRowHasHref = false;
    for (int f = sec.rows[linkRow].firstField; f <= sec.rows[linkRow].lastField && f < sec.fields.Size(); f++)
    {
        linkRowHasHref = linkRowHasHref || sec.fields[f].href.GetLength() > 0;
    }
    CHECK(linkRowHasHref);
    float reserveHeight = 0;
    for (int r = linkRow + 1; r < sec.rows.Size(); r++)
    {
        CHECK(sec.rows[r].bottom);
        reserveHeight += sec.rows[r].height;
        for (int f = sec.rows[r].firstField; f <= sec.rows[r].lastField && f < sec.fields.Size(); f++)
        {
            if (sec.fields[f].nextline)
            {
                continue; // the break that closed the row
            }
            // blank: no ink, no link, nothing to overlap
            CHECK(S(sec.fields[f].text) == " ");
            CHECK(sec.fields[f].href.GetLength() == 0);
        }
    }
    CHECK(reserveHeight == Catch::Approx(kBottomBarReserveRows * p));

    // the body can never grow into the reserve: the same budget that keeps it
    // off the footer rows charges those blank rows too
    float body = 0;
    for (int r = 0; r < sec.rows.Size(); r++)
    {
        if (!sec.rows[r].bottom)
        {
            body += sec.rows[r].height;
        }
    }
    const RenderMetrics m = MeasureContainer(html, in.uiAspect);
    CHECK(body + m.FooterHeight() <= m.Budget());
    CHECK(m.FooterHeight() == Catch::Approx(2.0f * p + kBottomBarReserveRows * p));
}

TEST_CASE("Journal render - FitBlocks keeps the largest run of leading blocks under the budget, never fewer than one",
          "[game][guerrilla][journal][render]")
{
    JournalPageHtml html;
    const int s = html.AddSection();
    html.AddName(s, "F");
    // an authored row ahead of the journal blocks, then three one-row blocks
    html.AddText(s, "authored", HFP, HALeft, false, false, "");
    html.AddBreak(s, false);
    const int firstOwnField = html.GetSection(s).fields.Size();
    AutoArray<int> starts;
    for (int b = 0; b < 3; b++)
    {
        starts.Add(html.GetSection(s).fields.Size());
        html.AddText(s, PaddedText(b).c_str(), HFP, HALeft, false, false, "");
        html.AddBreak(s, false);
    }
    starts.Add(html.GetSection(s).fields.Size());
    html.FormatSectionRows(s);
    REQUIRE(html.GetSection(s).rows.Size() == 4);

    // footer 2 + authored 1 = 3 used before the first block
    const AutoArray<bool> none;
    CHECK(FitBlocks(html, s, firstOwnField, starts, none, 10.0f, 2.0f) == 3);
    CHECK(FitBlocks(html, s, firstOwnField, starts, none, 5.5f, 2.0f) == 2);
    CHECK(FitBlocks(html, s, firstOwnField, starts, none, 4.5f, 2.0f) == 1);
    // even when the first block does not fit it stays (SplitSection is the net)
    CHECK(FitBlocks(html, s, firstOwnField, starts, none, 2.5f, 2.0f) == 1);
    // the authored rows are charged: treated as journal blocks they would not be
    AutoArray<int> all;
    all.Add(0);
    for (int i = 0; i < starts.Size(); i++)
    {
        all.Add(starts[i]);
    }
    CHECK(FitBlocks(html, s, 0, all, none, 5.5f, 2.0f) == 3); // authored + two blocks

    // keep-with-next: a cut that would end the page on block 1 walks back to
    // block 0; never below one block; never applied when nothing is cut
    AutoArray<bool> keep;
    keep.Add(false);
    keep.Add(true);
    keep.Add(false);
    CHECK(FitBlocks(html, s, firstOwnField, starts, keep, 5.5f, 2.0f) == 1);
    CHECK(FitBlocks(html, s, firstOwnField, starts, keep, 10.0f, 2.0f) == 3);
    AutoArray<bool> keepAll;
    keepAll.Add(true);
    keepAll.Add(true);
    keepAll.Add(true);
    CHECK(FitBlocks(html, s, firstOwnField, starts, keepAll, 5.5f, 2.0f) == 1);
    CHECK(FitBlocks(html, s, firstOwnField, starts, keepAll, 10.0f, 2.0f) == 3);
    // a short flag array reads false past its end
    AutoArray<bool> shortKeep;
    shortKeep.Add(true);
    CHECK(FitBlocks(html, s, firstOwnField, starts, shortKeep, 5.5f, 2.0f) == 2);
}

// ===========================================================================
// 1b. keep-with-next: a head never ends a page
// ===========================================================================

TEST_CASE("Journal render - a head and the row it introduces land on the same page, and no page opens as a bare gap",
          "[game][guerrilla][journal][render]")
{
    SECTION("the naive cut lands on the head: the head moves to the next page")
    {
        JournalDocument doc;
        {
            // written through Pen so the flags are the ones Compose sets
            JournalPage page = MakePage("K", "Kept", "Plan", "Operations");
            Pen pen(page);
            pen.Title("Kept");
            pen.Subtitle("Sub.");
            pen.Line("item a");
            pen.Head("Heading");
            pen.Line("item b");
            pen.Line("item c");
            doc.pages.Add(page);
        }
        REQUIRE(doc.pages[0].blocks.Size() == 6); // the head is ONE block (gap folded in)
        CHECK(doc.pages[0].blocks[3].leadGap);
        CHECK(doc.pages[0].blocks[3].keepWithNext);
        CHECK(doc.pages[0].blocks[0].keepWithNext);
        CHECK(doc.pages[0].blocks[1].keepWithNext);
        CHECK_FALSE(doc.pages[0].blocks[2].keepWithNext);
        CHECK_FALSE(doc.pages[0].blocks[2].leadGap);
        JournalPageHtml html;
        const float p = html.GetPHeight();
        // rows: title 1.45, subtitle 1.1, item 1, head 1 + 1.15, items 1 each;
        // footer block 4 (two rows plus the 2 P bar reserve).  Title..head sum
        // 7.7, so a budget of 10.2 (page 13.7) cuts naively after the head:
        // keep-with-next walks the cut back to item a
        html.pageHeight = 13.7f * p;
        JournalPageInputs in;
        RenderJournal(&html, doc, in);

        REQUIRE(html.FindSection("K") >= 0);
        REQUIRE(html.FindSection("K_2") >= 0);
        CHECK(html.FindSection("K_3") < 0);
        const std::string first = html.Text("K");
        CHECK(first.find("item a") != std::string::npos);
        CHECK(first.find("Heading") == std::string::npos);
        const int headAt = SectionOfText(html, "Heading");
        const int itemAt = SectionOfText(html, "item b");
        REQUIRE(headAt >= 0);
        CHECK(headAt == itemAt);
        CHECK(headAt == html.FindSection("K_2"));
        // the gap rides with the head: the continuation opens spacer, break
        // (AddBreak is a field of its own), head
        const HTMLSection& sec = html.GetSection(headAt);
        REQUIRE(sec.fields.Size() >= 4);
        CHECK(S(sec.fields[0].text) == " ");
        CHECK(sec.fields[0].format == HFP);
        CHECK(sec.fields[1].nextline);
        CHECK(S(sec.fields[2].text) == "Heading");
        CHECK(sec.fields[2].format == HFH4);
        CHECK(RowHeightOf(sec, " ") == Catch::Approx(1.0f * p));
        CHECK(RowHeightOf(sec, "Heading") == Catch::Approx(1.15f * p));
        for (int s = 0; s < html.NSections(); s++)
        {
            CHECK(RowSum(html.GetSection(s)) <= html.GetPageHeight());
            CHECK(EveryBlockStartsARow(html.GetSection(s)));
            CHECK_FALSE(OnlySpacers(html.GetSection(s)));
        }
    }

    SECTION("every continuation opens on a head with its first row, never on a spacer alone")
    {
        JournalDocument doc;
        {
            JournalPage page = MakePage("K", "Kept", "Plan", "Operations");
            Pen pen(page);
            pen.Title("Kept");
            pen.Line("item a");
            pen.Head("Head one");
            pen.Line("item b");
            pen.Head("Head two");
            pen.Line("item c");
            doc.pages.Add(page);
        }
        JournalPageHtml html;
        const float p = html.GetPHeight();
        // budget 7.5, footer block 4 (two rows plus the 2 P bar reserve): 3.5
        // usable; title + item a = 2.45, a head and its item = 3.15, so three
        // pages of two blocks each
        html.pageHeight = 11 * p;
        JournalPageInputs in;
        RenderJournal(&html, doc, in);

        CHECK(html.NSections() == 3);
        CHECK(SectionOfText(html, "Head one") == SectionOfText(html, "item b"));
        CHECK(SectionOfText(html, "Head two") == SectionOfText(html, "item c"));
        CHECK(SectionOfText(html, "Head one") == html.FindSection("K_2"));
        CHECK(SectionOfText(html, "Head two") == html.FindSection("K_3"));
        for (int s = 0; s < html.NSections(); s++)
        {
            const HTMLSection& sec = html.GetSection(s);
            CHECK_FALSE(OnlySpacers(sec));
            CHECK(RowSum(sec) <= html.GetPageHeight());
            CHECK(EveryBlockStartsARow(sec));
            // no page ends on a head
            int lastContent = -1;
            for (int f = 0; f < sec.fields.Size(); f++)
            {
                if (!sec.fields[f].bottom)
                {
                    lastContent = f;
                }
            }
            REQUIRE(lastContent >= 0);
            CHECK(sec.fields[lastContent].format != HFH4);
        }
    }

    SECTION("the hand is charged at 1.6 P: one hand row per page where two typed rows fit")
    {
        // the twelve-block case above pins two VoiceType rows per page at
        // this height (budget 6.5, footer block 4, 2.5 usable); two hand rows
        // are 3.2 and do not, so the slot sizes must be bound before the
        // measure
        JournalDocument doc;
        {
            JournalPage page = MakePage("H", "Hand", "Plan", "Operations");
            for (int i = 0; i < 4; i++)
            {
                char text[32];
                snprintf(text, sizeof(text), "hand %d", i);
                page.blocks.Add(TextBlock(text, VoiceHand, 0.04f));
            }
            doc.pages.Add(page);
        }
        JournalPageHtml html;
        const float p = html.GetPHeight();
        html.pageHeight = 10 * p;
        JournalPageInputs in;
        RenderJournal(&html, doc, in);
        CHECK(html.NSections() == 4);
        REQUIRE(html.FindSection("H_4") >= 0);
        CHECK(html.FindSection("H_5") < 0);
        for (int i = 0; i < 4; i++)
        {
            char text[32];
            snprintf(text, sizeof(text), "hand %d", i);
            const int s = SectionOfText(html, text);
            REQUIRE(s >= 0);
            CHECK(s == html.FindSection(ContinuationName("H", i + 1)));
            const HTMLSection& sec = html.GetSection(s);
            CHECK(RowHeightOf(sec, text) == Catch::Approx(1.6f * p));
            CHECK(RowSum(sec) <= html.GetPageHeight());
        }
    }
}

// ===========================================================================
// 2. aliases after the format (design D0)
// ===========================================================================

TEST_CASE("Journal render - a legacy alias resolves to page 0 after SplitSection has paginated the section",
          "[game][guerrilla][journal][render]")
{
    NoTexturesGuard noTextures;
    JournalDocument doc;
    {
        JournalPage page = MakePage("X", "Aliased", "Main", "Contents");
        page.aliases.Add(RString("OLD"));
        // 3000 characters at half a unit each on a 1000-wide page: two rows
        page.blocks.Add(TextBlock(Repeat("abcdefghi ", 300)));
        for (int i = 0; i < 3; i++)
        {
            page.blocks.Add(TextBlock(PaddedText(i)));
        }
        doc.pages.Add(page);
    }
    JournalPageHtml html;
    const float p = html.GetPHeight();
    // budget = 3 - 3.5 < 0: every block gets its own physical page, and the
    // first block (two rows + the four footer rows = 6 > 3) overflows its
    // page, so SplitSection fires on it
    html.pageHeight = 3 * p;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    const int x = html.FindSection("X");
    REQUIRE(x >= 0);
    CHECK(html.FindSection("X/0") == x); // the safety net did fire ...
    CHECK(html.FindSection("X/1") >= 0);
    CHECK(html.FindSection("X_2") >= 0); // ... and the chain is still there
    CHECK(html.FindSection("X_4") >= 0);

    // the alias lands on page 0 whether or not the net fired
    const int old = html.FindSection("OLD");
    REQUIRE(old >= 0);
    CHECK(old == x);
    const HTMLSection& page0 = html.GetSection(x);
    CHECK(NamesContain(page0, "X"));
    CHECK(NamesContain(page0, "OLD"));
    CHECK(SectionsNamed(html, "OLD") == 1);
    // page 0 carries the aliased content; the later net pages do not
    // carry the alias
    const int x1 = html.FindSection("X/1");
    CHECK_FALSE(NamesContain(html.GetSection(x1), "OLD"));
    CHECK_FALSE(NamesContain(html.GetSection(html.FindSection("X_2")), "OLD"));
}

TEST_CASE("Journal render - aliases stay on page 0 of a chain that fits without the safety net",
          "[game][guerrilla][journal][render]")
{
    JournalDocument doc;
    {
        JournalPage page = MakePage("GM_PLACES", "Places", "Main", "Contents");
        page.aliases.Add(RString("GM_ZONES"));
        for (int i = 0; i < 6; i++)
        {
            page.blocks.Add(TextBlock(PaddedText(i)));
        }
        doc.pages.Add(page);
        // Compose's own part 2 joins the same chain
        JournalPage part2 = MakePage("GM_PLACES_2", "Places", "Main", "Contents");
        part2.baseName = "GM_PLACES";
        part2.part = 2;
        part2.blocks.Add(TextBlock(PaddedText(6)));
        doc.pages.Add(part2);
    }
    JournalPageHtml html;
    html.pageHeight = 10 * html.GetPHeight();
    JournalPageInputs in;
    RenderJournal(&html, doc, in);
    // six blocks at two a page, then the composed part: GM_PLACES .. GM_PLACES_4
    CHECK(html.FindSection("GM_PLACES_4") >= 0);
    CHECK(html.FindSection("GM_PLACES_5") < 0);
    CHECK(html.FindSection("GM_ZONES") == html.FindSection("GM_PLACES"));
    CHECK(SectionsNamed(html, "GM_ZONES") == 1);
    CHECK(html.Text("GM_PLACES_4").find("block 06") != std::string::npos);
    // the chain's prev / next run through the renumbered pages; a top-level
    // page (parent Contents) links Contents once, never "Contents - Contents"
    const std::vector<std::string> want = {"#Main", "#GM_PLACES_2", "#GM_PLACES_4"};
    CHECK(FooterHrefs(html.GetSection(html.FindSection("GM_PLACES_3"))) == want);
    CHECK(FooterText(html.GetSection(html.FindSection("GM_PLACES_3")), "#Main") == "Contents");
}

// ===========================================================================
// 3. the portrait box
// ===========================================================================

TEST_CASE("Journal render - the portrait box is square on screen at every UI aspect and stays under half the budget",
          "[game][guerrilla][journal][render]")
{
    NoTexturesGuard noTextures;
    const float aspects[] = {4.0f / 3.0f, 16.0f / 10.0f, 16.0f / 9.0f, 21.0f / 9.0f};
    // the default height leaves the box unclamped; 20 P forces the clamp
    const float heights[] = {100000.0f, 20.0f};
    for (float aspect : aspects)
    {
        for (float pageHeight : heights)
        {
            CAPTURE(aspect, pageHeight);
            JournalDocument doc;
            {
                JournalPage page = MakePage("GM_WHO_TEST", "Test", "GM_PEOPLE", "People");
                page.blocks.Add(PortraitBlock(true));
                page.blocks.Add(TextBlock("Name under the photograph"));
                page.blocks.Add(PortraitBlock(false));
                doc.pages.Add(page);
            }
            JournalPageHtml html;
            html.pageHeight = pageHeight;
            const float p = html.GetPHeight();
            const float budget = pageHeight - 3.5f * p;
            JournalPageInputs in;
            in.uiAspect = aspect;
            RenderJournal(&html, doc, in);

            const std::vector<ImageAt> imgs = Images(html);
            REQUIRE(imgs.size() == 2);
            const HTMLField& present = html.GetSection(imgs[0].section).fields[imgs[0].field];
            const HTMLField& absent = html.GetSection(imgs[1].section).fields[imgs[1].field];

            // AddImage units: h480 = w640 * 0.75 * uiAspect ...
            const RenderMetrics m = MeasureContainer(html, aspect);
            const PortraitBox box = ComputePortraitBox(m);
            REQUIRE(box.w640 > 0);
            CHECK(box.h480 / box.w640 == Catch::Approx(0.75f * aspect).epsilon(1e-4));
            // ... which the field stores as width = w640 / 640, height = h480 / 480,
            // so the normalized box is square on screen iff height / width == uiAspect
            for (const HTMLField* fld : {&present, &absent})
            {
                REQUIRE(fld->width > 0);
                CHECK(fld->height / fld->width == Catch::Approx(aspect).epsilon(1e-4));
                CHECK(fld->width == Catch::Approx(box.w640 / 640.0f).epsilon(1e-4));
                CHECK(fld->height == Catch::Approx(box.h480 / 480.0f).epsilon(1e-4));
                CHECK(fld->height <= 0.5f * budget + 1e-4f);
                CHECK(fld->href.GetLength() == 0); // a linked image dims to 0.6 alpha
                CHECK(fld->texture1.IsNull());     // no engine: nothing is loaded
                CHECK(fld->text.GetLength() == 0); // no caption above the image
            }
            if (pageHeight > 1000)
            {
                CHECK(box.w640 == Catch::Approx(640.0f * kPortraitWidthFraction * html.GetPageWidth()));
            }
            else
            {
                // the clamp path: the box shrank to exactly half the budget
                CHECK(box.h480 / 480.0f == Catch::Approx(kPortraitMaxHeightFraction * budget).epsilon(1e-4));
            }
            // the unavailable photograph reserves the identical box and is
            // followed by the pencil line
            CHECK(absent.width == Catch::Approx(present.width));
            CHECK(absent.height == Catch::Approx(present.height));
            {
                const HTMLSection& sec = html.GetSection(imgs[1].section);
                REQUIRE(imgs[1].field + 2 < sec.fields.Size());
                CHECK(sec.fields[imgs[1].field + 1].nextline);
                const HTMLField& line = sec.fields[imgs[1].field + 2];
                CHECK(S(line.text) == "Photograph unavailable");
                CHECK(line.format == HFP);
                CHECK(line.hasColor);
                CHECK(IsPencil(line.color));
            }
            {
                // the present one is followed by its break and the caption block
                const HTMLSection& sec = html.GetSection(imgs[0].section);
                REQUIRE(imgs[0].field + 2 < sec.fields.Size());
                CHECK(sec.fields[imgs[0].field + 1].nextline);
                CHECK(S(sec.fields[imgs[0].field + 2].text) == "Name under the photograph");
            }
            // the image row is charged at its height and no page overruns
            for (int s = 0; s < html.NSections(); s++)
            {
                CHECK(RowSum(html.GetSection(s)) <= html.GetPageHeight());
            }
            CHECK_FALSE(html.HasFieldColor());
        }
    }
}

// ===========================================================================
// 4. the slots
// ===========================================================================

TEST_CASE("Journal render - slot binding scales the title, head, serif and hand from P and leaves P alone",
          "[game][guerrilla][journal][render]")
{
    JournalPageHtml html;
    const float p = html.GetPHeight();
    REQUIRE(p == Catch::Approx(1.0f));
    // a stale hand size from an earlier build must be recomputed, not compounded
    html.SetFormatSize(HFH6, 7.0f);

    JournalDocument doc;
    {
        JournalPage page = MakePage("Main", "Contents", "", "");
        page.blocks.Add(TextBlock("Resistance Dossier", VoiceTitle));
        page.blocks.Add(TextBlock("A head", VoiceHead));
        page.blocks.Add(TextBlock("A serif line", VoiceSerif));
        page.blocks.Add(TextBlock("A hand line", VoiceHand, 0.04f));
        JournalBlock gap;
        gap.kind = BlockGap;
        page.blocks.Add(gap);
        page.blocks.Add(TextBlock("A typed line", VoiceType));
        doc.pages.Add(page);
    }
    // a finite height, so the rows are laid against a real budget
    html.pageHeight = 100 * p;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    CHECK(html.GetFormatSize(HFH3) == Catch::Approx(1.45f * p));
    CHECK(html.GetFormatSize(HFH4) == Catch::Approx(1.15f * p));
    CHECK(html.GetFormatSize(HFH5) == Catch::Approx(1.1f * p));
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(1.6f * p));
    CHECK(html.GetFormatSize(HFP) == Catch::Approx(p));
    CHECK(html.GetFormatSize(HFH1) == Catch::Approx(1.0f)); // never touched
    CHECK(html.GetFormatSize(HFH2) == Catch::Approx(1.0f));
    CHECK(html.GetFormatSize(HFH3) != html.GetFormatSize(HFH4)); // title and head never share a format
    CHECK_FALSE(html.HasFieldColor());

    // the voices landed on their slots
    const HTMLSection& sec = html.GetSection(html.FindSection("Main"));
    // ... and the rows were measured at the bound sizes (row height is
    // max(P, slot size)): the budget arithmetic sees the 1.6x hand
    CHECK(RowHeightOf(sec, "Resistance Dossier") == Catch::Approx(1.45f * p));
    CHECK(RowHeightOf(sec, "A head") == Catch::Approx(1.15f * p));
    CHECK(RowHeightOf(sec, "A serif line") == Catch::Approx(1.1f * p));
    CHECK(RowHeightOf(sec, "A hand line") == Catch::Approx(1.6f * p));
    CHECK(RowHeightOf(sec, "A typed line") == Catch::Approx(1.0f * p));
    CHECK(RowSum(sec) <= html.GetPageHeight());
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        const std::string text = S(fld.text);
        if (text == "Resistance Dossier")
        {
            CHECK(fld.format == HFH3);
        }
        else if (text == "A head")
        {
            CHECK(fld.format == HFH4);
        }
        else if (text == "A serif line")
        {
            CHECK(fld.format == HFH5);
        }
        else if (text == "A hand line")
        {
            CHECK(fld.format == HFH6);
            CHECK(fld.hanging == Catch::Approx(0.04f * html.GetPageWidth()));
        }
        else if (text == " " && !fld.bottom)
        {
            CHECK(fld.format == HFP); // the gap rides on P, never the head slot
        }
    }
    // the hanging indent never leaks past the run
    const int s = html.FindSection("Main");
    html.AddText(s, "probe", HFP, HALeft, false, false, "");
    const HTMLField& probe = sec.fields[sec.fields.Size() - 1];
    CHECK(probe.hanging == 0.0f);
    CHECK_FALSE(probe.hasColor);

    // the table
    CHECK(SlotOf(VoiceTitle) == HFH3);
    CHECK(SlotOf(VoiceHead) == HFH4);
    CHECK(SlotOf(VoiceSerif) == HFH5);
    CHECK(SlotOf(VoiceHand) == HFH6);
    CHECK(SlotOf(VoiceType) == HFP);
    CHECK(SlotOf(VoiceSmallType) == HFP);
    CHECK(SlotOf(VoiceSpacer) == HFP);
    CHECK(ScaleOf(VoiceTitle) == Catch::Approx(1.45f));
    CHECK(ScaleOf(VoiceHead) == Catch::Approx(1.15f));
    CHECK(ScaleOf(VoiceSerif) == Catch::Approx(1.1f));
    CHECK(ScaleOf(VoiceHand) == Catch::Approx(1.6f));
    CHECK(ScaleOf(VoiceType) == Catch::Approx(1.0f));
    CHECK(ScaleOf(VoiceSmallType) == Catch::Approx(1.0f));
    CHECK(ScaleOf(VoiceSpacer) == Catch::Approx(1.0f));

    // idempotent: a second build on the same container recomputes from P
    RenderJournal(&html, doc, in);
    CHECK(html.GetFormatSize(HFH3) == Catch::Approx(1.45f * p));
    CHECK(html.GetFormatSize(HFH4) == Catch::Approx(1.15f * p));
    CHECK(html.GetFormatSize(HFH5) == Catch::Approx(1.1f * p));
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(1.6f * p));
    CHECK(html.GetFormatSize(HFP) == Catch::Approx(p));
    // BindJournalSlots alone does the same and is null-safe
    BindJournalSlots(nullptr);
    JournalPageHtml fresh;
    BindJournalSlots(&fresh);
    CHECK(fresh.GetFormatSize(HFH3) == Catch::Approx(1.45f * fresh.GetPHeight()));
    CHECK(fresh.GetFormatSize(HFH6) == Catch::Approx(1.6f * fresh.GetPHeight()));
    CHECK(fresh.GetFormatSize(HFP) == Catch::Approx(fresh.GetPHeight()));
}

// ===========================================================================
// 5. an authored section
// ===========================================================================

TEST_CASE("Journal render - an authored Main keeps its fields ahead of the journal blocks and charges its rows",
          "[game][guerrilla][journal][render]")
{
    JournalPageHtml html;
    html.LoadBuffer("inline.html", R"html(<html><body>
        <h1><a name="Main"></a>Mission notes</h1>
        <p>Authored notes.</p>
    </body></html>)html");
    REQUIRE(html.NSections() == 1);
    std::vector<std::string> authored;
    for (int f = 0; f < html.GetSection(0).fields.Size(); f++)
    {
        authored.push_back(S(html.GetSection(0).fields[f].text));
    }
    REQUIRE(authored.size() >= 2);

    JournalDocument doc;
    {
        JournalPage page = MakePage("Main", "Contents", "", "");
        for (int i = 0; i < 8; i++)
        {
            page.blocks.Add(TextBlock(PaddedText(i)));
        }
        doc.pages.Add(page);
    }
    // budget 7.5 P, footer block 4 P (two footer rows plus the group-bar
    // reserve): three one-row blocks per bare page; on page 1 the authored
    // rows eat into that
    html.pageHeight = 11 * html.GetPHeight();
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    CHECK(SectionsNamed(html, "Main") == 1); // appended to, never duplicated
    const int main = html.FindSection("Main");
    REQUIRE(main == 0);
    const HTMLSection& sec = html.GetSection(main);
    // the authored fields survive, untouched, ahead of the journal blocks
    REQUIRE(sec.fields.Size() > (int)authored.size());
    for (size_t f = 0; f < authored.size(); f++)
    {
        CHECK(S(sec.fields[(int)f].text) == authored[f]);
    }
    const std::string text = html.Text("Main");
    CHECK(text.find("Mission notes") != std::string::npos);
    CHECK(text.find("Authored notes.") != std::string::npos);
    CHECK(text.find("block 00") != std::string::npos);
    CHECK(text.find("Authored notes.") < text.find("block 00"));

    // page 1 holds fewer blocks than a bare page because the authored rows
    // are charged against its budget; the continuation pages hold three
    REQUIRE(html.FindSection("Main_2") >= 0);
    const int onMain = CountText(text, "block ");
    const int onMain2 = CountText(html.Text("Main_2"), "block ");
    CHECK(onMain2 == 3);
    CHECK(onMain < onMain2);
    CHECK(onMain >= 1);
    for (int s = 0; s < html.NSections(); s++)
    {
        CHECK(RowSum(html.GetSection(s)) <= html.GetPageHeight());
        CHECK(EveryBlockStartsARow(html.GetSection(s)));
    }
    // every block landed somewhere, once
    int total = 0;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& page = html.GetSection(s);
        total += CountText(html.Text(page.names[0]), "block ");
    }
    CHECK(total == 8);
    // the Contents footer on an authored Main is still the pencil word (no
    // link to itself, no parent); only the chain's next link carries an href
    const std::vector<std::string> want = {"#Main_2"};
    CHECK(FooterHrefs(sec) == want);
    CHECK(FooterText(sec, "#Main_2") == "next");
    bool pencilContents = false;
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        if (fld.bottom && S(fld.text) == "Contents")
        {
            pencilContents = true;
            CHECK(fld.href.GetLength() == 0);
            CHECK(fld.hasColor);
            CHECK(IsPencil(fld.color));
        }
    }
    CHECK(pencilContents);
}

TEST_CASE("Journal render - continuation names and a null container are safe", "[game][guerrilla][journal][render]")
{
    CHECK(S(ContinuationName("X", 0)) == "X");
    CHECK(S(ContinuationName("X", 1)) == "X");
    CHECK(S(ContinuationName("X", 2)) == "X_2");
    CHECK(S(ContinuationName("GM_RECORD", 12)) == "GM_RECORD_12");
    JournalDocument doc;
    JournalPageInputs in;
    RenderJournal(nullptr, doc, in); // no crash
    // an empty document leaves the container empty
    JournalPageHtml html;
    RenderJournal(&html, doc, in);
    CHECK(html.NSections() == 0);
    // a page with no blocks still gets its section and footer: Contents once
    // for a top-level page, Contents and the parent for a deeper one
    doc.pages.Add(MakePage("Empty", "Empty", "Main", "Contents"));
    doc.pages.Add(MakePage("Deeper", "Deeper", "Plan", "Operations"));
    RenderJournal(&html, doc, in);
    REQUIRE(html.FindSection("Empty") >= 0);
    const std::vector<std::string> want = {"#Main"};
    CHECK(FooterHrefs(html.GetSection(html.FindSection("Empty"))) == want);
    REQUIRE(html.FindSection("Deeper") >= 0);
    const std::vector<std::string> wantDeeper = {"#Main", "#Plan"};
    CHECK(FooterHrefs(html.GetSection(html.FindSection("Deeper"))) == wantDeeper);
}

// ===========================================================================
// 5b. link ink
//
// The control draws a link in its own _linkColor, a pale lavender that
// measures 1.6:1 against the notepad paper: the navigation labels came out the
// least legible text on the page.  Render therefore inks a stock link in the
// hand, and CHTMLContainer::FieldDrawColor lets that per-field colour win over
// _linkColor for a non-hovered link (test_uiControls.cpp pins the precedence).
// ===========================================================================

TEST_CASE("Journal render - every link carries a dark ink instead of the control's pale stock link colour",
          "[game][guerrilla][journal][render]")
{
    // the paper the 800x600 capture measured under the notepad's text
    const float paper = Luminance(PackedColor(195, 197, 206, 255));

    JournalDocument doc;
    {
        JournalPage page = MakePage("L", "Links", "Main", "Contents");
        Pen pen(page);
        pen.Title("Links");
        pen.LinkRow("Dispatches", "#GM_DISPATCH", "The day's page.");
        JournalRun red; // a danger link stays red
        red.text = "cover blown";
        red.href = "#GM_RECORD";
        red.ink = InkRed;
        pen.Run(red);
        pen.EndBlock();
        JournalRun pencil; // an aside link stays pencil
        pencil.text = "record";
        pencil.href = "#GM_RECORD";
        pencil.ink = InkPencil;
        pen.Run(pencil);
        pen.EndBlock();
        pen.Line("a plain typed line");
        doc.pages.Add(page);
    }
    JournalPageHtml html;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    int links = 0;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const HTMLField& fld = sec.fields[f];
            if (fld.href.GetLength() == 0)
            {
                continue;
            }
            links++;
            INFO("link: " << S(fld.text));
            CHECK(fld.hasColor); // never left on the stock link colour
            CHECK(Luminance(fld.color) < 0.35f * paper);
        }
    }
    // three in the body plus the footer's Contents link
    CHECK(links == 4);

    const HTMLSection& sec = html.GetSection(html.FindSection("L"));
    const HTMLField* dispatches = nullptr;
    const HTMLField* description = nullptr;
    const HTMLField* blown = nullptr;
    const HTMLField* record = nullptr;
    const HTMLField* plain = nullptr;
    const HTMLField* footer = nullptr;
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        const std::string text = S(fld.text);
        if (text == "Dispatches")
        {
            dispatches = &fld;
        }
        else if (text == "  The day's page.")
        {
            description = &fld;
        }
        else if (text == "cover blown")
        {
            blown = &fld;
        }
        else if (text == "record")
        {
            record = &fld;
        }
        else if (text == "a plain typed line")
        {
            plain = &fld;
        }
        else if (text == "Contents" && fld.bottom)
        {
            footer = &fld;
        }
    }
    REQUIRE(dispatches);
    REQUIRE(description);
    REQUIRE(blown);
    REQUIRE(record);
    REQUIRE(plain);
    REQUIRE(footer);
    // a stock-inked link takes the hand ink, exactly
    CHECK(IsHandInk(dispatches->color));
    CHECK(IsHandInk(footer->color));
    CHECK(S(footer->href) == "#Main");
    // an explicitly inked link keeps what Compose gave it
    CHECK(IsRedInk(blown->color));
    CHECK(IsPencil(record->color));
    // and nothing else is inked by the link rule: the extension is opt-in
    CHECK(description->href.GetLength() == 0);
    CHECK(IsPencil(description->color));
    CHECK_FALSE(plain->hasColor);
    CHECK_FALSE(html.HasFieldColor());
}

// ===========================================================================
// 6. cells
// ===========================================================================

TEST_CASE("Journal render - a fixed cell is cut to its width with an ellipsis and carries the width to the field",
          "[game][guerrilla][journal][render]")
{
    JournalDocument doc;
    const std::string longText = Repeat("abcdefghi ", 20); // 200 characters: 100 units at P
    {
        JournalPage page = MakePage("C", "Cells", "Plan", "Operations");
        JournalBlock row;
        JournalRun wide;
        wide.text = longText.c_str();
        wide.voice = VoiceType;
        wide.cellWidth = 0.09f;
        row.runs.Add(wide);
        JournalRun narrow;
        narrow.text = "short";
        narrow.voice = VoiceType;
        narrow.cellWidth = 0.09f;
        row.runs.Add(narrow);
        page.blocks.Add(row);
        // a one-character cell narrower than its own text comes back whole
        JournalBlock tiny;
        JournalRun one;
        one.text = "Z";
        one.voice = VoiceType;
        one.cellWidth = 0.0001f;
        tiny.runs.Add(one);
        page.blocks.Add(tiny);
        doc.pages.Add(page);
    }
    JournalPageHtml html;
    JournalPageInputs in;
    RenderJournal(&html, doc, in);

    const HTMLSection& sec = html.GetSection(html.FindSection("C"));
    const float maxW = 0.09f * html.GetPageWidth() * 0.97f;
    const HTMLField* wide = nullptr;
    const HTMLField* narrow = nullptr;
    const HTMLField* one = nullptr;
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        const HTMLField& fld = sec.fields[f];
        const std::string text = S(fld.text);
        if (text.rfind("abcdefghi", 0) == 0)
        {
            wide = &fld;
        }
        else if (text == "short")
        {
            narrow = &fld;
        }
        else if (text == "Z")
        {
            one = &fld;
        }
    }
    REQUIRE(wide);
    REQUIRE(narrow);
    REQUIRE(one);
    // the long cell: cut, ended in "...", within the cell, the width on the field
    const std::string cut = S(wide->text);
    CHECK(cut.size() >= 3);
    CHECK(cut.compare(cut.size() - 3, 3, "...") == 0);
    CHECK(cut.size() < longText.size());
    CHECK(html.GetTextWidth(html.GetFormatSize(HFP), nullptr, wide->text) <= maxW);
    CHECK(wide->format == HFP);
    CHECK(wide->tableWidth == Catch::Approx(0.09f * html.GetPageWidth()));
    // the short cell: byte-identical, the same width
    CHECK(S(narrow->text) == "short");
    CHECK(narrow->tableWidth == Catch::Approx(0.09f * html.GetPageWidth()));
    // the degenerate cell: never emptied
    CHECK(S(one->text) == "Z");
    CHECK(one->tableWidth == Catch::Approx(0.0001f * html.GetPageWidth()));
    CHECK_FALSE(html.HasFieldColor());
}
