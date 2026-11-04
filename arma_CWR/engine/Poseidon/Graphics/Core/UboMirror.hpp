#pragma once

#include <cstring>
#include <vector>

// What a uniform buffer holds, kept on the CPU.
//
// The GL33 backend keeps a CPU copy of each constant block and used to send
// the whole block on every flush: the 928-byte LitConstants block once per
// draw (a material and its lights), the 1,120-byte VSConstants block whenever
// a material or the view changed. With thousands of draws a frame that was
// about a megabyte of uploads, almost all of it unchanged. A mirror records
// what the buffer holds after every upload, so a flush can send only the
// slots that changed (or nothing).
//
// Every upload to the buffer must go through Store (whole or partial), and a
// newly created buffer starts unknown (Reset): then the first flush sends the
// whole block. Slots are vec4 (four floats). Pure, no GL.

namespace Poseidon::gpu
{

class UboMirror
{
  public:
    // A new buffer of `slots` vec4: its contents are not known yet.
    void Reset(int slots)
    {
        _slots = slots;
        _data.assign(static_cast<size_t>(slots) * 4, 0.0f);
        _known = false;
    }
    int Slots() const { return _slots; }
    bool Known() const { return _known; }

    // The buffer now holds `count` slots of `src` from slot `first` on (an
    // upload just made, partial or whole). Storing the whole block makes the
    // mirror known.
    void Store(const float* src, int first, int count)
    {
        if (first < 0 || count <= 0 || first + count > _slots)
            return;
        std::memcpy(&_data[static_cast<size_t>(first) * 4], src + static_cast<size_t>(first) * 4,
                    static_cast<size_t>(count) * 16);
        if (first == 0 && count == _slots)
            _known = true;
    }

    // What a flush of `src` (the whole CPU block) must upload, as one slot
    // range. With `dedupe` off, or while the buffer's contents are unknown,
    // the whole block. With it on, the range from the first to the last slot
    // that differs from the buffer, skipping slots in [ignoreFirst,
    // ignoreFirst + ignoreCount) (slots no shader reads); false when nothing
    // differs.
    bool Plan(const float* src, bool dedupe, int& first, int& count, int ignoreFirst = 0, int ignoreCount = 0) const
    {
        if (!dedupe || !_known)
        {
            first = 0;
            count = _slots;
            return _slots > 0;
        }
        int lo = -1, hi = -1;
        for (int i = 0; i < _slots; ++i)
        {
            if (i >= ignoreFirst && i < ignoreFirst + ignoreCount)
                continue;
            if (std::memcmp(&_data[static_cast<size_t>(i) * 4], src + static_cast<size_t>(i) * 4, 16) != 0)
            {
                if (lo < 0)
                    lo = i;
                hi = i;
            }
        }
        if (lo < 0)
            return false;
        first = lo;
        count = hi - lo + 1;
        return true;
    }

    // The mirror's copy of one slot (tests).
    const float* SlotData(int slot) const { return &_data[static_cast<size_t>(slot) * 4]; }

  private:
    int _slots = 0;
    bool _known = false;
    std::vector<float> _data;
};

} // namespace Poseidon::gpu
