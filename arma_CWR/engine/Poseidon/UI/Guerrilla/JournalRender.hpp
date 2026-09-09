#pragma once

// Guerrilla Mode journal: the Render stage.
//
// Render binds the briefing control's format slots to the journal's faces
// (the slot table below; P is engine-shared and never resized or rebound),
// then lays every JournalPage of a composed JournalDocument into the
// CHTMLContainer document model.  It owns the page budget: a page's blocks
// are laid, wrapped with FormatSectionRows, measured per block, and the
// blocks that overrun `GetPageHeight() - 3.5 * P` minus the footer move
// whole onto a continuation page named "<base>_2", "<base>_3" ... (never
// "<name>/<n>", which is SplitSection's own namespace).  Legacy aliases are
// attached after the page is formatted (design D0), and the footer's prev /
// next links are emitted once the whole chain is partitioned.
//
// Ink rule for links: a run (or a footer link) that carries an href and no ink
// of its own is drawn in the hand ink, the blue-black RGB(14, 16, 52), not the
// control's stock link colour, which is a pale lavender that reads at 1.6:1 on
// the notepad paper.  The engine's per-field colour wins over the stock link
// colour for a non-hovered link (CHTMLContainer::FieldDrawColor), so the ink
// reaches the screen; the hovered link still flips to the control's active
// link colour.  A link composed with an explicit ink (red, pencil) keeps it.

#include <Poseidon/UI/Controls/UIControlsBase.hpp> // CHTMLContainer, HTMLFormat
#include <Poseidon/UI/Guerrilla/JournalCompose.hpp>

namespace Poseidon::Guerrilla
{
struct JournalPageInputs;

// slot table (design D1 Typography); P is never resized or rebound
constexpr HTMLFormat kSlotTitle = HFH3;
constexpr HTMLFormat kSlotHead = HFH4;
constexpr HTMLFormat kSlotSerif = HFH5;
constexpr HTMLFormat kSlotHand = HFH6;
constexpr HTMLFormat kSlotType = HFP;
constexpr HTMLFormat kSlotSmallType = HFP;
constexpr HTMLFormat kSlotSpacer = HFP;
constexpr float kTitleScale = 1.45f;
constexpr float kHeadScale = 1.15f;
constexpr float kSerifScale = 1.1f;
constexpr float kHandScale = 1.6f;
constexpr const char* kTitleFace = "garamond";
constexpr const char* kHeadFace = "couriernewb";
constexpr const char* kSerifFace = "garamond";
constexpr const char* kHandFace = "cwrpen";
constexpr float kPortraitWidthFraction = 0.42f;    // of GetPageWidth()
constexpr float kPortraitMaxHeightFraction = 0.5f; // of (GetPageHeight() - 3.5 * P)

// The map screen's group bar, and why the footer is not flush with the page
// bottom.
//
// The in-game group bar (RscInGameUI >> GroupInfo, InGameUI::DrawGroupInfo)
// keeps drawing while the map is open, as soon as the player leads a group of
// two or more.  It is a fixed band of the UI region, not a scaled one: the
// frame is drawn at uiY * Height2D() with the stock config's top of 0.90, so
// it always covers the bottom tenth of the screen at every resolution
// (measured identical at 800x600 and 1440x1080).
//
// The notepad's page bottom lands inside that band.  triBriefingMetrics reports
// the briefing control at y 0.2364, h 0.6903, scale 1 at both capture lanes, so
// the page bottom projects to 0.9267 and the bar eats the last 0.0267 of the
// page, 1.18 rows at the stock P height of 0.0226.  A bottom-pinned row block
// ends exactly at the page bottom (the layout walks back from GetPageHeight(),
// CHTMLContainer::FindField and CHTML::Draw), so a footer pinned flush to the
// bottom is drawn under the squad icons and reads at roughly half contrast.
//
// The fix is two blank P rows pinned BELOW the footer's link row: the link row
// then ends at 0.8815 of the screen, 0.0185 (11 px at 600, 20 px at 1080) clear
// of the bar, and the footer still sits at the foot of the page.  The reserve is
// charged to FooterHeight() so the same page budget that keeps the body text off
// the footer keeps it off the reserve.
constexpr int kBottomBarReserveRows = 2;
// the measured screen geometry the reserve is sized against (UI-region units)
constexpr float kMapNotepadPageBottom = 0.9267f; // triBriefingMetrics y + h
constexpr float kMapGroupBarTop = 0.90f;         // RscInGameUI >> GroupInfo >> top

HTMLFormat SlotOf(JournalVoice v);
float ScaleOf(JournalVoice v); // 1.0 for Type/SmallType/Spacer

struct RenderMetrics
{
    float pageW = 0, pageH = 0, sizeP = 0, uiAspect = 4.0f / 3.0f;
    float Budget() const { return pageH - 3.5f * sizeP; } // SplitSection's own reserve
    // blank rows pinned below the footer, so the link row clears the group bar
    float BarReserve() const { return kBottomBarReserveRows * sizeP; }
    // spacer row + link row + the bar reserve, all bottom-pinned
    float FooterHeight() const { return 2.0f * sizeP + BarReserve(); }
};
RenderMetrics MeasureContainer(const CHTMLContainer& html, float uiAspect);

struct PortraitBox
{
    float w640 = 0, h480 = 0; // AddImage units; on-screen square iff h480 == w640 * 0.75 * uiAspect
};
PortraitBox ComputePortraitBox(const RenderMetrics& m); // applies kPortraitMaxHeightFraction clamp

// sizes always; faces only when GEngine exists and the face has Height() > 0 (LOG_WARN once per dead face)
void BindJournalSlots(CHTMLContainer* html);

// lays every page: budget, continuation naming, footer, aliases-after-format
void RenderJournal(CHTMLContainer* html, const JournalDocument& doc, const JournalPageInputs& in);

// exposed for test_journal_render: leading blocks that fit. blockStarts[b] = first field index of block b
// (blockStarts.Size() == nBlocks + 1, last = end); firstOwnField = fields authored before Render began;
// keepWithNext[b] (may be shorter than nBlocks, missing = false) walks a cut back off a title / subtitle / head.
int FitBlocks(const CHTMLContainer& html, int section, int firstOwnField, const AutoArray<int>& blockStarts,
              const AutoArray<bool>& keepWithNext, float budget, float footerHeight);
RString ContinuationName(const RString& base, int part); // part 1 -> base, else "<base>_<part>"
} // namespace Poseidon::Guerrilla
