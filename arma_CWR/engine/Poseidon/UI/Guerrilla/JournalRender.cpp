#include <Poseidon/UI/Guerrilla/JournalRender.hpp>

#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp> // JournalPageInputs
#include <Poseidon/UI/Guerrilla/JournalText.hpp>           // Fmt

#include <Poseidon/UI/Controls/UIControlsBase.hpp> // CHTMLContainer

#include <Poseidon/Foundation/Framework/DebugLog.hpp> // LOG_WARN, DoAssert
#include <Poseidon/Foundation/platform.hpp>           // stricmp
#include <Poseidon/Graphics/Core/Engine.hpp>          // GEngine, LoadFont
#include <Poseidon/Graphics/Rendering/Draw/Font.hpp>  // Font::Height
#include <Poseidon/IO/ParamFileExt.hpp>               // GetFontID

// Guerrilla Mode journal: the Render stage.
//
// Render is the only journal code that touches the CHTMLContainer, and it is
// pure: no singleton, no Journal write, no revision bump.  It binds the
// briefing control's format slots to the journal's faces, then lays every
// page of the composed JournalDocument into the document model.
//
// The page budget is measure-then-repartition: a page's blocks are laid into
// the section, FormatSectionRows wraps them into rows without paginating,
// the rows are summed per block (every block ends in a break, so a row never
// straddles two blocks), and the largest run of leading blocks that fits
// `GetPageHeight() - 3.5 * P` minus the footer stays; the rest is truncated
// off and laid again on the next physical page, named "<base>_2", "<base>_3"
// ... (never "<name>/<n>", which is SplitSection's own namespace).  Render is
// the sole namer of physical pages: Compose's five-entry parts are boundaries
// in the same stream, so a height split inserts a page and renumbers the
// chain, and the footer's prev / next links are emitted once the whole chain
// is partitioned.  Legacy aliases are attached after the page is formatted
// (design D0): re-resolved through FindSection, never a pre-format index.

