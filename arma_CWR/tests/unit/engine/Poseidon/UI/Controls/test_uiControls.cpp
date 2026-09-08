#include <Poseidon/UI/Controls/UIControls.hpp>
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/Foundation/Strings/Mbcs.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QStream.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <Poseidon/Foundation/Math/Math3D.hpp>

namespace
{
class ZeroEvaluatorFunctions final : public Poseidon::EvaluatorFunctions
{
  public:
    float EvaluateFloat(const char* /*expr*/, GameVarSpace* /*vars*/) override { return 0.0f; }
    float EvaluateFloatInternal(const char* /*expr*/) override { return 0.0f; }
};

class ScopedEvaluatorFunctions final
{
  public:
    explicit ScopedEvaluatorFunctions(Poseidon::EvaluatorFunctions* replacement)
        : _previous(Poseidon::ParamFile::DefaultEvalFunctions())
    {
        Poseidon::ParamFile::SetDefaultEvalFunctions(replacement);
    }

    ~ScopedEvaluatorFunctions() { Poseidon::ParamFile::SetDefaultEvalFunctions(_previous); }

    ScopedEvaluatorFunctions(const ScopedEvaluatorFunctions&) = delete;
    ScopedEvaluatorFunctions& operator=(const ScopedEvaluatorFunctions&) = delete;

  private:
    Poseidon::EvaluatorFunctions* _previous;
};
} // namespace

TEST_CASE("uiControls compiles", "[ui][controls]")
{
    REQUIRE(sizeof(Control) > 0);
}

TEST_CASE("legacy control IDs resolve without evaluator enum variables", "[ui][controls][resource]")
{
    const char* resource = "class StaticButton\n"
                           "{\n"
                           "    idc = \"IDC_STATIC\";\n"
                           "    type = 1;\n"
                           "    style = 2;\n"
                           "};\n";
    Poseidon::ParamFile pf;
    QIStream in(resource, static_cast<int>(strlen(resource)));
    pf.Parse(in);

    ZeroEvaluatorFunctions zeroEvaluator;
    ScopedEvaluatorFunctions scopedEvaluator(&zeroEvaluator);

    const Poseidon::ParamEntry& cls = pf >> "StaticButton";
    REQUIRE(ResolveLegacyControlInt(cls >> "idc") == -1);
    REQUIRE(ResolveLegacyControlInt(cls >> "type") == CT_BUTTON);
    REQUIRE(ResolveLegacyControlInt(cls >> "style") == ST_CENTER);
}

TEST_CASE("UTF-8 text element helpers keep Czech glyphs intact", "[ui][controls][utf8]")
{
    const char* text = "Šárka";

    REQUIRE(CountTextElements(text, English) == 5);
    REQUIRE(ByteOffsetForTextElements(text, 1, English) == 2);
    REQUIRE(ByteOffsetForTextElements(text, 5, English) == static_cast<int>(strlen(text)));

    char glyph[8];
    int next = CopyTextElement(text, 0, English, glyph, sizeof(glyph));
    REQUIRE(std::string(glyph) == "Š");
    REQUIRE(next == 2);
    REQUIRE(PrevTextElementPos(text, next, English) == 0);
}

TEST_CASE("UTF-8 codepoints encode to multi-byte edit input", "[ui][controls][utf8]")
{
    char buffer[8];
    REQUIRE(EncodeUtf8Codepoint(0x0160, buffer, sizeof(buffer)) == 2);
    REQUIRE(std::string(buffer) == "Š");
    REQUIRE(EncodeUtf8Codepoint(0x0158, buffer, sizeof(buffer)) == 2);
    REQUIRE(std::string(buffer) == "Ř");
}

