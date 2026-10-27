#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/UboMirror.hpp>

#include <cstring>
#include <random>
#include <vector>

// The GL33 backend's constant blocks (LitConstants, VSConstants) are written
// through a mirror of what the buffer holds: a flush sends only the slots that
// changed. The rule that keeps that exact: after every upload, whole or
// partial, the buffer equals the CPU block in every slot a shader reads. These
// tests drive the mirror the way EngineGL33 does (Plan, upload, Store) against
// a recording fake buffer.

using Poseidon::gpu::UboMirror;

namespace
{
// A uniform buffer that only records what was written to it.
struct FakeBuffer
{
    std::vector<float> data;
    int uploads = 0;
    int slotsSent = 0;
    explicit FakeBuffer(int slots) : data(static_cast<size_t>(slots) * 4, -12345.0f) {}
    void Write(const float* src, int first, int count)
    {
        std::memcpy(&data[static_cast<size_t>(first) * 4], src + static_cast<size_t>(first) * 4,
                    static_cast<size_t>(count) * 16);
        ++uploads;
        slotsSent += count;
    }
};

// EngineGL33::FlushLitConstants / FlushVSConstants.
void Flush(UboMirror& m, FakeBuffer& gpu, const float* cpu, bool dedupe, int ignoreFirst = 0, int ignoreCount = 0)
{
    int first = 0, count = 0;
    if (!m.Plan(cpu, dedupe, first, count, ignoreFirst, ignoreCount))
        return;
    gpu.Write(cpu, first, count);
    m.Store(cpu, first, count);
}

// EngineGL33::SetLitOpaque / UploadVSCrown / UploadVSWorldMatrix: a partial
// upload of a few slots.
void Partial(UboMirror& m, FakeBuffer& gpu, const float* cpu, int first, int count)
{
    gpu.Write(cpu, first, count);
    m.Store(cpu, first, count);
}

bool SameOutside(const FakeBuffer& gpu, const std::vector<float>& cpu, int ignoreFirst, int ignoreCount)
{
    const int slots = static_cast<int>(cpu.size() / 4);
    for (int i = 0; i < slots; ++i)
    {
        if (i >= ignoreFirst && i < ignoreFirst + ignoreCount)
            continue;
        if (std::memcmp(&gpu.data[static_cast<size_t>(i) * 4], &cpu[static_cast<size_t>(i) * 4], 16) != 0)
            return false;
    }
    return true;
}
} // namespace

TEST_CASE("UboMirror: the buffer equals the CPU block after every flush and partial upload", "[gpu][UboMirror]")
{
    constexpr int kSlots = 58; // LitConstants
    for (const bool dedupe : {true, false})
    {
        UboMirror m;
        m.Reset(kSlots);
        FakeBuffer gpu(kSlots);
        std::vector<float> cpu(static_cast<size_t>(kSlots) * 4, 0.0f);
        std::mt19937 rng(dedupe ? 3 : 4);
        std::uniform_int_distribution<int> slot(0, kSlots - 1), op(0, 9);
        std::uniform_real_distribution<float> val(-2.0f, 2.0f);
        // A new buffer holds garbage until its first flush, which sends the
        // whole block (the mirror does not know it yet).
        Flush(m, gpu, cpu.data(), dedupe);
        REQUIRE(gpu.slotsSent == kSlots);
        bool pending = false; // a CPU write no flush has carried yet
        for (int step = 0; step < 4000; ++step)
        {
            const int o = op(rng);
            if (o < 5)
            {
                // A draw changes a few slots (its material and lights).
                const int n = 1 + o;
                for (int k = 0; k < n; ++k)
                    cpu[static_cast<size_t>(slot(rng)) * 4 + static_cast<size_t>(k % 4)] = val(rng);
                Flush(m, gpu, cpu.data(), dedupe);
                pending = false;
            }
            else if (o < 7)
            {
                // A partial upload of one slot (the flags), changed or not.
                const int s = slot(rng);
                if (o == 5)
                    cpu[static_cast<size_t>(s) * 4] = val(rng);
                Partial(m, gpu, cpu.data(), s, 1);
            }
            else if (o == 7)
            {
                // A CPU write with no flush yet: the next flush must carry it.
                cpu[static_cast<size_t>(slot(rng)) * 4 + 2] = val(rng);
                pending = true;
                continue;
            }
            else if (o == 8)
            {
                Flush(m, gpu, cpu.data(), dedupe); // nothing changed since the last write
                pending = false;
            }
            else
            {
                // The buffer is recreated (a context reset, a shader reload):
                // it holds garbage until the next flush.
                gpu = FakeBuffer(kSlots);
                m.Reset(kSlots);
                Flush(m, gpu, cpu.data(), dedupe);
                pending = false;
            }
            if (pending)
                continue; // only a flush promises the whole block
            INFO("step " << step << " op " << o);
            REQUIRE(SameOutside(gpu, cpu, 0, 0));
        }
    }
}