namespace Poseidon::Guerrilla
{

namespace
{

// ===========================================================================
// ink on paper
//
// The notepad keeps its stock look; the journal adds only inks: the
// handwritten entries in blue-black, a red pen for alert and loss, pencil
// grey for stamps, labels and asides, a faded hand for what is done.
// InkStock is the control's own text colour (no SetFieldColor).
// ===========================================================================

inline PackedColor RGB(int r, int g, int b, int a = 255)
{
    return PackedColor(r, g, b, a);
}

const PackedColor kHandInk = RGB(14, 16, 52);    // fountain pen, blue-black
const PackedColor kRedInk = RGB(150, 22, 18);    // red pen: alert, loss, blown
const PackedColor kPencil = RGB(50, 46, 42);     // stamps, labels, asides
const PackedColor kFadedHand = RGB(84, 88, 122); // done items

// null = the control's stock text colour
const PackedColor* InkOf(JournalInk ink)
{
    switch (ink)
    {
        case InkHand:
            return &kHandInk;
        case InkRed:
            return &kRedInk;
        case InkPencil:
            return &kPencil;
        case InkFaded:
            return &kFadedHand;
        case InkStock:
        default:
            return nullptr;
    }
}

HTMLAlign AlignOf(JournalAlign a)
{
    return a == AlignRight ? HARight : HALeft;
}

// ===========================================================================
// the slot table
// ===========================================================================

struct SlotBinding
{
    HTMLFormat slot;
    float scale;
    const char* face;
};

// H3 title, H4 head, H5 serif, H6 hand.  P (Courier, engine-shared) and
// H1 / H2 (Group / Gear / Pool sections) are never touched.
const SlotBinding kBindings[] = {
    {kSlotTitle, kTitleScale, kTitleFace},
    {kSlotHead, kHeadScale, kHeadFace},
    {kSlotSerif, kSerifScale, kSerifFace},
    {kSlotHand, kHandScale, kHandFace},
};

// ===========================================================================
// emitting runs and blocks
// ===========================================================================

// truncate to a cell width with "..." (Sheet::Fit); a field with a table
// width neither wraps nor clips, so an overlong cell would overdraw its
// neighbour.  Parser-only containers measure by character count, so the
// unit tests keep whole strings.
RString FitCell(const CHTMLContainer& html, const RString& text, float w, HTMLFormat f, float pageW)
{
    Font* font = html.GetFormatFont(f, false);
    const float size = html.GetFormatSize(f);
    const float maxW = w * pageW * 0.97f;
    if (html.GetTextWidth(size, font, text) <= maxW)
    {
        return text;
    }
    RString cut = text;
    while (cut.GetLength() > 1)
    {
        cut = cut.Substring(0, cut.GetLength() - 1);
        RString probe = cut + RString("...");
        if (html.GetTextWidth(size, font, probe) <= maxW)
        {
            return probe;
        }
    }
    return cut;
}

// one field, with its ink and hanging indent set around the AddText and
// reset after it (the pending state is sticky; a test pins HasFieldColor()
// false after a build)
void EmitRun(CHTMLContainer* html, int s, const JournalRun& run, const RenderMetrics& m)
{
    const HTMLFormat slot = SlotOf(run.voice);
    if (const PackedColor* ink = InkOf(run.ink))
    {
        html->SetFieldColor(*ink);
    }
    html->SetHanging(run.hanging * m.pageW);
    if (run.cellWidth > 0)
    {
        html->AddText(s, FitCell(*html, run.text, run.cellWidth, slot, m.pageW), slot, AlignOf(run.align), false,
                      run.bold, run.href, run.cellWidth * m.pageW);
    }
    else
    {
        html->AddText(s, run.text, slot, AlignOf(run.align), false, run.bold, run.href, 0);
    }
    html->ClearFieldColor();
    html->SetHanging(0);
}

// a pencil small-type line (P in pencil ink: a voice, not a slot)
void EmitPencilLine(CHTMLContainer* html, int s, const char* text)
{
    html->SetFieldColor(kPencil);
    html->AddText(s, RString(text), kSlotSmallType, HALeft, false, false, RString(), 0);
    html->ClearFieldColor();
}

// one block; every block ends in a break so rows never straddle blocks (a
// leadGap spacer row is laid inside its block, ahead of the runs)
void EmitBlock(CHTMLContainer* html, int s, const JournalBlock& block, const RenderMetrics& m, const PortraitBox& box)
{
    switch (block.kind)
    {
        case BlockGap:
            // a short blank line on the P slot (never the head slot)
            html->AddText(s, RString(" "), kSlotSpacer, HALeft, false, false, RString(), 0);
            html->AddBreak(s, false);
            return;
        case BlockPortrait:
        {
            // explicit 640x480-unit sizes: an empty src reserves the same box
            // (UIControlsExt.cpp AddImage applies the same split units on the
            // no-texture branch), loads no texture and works in the parser-only
            // container.  Never an href on the image: a linked picture is
            // drawn at 0.6 alpha.  The src is only handed over with an engine
            // up: GlobLoadTexture reaches the texture bank.
            const bool load = block.portraitPresent && GEngine != nullptr;
            html->AddImage(s, load ? block.portraitSrc : RString(), HALeft, false, box.w640, box.h480, RString(),
                           RString(), 0);
            html->AddBreak(s, false);
            if (!block.portraitPresent)
            {
                EmitPencilLine(html, s, "Photograph unavailable");
                html->AddBreak(s, false);
            }
            return;
        }
        case BlockText:
        default:
            if (block.leadGap)
            {
                // a Head's short blank line, inside the head's own block so
                // the two rows move together (both start inside the block)
                html->AddText(s, RString(" "), kSlotSpacer, HALeft, false, false, RString(), 0);
                html->AddBreak(s, false);
            }
            for (int i = 0; i < block.runs.Size(); i++)
            {
                EmitRun(html, s, block.runs[i], m);
            }
            html->AddBreak(s, false);
            return;
    }
}

// ===========================================================================
// sections and chains
// ===========================================================================

// the section named `name`, reusing an authored briefing.html section of the
// same name (an authored "Main" / "Plan" is appended to, never replaced)
int EnsureSection(CHTMLContainer* html, const RString& name)
{
    int s = html->FindSection(name);
    if (s < 0)
    {
        s = html->AddSection();
        html->AddName(s, name);
    }
    return s;
}

// the physical pages of one base name, in doc order (Compose's parts)
struct Chain
{
    RString base;
    AutoArray<int> pages; // indices into doc.pages
};

void GroupChains(const JournalDocument& doc, AutoArray<Chain>& chains)
{
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        const RString base = page.baseName.GetLength() > 0 ? page.baseName : page.name;
        int c = -1;
        for (int i = 0; i < chains.Size(); i++)
        {
            if (stricmp(chains[i].base, base) == 0)
            {
                c = i;
                break;
            }
        }
        if (c < 0)
        {
            Chain chain;
            chain.base = base;
            c = chains.Add(chain);
        }
        chains[c].pages.Add(p);
    }
}

// ===========================================================================
// the footer
//
// Two bottom-pinned rows: a spacer, then the link row at P:
// Contents - <parent> - prev - next, with pencil " - " separators; the
// current page is never listed.  On Contents itself the footer is the single
// pencil word Contents with no href.  The parent link is omitted when the
// parent is Contents itself (the six top-level pages): the Contents link
// already leads there, and "Contents - Contents" would eat the width prev /
// next need at 800x600.  Height 2 * P, charged against the page budget.
// ===========================================================================

void FooterPencil(CHTMLContainer* html, int s, const char* text)
{
    html->SetFieldColor(kPencil);
    html->AddText(s, RString(text), kSlotType, HALeft, true, false, RString(), 0);
    html->ClearFieldColor();
}

void FooterLink(CHTMLContainer* html, int s, const RString& text, const RString& href)
{
    html->AddText(s, text, kSlotType, HALeft, true, false, href, 0);
}

void EmitFooter(CHTMLContainer* html, int s, const JournalPage& first, int i, const AutoArray<RString>& chain)
{
    html->AddText(s, RString(" "), kSlotSpacer, HALeft, true, false, RString(), 0);
    html->AddBreak(s, true);
    const bool hasParent = first.parentName.GetLength() > 0;
    const bool parentIsContents = hasParent && stricmp(first.parentName, "Main") == 0;
    if (i == 0 && !hasParent)
    {
        FooterPencil(html, s, "Contents");
    }
    else
    {
        FooterLink(html, s, RString("Contents"), RString("#Main"));
    }
    if (hasParent && !parentIsContents)
    {
        FooterPencil(html, s, " - ");
        FooterLink(html, s, first.parentTitle, RString("#") + first.parentName);
    }
    if (i > 0)
    {
        FooterPencil(html, s, " - ");
        FooterLink(html, s, RString("prev"), RString("#") + chain[i - 1]);
    }
    if (i + 1 < chain.Size())
    {
        FooterPencil(html, s, " - ");
        FooterLink(html, s, RString("next"), RString("#") + chain[i + 1]);
    }
    html->AddBreak(s, true);
}

} // namespace