TEST_CASE("UTF-8 decode and advance helpers share one runtime path", "[ui][controls][utf8]")
{
    const char* text = "Ahoj 😀";

    uint32_t cp = 0;
    REQUIRE(DecodeUtf8Codepoint(text, &cp) == 1);
    REQUIRE(cp == 'A');

    const char* emoji = Utf8AdvanceCodepoints(text, 5);
    REQUIRE(emoji != nullptr);
    REQUIRE(Utf8CodepointBytes(emoji) == 4);
    REQUIRE(DecodeUtf8Codepoint(emoji, &cp) == 4);
    REQUIRE(cp == 0x1F600);
    REQUIRE(CountUtf8Codepoints(text) == 6);
}

TEST_CASE("multi-line controls trim explicit line break delimiters", "[ui][controls][text]")
{
    RString text = "vyžaduje:\n  @csla nainstalováno\r\nhotovo";
    std::string raw = static_cast<const char*>(text);

    int firstEnd = static_cast<int>(raw.find('\n')) + 1;
    int secondStart = firstEnd;
    int secondEnd = static_cast<int>(raw.find('\n', secondStart)) + 1;

    REQUIRE(std::string(static_cast<const char*>(ControlLineTextWithoutTrailingBreaks(text, 0, firstEnd))) ==
            "vyžaduje:");
    REQUIRE(std::string(static_cast<const char*>(ControlLineTextWithoutTrailingBreaks(text, secondStart, secondEnd))) ==
            "  @csla nainstalováno");
    REQUIRE(std::string(static_cast<const char*>(ControlLineTextWithoutTrailingBreaks(text, secondEnd, raw.size()))) ==
            "hotovo");
}

// ─── C3DScrollBar — embedded helper used by C3DListBox / C3DCombo / the new
// ─── CT_3DSCROLLBAR Control3D wrapper.  The math is pure (no GEngine
// ─── dependency until OnDraw), so it's worth pinning down with unit tests
// ─── since every Options page that grows scrollable content will inherit
// ─── this exact behaviour.
//
// Geometry convention used throughout: _right is horizontal (width),
// _down is vertical (height).  v passed to OnLButtonDown / OnMouseHold is
// a [0..1] fraction of _down.Size().  Top caret occupies v ∈ [0, 0.8 * w/h);
// bottom caret occupies v ∈ [1 - 0.8 * w/h, 1].
namespace
{
// Helper — set up a scrollbar matching the audio page geometry: width
// 0.035 against a height 0.585.  spinSize/down = 0.8*0.035/0.585 ≈ 0.0479
// so caret zones span roughly v ∈ [0, 0.048] and [0.952, 1].
C3DScrollBar MakeAudioGeometry()
{
    C3DScrollBar sb;
    sb.SetPosition(Vector3(0, 0, 0), Vector3(0.035f, 0, 0), Vector3(0, 0.585f, 0));
    return sb;
}
} // namespace

TEST_CASE("C3DScrollBar default is unlocked", "[ui][scrollbar]")
{
    C3DScrollBar sb;
    REQUIRE_FALSE(sb.IsLocked());
}

TEST_CASE("C3DScrollBar SetRange + GetPos", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetPos(0);
    REQUIRE(sb.GetPos() == 0.0f);
    sb.SetPos(3);
    REQUIRE(sb.GetPos() == 3.0f);

    SECTION("SetPos saturates above max")
    {
        // SetPos clamps to [min, max], not [min, max-page] — interaction
        // (caret / track / drag) clamps further to max-page on the next
        // input event, so callers that always re-push from a clamped
        // model (scrollOffset) never see the difference.  Document the
        // behaviour so future refactors don't break it.
        sb.SetPos(20);
        REQUIRE(sb.GetPos() == 12.0f);
    }
    SECTION("SetPos saturates below min")
    {
        sb.SetPos(-3);
        REQUIRE(sb.GetPos() == 0.0f);
    }
}

