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

HTMLFormat SlotOf(JournalVoice v);
float ScaleOf(JournalVoice v); // 1.0 for Type/SmallType/Spacer

struct RenderMetrics
{
    float pageW = 0, pageH = 0, sizeP = 0, uiAspect = 4.0f / 3.0f;
    float Budget() const { return pageH - 3.5f * sizeP; } // SplitSection's own reserve
    float FooterHeight() const { return 2.0f * sizeP; }   // spacer row + link row, both bottom-pinned
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