// ===========================================================================
// voices -> slots
// ===========================================================================

HTMLFormat SlotOf(JournalVoice v)
{
    switch (v)
    {
        case VoiceTitle:
            return kSlotTitle;
        case VoiceSerif:
            return kSlotSerif;
        case VoiceHead:
            return kSlotHead;
        case VoiceHand:
            return kSlotHand;
        case VoiceSmallType:
            return kSlotSmallType;
        case VoiceSpacer:
            return kSlotSpacer;
        case VoiceType:
        default:
            return kSlotType;
    }
}

float ScaleOf(JournalVoice v)
{
    switch (v)
    {
        case VoiceTitle:
            return kTitleScale;
        case VoiceSerif:
            return kSerifScale;
        case VoiceHead:
            return kHeadScale;
        case VoiceHand:
            return kHandScale;
        default:
            return 1.0f;
    }
}

// ===========================================================================
// measures
// ===========================================================================

RenderMetrics MeasureContainer(const CHTMLContainer& html, float uiAspect)
{
    RenderMetrics m;
    m.pageW = html.GetPageWidth();
    m.pageH = html.GetPageHeight();
    m.sizeP = html.GetPHeight();
    m.uiAspect = uiAspect > 0 ? uiAspect : 4.0f / 3.0f;
    return m;
}

// AddImage stores width = w / 640 and height = h / 480 (UIControlsExt.cpp
// :581-582) and the control draws them times Width2D() / Height2D()
// (UIControlsHTML.cpp :392-393), so the on-screen aspect of the box is
// (w640 / h480) * 0.75 * uiAspect: an on-screen square needs
// h480 = w640 * 0.75 * uiAspect (design D1 Images; 4:3 -> h480 == w640,
// 16:9 -> h480 = w640 * 4/3).  Both shrink together when the box would take
// more than half the page budget: an image row is the only row that can
// exceed a page, and SplitSection reads rows[i - 1] with no i == 0 guard
// (UIControlsExt.cpp :2265), so a single row is kept well under the budget.
PortraitBox ComputePortraitBox(const RenderMetrics& m)
{
    PortraitBox box;
    box.w640 = 640.0f * kPortraitWidthFraction * m.pageW;
    box.h480 = box.w640 * 0.75f * m.uiAspect;
    const float maxHeight = kPortraitMaxHeightFraction * m.Budget();
    const float height = box.h480 / 480.0f;
    if (maxHeight > 0 && height > maxHeight)
    {
        const float scale = maxHeight / height;
        box.w640 *= scale;
        box.h480 *= scale;
    }
    return box;
}