TEST_CASE("C3DScrollBar top caret steps line up", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(3);

    sb.OnLButtonDown(0.02f); // top caret zone
    REQUIRE(sb.GetPos() == 2.0f);
    REQUIRE_FALSE(sb.IsLocked());

    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar bottom caret steps line down", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(3);

    sb.OnLButtonDown(0.99f); // bottom caret zone
    REQUIRE(sb.GetPos() == 4.0f);
    REQUIRE_FALSE(sb.IsLocked());

    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar caret saturates at min/max", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);

    sb.SetPos(0);
    sb.OnLButtonDown(0.02f); // top caret while at min
    REQUIRE(sb.GetPos() == 0.0f);
    sb.OnLButtonUp();

    sb.SetPos(5);
    sb.OnLButtonDown(0.99f); // bottom caret while at max
    REQUIRE(sb.GetPos() == 5.0f);
    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar track click steps page", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(0);

    // Track click below the thumb at scrollOffset=0 — thumb spans v
    // roughly [0.048, 0.62], so v=0.85 lands in the page-down zone.
    sb.OnLButtonDown(0.85f);
    REQUIRE(sb.GetPos() == 5.0f); // 0 + 7 saturated to max-page=5
    REQUIRE_FALSE(sb.IsLocked());
    sb.OnLButtonUp();

    // From scrollOffset=5, track click above thumb — thumb at the
    // bottom; v=0.10 is above it (in page-up zone).
    sb.OnLButtonDown(0.10f);
    REQUIRE(sb.GetPos() == 0.0f); // 5 - 7 saturated to 0
    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar thumb click locks the thumb", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(0);

    // At scrollOffset=0 the thumb spans roughly v=[0.048, 0.62].
    // v=0.30 lands inside the thumb body.
    sb.OnLButtonDown(0.30f);
    REQUIRE(sb.IsLocked());
    REQUIRE(sb.GetPos() == 0.0f); // pressing the thumb itself doesn't move it

    sb.OnLButtonUp();
    REQUIRE_FALSE(sb.IsLocked());
}

TEST_CASE("C3DScrollBar drag updates curPos", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(0);

    sb.OnLButtonDown(0.30f); // grab thumb
    REQUIRE(sb.IsLocked());

    sb.OnMouseHold(0.95f); // drag past bottom — curPos saturates to max
    REQUIRE(sb.GetPos() == 5.0f);

    sb.OnMouseHold(0.05f); // drag past top — curPos saturates to 0
    REQUIRE(sb.GetPos() == 0.0f);

    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar OnMouseHold is a no-op when not locked", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(1, 7);
    sb.SetPos(2);

    REQUIRE_FALSE(sb.IsLocked());
    sb.OnMouseHold(0.95f);
    REQUIRE(sb.GetPos() == 2.0f); // unchanged
}