TEST_CASE("UboMirror: dedupe sends only the changed range, or nothing", "[gpu][UboMirror]")
{
    constexpr int kSlots = 58;
    UboMirror m;
    m.Reset(kSlots);
    CHECK_FALSE(m.Known());
    FakeBuffer gpu(kSlots);
    std::vector<float> cpu(static_cast<size_t>(kSlots) * 4, 1.0f);

    Flush(m, gpu, cpu.data(), true); // unknown buffer: the whole block
    CHECK(gpu.slotsSent == kSlots);
    CHECK(m.Known());

    Flush(m, gpu, cpu.data(), true); // unchanged: no upload
    CHECK(gpu.uploads == 1);

    cpu[20 * 4] = 3.0f; // one material slot
    cpu[24 * 4 + 1] = 4.0f;
    Flush(m, gpu, cpu.data(), true);
    CHECK(gpu.uploads == 2);
    CHECK(gpu.slotsSent == kSlots + 5); // slots 20..24

    Flush(m, gpu, cpu.data(), false); // dedupe off: the whole block, as before the mirror
    CHECK(gpu.slotsSent == kSlots + 5 + kSlots);
}

TEST_CASE("UboMirror: slots no shader reads never count as a change", "[gpu][UboMirror]")
{
    // VSConstants with the WorldInstances buffer: its world slots (8..11) are
    // std140 padding, written on every draw but read by no shader.
    constexpr int kSlots = 70;
    UboMirror m;
    m.Reset(kSlots);
    FakeBuffer gpu(kSlots);
    std::vector<float> cpu(static_cast<size_t>(kSlots) * 4, 0.5f);
    Flush(m, gpu, cpu.data(), true, 8, 4);
    const int before = gpu.uploads;
    for (int i = 0; i < 16; ++i)
        cpu[8 * 4 + static_cast<size_t>(i)] = static_cast<float>(i);
    Flush(m, gpu, cpu.data(), true, 8, 4);
    CHECK(gpu.uploads == before);
    CHECK(SameOutside(gpu, cpu, 8, 4));
    cpu[30 * 4] = 9.0f;
    Flush(m, gpu, cpu.data(), true, 8, 4);
    CHECK(gpu.uploads == before + 1);
    CHECK(SameOutside(gpu, cpu, 8, 4));
}

TEST_CASE("UboMirror: a partial upload is recorded, so setting the slot back is sent", "[gpu][UboMirror]")
{
    // SetLitOpaque flips the flags slot by a partial upload between draws; a
    // later flush that sets it back must reach the buffer.
    constexpr int kSlots = 58, kFlags = 57;
    UboMirror m;
    m.Reset(kSlots);
    FakeBuffer gpu(kSlots);
    std::vector<float> cpu(static_cast<size_t>(kSlots) * 4, 0.0f);
    cpu[kFlags * 4] = 1.0f;
    Flush(m, gpu, cpu.data(), true);
    cpu[kFlags * 4] = 0.0f; // blend draw: partial upload
    Partial(m, gpu, cpu.data(), kFlags, 1);
    CHECK(gpu.data[kFlags * 4] == 0.0f);
    cpu[kFlags * 4] = 1.0f; // opaque again, carried by the next flush
    Flush(m, gpu, cpu.data(), true);
    CHECK(gpu.data[kFlags * 4] == 1.0f);
    CHECK(SameOutside(gpu, cpu, 0, 0));
}