RString ContinuationName(const RString& base, int part)
{
    if (part <= 1)
    {
        return base;
    }
    return JournalText::Fmt("%s_%d", (const char*)base, part);
}

// the largest k >= 1 such that the authored rows, blocks[0..k) and the footer
// fit the budget; rows never straddle blocks (every block ends in a break),
// so a block's height is the sum of the rows that start inside it.  A
// portrait row counts its image height (FormatSection gives the row
// field.height; the journal sets no image caption, so no extra P).  k = 1
// when even the first block does not fit: it overflows its page and
// SplitSection is the safety net.  When the cut would leave a keep-with-next
// block (a title, a subtitle, a head) as the last on the page, the cut walks
// back over it, never below k = 1; keepWithNext may be shorter than the
// block count (missing entries read false).
int FitBlocks(const CHTMLContainer& html, int section, int firstOwnField, const AutoArray<int>& blockStarts,
              const AutoArray<bool>& keepWithNext, float budget, float footerHeight)
{
    const int nBlocks = blockStarts.Size() - 1;
    if (nBlocks <= 0 || section < 0 || section >= html.NSections())
    {
        return 0;
    }
    const HTMLSection& sec = html.GetSection(section);
#ifdef _DEBUG
    // every non-empty block starts a row of its own
    for (int b = 0; b < nBlocks; b++)
    {
        if (blockStarts[b] >= blockStarts[b + 1])
        {
            continue;
        }
        bool startsRow = false;
        for (int r = 0; r < sec.rows.Size() && !startsRow; r++)
        {
            startsRow = sec.rows[r].firstField == blockStarts[b];
        }
        DoAssert(startsRow);
    }
#endif
    float used = footerHeight;
    for (int r = 0; r < sec.rows.Size(); r++)
    {
        if (sec.rows[r].firstField < firstOwnField)
        {
            used += sec.rows[r].height;
        }
    }
    int fit = 0;
    for (int b = 0; b < nBlocks; b++)
    {
        float blockHeight = 0;
        for (int r = 0; r < sec.rows.Size(); r++)
        {
            const int first = sec.rows[r].firstField;
            if (first >= blockStarts[b] && first < blockStarts[b + 1])
            {
                blockHeight += sec.rows[r].height;
            }
        }
        if (fit > 0 && used + blockHeight > budget)
        {
            break;
        }
        used += blockHeight;
        fit = b + 1;
    }
    // keep-with-next: a page is never ended on a block that introduces the
    // one after it (only when there is a cut at all)
    while (fit > 1 && fit < nBlocks && fit - 1 < keepWithNext.Size() && keepWithNext[fit - 1])
    {
        fit--;
    }
    return fit;
}

// ===========================================================================
// binding the slots
//
// Sizes are set unconditionally (relative to the configured P, so a repaint
// is idempotent and the parser-only test container gets the same metrics);
// faces only with an engine up and only when the face has glyphs.  LoadFont
// never returns null (Font.cpp :389-410): an unmapped name comes back as a
// zero-height face, which GetTextWidth measures 0 wide (FontDraw.cpp
// :570-573) and would under-count the page budget, so such a face is left
// alone with one warning and the slot keeps its configured face at the
// computed size (SetFormatFont returns before SetFormatSize on a null font,
// UIControlsExt.cpp :378, so the size is set first and separately).  Binding
// by the exact table row names: FindFontMapping is a prefix match on the
// row, "couriernew" would miss "couriernewb".  Rebinding H3 / H4 / H5 also
// restyles <h3> / <h4> / <h5> in any authored briefing.html rendered into the
// same control; no Guerrilla template authors one.  P is never resized or
// rebound; H1 / H2 are never touched.
// ===========================================================================

