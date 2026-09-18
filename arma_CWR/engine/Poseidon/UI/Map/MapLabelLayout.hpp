#pragma once

// Per-frame collision bookkeeping for the text labels the map draws: island
// Names entries and the text of icon markers (setMarkerText, the Guerrilla
// zone labels).  Pure screen-space geometry, no engine dependencies beyond
// Rect2DFloat/RString, so the rules are unit-testable without a renderer.
//
// Rules (CStaticMap::ReserveMarkerLabels / DrawName apply them in this order):
//   1. every icon marker's label rectangle is claimed first, in markersMap
//      order; a label that intersects one already claimed is hidden for the
//      frame (thinning, mirroring the distance thinning Names already get);
//   2. every icon marker registers its icon's footprint;
//   3. an island Name whose rectangle starts under an icon slides to the
//      icon's right edge (where the marker's own label sits, if it has one);
//   4. an island Name that then lands under a claimed label yields: it is not
//      drawn when that label begins with the name (a Guerrilla zone seeded
//      from this town carries the name itself), and moves to the row below
//      the label otherwise (a zone with its own name on a town's anchor).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>

namespace Poseidon
{

class MapLabelLayout
{
  public:
    void Clear()
    {
        _labels.Clear();
        _labelTexts.Clear();
        _icons.Clear();
    }

    static bool Intersects(const Rect2DFloat& a, const Rect2DFloat& b)
    {
        // open intervals: labels that merely touch do not collide
        return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
    }

    // index of the first claimed label r intersects, -1 when r is free
    int Blocker(const Rect2DFloat& r) const
    {
        for (int i = 0; i < _labels.Size(); i++)
        {
            if (Intersects(r, _labels[i]))
            {
                return i;
            }
        }
        return -1;
    }

    bool Blocked(const Rect2DFloat& r) const { return Blocker(r) >= 0; }

    // claim r for a label; false (and nothing recorded) when it collides
    // with a label claimed earlier this frame
    bool ClaimLabel(const Rect2DFloat& r, RString text = RString())
    {
        if (Blocked(r))
        {
            return false;
        }
        _labels.Add(r);
        _labelTexts.Add(text);
        return true;
    }

    const Rect2DFloat& LabelRect(int index) const { return _labels[index]; }
    RString LabelText(int index) const { return _labelTexts[index]; }

    void AddIcon(const Rect2DFloat& r) { _icons.Add(r); }

    // right edge of the right-most icon overlapping r, if any
    bool IconRightEdge(const Rect2DFloat& r, float& right) const
    {
        bool found = false;
        for (int i = 0; i < _icons.Size(); i++)
        {
            const Rect2DFloat& icon = _icons[i];
            if (!Intersects(r, icon))
            {
                continue;
            }
            float edge = icon.x + icon.w;
            if (!found || edge > right)
            {
                right = edge;
            }
            found = true;
        }
        return found;
    }

    int LabelCount() const { return _labels.Size(); }
    int IconCount() const { return _icons.Size(); }

  private:
    AutoArray<Rect2DFloat> _labels;
    AutoArray<RString> _labelTexts;
    AutoArray<Rect2DFloat> _icons;
};

} // namespace Poseidon