TEST_CASE("C3DScrollBar SetSpeed customizes step amounts", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    sb.SetRange(0, 12, 7);
    sb.SetSpeed(2, 3); // line=2, page=3
    sb.SetPos(0);

    sb.OnLButtonDown(0.99f);      // bottom caret
    REQUIRE(sb.GetPos() == 2.0f); // 0 + line(2)
    sb.OnLButtonUp();

    sb.SetPos(0);
    sb.OnLButtonDown(0.85f);      // track below thumb
    REQUIRE(sb.GetPos() == 3.0f); // 0 + page(3)
    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar disabled when content fits", "[ui][scrollbar]")
{
    C3DScrollBar sb = MakeAudioGeometry();
    // page covers the whole range — SetRange clamps maxPos so curPos
    // can't actually move.
    sb.SetRange(0, 5, 5);
    sb.SetPos(0);
    REQUIRE(sb.GetPos() == 0.0f);

    sb.OnLButtonDown(0.02f);
    REQUIRE(sb.GetPos() == 0.0f); // saturated — can't go below 0
    sb.OnLButtonUp();

    sb.OnLButtonDown(0.99f);
    REQUIRE(sb.GetPos() == 0.0f); // saturated — max-page = 0
    sb.OnLButtonUp();
}

TEST_CASE("C3DScrollBar Enable toggle", "[ui][scrollbar]")
{
    C3DScrollBar sb;
    sb.Enable(false);
    REQUIRE_FALSE(sb.IsEnabled());
    sb.Enable(true);
    REQUIRE(sb.IsEnabled());
}

// ─── CHTMLContainer::SplitSection: the pagination safety net behind
// ─── FormatSection.  Parser-only container: no engine, no fonts, page metrics
// ─── are ours (P row = 1 unit, an image row = h / 480 units).
namespace
{
class PagedHtmlContainer final : public CHTMLContainer
{
  public:
    explicit PagedHtmlContainer(float pageHeight) : _pageHeight(pageHeight) {}

    float GetPageWidth() const override { return 1000; }
    float GetPageHeight() const override { return _pageHeight; }
    float GetTextWidth(float, Font*, const char* text) const override { return (float)strlen(text); }

  private:
    float _pageHeight;
};

// SplitSection decorates every page with sipka_*.paa arrow links; keep the
// texture bank out of the parser-only test whatever the working directory holds.
struct NoTexturesGuard
{
    bool previous;
    NoTexturesGuard() : previous(Poseidon::NoTextures) { Poseidon::NoTextures = true; }
    ~NoTexturesGuard() { Poseidon::NoTextures = previous; }
};

void AddParagraphRows(CHTMLContainer& html, int section, int count)
{
    for (int i = 0; i < count; i++)
    {
        html.AddText(section, "row", HFP, HALeft, false, false, "");
        html.AddBreak(section, false);
    }
}

// UD extension: the draw-time colour precedence both OnDraw bodies resolve
// through CHTMLContainer::FieldDrawColor.  Parser-only container that publishes
// the palette and the active field the draw bodies read.
class ColorHtmlContainer final : public CHTMLContainer
{
  public:
    ColorHtmlContainer()
    {
        _textColor = PackedColor(10, 12, 14, 255);
        _boldColor = PackedColor(20, 22, 24, 255);
        _linkColor = PackedColor(152, 151, 203, 255); // the stock pale lavender
        _activeLinkColor = PackedColor(200, 40, 40, 255);
    }

    PackedColor BoldColor() const { return _boldColor; }
    PackedColor ActiveLinkColor() const { return _activeLinkColor; }

    float GetPageWidth() const override { return 1000; }
    float GetPageHeight() const override { return 1000; }
    float GetTextWidth(float, Font*, const char* text) const override { return (float)strlen(text); }
};

bool SameColor(PackedColor a, PackedColor b)
{
    return a.R8() == b.R8() && a.G8() == b.G8() && a.B8() == b.B8() && a.A8() == b.A8();
}
} // namespace

TEST_CASE("FieldDrawColor: a per-field colour wins over the stock link colour, the hovered link keeps its own",
          "[ui][html][color]")
{
    // The stock order is link > bold > text.  The UD extension makes a field's
    // own colour win over the LINK colour too, so a journal page can ink its
    // navigation dark enough to read on paper; a field that sets no colour is
    // drawn exactly as before, and the hovered link still flips to the active
    // link colour so the hover feedback survives.
    ColorHtmlContainer html;
    const int s = html.AddSection();
    html.AddName(s, "S");
    html.AddText(s, "plain", HFP, HALeft, false, false, "");
    html.AddText(s, "bold", HFP, HALeft, false, true, "");
    html.AddText(s, "link", HFP, HALeft, false, false, "#A");
    html.AddText(s, "bold link", HFP, HALeft, false, true, "#A");
    const PackedColor ink(14, 16, 52, 255); // the journal's blue-black hand
    html.SetFieldColor(ink);
    html.AddText(s, "inked link", HFP, HALeft, false, false, "#B");
    html.AddText(s, "inked text", HFP, HALeft, false, false, "");
    html.ClearFieldColor();

    const HTMLSection& sec = html.GetSection(s);
    REQUIRE(sec.fields.Size() == 6);
    const HTMLField& plain = sec.fields[0];
    const HTMLField& bold = sec.fields[1];
    const HTMLField& link = sec.fields[2];
    const HTMLField& boldLink = sec.fields[3];
    const HTMLField& inkedLink = sec.fields[4];
    const HTMLField& inkedText = sec.fields[5];

    // stock behaviour, unchanged: no field here sets a colour
    CHECK_FALSE(plain.hasColor);
    CHECK(SameColor(html.FieldDrawColor(plain, false), html.GetTextColor()));
    CHECK(SameColor(html.FieldDrawColor(bold, false), html.BoldColor()));
    CHECK(SameColor(html.FieldDrawColor(link, false), html.GetLinkColor()));
    CHECK(SameColor(html.FieldDrawColor(boldLink, false), html.GetLinkColor())); // a link is never bold-coloured
    CHECK(SameColor(html.FieldDrawColor(link, true), html.ActiveLinkColor()));
    // the hover colour belongs to links: a plain field never takes it
    CHECK(SameColor(html.FieldDrawColor(plain, true), html.GetTextColor()));

    // the extension: the field's own colour beats both the link and the bold
    // colour, and survives on a non-hovered link
    REQUIRE(inkedLink.hasColor);
    REQUIRE(inkedText.hasColor);
    CHECK(SameColor(html.FieldDrawColor(inkedLink, false), ink));
    CHECK(SameColor(html.FieldDrawColor(inkedText, false), ink));
    CHECK(SameColor(html.FieldDrawColor(inkedText, true), ink));
    // ... except while it is the active field, where the hover wins
    CHECK(SameColor(html.FieldDrawColor(inkedLink, true), html.ActiveLinkColor()));
}

TEST_CASE("SplitSection keeps an oversized first row on page 0 instead of reading rows[-1]", "[ui][html][split]")
{
    NoTexturesGuard noTextures;
    // page height 5 P units; SplitSection reserves 3.5 of them for its own
    // spacer + arrow rows, so the image row (10 units) is taller than a page
    PagedHtmlContainer html(5);
    int s = html.AddSection();
    html.AddName(s, "S");
    REQUIRE(html.AddImage(s, "", HALeft, false, 640, 480 * 10, "") != nullptr);
    html.AddBreak(s, false);
    AddParagraphRows(html, s, 3);

    html.FormatSection(s);

    int page0 = html.FindSection("S");
    REQUIRE(page0 >= 0);
    CHECK(html.FindSection("S/0") == page0);
    CHECK(html.FindSection("S/1") >= 0);

    const HTMLSection& first = html.GetSection(page0);
    REQUIRE(first.names.Size() >= 1);
    CHECK(std::string(static_cast<const char*>(first.names[0])) == "S");
    bool imageOnPage0 = false;
    for (int f = 0; f < first.fields.Size(); f++)
    {
        if (first.fields[f].format == HFImg)
        {
            imageOnPage0 = true;
        }
    }
    CHECK(imageOnPage0);
    // the image row is the only content row page 0 keeps
    REQUIRE(first.rows.Size() >= 1);
    CHECK_THAT(first.rows[0].height, Catch::Matchers::WithinAbs(10.0, 1e-4));
}

TEST_CASE("SplitSection paginates into <name>/<n> pages that all keep names[0]", "[ui][html][split]")
{
    NoTexturesGuard noTextures;
    PagedHtmlContainer html(5);
    int s = html.AddSection();
    html.AddName(s, "S");
    AddParagraphRows(html, s, 20);

    html.FormatSection(s);

    // 20 one-unit rows against 1.5 units of usable page: one row per page
    REQUIRE(html.NSections() == 20);
    int page0 = html.FindSection("S");
    REQUIRE(page0 >= 0);
    CHECK(html.FindSection("S/0") == page0);
    for (int p = 0; p < 20; p++)
    {
        char name[16];
        snprintf(name, sizeof(name), "S/%d", p);
        int sec = html.FindSection(name);
        REQUIRE(sec >= 0);
        const HTMLSection& section = html.GetSection(sec);
        REQUIRE(section.names.Size() == 2);
        CHECK(std::string(static_cast<const char*>(section.names[0])) == "S");
        CHECK(std::string(static_cast<const char*>(section.names[1])) == name);
    }
    CHECK(html.FindSection("S/20") < 0);
}