void BindJournalSlots(CHTMLContainer* html)
{
    if (!html)
    {
        return;
    }
    static bool warned[HFImg + 1] = {};
    static bool warnedP = false;
    const float sizeP = html->GetPHeight();
    for (const SlotBinding& b : kBindings)
    {
        const float size = b.scale * sizeP;
        html->SetFormatSize(b.slot, size);
        if (!GEngine)
        {
            continue;
        }
        Font* font = GEngine->LoadFont(GetFontID(b.face));
        if (font && font->Height() > 0)
        {
            html->SetFormatFont(b.slot, font, font, size);
        }
        else if (!warned[b.slot])
        {
            warned[b.slot] = true;
            LOG_WARN(Core, "Guerrilla journal: face '{}' for slot {} has no glyphs; keeping the configured face",
                     b.face, (int)b.slot);
        }
    }
    if (GEngine && !warnedP)
    {
        Font* p = html->GetFormatFont(HFP, false);
        if (!p || p->Height() <= 0 || html->GetFormatSize(HFP) <= 0)
        {
            warnedP = true;
            LOG_WARN(Core, "Guerrilla journal: the P slot has no usable face or size; the page budget will "
                           "under-count");
        }
    }
}

// ===========================================================================
// laying the document
// ===========================================================================

void RenderJournal(CHTMLContainer* html, const JournalDocument& doc, const JournalPageInputs& in)
{
    if (!html)
    {
        return;
    }
    BindJournalSlots(html);
    const RenderMetrics m = MeasureContainer(*html, in.uiAspect);
    const PortraitBox box = ComputePortraitBox(m);

    AutoArray<Chain> chains;
    GroupChains(doc, chains);

    for (int c = 0; c < chains.Size(); c++)
    {
        const Chain& chain = chains[c];
        const JournalPage& first = doc.pages[chain.pages[0]];
        AutoArray<RString> laid; // the physical pages, in order
        int part = 1;

        // Compose's parts are boundaries in one stream: a physical page never
        // crosses a part, and a part that overruns the budget continues on
        // the next physical page, renumbering the chain
        for (int p = 0; p < chain.pages.Size(); p++)
        {
            const JournalPage& page = doc.pages[chain.pages[p]];
            int next = 0;
            do
            {
                const RString name = ContinuationName(chain.base, part);
                const int s = EnsureSection(html, name);
                {
                    // an authored section may end mid-row: close it so the
                    // first block starts a row of its own and the authored
                    // rows are charged as authored
                    const HTMLSection& sec = html->GetSection(s);
                    if (sec.fields.Size() > 0 && !sec.fields[sec.fields.Size() - 1].nextline)
                    {
                        html->AddBreak(s, false);
                    }
                }
                const int firstOwnField = html->GetSection(s).fields.Size();
                AutoArray<int> blockStarts;
                AutoArray<bool> keepWithNext;
                for (int b = next; b < page.blocks.Size(); b++)
                {
                    blockStarts.Add(html->GetSection(s).fields.Size());
                    keepWithNext.Add(page.blocks[b].keepWithNext);
                    EmitBlock(html, s, page.blocks[b], m, box);
                }
                blockStarts.Add(html->GetSection(s).fields.Size());
                const int emitted = page.blocks.Size() - next;
                int keep = emitted;
                if (emitted > 0)
                {
                    html->FormatSectionRows(s);
                    keep = FitBlocks(*html, s, firstOwnField, blockStarts, keepWithNext, m.Budget(), m.FooterHeight());
                    if (keep < 1)
                    {
                        keep = 1;
                    }
                    if (keep < emitted)
                    {
                        // the cut never reaches below firstOwnField:
                        // blockStarts[0] == firstOwnField
                        html->TruncateSection(s, blockStarts[keep]);
                    }
                }
                laid.Add(name);
                part++;
                next += keep;
            } while (next < page.blocks.Size());
        }

        // the footer once the chain is partitioned (prev / next need the
        // neighbours' names), then the full FormatSection with SplitSection
        // as the never-triggered safety net.  Sections are re-resolved by
        // name every time: SplitSection deletes and re-appends sections
        for (int i = 0; i < laid.Size(); i++)
        {
            const int s = html->FindSection(laid[i]);
            if (s < 0)
            {
                continue;
            }
            EmitFooter(html, s, first, i, laid);
            html->FormatSection(s);
        }

        // legacy aliases on page 0 of the chain, attached AFTER the format
        // (design D0): SplitSection keeps names[0] on every page it emits and
        // FindSection returns the first match, so the alias lands on page 0
        // whether or not the safety net fired
        const int idx = html->FindSection(laid[0]);
        if (idx >= 0)
        {
            for (int a = 0; a < first.aliases.Size(); a++)
            {
                html->AddName(idx, first.aliases[a]);
            }
        }
    }
    html->ClearFieldColor();
    html->SetHanging(0);
}

} // namespace Poseidon::Guerrilla
