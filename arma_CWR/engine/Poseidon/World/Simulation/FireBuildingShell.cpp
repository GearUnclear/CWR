#include <Poseidon/World/Simulation/FireBuildingShell.hpp>

#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Rendering/Draw/SpecLods.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/World/Scene/Object.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <tuple>
#include <utility>
#include <unordered_map>
#include <vector>

namespace Poseidon::firespread
{

namespace
{

// The rule's measures (m; model space, y up).
constexpr float kUprightNy = 0.3f;        // |normal y| at most this: an upright face
constexpr float kRoofNy = 0.25f;          // a roof face's normal y at least this
constexpr float kRoofAboveFootM = 2.0f;   // a roof is higher than this over the model's foot
constexpr float kRoofBigM2 = 0.5f;        // roof faces this big set the eaves (not sills or porch tops)
constexpr float kWallUnderEavesM = 0.5f;  // an upright face reaching this far under its eaves is a wall
constexpr float kFlatOverEavesM = 0.3f;   // a flat face over its eaves less this is under the roof
constexpr float kWallFootM = 1.5f;        // the walls' box: upright faces reaching within this of the foot
constexpr float kInsideM = 0.4f;          // inside the walls by this: a room's floor or ceiling
constexpr float kUpperFloorM = 1.2f;      // flats this far over the ground floor: upper floors, stairs
constexpr float kTimberOverFloorM = 0.5f; // timber by name this far over the ground floor goes
constexpr float kCoplanarCos = 0.985f;    // a gable continues a wall: normals this close ...
constexpr float kCoplanarM = 0.15f;       // ... planes this close ...
constexpr float kContactM = 0.25f;        // ... and it starts within this of the wall's top
constexpr float kStackMaxM = 1.8f;        // a chimney stack's plan at most this a side ...
constexpr float kStackMinM = 0.2f;        // ... and at least this
constexpr float kStackTallM = 0.5f;       // ... at least this tall
constexpr float kStackOverRidgeM = 1.5f;  // ... its top at least the ridge less this ...
constexpr float kStackOverEavesM = 1.0f;  // ... and the eaves plus this
constexpr int kPierCopiesMax = 40;
constexpr float kWeldM = 0.02f; // corners this close are one (a part's faces)

// Glass, frames, doors, shutters and their kind go wherever they are; boards,
// beams, stairs and railings go above the ground floor.
const char* const kGlassNames[] = {"okno", "sklo", "futra", "dvere", "okenic", "_ram", "ram_", "clear_empty", "vrata"};
// A house's contents (proxies): wooden ones burn, metal ones (stoves, cookers,
// radiators, sinks, iron beds) and the charred wardrobe stay on the ground floor.
const char* const kWoodenContents[] = {"zidl",  "skrin", "stul",    "stol", "krabic",
                                       "truhl", "obraz", "televiz", "lamp", "postel"};
const char* const kLastingContents[] = {"opalen", "kov", "kamna", "sporak", "topeni", "drez"};
const char* const kTimberNames[] = {"zabradl", "schody",  "obrazk",        "prkn", "tram",
                                    "drev",    "oldwood", "spinapodkomin", "vikir"};

template <std::size_t N>
bool NameHas(const std::string& t, const char* const (&keys)[N])
{
    for (const char* k : keys)
        if (t.find(k) != std::string::npos)
            return true;
    return false;
}

struct Info
{
    float c[3] = {};
    float nrm[3] = {};
    float area = 0.0f;
    float lo[3] = {1e9f, 1e9f, 1e9f}, hi[3] = {-1e9f, -1e9f, -1e9f};
    bool up = false;
    bool use = false;
};

// The face's height at (x, z) when (x, z) lies in its plan, else false.
bool HeightAt(const ShellFace& f, float x, float z, float& y)
{
    for (int t = 0; t + 2 < std::max(f.n, 3); ++t)
    {
        const float* a = f.p[0];
        const float* b = f.p[t + 1];
        const float* c = f.p[t + 2];
        const float det = (b[2] - c[2]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[2] - c[2]);
        if (std::fabs(det) < 1e-9f)
            continue;
        const float l1 = ((b[2] - c[2]) * (x - c[0]) + (c[0] - b[0]) * (z - c[2])) / det;
        const float l2 = ((c[2] - a[2]) * (x - c[0]) + (a[0] - c[0]) * (z - c[2])) / det;
        const float l3 = 1.0f - l1 - l2;
        if (l1 < -1e-4f || l2 < -1e-4f || l3 < -1e-4f)
            continue;
        y = l1 * a[1] + l2 * b[1] + l3 * c[1];
        return true;
    }
    return false;
}

struct UnionFind
{
    std::vector<int> p;
    explicit UnionFind(std::size_t n) : p(n)
    {
        for (std::size_t i = 0; i < n; ++i)
            p[i] = static_cast<int>(i);
    }
    int Find(int i)
    {
        while (p[static_cast<std::size_t>(i)] != i)
        {
            p[static_cast<std::size_t>(i)] = p[static_cast<std::size_t>(p[static_cast<std::size_t>(i)])];
            i = p[static_cast<std::size_t>(i)];
        }
        return i;
    }
    void Join(int a, int b) { p[static_cast<std::size_t>(Find(a))] = Find(b); }
};

// Groups the faces `idx` by shared corners: each group a list of face indices.
std::vector<std::vector<int>> Parts(const std::vector<ShellFace>& faces, const std::vector<int>& idx)
{
    UnionFind uf(idx.size());
    std::map<std::tuple<long, long, long>, int> owner;
    for (std::size_t i = 0; i < idx.size(); ++i)
    {
        const ShellFace& f = faces[static_cast<std::size_t>(idx[i])];
        for (int k = 0; k < f.n; ++k)
        {
            const auto key = std::make_tuple(std::lround(f.p[k][0] / kWeldM), std::lround(f.p[k][1] / kWeldM),
                                             std::lround(f.p[k][2] / kWeldM));
            const auto it = owner.find(key);
            if (it != owner.end())
                uf.Join(static_cast<int>(i), it->second);
            else
                owner.emplace(key, static_cast<int>(i));
        }
    }
    std::map<int, std::vector<int>> groups;
    for (std::size_t i = 0; i < idx.size(); ++i)
        groups[uf.Find(static_cast<int>(i))].push_back(idx[i]);
    std::vector<std::vector<int>> out;
    out.reserve(groups.size());
    for (auto& g : groups)
        out.push_back(std::move(g.second));
    return out;
}

} // namespace

ShellPlan PlanBurntShell(const std::vector<ShellFace>& faces, bool church)
{
    ShellPlan plan;
    const std::size_t n = faces.size();
    plan.part.assign(n, ShellPart::Wall);
    std::vector<Info> in(n);
    float foot = 1e9f;
    for (std::size_t i = 0; i < n; ++i)
    {
        const ShellFace& f = faces[i];
        Info& q = in[i];
        if (f.hidden || f.n < 3 || f.n > 4)
            continue;
        for (int k = 0; k < f.n; ++k)
            for (int a = 0; a < 3; ++a)
            {
                q.c[a] += f.p[k][a] / static_cast<float>(f.n);
                q.lo[a] = std::min(q.lo[a], f.p[k][a]);
                q.hi[a] = std::max(q.hi[a], f.p[k][a]);
            }
        for (int t = 0; t + 2 < f.n; ++t)
        {
            const float u[3] = {f.p[t + 1][0] - f.p[0][0], f.p[t + 1][1] - f.p[0][1], f.p[t + 1][2] - f.p[0][2]};
            const float w[3] = {f.p[t + 2][0] - f.p[0][0], f.p[t + 2][1] - f.p[0][1], f.p[t + 2][2] - f.p[0][2]};
            const float cx = u[1] * w[2] - u[2] * w[1], cy = u[2] * w[0] - u[0] * w[2], cz = u[0] * w[1] - u[1] * w[0];
            q.area += 0.5f * std::sqrt(cx * cx + cy * cy + cz * cz);
        }
        const float l = std::sqrt(f.nrm[0] * f.nrm[0] + f.nrm[1] * f.nrm[1] + f.nrm[2] * f.nrm[2]);
        if (q.area < 1e-5f || l < 1e-6f)
            continue;
        for (int a = 0; a < 3; ++a)
            q.nrm[a] = f.nrm[a] / l;
        q.up = std::fabs(q.nrm[1]) <= kUprightNy;
        q.use = true;
        foot = std::min(foot, q.lo[1]);
    }
    if (foot > 1e8f)
        return plan;
    auto inPlan = [&](const Info& g, float x, float z, float pad)
    { return x >= g.lo[0] - pad && x <= g.hi[0] + pad && z >= g.lo[2] - pad && z <= g.hi[2] + pad; };

    // The roof: up-facing faces seen from straight above, over kRoofAboveFootM.
    std::vector<char> roof(n, 0);
    std::vector<int> big;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Info& q = in[i];
        if (!q.use || q.nrm[1] <= kRoofNy || q.c[1] < foot + kRoofAboveFootM)
            continue;
        bool top = true;
        for (std::size_t j = 0; j < n && top; ++j)
        {
            const Info& g = in[j];
            if (j == i || !g.use || g.up || g.hi[1] < q.c[1] + 0.05f || !inPlan(g, q.c[0], q.c[2], 0.0f))
                continue;
            float y = 0.0f;
            if (HeightAt(faces[j], q.c[0], q.c[2], y) && y > q.c[1] + 0.05f)
                top = false;
        }
        if (!top)
            continue;
        roof[i] = 1;
        if (q.area > kRoofBigM2)
            big.push_back(static_cast<int>(i));
    }
    if (big.empty())
        for (std::size_t i = 0; i < n; ++i)
            if (roof[i])
                big.push_back(static_cast<int>(i));
    if (big.empty())
        return plan;
    float eaves = 1e9f, ridge = -1e9f;
    for (const int i : big)
    {
        eaves = std::min(eaves, in[static_cast<std::size_t>(i)].lo[1]);
        ridge = std::max(ridge, in[static_cast<std::size_t>(i)].hi[1]);
    }
    // Each part of the roof (its faces sharing corners) has its own eaves:
    // a lean-to or a porch roof sits lower than the house's.
    std::vector<float> partEaves(n, eaves);
    for (const std::vector<int>& g : Parts(faces, big))
    {
        float e = 1e9f;
        for (const int i : g)
            e = std::min(e, in[static_cast<std::size_t>(i)].lo[1]);
        for (const int i : g)
            partEaves[static_cast<std::size_t>(i)] = e;
    }
    auto eavesAt = [&](float x, float z)
    {
        float e = 1e9f;
        for (const int i : big)
        {
            const Info& g = in[static_cast<std::size_t>(i)];
            float y = 0.0f;
            if (inPlan(g, x, z, 0.05f) && HeightAt(faces[static_cast<std::size_t>(i)], x, z, y))
                e = std::min(e, partEaves[static_cast<std::size_t>(i)]);
        }
        return e > 1e8f ? eaves : e;
    };
    // The walls' box: upright faces from the foot.
    float wx0 = 1e9f, wx1 = -1e9f, wz0 = 1e9f, wz1 = -1e9f;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Info& q = in[i];
        if (!q.use || !q.up || q.lo[1] >= foot + kWallFootM || NameHas(faces[i].tex, kGlassNames))
            continue;
        wx0 = std::min(wx0, q.lo[0]);
        wx1 = std::max(wx1, q.hi[0]);
        wz0 = std::min(wz0, q.lo[2]);
        wz1 = std::max(wz1, q.hi[2]);
    }
    if (wx0 > wx1)
        return plan;
    auto inside = [&](const Info& q)
    {
        return q.c[0] > wx0 + kInsideM && q.c[0] < wx1 - kInsideM && q.c[2] > wz0 + kInsideM && q.c[2] < wz1 - kInsideM;
    };
    // The ground floor: the lowest big up-facing face inside the walls, else the foot.
    float ground = 1e9f;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Info& q = in[i];
        if (q.use && q.nrm[1] > 0.9f && q.area > 1.0f && inside(q) && q.c[1] < eaves - 1.0f)
            ground = std::min(ground, q.c[1]);
    }
    if (ground > foot + 2.5f)
        ground = foot;

    std::vector<char> set(n, 0);
    auto put = [&](std::size_t i, ShellPart p)
    {
        plan.part[i] = p;
        set[i] = 1;
    };
    for (std::size_t i = 0; i < n; ++i)
        if (!in[i].use)
            set[i] = 1; // hidden or degenerate: left as it is (Wall: untouched)
    // Glass and frames; timber over the ground floor.
    for (std::size_t i = 0; i < n; ++i)
        if (!set[i] && (NameHas(faces[i].tex, kGlassNames) ||
                        (NameHas(faces[i].tex, kTimberNames) && in[i].c[1] > ground + kTimberOverFloorM)))
            put(i, ShellPart::Timber);
    // Walls, and the upright faces they carry above their eaves.
    std::vector<int> kept;
    for (std::size_t i = 0; i < n; ++i)
        if (!set[i] && in[i].up && in[i].lo[1] < eavesAt(in[i].c[0], in[i].c[2]) - kWallUnderEavesM)
        {
            put(i, ShellPart::Wall);
            kept.push_back(static_cast<int>(i));
        }
    auto planeD = [&](std::size_t i)
    { return in[i].nrm[0] * in[i].c[0] + in[i].nrm[1] * in[i].c[1] + in[i].nrm[2] * in[i].c[2]; };
    for (bool grew = true; grew;)
    {
        grew = false;
        for (std::size_t i = 0; i < n; ++i)
        {
            if (set[i] || !in[i].up)
                continue;
            const Info& q = in[i];
            const float d = planeD(i);
            for (const int ki : kept)
            {
                const Info& k = in[static_cast<std::size_t>(ki)];
                if (k.nrm[0] * q.nrm[0] + k.nrm[2] * q.nrm[2] < kCoplanarCos ||
                    std::fabs(planeD(static_cast<std::size_t>(ki)) - d) > kCoplanarM || k.hi[1] < q.lo[1] - kContactM)
                    continue;
                if (k.hi[0] < q.lo[0] - 0.05f || k.lo[0] > q.hi[0] + 0.05f || k.hi[2] < q.lo[2] - 0.05f ||
                    k.lo[2] > q.hi[2] + 0.05f)
                    continue;
                put(i, ShellPart::Carried);
                kept.push_back(static_cast<int>(i));
                grew = true;
                break;
            }
        }
    }
    // Upright parts left above the eaves: a part standing on kept faces is
    // carried, a chimney stack stands on the roof, the rest is loose.
    std::vector<int> loose;
    for (std::size_t i = 0; i < n; ++i)
        if (!set[i] && in[i].up)
            loose.push_back(static_cast<int>(i));
    std::vector<std::vector<int>> groups = Parts(faces, loose);
    auto box = [&](const std::vector<int>& g, float* lo, float* hi)
    {
        for (int a = 0; a < 3; ++a)
        {
            lo[a] = 1e9f;
            hi[a] = -1e9f;
        }
        for (const int i : g)
            for (int a = 0; a < 3; ++a)
            {
                lo[a] = std::min(lo[a], in[static_cast<std::size_t>(i)].lo[a]);
                hi[a] = std::max(hi[a], in[static_cast<std::size_t>(i)].hi[a]);
            }
    };
    // a stack's cap and body are one
    for (bool merged = true; merged;)
    {
        merged = false;
        for (std::size_t a = 0; a < groups.size() && !merged; ++a)
            for (std::size_t b = a + 1; b < groups.size() && !merged; ++b)
            {
                float al[3], ah[3], bl[3], bh[3];
                box(groups[a], al, ah);
                box(groups[b], bl, bh);
                if (std::max({ah[0] - al[0], ah[2] - al[2], bh[0] - bl[0], bh[2] - bl[2]}) > kStackMaxM)
                    continue;
                if (al[0] <= bh[0] && bl[0] <= ah[0] && al[2] <= bh[2] && bl[2] <= ah[2])
                {
                    groups[a].insert(groups[a].end(), groups[b].begin(), groups[b].end());
                    groups.erase(groups.begin() + static_cast<std::ptrdiff_t>(b));
                    merged = true;
                }
            }
    }
    std::vector<std::vector<int>> stacks;
    std::vector<std::pair<std::vector<int>, ShellPart>> capped; // a stack's or a carried stack's cap is its own
    for (const std::vector<int>& g : groups)
    {
        float lo[3], hi[3];
        box(g, lo, hi);
        const float sx = hi[0] - lo[0], sz = hi[2] - lo[2];
        bool carried = false;
        if (sx <= kStackMaxM && sz <= kStackMaxM)
            for (const int ki : kept)
            {
                const Info& k = in[static_cast<std::size_t>(ki)];
                if (k.hi[1] >= lo[1] - kContactM && k.lo[1] < lo[1] && k.c[0] >= lo[0] - 0.1f &&
                    k.c[0] <= hi[0] + 0.1f && k.c[2] >= lo[2] - 0.1f && k.c[2] <= hi[2] + 0.1f)
                {
                    carried = true;
                    break;
                }
            }
        if (carried)
        {
            for (const int i : g)
            {
                put(static_cast<std::size_t>(i), ShellPart::Carried);
                kept.push_back(i);
            }
            capped.emplace_back(g, ShellPart::Carried);
            continue;
        }
        int dirs = 0;
        bool seen[4] = {false, false, false, false};
        for (const int i : g)
        {
            const Info& q = in[static_cast<std::size_t>(i)];
            const int d =
                std::fabs(q.nrm[0]) > std::fabs(q.nrm[2]) ? (q.nrm[0] > 0.0f ? 0 : 1) : (q.nrm[2] > 0.0f ? 2 : 3);
            if (!seen[d])
            {
                seen[d] = true;
                ++dirs;
            }
        }
        const bool stack = !church && sx <= kStackMaxM && sz <= kStackMaxM && sx >= kStackMinM && sz >= kStackMinM &&
                           hi[1] - lo[1] >= kStackTallM &&
                           hi[1] >= std::max(eaves + kStackOverEavesM, ridge - kStackOverRidgeM) && dirs >= 3;
        for (const int i : g)
            put(static_cast<std::size_t>(i), stack ? ShellPart::Stack : ShellPart::Loose);
        if (stack)
        {
            stacks.push_back(g);
            capped.emplace_back(g, ShellPart::Stack);
        }
    }
    // A stack's cap (its flat top) is the stack's.
    for (const auto& [g, capPart] : capped)
    {
        float lo[3], hi[3];
        box(g, lo, hi);
        for (std::size_t i = 0; i < n; ++i)
        {
            const Info& q = in[i];
            if (set[i] || q.up || q.lo[1] < lo[1] - 0.05f || q.hi[1] > hi[1] + 0.05f)
                continue;
            if (q.c[0] < lo[0] - 0.05f || q.c[0] > hi[0] + 0.05f || q.c[2] < lo[2] - 0.05f || q.c[2] > hi[2] + 0.05f)
                continue;
            put(i, capPart);
        }
    }
    // The flat faces left: under the roof, upper floors, or kept.
    for (std::size_t i = 0; i < n; ++i)
    {
        if (set[i])
            continue;
        const Info& q = in[i];
        if (q.c[1] > eavesAt(q.c[0], q.c[2]) - kFlatOverEavesM)
            put(i, roof[i] ? ShellPart::Roof : ShellPart::Attic);
        else if (q.c[1] > ground + kUpperFloorM && inside(q))
            put(i, ShellPart::Floor);
        else
            put(i, ShellPart::Wall);
    }
    // Each stack's pier: its lowest band of sides, stepped down to the ground floor.
    for (const std::vector<int>& g : stacks)
    {
        float y0 = 1e9f;
        for (const int i : g)
            if (in[static_cast<std::size_t>(i)].up)
                y0 = std::min(y0, in[static_cast<std::size_t>(i)].lo[1]);
        float y1 = 1e9f;
        for (const int i : g)
            if (in[static_cast<std::size_t>(i)].up && in[static_cast<std::size_t>(i)].lo[1] < y0 + 0.1f)
                y1 = std::min(y1, in[static_cast<std::size_t>(i)].hi[1]);
        ShellPier pier;
        for (const int i : g)
        {
            const Info& q = in[static_cast<std::size_t>(i)];
            if (q.up && q.lo[1] < y0 + 0.1f && q.hi[1] <= y1 + 0.1f)
                pier.band.push_back(i);
        }
        pier.step = std::max(y1 - y0, 0.3f);
        while (pier.copies < kPierCopiesMax && y0 - static_cast<float>(pier.copies) * pier.step > ground)
            ++pier.copies;
        if (!pier.band.empty() && pier.copies > 0)
            plan.piers.push_back(std::move(pier));
    }
    // The kept walls' inner sides: kept upright faces turned away from the
    // walls' middle with no face of their own behind them.
    const float mx = 0.5f * (wx0 + wx1), mz = 0.5f * (wz0 + wz1);
    for (std::size_t i = 0; i < n; ++i)
    {
        const ShellPart p = plan.part[i];
        if (!in[i].use || !in[i].up || (p != ShellPart::Wall && p != ShellPart::Carried))
            continue;
        const Info& q = in[i];
        if (q.nrm[0] * (q.c[0] - mx) + q.nrm[2] * (q.c[2] - mz) <= 0.05f)
            continue;
        const float d = planeD(i);
        bool backed = false;
        for (std::size_t j = 0; j < n && !backed; ++j)
        {
            const Info& g = in[j];
            if (j == i || !g.use || !g.up || g.nrm[0] * q.nrm[0] + g.nrm[2] * q.nrm[2] > -kCoplanarCos ||
                std::fabs(planeD(j) + d) > 0.03f)
                continue;
            if (g.hi[0] < q.lo[0] || g.lo[0] > q.hi[0] || g.hi[2] < q.lo[2] || g.lo[2] > q.hi[2] || g.hi[1] < q.lo[1] ||
                g.lo[1] > q.hi[1])
                continue;
            backed = true;
        }
        if (!backed)
            plan.inner.push_back(static_cast<int>(i));
    }
    plan.footY = foot;
    plan.eavesY = eaves;
    plan.ridgeY = ridge;
    plan.groundY = ground;
    plan.wallBox[0] = wx0;
    plan.wallBox[1] = wx1;
    plan.wallBox[2] = wz0;
    plan.wallBox[3] = wz1;
    plan.ok = true;
    return plan;
}

// ---- The broken tops (v2: the owner's "these triangles") ----
namespace
{

constexpr float kCapDepthM = 0.45f; // a wall's broken top is rebuilt from this far under its lowest top
constexpr float kMasonryM = 0.45f;  // the rebuilt masonry's thickness, give or take kMasonryJitterM
constexpr float kMasonryJitterM = 0.05f;
constexpr float kColumnM = 0.38f;       // a column of masonry (one step of a broken edge) ...
constexpr float kColumnJitterM = 0.12f; // ... give or take this
constexpr float kCourseM = 0.25f;       // a course: the steps' height up a broken gable
constexpr float kGableRiseM = 1.0f;     // a wall rising this much over its lowest top carries a gable
constexpr float kOuterM = 0.8f;         // an outer wall's plane lies within this of the walls' plan edge
constexpr float kGroupMinM = 1.5f;      // narrower planes (a chimney's breast, a pier) keep their edges
constexpr float kWallBiteLoM = 0.1f, kWallBiteHiM = 0.3f; // a wall top's courses lost ...
constexpr float kDeepBiteLoM = 0.5f, kDeepBiteHiM = 1.0f; // ... and where a tie beam tore out
// The owner's choice (option 3): every gable is cut to a short ragged stump,
// keeping this share of its rise; now and then (kTallGableShare of the houses)
// one of a house's gables keeps most of its height as a broken stepped piece.
constexpr float kStumpKeepLo = 0.12f, kStumpKeepHi = 0.35f;
constexpr float kTallGableShare = 0.3f;
constexpr float kTallKeepLo = 0.6f, kTallKeepHi = 0.85f;
constexpr float kApexLoM = 0.2f, kApexHiM = 0.5f; // a tall piece's apex lost
constexpr float kPocketM = 0.22f;                 // a purlin pocket's side
constexpr float kStraightMaxM = 0.9f;             // no broken top runs straight further than this

std::uint32_t Mix(std::uint32_t a, std::uint32_t b)
{
    std::uint32_t h = a * 0x9E3779B1U ^ (b + 0x7F4A7C15U + (a << 6) + (a >> 2));
    h ^= h >> 16;
    h *= 0x85EBCA6BU;
    h ^= h >> 13;
    h *= 0xC2B2AE35U;
    h ^= h >> 16;
    return h;
}
float U01(std::uint32_t h)
{
    return static_cast<float>(h >> 8) * (1.0f / 16777216.0f);
}

// Value noise along a wall (knots every unit of x, smoothed between): a
// break that wanders as a broken wall does, not a row of teeth.
float SmoothNoise(std::uint32_t key, float x)
{
    const float fl = std::floor(x);
    const int i = static_cast<int>(fl);
    const float t = x - fl, s = t * t * (3.0f - 2.0f * t);
    const float a = U01(Mix(key, static_cast<std::uint32_t>(i + 100000)));
    const float b = U01(Mix(key, static_cast<std::uint32_t>(i + 100001)));
    return a + (b - a) * s;
}

struct Span
{
    float lo, hi;
};

using PlaneOutline = std::vector<std::vector<std::pair<float, float>>>; // a plane's faces in (u, y)
using OutlineBoxes = std::vector<std::array<float, 4>>;                 // each face's u from, u to, y from, y to

// What a wall covers along the vertical at u (merged spans, low to high).
// `box` (each face's extent, when known) passes over the faces u misses.
std::vector<Span> CoverAt(const PlaneOutline& poly, float u, const OutlineBoxes* box = nullptr)
{
    std::vector<Span> sp;
    for (std::size_t g = 0; g < poly.size(); ++g)
    {
        if (box && (u < (*box)[g][0] || u > (*box)[g][1]))
            continue;
        const auto& q = poly[g];
        float lo = 1e9f, hi = -1e9f;
        for (std::size_t k = 0; k < q.size(); ++k)
        {
            const auto& a = q[k];
            const auto& b = q[(k + 1) % q.size()];
            if ((a.first - u) * (b.first - u) > 0.0f || std::fabs(b.first - a.first) < 1e-6f)
                continue;
            const float t = (u - a.first) / (b.first - a.first);
            const float y = a.second + t * (b.second - a.second);
            lo = std::min(lo, y);
            hi = std::max(hi, y);
        }
        if (hi > lo + 1e-4f)
            sp.push_back({lo, hi});
    }
    std::sort(sp.begin(), sp.end(), [](const Span& a, const Span& b) { return a.lo < b.lo; });
    std::vector<Span> out;
    for (const Span& x : sp)
    {
        if (!out.empty() && x.lo <= out.back().hi + 0.02f)
            out.back().hi = std::max(out.back().hi, x.hi);
        else
            out.push_back(x);
    }
    return out;
}

float TopAt(const PlaneOutline& poly, float u, const OutlineBoxes* box = nullptr)
{
    const std::vector<Span> sp = CoverAt(poly, u, box);
    return sp.empty() ? -1e9f : sp.back().hi;
}

// One outer wall's plane: its faces in (u along it, y up) and its frame.
struct WallPlane
{
    std::vector<int> faces;
    std::vector<int> backing;  // the wall's own inner faces behind it (cut with it)
    float n[2] = {1.0f, 0.0f}; // outward (horizontal)
    float d = 0.0f;            // n . p of the plane
    std::uint32_t key = 0;     // the same wall in every level
    // (measured before the masonry is laid: its outline, its extent along
    // it, its lowest top and its peak)
    PlaneOutline poly;
    OutlineBoxes box; // (the tops agent) each face's extent in (u, y)
    float umin = 1e9f, umax = -1e9f, lowTop = 1e9f, peak = -1e9f, peakU = 0.0f;
    bool gable = false;
};

// ---- The broken line (the tops agent: Ragged and Restrained) ----
// A wall's broken top follows a line along it, not a row of columns: knots
// (u, y) by u, straight between them and flat past its ends.
struct Knot
{
    float u, y;
};
using Line = std::vector<Knot>;

constexpr ShellTopsStyle kShellTopsDefault = ShellTopsStyle::Ragged;
constexpr float kWalkOriginM = -48.0f; // every walk along a wall starts here (model space): each level breaks alike
constexpr float kKnotMergeM = 0.01f;   // knots closer than this along a wall are one
constexpr float kUnderBreakM = 0.2f;   // a wall is cut at least this far under the lowest of its broken line
// Ragged: the loss along a wall top (m: the mean it wanders about, drawn per
// wall, and its bounds), the corners holding best (the loss at a corner, a
// share of the wall's), and the deep bites (kDeepBiteLoM to kDeepBiteHiM).
constexpr float kRaggedMeanLoM = 0.15f, kRaggedMeanHiM = 0.22f;
constexpr float kRaggedLossLoM = 0.05f, kRaggedLossHiM = 0.32f;
constexpr float kCornerHoldLo = 0.3f, kCornerHoldHi = 0.55f;
constexpr float kCornerRunM = 0.75f;   // a corner holds over this
constexpr float kRaggedRunMaxM = 0.9f; // no straight run of a broken edge longer than this
constexpr float kCoarseTolM = 0.05f;   // a house's coarser levels leave out bends of the line smaller than this
// Ragged: a gable stump's fracture lines (shares of its rise) and their
// jitter; a tall piece's top (kTallKeepLo to kTallKeepHi, tilted by this share).
constexpr float kFractureLo = 0.05f, kFractureHi = 0.42f;
constexpr float kFractureJitterM = 0.14f;
constexpr float kTallTiltLo = 0.05f, kTallTiltHi = 0.17f;
// Ragged: the cap's inner edge and middle against its outer edge (m).
constexpr float kRaggedCapInLoM = -0.12f, kRaggedCapInHiM = 0.04f;
constexpr float kRaggedCapMidLoM = -0.07f, kRaggedCapMidHiM = 0.05f;
// Restrained: the cut under the eaves (m), its undulation and wavelength;
// a gable's cut (a share of its rise) and its diagonal across it (m).
constexpr float kGentleCutLoM = 0.12f, kGentleCutHiM = 0.2f;
constexpr float kGentleAmpLoM = 0.03f, kGentleAmpHiM = 0.05f;
constexpr float kGentleWaveLoM = 2.2f, kGentleWaveHiM = 3.6f;
constexpr float kGentleGableLo = 0.08f, kGentleGableHi = 0.2f;
constexpr float kGentleTiltLoM = 0.12f, kGentleTiltHiM = 0.35f;
constexpr float kGentleCapInLoM = -0.06f, kGentleCapInHiM = 0.02f;
constexpr float kGentleCapMidLoM = -0.03f, kGentleCapMidHiM = 0.03f;

float LineAt(const Line& l, float u)
{
    if (l.empty())
        return 0.0f;
    if (u <= l.front().u)
        return l.front().y;
    if (u >= l.back().u)
        return l.back().y;
    const auto it = std::upper_bound(l.begin(), l.end(), u, [](float x, const Knot& k) { return x < k.u; });
    const Knot& b = *it;
    const Knot& a = *(it - 1);
    const float du = b.u - a.u;
    return du > 1e-6f ? a.y + (b.y - a.y) * (u - a.u) / du : b.y;
}

// Two lines made one over [u0, u1], knot by knot: their sum, difference,
// product, least or most (a least or most also takes the points where the
// two cross, so it stays exact between its knots).
enum class LineOp : std::uint8_t
{
    Add,
    Sub,
    Mul,
    Min,
    Max,
};
Line Combine(const Line& a, const Line& b, LineOp op, float u0, float u1)
{
    std::vector<float> us{u0, u1};
    for (const Line* l : {&a, &b})
        for (const Knot& k : *l)
            if (k.u > u0 && k.u < u1)
                us.push_back(k.u);
    std::sort(us.begin(), us.end());
    us.erase(std::unique(us.begin(), us.end(), [](float x, float y) { return y - x < 1e-4f; }), us.end());
    auto f = [&](float u)
    {
        const float x = LineAt(a, u), y = LineAt(b, u);
        switch (op)
        {
            case LineOp::Add:
                return x + y;
            case LineOp::Sub:
                return x - y;
            case LineOp::Mul:
                return x * y;
            case LineOp::Min:
                return std::min(x, y);
            default:
                return std::max(x, y);
        }
    };
    Line out;
    out.reserve(us.size() + 8);
    for (std::size_t i = 0; i < us.size(); ++i)
    {
        if (i > 0 && (op == LineOp::Min || op == LineOp::Max))
        {
            const float ua = us[i - 1], ub = us[i];
            const float da = LineAt(a, ua) - LineAt(b, ua), db = LineAt(a, ub) - LineAt(b, ub);
            if ((da > 1e-5f && db < -1e-5f) || (da < -1e-5f && db > 1e-5f))
            {
                const float uc = ua + (ub - ua) * da / (da - db);
                out.push_back({uc, f(uc)});
            }
        }
        out.push_back({us[i], f(us[i])});
    }
    return out;
}

// What a broken top loses along a wall (Ragged), walked from kWalkOriginM so
// every level breaks alike: mostly short slopes of 0.3 to 1 m, shallow
// notches where a cluster of stones went, now and then a drop of a course or
// a longer run with a kink in it, so no straight edge runs much over a metre
// and no unit repeats. It wanders about `mean` within [lo, hi] (bounced off
// them, never held flat on them); `scale` sizes its steps.
Line RaggedWalk(std::uint32_t key, float u1, float mean, float lo, float hi, float scale)
{
    Line l;
    float u = kWalkOriginM, v = mean, slope = 0.0f;
    l.push_back({u, v});
    // the next knot du on, dv up or down (bounced off the bounds; never on
    // at the slope it came in on, or two stretches would read as one)
    auto go = [&](float du, float dv)
    {
        if (std::fabs(dv / du - slope) < 0.035f)
            dv = std::fabs(-dv / du - slope) >= 0.035f ? -dv : dv + 0.05f * du * (slope > 0.0f ? -1.0f : 1.0f);
        u += du;
        const float was = v;
        v += dv;
        if (v > hi)
            v = std::max(lo, 2.0f * hi - v);
        if (v < lo)
            v = std::min(hi, 2.0f * lo - v);
        slope = (v - was) / du;
        l.push_back({u, v});
    };
    for (std::uint32_t k = 0; u < u1 + 1.0f && k < 4000U; ++k)
    {
        auto r = [&](std::uint32_t salt) { return U01(Mix(key, k * 16U + salt)); };
        const float toward = v > mean ? -1.0f : 1.0f;
        const float sign = r(1) < 0.65f ? toward : -toward;
        const float pick = r(0);
        if (pick < 0.56f) // a slope
            go(0.3f + 0.7f * std::pow(r(2), 1.4f), sign * scale * (0.04f + 0.12f * r(3)));
        else if (pick < 0.8f) // a shallow notch: a cluster of stones gone
        {
            const float d = scale * (0.07f + 0.13f * r(3));
            go(0.08f + 0.17f * r(2), d);
            go(0.15f + 0.35f * r(4), scale * 0.06f * (r(5) - 0.5f));
            go(0.08f + 0.2f * r(6), -d * (0.6f + 0.6f * r(7)));
        }
        else if (pick < 0.89f) // a drop of a course, not quite plumb
            go(0.03f + 0.05f * r(2), sign * scale * (0.08f + 0.1f * r(3)));
        else // a longer run with a kink
        {
            go(0.4f + 0.35f * r(2), sign * scale * 0.04f * (0.5f + r(3)));
            go(0.4f + 0.35f * r(4), scale * 0.08f * (r(5) - 0.5f));
        }
    }
    return l;
}

// (Ragged) No straight run of a broken edge longer than `maxRun` (its
// stretches at one slope, joined): the middle of such a run drops a few
// centimetres (a stone more gone), until none is left.
Line Kinked(Line l, std::uint32_t key, float maxRun, float umin, float umax)
{
    auto slopeOf = [&](std::size_t i) { return (l[i + 1].y - l[i].y) / std::max(l[i + 1].u - l[i].u, 1e-5f); };
    for (int pass = 0; pass < 24; ++pass)
    {
        bool changed = false;
        std::size_t start = 0;
        while (start + 1 < l.size() && !changed)
        {
            // the run from start: its stretches at one slope
            std::size_t end = start + 1;
            while (end + 1 < l.size() && std::fabs(slopeOf(end) - slopeOf(start)) < 0.03f)
                ++end;
            const float du = l[end].u - l[start].u, dy = l[end].y - l[start].y;
            if (std::sqrt(du * du + dy * dy) > maxRun && std::min(l[end].u, umax) - std::max(l[start].u, umin) > 0.3f)
            {
                const float um = 0.5f * (std::max(l[start].u, umin) + std::min(l[end].u, umax));
                const float drop = 0.04f + 0.03f * U01(Mix(key, static_cast<std::uint32_t>(std::lround(um * 100.0f))));
                std::size_t best = 0;
                for (std::size_t k = start + 1; k < end; ++k)
                    if (best == 0 || std::fabs(l[k].u - um) < std::fabs(l[best].u - um))
                        best = k;
                if (best != 0 && std::fabs(l[best].u - um) < 0.2f * du)
                    l[best].y -= drop;
                else
                {
                    const auto at =
                        std::upper_bound(l.begin(), l.end(), um, [](float x, const Knot& k) { return x < k.u; });
                    l.insert(at, Knot{um, LineAt(l, um) - drop});
                }
                changed = true;
            }
            start = end;
        }
        if (!changed)
            break;
    }
    return l;
}

// A gentle line (Restrained): `mean` give or take `amp`, wandering over about
// `wave` metres (two octaves of value noise), a knot every 0.35 to 0.6 m.
Line GentleLine(std::uint32_t key, float u1, float mean, float amp, float wave)
{
    Line l;
    float u = kWalkOriginM;
    for (std::uint32_t k = 0; u < u1 + 1.0f && k < 4000U; ++k)
    {
        const float x = u - kWalkOriginM;
        const float nz = 0.8f * (2.0f * SmoothNoise(Mix(key, 1U), x / wave) - 1.0f) +
                         0.2f * (2.0f * SmoothNoise(Mix(key, 2U), x / (0.37f * wave)) - 1.0f);
        l.push_back({u, mean + amp * nz});
        u += 0.35f + 0.25f * U01(Mix(key, k + 7U));
    }
    return l;
}

// A tall gable piece's slopes (Ragged): its courses lost in irregular steps
// that climb with the old slope (`top`), treads of 0.18 to 0.58 m keeping a
// share of the slope, risers short and steep but not plumb.
Line SlopeSteps(std::uint32_t key, const Line& top, float umin, float umax, float peakU)
{
    Line side[2];
    for (int s = 0; s < 2; ++s)
    {
        Line& out = side[s];
        const float dir = s == 0 ? 1.0f : -1.0f;
        float u = s == 0 ? umin : umax;
        float y = LineAt(top, u) - (0.03f + 0.07f * U01(Mix(key, 900U + static_cast<std::uint32_t>(s))));
        out.push_back({u, y});
        for (std::uint32_t k = 0; (peakU - u) * dir > 0.05f && k < 200U; ++k)
        {
            auto r = [&](std::uint32_t salt)
            { return U01(Mix(key, (static_cast<std::uint32_t>(s) * 1000U + k) * 8U + salt)); };
            const float a = std::min(0.18f + 0.4f * r(1), (peakU - u) * dir);
            const float slope = (LineAt(top, u + dir * a) - LineAt(top, u)) / a;
            u += dir * a;
            y += (0.15f + 0.45f * r(2)) * slope * a;
            out.push_back({u, y});
            if ((peakU - u) * dir <= 0.05f)
                break;
            u += dir * (0.03f + 0.06f * r(3));
            y = std::max(y, LineAt(top, u) - (0.04f + 0.12f * r(4)));
            out.push_back({u, y});
        }
    }
    std::reverse(side[1].begin(), side[1].end());
    Line l = side[0];
    for (const Knot& k : side[1])
        if (l.empty() || k.u > l.back().u + 1e-4f)
            l.push_back(k);
    return l;
}

// (a house's coarser levels) The line with the knots that bend it by less
// than `tol` dropped (Douglas-Peucker on height).
Line Simplified(const Line& l, float tol)
{
    if (l.size() < 3)
        return l;
    std::vector<char> keep(l.size(), 0);
    keep.front() = 1;
    keep.back() = 1;
    std::vector<std::pair<std::size_t, std::size_t>> todo{{0, l.size() - 1}};
    while (!todo.empty())
    {
        const auto [a, b] = todo.back();
        todo.pop_back();
        float worst = 0.0f;
        std::size_t wi = 0;
        for (std::size_t k = a + 1; k < b; ++k)
        {
            const float f = (l[k].u - l[a].u) / std::max(l[b].u - l[a].u, 1e-6f);
            const float d = std::fabs(l[k].y - (l[a].y + f * (l[b].y - l[a].y)));
            if (d > worst)
            {
                worst = d;
                wi = k;
            }
        }
        if (worst > tol)
        {
            keep[wi] = 1;
            todo.push_back({a, wi});
            todo.push_back({wi, b});
        }
    }
    Line out;
    for (std::size_t i = 0; i < l.size(); ++i)
        if (keep[i])
            out.push_back(l[i]);
    return out;
}

// A purlin's pocket in a tall gable piece: its middle (u, y).
struct Pocket
{
    float u, y;
};

// One plane's broken line (Ragged, Restrained): the cap's outer edge (the
// wall's face), its middle and its inner edge, the pockets, and the lowest
// any of them goes.
struct BrokenTop
{
    Line out, mid, in;
    std::vector<Pocket> pockets;
    float lowest = 1e9f;
};

BrokenTop BrokenTopOf(const WallPlane& w, bool tall, std::uint32_t wk, ShellTopsStyle style, bool coarse)
{
    BrokenTop bt;
    const float umin = w.umin, umax = w.umax, lowTop = w.lowTop, peak = w.peak, peakU = w.peakU;
    const float wid = umax - umin, rise = std::max(peak - lowTop, 0.0f);
    const float a0 = umin - 0.5f, a1 = umax + 0.5f;
    auto r = [&](std::uint32_t salt) { return U01(Mix(wk, salt)); };
    // The plane's own top as a line: a knot at each corner of its outline
    // (a step in it, two knots close together).
    Line top;
    {
        std::vector<float> us;
        for (std::size_t g = 0; g < w.poly.size(); ++g)
        {
            // (a face lower than the plane's lowest top is never its top)
            if (w.box[g][3] < lowTop - 0.01f)
                continue;
            for (const auto& c : w.poly[g])
                if (c.first >= umin - 1e-3f && c.first <= umax + 1e-3f)
                    us.push_back(c.first);
        }
        us.push_back(umin);
        us.push_back(umax);
        std::sort(us.begin(), us.end());
        us.erase(std::unique(us.begin(), us.end(), [](float x, float y) { return y - x < 0.005f; }), us.end());
        for (const float u : us)
        {
            const float ta = TopAt(w.poly, std::max(u - 0.002f, umin + 1e-4f), &w.box);
            const float tb = TopAt(w.poly, std::min(u + 0.002f, umax - 1e-4f), &w.box);
            const float ya = ta > -1e8f ? ta : lowTop, yb = tb > -1e8f ? tb : lowTop;
            if (std::fabs(ya - yb) > 0.02f)
            {
                top.push_back({u - 0.002f, ya});
                top.push_back({u + 0.002f, yb});
            }
            else
                top.push_back({u, 0.5f * (ya + yb)});
        }
        if (top.empty())
            top = {{umin, lowTop}, {umax, lowTop}};
    }
    Line out, capBase;
    float inLo, inHi, midLo, midHi;
    if (style == ShellTopsStyle::Ragged)
    {
        inLo = kRaggedCapInLoM;
        inHi = kRaggedCapInHiM;
        midLo = kRaggedCapMidLoM;
        midHi = kRaggedCapMidHiM;
        if (!w.gable)
        {
            const float mean = kRaggedMeanLoM + (kRaggedMeanHiM - kRaggedMeanLoM) * r(50);
            const Line walk = RaggedWalk(Mix(wk, 51), a1, mean, kRaggedLossLoM, kRaggedLossHiM, 1.0f);
            // the corners hold best (two walls bind there)
            const float hold = kCornerHoldLo + (kCornerHoldHi - kCornerHoldLo) * r(52);
            Line ramp;
            if (wid > 2.0f * kCornerRunM)
                ramp = {{umin, hold},
                        {umin + 0.4f * kCornerRunM, 0.5f + 0.5f * hold},
                        {umin + kCornerRunM, 1.0f},
                        {umax - kCornerRunM, 1.0f},
                        {umax - 0.4f * kCornerRunM, 0.5f + 0.5f * hold},
                        {umax, hold}};
            else
                ramp = {{umin, hold}, {0.5f * (umin + umax), 1.0f}, {umax, hold}};
            Line loss = Combine(walk, ramp, LineOp::Mul, a0, a1);
            // A long wall's deep bites (a tie beam torn out): an uneven V,
            // its sides steep but sloping, its bottom tilted.
            const int nb = (wid > 5.0f ? 1 : 0) + (wid > 9.0f ? 1 : 0);
            for (int j = 0; j < nb; ++j)
            {
                const std::uint32_t b = 60U + static_cast<std::uint32_t>(j) * 10U;
                const float from = nb == 2 ? (j == 0 ? 0.12f : 0.55f) : 0.15f, span = nb == 2 ? 0.33f : 0.7f;
                const float ub = umin + (from + span * r(b)) * wid;
                const float deep = kDeepBiteLoM + (kDeepBiteHiM - kDeepBiteLoM) * r(b + 2);
                const float h = 0.2f + (0.6f + 0.4f * r(b + 1)) * deep; // half its width (its sides under about 2:1)
                const float s = r(b + 3) < 0.5f ? 1.0f : -1.0f;
                Line bite = {{ub - s * h, 0.0f},
                             {ub - s * h * (0.3f + 0.25f * r(b + 4)), deep * (0.45f + 0.35f * r(b + 5))},
                             {ub - s * h * 0.25f * r(b + 6), deep},
                             {ub + s * h * (0.2f + 0.3f * r(b + 7)), deep * (0.6f + 0.3f * r(b + 8))},
                             {ub + s * h, 0.0f}};
                if (s < 0.0f)
                    std::reverse(bite.begin(), bite.end());
                // (its sides and bottom broken too: a stone or two more gone
                // or left halfway along each)
                Line rough{{a0, 0.0f}};
                for (std::size_t k = 0; k < bite.size(); ++k)
                {
                    if (k > 0)
                    {
                        const std::uint32_t c = b * 16U + static_cast<std::uint32_t>(k);
                        const float f = 0.35f + 0.3f * U01(Mix(wk, c));
                        const float y =
                            bite[k - 1].y + f * (bite[k].y - bite[k - 1].y) +
                            (0.03f + 0.06f * U01(Mix(wk, c + 7U))) * (U01(Mix(wk, c + 11U)) < 0.5f ? -1.0f : 1.0f);
                        rough.push_back({bite[k - 1].u + f * (bite[k].u - bite[k - 1].u), std::clamp(y, 0.0f, deep)});
                    }
                    rough.push_back(bite[k]);
                }
                rough.push_back({a1, 0.0f});
                loss = Combine(loss, rough, LineOp::Max, a0, a1);
            }
            out = Kinked(Combine(top, loss, LineOp::Sub, a0, a1), Mix(wk, 99), kRaggedRunMaxM, umin, umax);
            capBase = walk;
        }
        else
        {
            // Its sloping edges ragged where the verge went, and broken
            // across along one or two diagonal fracture lines (a stump), or
            // (the house's tall piece) kept to most of its height, its slopes
            // lost in irregular steps and its apex gone.
            const Line edge = RaggedWalk(Mix(wk, 70), a1, tall ? 0.07f : 0.12f, 0.03f, 0.26f, tall ? 0.6f : 0.9f);
            const Line jit = RaggedWalk(Mix(wk, 71), a1, 0.0f, -kFractureJitterM, kFractureJitterM, 1.0f);
            auto at = [&](float k) { return lowTop + std::clamp(k, 0.0f, 1.0f) * rise; };
            Line frac;
            float apexY = 1e9f;
            if (tall)
            {
                const float keep = kTallKeepLo + (kTallKeepHi - kTallKeepLo) * r(72);
                const float tilt = (kTallTiltLo + (kTallTiltHi - kTallTiltLo) * r(73)) * (r(74) < 0.5f ? -1.0f : 1.0f);
                apexY = peak - (kApexLoM + (kApexHiM - kApexLoM) * r(75));
                frac = {{a0, at(keep - tilt)}, {a1, at(keep + tilt)}};
            }
            else if (r(76) < 0.5f)
            {
                float ka = kFractureLo + 0.15f * r(77), kb = 0.22f + 0.16f * r(78);
                if (r(79) < 0.5f)
                    std::swap(ka, kb);
                frac = {{a0, at(ka)}, {a1, at(kb)}};
            }
            else
            {
                // (two lines, never a rounded hump: one diagonal kinked
                // steeper partway, or down one way and a little up the other)
                const float um = umin + (0.3f + 0.4f * r(80)) * wid;
                float ka, km, kb;
                if (r(81) < 0.45f)
                {
                    ka = 0.3f + (kFractureHi - 0.3f) * r(82);
                    km = 0.17f + 0.1f * r(83);
                    kb = kFractureLo + 0.07f * r(84);
                }
                else
                {
                    ka = 0.26f + 0.14f * r(82);
                    km = kFractureLo + 0.1f * r(83);
                    kb = 0.13f + 0.12f * r(84);
                }
                if (r(85) < 0.5f)
                    std::swap(ka, kb);
                frac = {{a0, at(ka)}, {um, at(km)}, {a1, at(kb)}};
            }
            out = Combine(Combine(top, edge, LineOp::Sub, a0, a1), Combine(frac, jit, LineOp::Sub, a0, a1), LineOp::Min,
                          a0, a1);
            if (tall)
            {
                out = Combine(out, SlopeSteps(Mix(wk, 86), top, umin, umax, peakU), LineOp::Min, a0, a1);
                out = Combine(out, {{a0, apexY}, {a1, apexY}}, LineOp::Min, a0, a1); // its apex gone
            }
            out = Kinked(std::move(out), Mix(wk, 99), kRaggedRunMaxM, umin, umax);
            capBase = jit;
        }
    }
    else
    {
        inLo = kGentleCapInLoM;
        inHi = kGentleCapInHiM;
        midLo = kGentleCapMidLoM;
        midHi = kGentleCapMidHiM;
        if (!w.gable)
        {
            const Line loss = GentleLine(Mix(wk, 51), a1, kGentleCutLoM + (kGentleCutHiM - kGentleCutLoM) * r(50),
                                         kGentleAmpLoM + (kGentleAmpHiM - kGentleAmpLoM) * r(52),
                                         kGentleWaveLoM + (kGentleWaveHiM - kGentleWaveLoM) * r(53));
            out = Combine(top, loss, LineOp::Sub, a0, a1);
            capBase = loss;
        }
        else
        {
            const Line edge = GentleLine(Mix(wk, 70), a1, 0.07f, 0.02f, 1.6f);
            const Line und = GentleLine(Mix(wk, 71), a1, 0.0f, 0.03f, 1.8f);
            const float k0 = kGentleGableLo + (kGentleGableHi - kGentleGableLo) * r(72);
            const float tilt = (kGentleTiltLoM + (kGentleTiltHiM - kGentleTiltLoM) * r(73)) *
                               (r(74) < 0.5f ? -1.0f : 1.0f) * (a1 - a0) / std::max(wid, 1.0f);
            const Line frac = {{a0, lowTop + k0 * rise - 0.5f * tilt}, {a1, lowTop + k0 * rise + 0.5f * tilt}};
            out = Combine(Combine(top, edge, LineOp::Sub, a0, a1), Combine(frac, und, LineOp::Add, a0, a1), LineOp::Min,
                          a0, a1);
            capBase = und;
        }
    }
    // (a coarser level: the same line with its smallest bends left out)
    if (coarse)
    {
        out = Combine(Simplified(out, kCoarseTolM), Combine(top, {{a0, 0.01f}, {a1, 0.01f}}, LineOp::Sub, a0, a1),
                      LineOp::Min, a0, a1);
        capBase = out; // (the cap's unevenness at the same knots: none added)
    }
    // The cap over the thickness: its middle and inner edge a little above
    // or below its outer edge, knot by knot (never above the old top).
    Line dIn, dMid;
    for (std::size_t i = 0; i < capBase.size(); ++i)
    {
        const std::uint32_t k = static_cast<std::uint32_t>(i);
        dIn.push_back({capBase[i].u, inLo + (inHi - inLo) * U01(Mix(wk, 3000U + k))});
        dMid.push_back({capBase[i].u, midLo + (midHi - midLo) * U01(Mix(wk, 9000U + k))});
    }
    bt.out = std::move(out);
    bt.mid = Combine(Combine(bt.out, dMid, LineOp::Add, a0, a1), top, LineOp::Min, a0, a1);
    bt.in = Combine(Combine(bt.out, dIn, LineOp::Add, a0, a1), top, LineOp::Min, a0, a1);
    for (const Line* l : {&bt.out, &bt.mid, &bt.in})
    {
        bt.lowest = std::min({bt.lowest, LineAt(*l, umin), LineAt(*l, umax)});
        for (const Knot& k : *l)
            if (k.u > umin && k.u < umax)
                bt.lowest = std::min(bt.lowest, k.y);
    }
    // A tall piece keeps its purlins' pockets: the ridge's under its broken
    // top, and on each slope at about a third and two thirds of its height,
    // 0.75 m under the slope.
    if (tall)
    {
        const float pieceTop = LineAt(bt.out, peakU);
        std::vector<Pocket> all;
        if (pieceTop - 1.0f > lowTop + 0.4f)
            all.push_back({peakU, pieceTop - 1.0f});
        for (const float frac : {0.35f, 0.65f})
        {
            const float y = lowTop + frac * (pieceTop - lowTop);
            for (const float dir : {-1.0f, 1.0f})
                for (float u = peakU; u > umin + 0.3f && u < umax - 0.3f; u += dir * 0.05f)
                    if (LineAt(bt.out, u) <= y + 0.75f)
                    {
                        all.push_back({u - dir * 0.05f, y});
                        break;
                    }
        }
        const int want = 3 + static_cast<int>(r(40) * 2.99f);
        for (std::size_t j = 0; j < all.size() && static_cast<int>(bt.pockets.size()) < want; ++j)
            bt.pockets.push_back(all[j]);
    }
    return bt;
}

// The masonry laid to a plane's broken line (Ragged, Restrained): along the
// wall, between the knots of its line and the corners of its outline, each
// span the wall covers above the cut stands as thick as the wall, its face
// and its inner side up to the line, its cap over the thickness following
// the line (a little uneven across), its underside at the cut or over an
// opening; ends only where the wall itself ends (its corners, its openings'
// jambs, a pocket's sides): no steps of its own. `crease`: the cap bends
// along its middle too (a house's finest level).
void LayBrokenMasonry(const WallPlane& w, const BrokenTop& bt, float yCut, float thick, bool crease, ShellTops& tops)
{
    const float umin = w.umin, umax = w.umax;
    const float eu[2] = {-w.n[1], w.n[0]};
    const float base[3] = {w.n[0] * w.d, 0.0f, w.n[1] * w.d};
    auto at = [&](float u, float y, float depth, float* o)
    {
        o[0] = base[0] + eu[0] * u - w.n[0] * depth;
        o[1] = y;
        o[2] = base[2] + eu[1] * u - w.n[1] * depth;
    };
    // the face whose texture a piece of masonry takes
    auto source = [&](float u, float y)
    {
        int best = w.faces.front();
        float bestD = 1e9f;
        for (std::size_t g = 0; g < w.box.size(); ++g)
        {
            const auto& bx = w.box[g];
            const float dd = std::max({bx[0] - u, u - bx[1], 0.0f}) + std::max({bx[2] - y, y - bx[3], 0.0f});
            if (dd < bestD)
            {
                bestD = dd;
                best = w.faces[g];
            }
        }
        return best;
    };
    // One face: corners (u, y, depth into the wall) round it, the points
    // (u, y) on the wall's plane whose texture they take; its normal from its
    // corners, turned toward `hint` (corners that meet are one).
    auto put = [&](int nc, const float (&c)[4][3], const float (&t)[4][2], float hx, float hy, float hz, int src)
    {
        ShellQuad q;
        q.src = src;
        int m = 0;
        for (int k = 0; k < nc; ++k)
        {
            float p[3];
            at(c[k][0], c[k][1], c[k][2], p);
            if (m > 0 &&
                std::fabs(p[0] - q.p[m - 1][0]) + std::fabs(p[1] - q.p[m - 1][1]) + std::fabs(p[2] - q.p[m - 1][2]) <
                    1e-4f)
                continue;
            std::copy(p, p + 3, q.p[m]);
            at(t[k][0], t[k][1], 0.0f, q.tp[m]);
            ++m;
        }
        if (m > 1 && std::fabs(q.p[0][0] - q.p[m - 1][0]) + std::fabs(q.p[0][1] - q.p[m - 1][1]) +
                             std::fabs(q.p[0][2] - q.p[m - 1][2]) <
                         1e-4f)
            --m;
        if (m < 3)
            return;
        float nx = 0.0f, ny = 0.0f, nz = 0.0f;
        for (int k = 0; k < m; ++k)
        {
            const float* a = q.p[k];
            const float* b = q.p[(k + 1) % m];
            nx += (a[1] - b[1]) * (a[2] + b[2]);
            ny += (a[2] - b[2]) * (a[0] + b[0]);
            nz += (a[0] - b[0]) * (a[1] + b[1]);
        }
        const float l = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (l < 2e-5f)
            return;
        const float sgn = nx * hx + ny * hy + nz * hz < 0.0f ? -1.0f : 1.0f;
        q.nrm[0] = sgn * nx / l;
        q.nrm[1] = sgn * ny / l;
        q.nrm[2] = sgn * nz / l;
        q.n = m;
        tops.quads.push_back(q);
    };
    const float t = thick, th = 0.5f * thick;
    const float ox = w.n[0], oz = w.n[1];
    // The knots: the line's, the corners of the outline above the cut, the
    // pockets' sides.
    std::vector<float> us{umin, umax};
    for (const Line* l : {&bt.out, &bt.mid, &bt.in})
        for (const Knot& k : *l)
            if (k.u > umin && k.u < umax)
                us.push_back(k.u);
    for (std::size_t g = 0; g < w.poly.size(); ++g)
    {
        if (w.box[g][3] <= yCut + 0.01f)
            continue;
        for (const auto& c : w.poly[g])
            if (c.first > umin && c.first < umax)
                us.push_back(c.first);
    }
    for (const Pocket& pk : bt.pockets)
        for (const float b : {pk.u - 0.5f * kPocketM, pk.u + 0.5f * kPocketM})
            if (b > umin && b < umax)
                us.push_back(b);
    auto knotsOf = [&](std::vector<float> v)
    {
        std::sort(v.begin(), v.end());
        std::vector<float> o;
        for (const float u : v)
            if (o.empty() || u - o.back() >= kKnotMergeM)
                o.push_back(u);
        if (o.size() > 1 && umax - o.back() < kKnotMergeM)
            o.back() = umax;
        else if (o.back() < umax)
            o.push_back(umax);
        return o;
    };
    std::vector<float> ks = knotsOf(us);
    // (where the line crosses an opening's sill or head the wall changes
    // there too: a knot)
    {
        std::vector<float> more = ks;
        for (std::size_t i = 0; i + 1 < ks.size(); ++i)
        {
            const float u0 = ks[i], u1 = ks[i + 1];
            const float p0 = LineAt(bt.out, u0), p1 = LineAt(bt.out, u1);
            for (const Span& s : CoverAt(w.poly, 0.5f * (u0 + u1), &w.box))
                for (const float y : {std::max(s.lo, yCut), s.hi})
                    if ((p0 - y) * (p1 - y) < 0.0f)
                    {
                        const float uc = u0 + (u1 - u0) * (p0 - y) / (p0 - p1);
                        if (uc > u0 + kKnotMergeM && uc < u1 - kKnotMergeM)
                            more.push_back(uc);
                    }
        }
        ks = knotsOf(more);
    }
    // Each stretch's spans, at its two ends.
    struct End
    {
        float lo, to, tm, ti; // its foot, and its top at the face, the middle and the inner side
    };
    struct Stretch
    {
        float u0, u1;
        std::vector<End> e0, e1;
    };
    std::vector<Stretch> st;
    std::vector<char> pocketCut(bt.pockets.size(), 0);
    // the underside along the cut, run on over the stretches standing on it
    struct Under
    {
        float u0, u1;
        int src;
    } under{0.0f, 0.0f, -1};
    auto flushUnder = [&]
    {
        if (under.src >= 0)
            put(4, {{under.u1, yCut, 0}, {under.u0, yCut, 0}, {under.u0, yCut, t}, {under.u1, yCut, t}},
                {{under.u1, yCut}, {under.u0, yCut}, {under.u0, yCut + t}, {under.u1, yCut + t}}, 0.0f, -1.0f, 0.0f,
                under.src);
        under.src = -1;
    };
    for (std::size_t i = 0; i + 1 < ks.size(); ++i)
    {
        Stretch s{ks[i], ks[i + 1], {}, {}};
        const float u0 = s.u0, u1 = s.u1, du = u1 - u0, uc = 0.5f * (u0 + u1);
        // the old top at the ends (a step in it stays on its own side)
        const float q1 = TopAt(w.poly, u0 + 0.25f * du, &w.box), q3 = TopAt(w.poly, u0 + 0.75f * du, &w.box);
        const std::vector<Span> cov = CoverAt(w.poly, uc, &w.box);
        const float P[2] = {LineAt(bt.out, u0), LineAt(bt.out, u1)};
        const float N[2] = {LineAt(bt.in, u0), LineAt(bt.in, u1)};
        const float M[2] = {crease ? LineAt(bt.mid, u0) : 0.5f * (P[0] + N[0]),
                            crease ? LineAt(bt.mid, u1) : 0.5f * (P[1] + N[1])};
        const float pc = LineAt(bt.out, uc);
        std::vector<std::pair<Span, bool>> spans; // a span, and whether it is the top one (to the old outline's top)
        bool atCut = false;                       // (one stands on the cut)
        for (std::size_t j = 0; j < cov.size(); ++j)
        {
            const Span& c = cov[j];
            const float lo = std::max(c.lo, yCut);
            if (std::min(c.hi, pc) <= lo + 0.01f)
                continue;
            spans.push_back({{lo, c.hi}, j + 1 == cov.size()});
        }
        // a pocket in this stretch: its hole cut out of the spans
        for (std::size_t p = 0; p < bt.pockets.size(); ++p)
        {
            const Pocket& pk = bt.pockets[p];
            if (std::fabs(uc - pk.u) > 0.5f * kPocketM - 0.005f || pc < pk.y + 0.5f * kPocketM + 0.15f)
                continue;
            const float plo = pk.y - 0.5f * kPocketM, phi = pk.y + 0.5f * kPocketM;
            std::vector<std::pair<Span, bool>> cut;
            for (const auto& sp : spans)
            {
                if (sp.first.hi <= plo || sp.first.lo >= phi)
                {
                    cut.push_back(sp);
                    continue;
                }
                if (sp.first.lo < plo)
                    cut.push_back({{sp.first.lo, plo}, false});
                if (sp.first.hi > phi)
                    cut.push_back({{phi, sp.first.hi}, sp.second});
            }
            spans = cut;
            pocketCut[p] = 1;
        }
        for (const auto& [sp, topSpan] : spans)
        {
            const float lo = sp.lo;
            // its own top at the ends: the old top for the top span, else flat
            float hi[2] = {sp.hi, sp.hi};
            if (topSpan && q1 > -1e8f && q3 > -1e8f)
            {
                hi[0] = q1 - 0.5f * (q3 - q1);
                hi[1] = q3 + 0.5f * (q3 - q1);
            }
            const bool broken = pc < 0.5f * (hi[0] + hi[1]) - 0.005f;
            End e[2];
            for (int k = 0; k < 2; ++k)
            {
                e[k].lo = lo;
                e[k].to = std::max(lo, broken ? std::min(P[k], hi[k]) : hi[k]);
                e[k].tm = std::max(lo, broken ? std::min(M[k], hi[k]) : hi[k]);
                e[k].ti = std::max(lo, broken ? std::min(N[k], hi[k]) : hi[k]);
            }
            if (std::max(e[0].to, e[1].to) <= lo + 0.01f)
                continue;
            s.e0.push_back(e[0]);
            s.e1.push_back(e[1]);
            const int src = source(uc, 0.5f * (lo + 0.5f * (e[0].to + e[1].to)));
            // its face, its inner side, its underside (one along the cut for
            // the stretches standing on it side by side)
            put(4, {{u0, lo, 0}, {u1, lo, 0}, {u1, e[1].to, 0}, {u0, e[0].to, 0}},
                {{u0, lo}, {u1, lo}, {u1, e[1].to}, {u0, e[0].to}}, ox, 0.0f, oz, src);
            put(4, {{u1, lo, t}, {u0, lo, t}, {u0, e[0].ti, t}, {u1, e[1].ti, t}},
                {{u1, lo}, {u0, lo}, {u0, e[0].ti}, {u1, e[1].ti}}, -ox, 0.0f, -oz, src);
            if (lo <= yCut + 1e-4f)
            {
                atCut = true;
                if (under.src < 0 || under.u1 < u0 - 1e-4f)
                {
                    flushUnder();
                    under = {u0, u1, src};
                }
                else
                    under.u1 = u1;
            }
            else
                put(4, {{u1, lo, 0}, {u0, lo, 0}, {u0, lo, t}, {u1, lo, t}},
                    {{u1, lo}, {u0, lo}, {u0, lo + t}, {u1, lo + t}}, 0.0f, -1.0f, 0.0f, src);
            // its cap: from the face's edge (over the middle) to the inner
            // side's (each point takes the wall's texture as far under the
            // face's top as it lies in from the face)
            if (broken)
            {
                const float o0[3] = {u0, e[0].to, 0}, o1[3] = {u1, e[1].to, 0};
                const float m0[3] = {u0, e[0].tm, th}, m1[3] = {u1, e[1].tm, th};
                const float i0[3] = {u0, e[0].ti, t}, i1[3] = {u1, e[1].ti, t};
                auto tri = [&](const float* a, const float* b, const float* c)
                {
                    put(3, {{a[0], a[1], a[2]}, {b[0], b[1], b[2]}, {c[0], c[1], c[2]}, {0, 0, 0}},
                        {{a[0], a[1] - a[2]}, {b[0], b[1] - b[2]}, {c[0], c[1] - c[2]}, {0, 0}}, 0.0f, 1.0f, 0.0f, src);
                };
                if (crease)
                {
                    tri(o0, o1, m1);
                    tri(o0, m1, m0);
                    tri(m0, m1, i1);
                    tri(m0, i1, i0);
                }
                else
                {
                    tri(o0, o1, i1);
                    tri(o0, i1, i0);
                }
            }
            else
                put(4, {{u0, e[0].to, 0}, {u1, e[1].to, 0}, {u1, e[1].to, t}, {u0, e[0].to, t}},
                    {{u0, e[0].to}, {u1, e[1].to}, {u1, e[1].to - t}, {u0, e[0].to - t}}, 0.0f, 1.0f, 0.0f, src);
        }
        if (!atCut)
            flushUnder();
        st.push_back(std::move(s));
    }
    flushUnder();
    for (const char c : pocketCut)
        tops.pockets += c;
    // The ends: where a stretch has masonry its neighbour has not (the
    // plane's ends, an opening's jamb, a pocket's side, a step in the old
    // top), faced along the wall.
    auto ends = [&](const std::vector<End>& from, const std::vector<End>* minus, float ub, float dir, float uc)
    {
        for (const End& e : from)
        {
            std::vector<std::pair<float, float>> pcs{{e.lo, e.to}};
            if (minus)
                for (const End& m : *minus)
                {
                    std::vector<std::pair<float, float>> next;
                    for (const auto& [a, b] : pcs)
                    {
                        if (m.to <= a || m.lo >= b)
                        {
                            next.push_back({a, b});
                            continue;
                        }
                        if (m.lo > a)
                            next.push_back({a, m.lo});
                        if (m.to < b)
                            next.push_back({m.to, b});
                    }
                    pcs = std::move(next);
                }
            for (const auto& [a, b] : pcs)
            {
                if (b - a < 0.01f)
                    continue;
                const bool own = b >= e.to - 1e-4f;
                const float to = own ? e.to : b, tm = own ? e.tm : b, ti = own ? e.ti : b;
                const int src = source(uc, 0.5f * (a + b));
                const float c0[3] = {ub, a, 0}, ct[3] = {ub, a, t}, ci[3] = {ub, ti, t}, cm[3] = {ub, tm, th},
                            co[3] = {ub, to, 0};
                auto tri = [&](const float* p, const float* q, const float* s)
                {
                    put(3, {{p[0], p[1], p[2]}, {q[0], q[1], q[2]}, {s[0], s[1], s[2]}, {0, 0, 0}},
                        {{ub - dir * p[2], p[1]}, {ub - dir * q[2], q[1]}, {ub - dir * s[2], s[1]}, {0, 0}},
                        dir * eu[0], 0.0f, dir * eu[1], src);
                };
                tri(c0, ct, ci);
                tri(c0, ci, cm);
                tri(c0, cm, co);
            }
        }
    };
    for (std::size_t i = 0; i <= st.size(); ++i)
    {
        const Stretch* l = i > 0 ? &st[i - 1] : nullptr;
        const Stretch* rr = i < st.size() ? &st[i] : nullptr;
        if (l)
            ends(l->e1, rr ? &rr->e0 : nullptr, l->u1, 1.0f, 0.5f * (l->u0 + l->u1));
        if (rr)
            ends(rr->e0, l ? &l->e1 : nullptr, rr->u0, -1.0f, 0.5f * (rr->u0 + rr->u1));
    }
}

ShellTopsStyle StyleFromEnv()
{
    const char* v = std::getenv("POSEIDON_XP_FIRESPREAD_SHELLTOPS");
    if (v && v[0] >= '0' && v[0] <= '2' && v[1] == '\0')
        return static_cast<ShellTopsStyle>(v[0] - '0');
    return kShellTopsDefault;
}

} // namespace

ShellTopsStyle ShellTopsStyleNow()
{
    static const ShellTopsStyle style = []
    {
        const ShellTopsStyle s = StyleFromEnv();
        LOG_INFO(Script, "UDFIRE;event=shelltops;style={}", static_cast<int>(s));
        return s;
    }();
    return style;
}

ShellTops PlanBurntTops(const std::vector<ShellFace>& faces, const ShellPlan& plan, std::uint32_t seed,
                        ShellTopsStyle style, bool coarse)
{
    ShellTops tops;
    if (!plan.ok)
        return tops;
    const std::size_t n = faces.size();
    const float mx = 0.5f * (plan.wallBox[0] + plan.wallBox[1]), mz = 0.5f * (plan.wallBox[2] + plan.wallBox[3]);
    // Kept upright faces of the outer walls, by plane.
    std::vector<WallPlane> planes;
    std::vector<float> cx(n), cz(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const ShellFace& f = faces[i];
        if (f.hidden || f.n < 3 || f.n > 4 || (plan.part[i] != ShellPart::Wall && plan.part[i] != ShellPart::Carried))
            continue;
        const float l = std::sqrt(f.nrm[0] * f.nrm[0] + f.nrm[1] * f.nrm[1] + f.nrm[2] * f.nrm[2]);
        if (l < 1e-6f || std::fabs(f.nrm[1] / l) > kUprightNy)
            continue;
        const float h = std::sqrt(f.nrm[0] * f.nrm[0] + f.nrm[2] * f.nrm[2]);
        const float nx = f.nrm[0] / h, nz = f.nrm[2] / h;
        float c[3] = {0.0f, 0.0f, 0.0f};
        for (int k = 0; k < f.n; ++k)
            for (int a = 0; a < 3; ++a)
                c[a] += f.p[k][a] / static_cast<float>(f.n);
        if (nx * (c[0] - mx) + nz * (c[2] - mz) <= 0.05f)
            continue;
        const float d = nx * c[0] + nz * c[2];
        float ext = -1e9f;
        for (int k = 0; k < 4; ++k)
            ext = std::max(ext, nx * plan.wallBox[k & 1] + nz * plan.wallBox[2 + (k >> 1)]);
        if (ext - d > kOuterM)
            continue;
        WallPlane* wp = nullptr;
        for (WallPlane& w : planes)
            if (w.n[0] * nx + w.n[1] * nz >= kCoplanarCos && std::fabs(w.d - d) <= kCoplanarM)
                wp = &w;
        if (!wp)
        {
            planes.emplace_back();
            wp = &planes.back();
            wp->n[0] = nx;
            wp->n[1] = nz;
            wp->d = d;
            const int sector = static_cast<int>(std::lround(std::atan2(nz, nx) * (16.0f / 3.14159265f)));
            wp->key =
                Mix(static_cast<std::uint32_t>(sector + 64), static_cast<std::uint32_t>(std::lround(d * 2.0f) + 4096));
        }
        wp->faces.push_back(static_cast<int>(i));
    }
    // A wall modelled with its thickness has its inner face behind it (a
    // gable's inner side): kept upright faces turned the other way, 0.05 to
    // kOuterM behind a plane and beside it, are cut with it (or a tall card
    // would stand behind the broken wall).
    for (std::size_t i = 0; i < n; ++i)
    {
        const ShellFace& f = faces[i];
        if (f.hidden || f.n < 3 || f.n > 4 || (plan.part[i] != ShellPart::Wall && plan.part[i] != ShellPart::Carried))
            continue;
        const float h = std::sqrt(f.nrm[0] * f.nrm[0] + f.nrm[2] * f.nrm[2]);
        const float l = std::sqrt(h * h + f.nrm[1] * f.nrm[1]);
        if (l < 1e-6f || std::fabs(f.nrm[1] / l) > kUprightNy)
            continue;
        const float nx = f.nrm[0] / h, nz = f.nrm[2] / h;
        float c[3] = {0.0f, 0.0f, 0.0f};
        for (int k = 0; k < f.n; ++k)
            for (int a = 0; a < 3; ++a)
                c[a] += f.p[k][a] / static_cast<float>(f.n);
        for (WallPlane& w : planes)
        {
            const float behind = w.d - (w.n[0] * c[0] + w.n[1] * c[2]);
            if (w.n[0] * nx + w.n[1] * nz > -kCoplanarCos || behind < 0.05f || behind > kOuterM)
                continue;
            w.backing.push_back(static_cast<int>(i));
            break;
        }
    }
    // Each plane measured: its outline, its lowest top (the eaves line, or a
    // gable's foot) and its peak.
    for (WallPlane& w : planes)
    {
        const float eu[2] = {-w.n[1], w.n[0]};
        for (const int i : w.faces)
        {
            const ShellFace& f = faces[static_cast<std::size_t>(i)];
            std::vector<std::pair<float, float>> q;
            std::array<float, 4> b = {1e9f, -1e9f, 1e9f, -1e9f};
            for (int k = 0; k < f.n; ++k)
            {
                const float u = eu[0] * f.p[k][0] + eu[1] * f.p[k][2];
                q.emplace_back(u, f.p[k][1]);
                w.umin = std::min(w.umin, u);
                w.umax = std::max(w.umax, u);
                b = {std::min(b[0], u), std::max(b[1], u), std::min(b[2], f.p[k][1]), std::max(b[3], f.p[k][1])};
            }
            w.poly.push_back(std::move(q));
            w.box.push_back(b);
        }
        w.peakU = 0.5f * (w.umin + w.umax);
        for (float u = w.umin + 0.05f; u <= w.umax - 0.05f; u += 0.1f)
        {
            const float t = TopAt(w.poly, u);
            if (t < -1e8f)
                continue;
            w.lowTop = std::min(w.lowTop, t);
            if (t > w.peak)
            {
                w.peak = t;
                w.peakU = u;
            }
        }
        w.gable = w.umax - w.umin >= kGroupMinM && w.lowTop < 1e8f && w.peak > w.lowTop + kGableRiseM;
    }
    // Now and then the house keeps most of one gable (the same one in every
    // level: the gable whose key draws highest); never in the restrained break.
    std::uint32_t tallKey = 0;
    bool anyTall = false;
    if (style != ShellTopsStyle::Restrained && U01(Mix(seed, 0x7A11U)) < kTallGableShare)
    {
        std::uint32_t best = 0;
        for (const WallPlane& w : planes)
            if (w.gable && (!anyTall || Mix(seed, w.key) > best))
            {
                best = Mix(seed, w.key);
                tallKey = w.key;
                anyTall = true;
            }
    }
    for (const WallPlane& w : planes)
    {
        const float eu[2] = {-w.n[1], w.n[0]};
        const PlaneOutline& poly = w.poly;
        const float umin = w.umin, umax = w.umax;
        if (umax - umin < kGroupMinM)
            continue;
        auto cover = [&](float u) { return CoverAt(poly, u); };
        auto topAt = [&](float u) { return TopAt(poly, u); };
        const float lowTop = w.lowTop, peak = w.peak, peakU = w.peakU;
        if (lowTop > 1e8f)
            continue;
        const bool gable = w.gable;
        const bool tall = gable && anyTall && w.key == tallKey;
        ++(gable ? tops.gables : tops.walls);
        const std::uint32_t wk = Mix(seed, w.key);
        const float thick = kMasonryM + kMasonryJitterM * (2.0f * U01(Mix(wk, 1)) - 1.0f);
        // (the tops agent) the broken line; under a deep bite the cut goes lower
        float yCut = lowTop - kCapDepthM;
        BrokenTop bt;
        if (style != ShellTopsStyle::Columns)
        {
            bt = BrokenTopOf(w, tall, wk, style, coarse);
            yCut = std::min(yCut, bt.lowest - kUnderBreakM);
        }
        // Its faces above the cut go (rebuilt below), those across it are cut.
        for (std::size_t g = 0; g < w.faces.size(); ++g)
        {
            float lo = 1e9f, hi = -1e9f;
            for (const auto& c : poly[g])
            {
                lo = std::min(lo, c.second);
                hi = std::max(hi, c.second);
            }
            if (hi <= yCut + 0.01f)
                continue;
            if (lo >= yCut - 0.01f)
                tops.hide.push_back(w.faces[g]);
            else
                tops.clip.emplace_back(w.faces[g], yCut);
        }
        for (const int i : w.backing)
        {
            const ShellFace& f = faces[static_cast<std::size_t>(i)];
            float lo = 1e9f, hi = -1e9f, ulo = 1e9f, uhi = -1e9f;
            for (int k = 0; k < f.n; ++k)
            {
                const float u = eu[0] * f.p[k][0] + eu[1] * f.p[k][2];
                lo = std::min(lo, f.p[k][1]);
                hi = std::max(hi, f.p[k][1]);
                ulo = std::min(ulo, u);
                uhi = std::max(uhi, u);
            }
            if (hi <= yCut + 0.01f || uhi < umin - 0.1f || ulo > umax + 0.1f)
                continue;
            if (lo >= yCut - 0.01f)
                tops.hide.push_back(i);
            else
                tops.clip.emplace_back(i, yCut);
        }
        if (style != ShellTopsStyle::Columns)
        {
            if (gable)
                ++(tall ? tops.tall : tops.stumps);
            LayBrokenMasonry(w, bt, yCut, thick, !coarse, tops);
            continue;
        }
        // (Columns, v2) A gable: a short ragged stump, or (the house's one tall piece) most
        // of its height with its apex broken; the break tilted across it.
        float stumpY = 1e9f, stumpTilt = 0.0f, apexY = 1e9f;
        if (gable)
        {
            const float keep = tall ? kTallKeepLo + (kTallKeepHi - kTallKeepLo) * U01(Mix(wk, 3))
                                    : kStumpKeepLo + (kStumpKeepHi - kStumpKeepLo) * U01(Mix(wk, 3));
            stumpY = lowTop + keep * (peak - lowTop);
            stumpTilt = (2.0f * U01(Mix(wk, 4)) - 1.0f) * (tall ? 0.6f : 0.4f);
            apexY = peak - (kApexLoM + (kApexHiM - kApexLoM) * U01(Mix(wk, 5)));
            ++(tall ? tops.tall : tops.stumps);
        }
        // A long wall's deep bites (a tie beam torn out).
        struct Bite
        {
            float u, half, depth;
        };
        std::vector<Bite> deep;
        if (!gable)
        {
            const int nb = (umax - umin > 5.0f ? 1 : 0) + (umax - umin > 9.0f ? 1 : 0);
            for (int j = 0; j < nb; ++j)
                deep.push_back({umin + (0.15f + 0.7f * U01(Mix(wk, 10 + j))) * (umax - umin),
                                0.35f + 0.5f * U01(Mix(wk, 20 + j)),
                                kDeepBiteLoM + (kDeepBiteHiM - kDeepBiteLoM) * U01(Mix(wk, 30 + j))});
        }
        // The columns: boundaries on a grid along the wall (the same in every
        // level), jittered.
        std::vector<float> cuts;
        const int k0 = static_cast<int>(std::floor(umin / kColumnM)) - 1,
                  k1 = static_cast<int>(std::ceil(umax / kColumnM)) + 1;
        for (int k = k0; k <= k1; ++k)
        {
            const float b =
                static_cast<float>(k) * kColumnM +
                kColumnJitterM * (2.0f * U01(Mix(wk, 1000U + static_cast<std::uint32_t>(k + 100000))) - 1.0f);
            if (b > umin + 0.05f && b < umax - 0.05f)
                cuts.push_back(b);
        }
        cuts.insert(cuts.begin(), umin);
        cuts.push_back(umax);
        // Purlin pockets: the ridge's under the apex, and on each slope at a
        // third and two thirds of the gable's height, 0.75 m in from its edge.
        struct Pocket
        {
            float u, y;
        };
        std::vector<Pocket> pockets;
        if (tall)
        {
            // (under the piece's own broken top, not the old roof's)
            const float pieceTop = std::min(stumpY, apexY);
            std::vector<Pocket> all;
            all.push_back({peakU, pieceTop - 1.0f});
            for (const float frac : {0.35f, 0.65f})
            {
                const float y = lowTop + frac * (pieceTop - lowTop);
                for (const float dir : {-1.0f, 1.0f})
                {
                    for (float u = peakU; u > umin + 0.3f && u < umax - 0.3f; u += dir * 0.05f)
                        if (topAt(u) <= y + 0.75f)
                        {
                            all.push_back({u - dir * 0.05f, y});
                            break;
                        }
                }
            }
            const int want = 3 + static_cast<int>(U01(Mix(wk, 40)) * 2.99f);
            for (std::size_t j = 0; j < all.size() && static_cast<int>(pockets.size()) < want; ++j)
                pockets.push_back(all[j]);
            for (const Pocket& pk : pockets)
            {
                // a pocket is its own narrow column
                for (const float b : {pk.u - 0.5f * kPocketM, pk.u + 0.5f * kPocketM})
                    if (b > umin + 0.02f && b < umax - 0.02f)
                        cuts.push_back(b);
            }
            std::sort(cuts.begin(), cuts.end());
        }
        const float base[3] = {w.n[0] * w.d, 0.0f, w.n[1] * w.d};
        auto at = [&](float u, float y, float depth, float* out)
        {
            out[0] = base[0] + eu[0] * u - w.n[0] * depth;
            out[1] = y;
            out[2] = base[2] + eu[1] * u - w.n[1] * depth;
        };
        // the face whose texture a piece of masonry takes
        auto source = [&](float u, float y)
        {
            int best = w.faces.front();
            float bestD = 1e9f;
            for (std::size_t g = 0; g < poly.size(); ++g)
            {
                float lo = 1e9f, hi = -1e9f, ulo = 1e9f, uhi = -1e9f;
                for (const auto& c : poly[g])
                {
                    lo = std::min(lo, c.second);
                    hi = std::max(hi, c.second);
                    ulo = std::min(ulo, c.first);
                    uhi = std::max(uhi, c.first);
                }
                const float du = std::max({ulo - u, u - uhi, 0.0f}), dy = std::max({lo - y, y - hi, 0.0f});
                const float dd = du + dy;
                if (dd < bestD)
                {
                    bestD = dd;
                    best = w.faces[g];
                }
            }
            return best;
        };
        auto quad = [&](const float (&u)[4], const float (&y)[4], const float (&dp)[4], const float (&tu)[4],
                        const float (&ty)[4], float nx, float ny, float nz, int src)
        {
            ShellQuad q;
            for (int k = 0; k < 4; ++k)
            {
                at(u[k], y[k], dp[k], q.p[k]);
                at(tu[k], ty[k], 0.0f, q.tp[k]);
            }
            q.nrm[0] = nx;
            q.nrm[1] = ny;
            q.nrm[2] = nz;
            q.src = src;
            tops.quads.push_back(q);
        };
        float prevTop = -1e9f, run = 0.0f; // no straight top edge longer than about a metre
        for (std::size_t c = 0; c + 1 < cuts.size(); ++c)
        {
            const float u0 = cuts[c], u1 = cuts[c + 1];
            if (u1 - u0 < 0.04f)
                continue;
            const float uc = 0.5f * (u0 + u1);
            float pcol = 1e9f;
            for (const float u : {u0 + 0.02f, uc, u1 - 0.02f})
            {
                const float t = topAt(u);
                if (t > -1e8f)
                    pcol = std::min(pcol, t);
            }
            if (pcol > 1e8f)
                continue;
            const std::uint32_t ck = Mix(wk, 5000U + static_cast<std::uint32_t>(std::lround(uc * 20.0f) + 100000));
            float top;
            if (gable)
            {
                const float cap = std::min({pcol, apexY,
                                            stumpY + stumpTilt * (uc - peakU) / std::max(umax - umin, 1.0f) +
                                                0.45f * SmoothNoise(Mix(wk, 7), uc / 1.3f) + 0.1f * U01(Mix(ck, 1))});
                top = yCut + std::floor((cap - yCut) / kCourseM) * kCourseM;
                // (a pocket's own column is not bitten: its pocket stays readable)
                bool pocketCol = false;
                for (const Pocket& pk : pockets)
                    pocketCol = pocketCol || std::fabs(uc - pk.u) <= 0.5f * kPocketM;
                const float r = pocketCol ? 1.0f : U01(Mix(ck, 2));
                if (r < 0.06f)
                    top -= kCourseM * (1.0f + std::floor(U01(Mix(ck, 3)) * 1.99f));
            }
            else
            {
                float bite =
                    kWallBiteLoM + (kWallBiteHiM - kWallBiteLoM) * (0.7f * SmoothNoise(Mix(wk, 6), uc / 1.6f) +
                                                                    0.3f * SmoothNoise(Mix(wk, 8), uc / 0.55f));
                for (const Bite& b : deep)
                    if (std::fabs(uc - b.u) < b.half)
                        bite = std::max(bite, b.depth * (1.0f - 0.5f * std::fabs(uc - b.u) / b.half));
                top = pcol - std::round(bite * 10.0f) / 10.0f;
            }
            if (std::fabs(top - prevTop) < 0.02f && run + (u1 - u0) > kStraightMaxM)
            {
                top -= gable ? kCourseM : 0.05f;
                run = 0.0f;
            }
            run = std::fabs(top - prevTop) < 0.02f ? run + (u1 - u0) : (u1 - u0);
            prevTop = top;
            if (top <= yCut + 0.05f)
                continue;
            std::vector<Span> spans;
            for (const Span& s : cover(uc))
            {
                const float lo = std::max(s.lo, yCut), hi = std::min(s.hi, top);
                if (hi > lo + 0.05f)
                    spans.push_back({lo, hi});
            }
            for (const Pocket& pk : pockets)
            {
                if (std::fabs(uc - pk.u) > 0.5f * kPocketM - 0.01f || top < pk.y + 0.35f)
                    continue;
                std::vector<Span> cut;
                for (const Span& s : spans)
                {
                    if (s.hi <= pk.y - 0.5f * kPocketM || s.lo >= pk.y + 0.5f * kPocketM)
                    {
                        cut.push_back(s);
                        continue;
                    }
                    if (s.lo < pk.y - 0.5f * kPocketM)
                        cut.push_back({s.lo, pk.y - 0.5f * kPocketM});
                    if (s.hi > pk.y + 0.5f * kPocketM)
                        cut.push_back({pk.y + 0.5f * kPocketM, s.hi});
                }
                spans = cut;
                ++tops.pockets;
            }
            for (const Span& s : spans)
            {
                const int src = source(uc, 0.5f * (s.lo + s.hi));
                const float t = thick;
                // its outer face, its inner face, its top and underside, its ends
                quad({u0, u1, u1, u0}, {s.lo, s.lo, s.hi, s.hi}, {0, 0, 0, 0}, {u0, u1, u1, u0},
                     {s.lo, s.lo, s.hi, s.hi}, w.n[0], 0.0f, w.n[1], src);
                quad({u1, u0, u0, u1}, {s.lo, s.lo, s.hi, s.hi}, {t, t, t, t}, {u1, u0, u0, u1},
                     {s.lo, s.lo, s.hi, s.hi}, -w.n[0], 0.0f, -w.n[1], src);
                quad({u0, u1, u1, u0}, {s.hi, s.hi, s.hi, s.hi}, {0, 0, t, t}, {u0, u1, u1, u0},
                     {s.hi, s.hi, s.hi - t, s.hi - t}, 0.0f, 1.0f, 0.0f, src);
                quad({u1, u0, u0, u1}, {s.lo, s.lo, s.lo, s.lo}, {0, 0, t, t}, {u1, u0, u0, u1},
                     {s.lo, s.lo, s.lo + t, s.lo + t}, 0.0f, -1.0f, 0.0f, src);
                quad({u0, u0, u0, u0}, {s.lo, s.hi, s.hi, s.lo}, {0, 0, t, t}, {u0, u0, u0 + t, u0 + t},
                     {s.lo, s.hi, s.hi, s.lo}, -eu[0], 0.0f, -eu[1], src);
                quad({u1, u1, u1, u1}, {s.hi, s.lo, s.lo, s.hi}, {0, 0, t, t}, {u1, u1, u1 - t, u1 - t},
                     {s.hi, s.lo, s.lo, s.hi}, eu[0], 0.0f, eu[1], src);
            }
        }
    }
    return tops;
}

namespace
{

// A house shape and its shells, by weak links only: a shell lives while a
// building wears it, as a shape lives while an object does (a strong hold
// here would keep the faces' texture pointers past the texture banks: the
// ShapeBank's note). A stage nobody wears is built again when next asked.
// Stage 1 is a building's own (its broken tops are drawn from its seed).
struct ShellFamily
{
    Link<LODShapeWithShadow> src;
    Link<LODShapeWithShadow> stage0;
    std::vector<std::pair<std::uint32_t, Link<LODShapeWithShadow>>> stage1; // by seed
    bool church = false;
    bool none = false; // the rule finds no roof on any of its levels
    bool Has(const LODShapeWithShadow* shape) const
    {
        if (stage0.GetTypeRef() == shape)
            return true;
        for (const auto& s1 : stage1)
            if (s1.second.GetTypeRef() == shape)
                return true;
        return false;
    }
};
std::vector<ShellFamily>& Families()
{
    static std::vector<ShellFamily> f;
    return f;
}

std::string TexName(const Texture* t)
{
    if (!t)
        return std::string();
    std::string s = t->Name();
    const std::size_t cut = s.find_last_of("\\/");
    if (cut != std::string::npos)
        s = s.substr(cut + 1);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

} // namespace

bool IsBurntShell(const LODShapeWithShadow* shape)
{
    if (!shape)
        return false;
    for (const ShellFamily& f : Families())
        if (f.Has(shape))
            return true;
    return false;
}

ShellStats AddBurntShellVisual(LODShapeWithShadow* shape, bool church, bool hideGone, std::uint32_t seed)
{
    ShellStats st;
    if (!shape)
        return st;
    const int levels = shape->NLevels();
    for (int l = 0; l < levels; ++l)
    {
        Shape* s = shape->Level(l);
        if (!s || shape->Resolution(l) >= 900.0f || s->NFaces() <= 0 || s->NPos() != s->NNorm())
            continue;
        // How this shape winds its faces against its (inward) vertex
        // normals: the added faces wind the same way.
        float windSign = 0.0f;
        std::vector<ShellFace> faces;
        std::vector<Offset> offs;
        faces.reserve(static_cast<std::size_t>(s->NFaces()));
        for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
        {
            const Poly& p = s->Face(o);
            ShellFace f;
            f.n = p.N();
            f.hidden = (p.Special() & (IsHidden | IsHiddenProxy)) != 0 || f.n < 3 || f.n > 4;
            float nx = 0.0f, ny = 0.0f, nz = 0.0f;
            for (int k = 0; k < std::min(f.n, 4); ++k)
            {
                const int v = p.GetVertex(k);
                const Vector3 pos = s->Pos(v), nrm = s->Norm(v);
                f.p[k][0] = pos.X();
                f.p[k][1] = pos.Y();
                f.p[k][2] = pos.Z();
                nx += nrm.X();
                ny += nrm.Y();
                nz += nrm.Z();
            }
            // The face's own plane (smoothed vertex normals would tilt a wall),
            // turned outward as its vertices' normals say: a shape's vertex
            // normals point inward, the old engine's way (the vertex buffer
            // negates them, VertexBufferGL33::FillVertices).
            const float ux = f.p[1][0] - f.p[0][0], uy = f.p[1][1] - f.p[0][1], uz = f.p[1][2] - f.p[0][2];
            const float wx = f.p[2][0] - f.p[0][0], wy = f.p[2][1] - f.p[0][1], wz = f.p[2][2] - f.p[0][2];
            float cx = uy * wz - uz * wy, cy = uz * wx - ux * wz, cz = ux * wy - uy * wx;
            if (cx * cx + cy * cy + cz * cz < 1e-12f)
            {
                cx = -nx;
                cy = -ny;
                cz = -nz;
            }
            else if (cx * -nx + cy * -ny + cz * -nz < 0.0f)
            {
                cx = -cx;
                cy = -cy;
                cz = -cz;
            }
            f.nrm[0] = cx;
            f.nrm[1] = cy;
            f.nrm[2] = cz;
            f.tex = TexName(p.GetTexture());
            if (f.n >= 3 && !f.hidden)
            {
                const Vector3 a = s->Pos(p.GetVertex(0)), b = s->Pos(p.GetVertex(1)), c = s->Pos(p.GetVertex(2));
                const Vector3 cr = (b - a).CrossProduct(c - a);
                windSign += cr.DotProduct(s->Norm(p.GetVertex(0))) > 0.0f ? 1.0f : -1.0f;
            }
            faces.push_back(std::move(f));
            offs.push_back(o);
        }
        const ShellPlan plan = PlanBurntShell(faces, church);
        if (!plan.ok)
            continue;
        ++st.levels;
        // What the fire took: hidden (its sections are not drawn, the shadow
        // pass skips its faces), while the offsets still hold.
        if (hideGone)
            for (std::size_t i = 0; i < offs.size(); ++i)
                if (!ShellKeeps(plan.part[i]))
                {
                    s->Face(offs[i]).OrSpecial(IsHidden);
                    ++st.gone;
                }
        // Contents left hanging where the upper floor was, and the wooden ones, go.
        if (hideGone)
            for (int i = s->NProxies() - 1; i >= 0; --i)
            {
                const ProxyObject& px = s->Proxy(i);
                std::string pn = px.name.GetLength() > 0 ? std::string((const char*)px.name) : std::string();
                std::transform(pn.begin(), pn.end(), pn.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                const bool high = px.obj && px.obj->Position().Y() > plan.groundY + kUpperFloorM;
                if (high || (NameHas(pn, kWoodenContents) && !NameHas(pn, kLastingContents)))
                {
                    s->DeleteProxy(i);
                    ++st.contents;
                }
            }
        // v2: the walls' and gables' broken tops (what stood above each
        // wall's cut is rebuilt as thick stepped masonry).
        ShellTops tops;
        std::vector<float> cutAt(offs.size(), 1e9f); // a kept face's cut (1e9 none; -1e9 all of it rebuilt)
        if (hideGone)
        {
            tops = PlanBurntTops(faces, plan, seed, ShellTopsStyleNow(), st.levels > 1);
            for (const int i : tops.hide)
                cutAt[static_cast<std::size_t>(i)] = -1e9f;
            for (const auto& c : tops.clip)
                cutAt[static_cast<std::size_t>(c.first)] = c.second;
            st.walls += tops.walls;
            st.gables += tops.gables;
            st.stumps += tops.stumps;
            st.pockets += tops.pockets;
        }
        // The added faces, built in a shape of their own and merged in after
        // the model's: each pier's band stepped down, the walls' inner sides,
        // the cut walls' parts under their cut, the rebuilt masonry.
        Ref<Shape> add = new Shape();
        // one face of the model, its corners moved by dy, turned inside out
        // (flip) and cut at y (its part under y kept; 1e9 not cut)
        auto copyFace = [&](int fi, float dy, bool flip, float cutY)
        {
            const Poly& src = s->Face(offs[static_cast<std::size_t>(fi)]);
            struct Corner
            {
                Vector3 pos, nrm;
                ClipFlags clip;
                float u, v;
            };
            std::vector<Corner> in, out;
            for (int k = 0; k < src.N(); ++k)
            {
                const int v = src.GetVertex(k);
                Vector3 pos = s->Pos(v);
                pos[1] += dy;
                in.push_back({pos, flip ? -s->Norm(v) : s->Norm(v), s->Clip(v), s->U(v), s->V(v)});
            }
            if (cutY < 1e8f)
            {
                for (std::size_t k = 0; k < in.size(); ++k)
                {
                    const Corner& a = in[k];
                    const Corner& b = in[(k + 1) % in.size()];
                    const bool ain = a.pos.Y() <= cutY, bin = b.pos.Y() <= cutY;
                    if (ain)
                        out.push_back(a);
                    if (ain != bin)
                    {
                        const float t = (cutY - a.pos.Y()) / (b.pos.Y() - a.pos.Y());
                        out.push_back({a.pos + (b.pos - a.pos) * t, a.nrm + (b.nrm - a.nrm) * t, a.clip,
                                       a.u + (b.u - a.u) * t, a.v + (b.v - a.v) * t});
                    }
                }
            }
            else
                out = in;
            if (out.size() < 3)
                return;
            // (fanned into faces of at most four corners)
            for (std::size_t first = 1; first + 1 < out.size();)
            {
                const std::size_t nk = std::min<std::size_t>(out.size() - first, 3);
                Poly p = src;
                p.AndSpecial(~IsHidden); // (a cut wall's own face is hidden by now)
                p.SetN(static_cast<VertexIndex>(nk + 1));
                const Corner& o = out[0];
                p.Set(0, add->AddVertexFast(o.pos, o.nrm, o.clip, o.u, o.v));
                for (std::size_t k = 0; k < nk; ++k)
                {
                    const Corner& c = out[first + k];
                    p.Set(static_cast<int>(k + 1), add->AddVertexFast(c.pos, c.nrm, c.clip, c.u, c.v));
                }
                if (flip)
                    p.Reverse();
                add->AddFace(p);
                first += nk - 1;
                if (nk < 3)
                    break;
            }
        };
        // a piece of rebuilt masonry: its source face's texture continued
        // over its plane (an affine map from three of its corners)
        auto addQuad = [&](const ShellQuad& q)
        {
            const Poly& src = s->Face(offs[static_cast<std::size_t>(q.src)]);
            const int i0 = src.GetVertex(0), i1 = src.GetVertex(1), i2 = src.GetVertex(2);
            const Vector3 p0 = s->Pos(i0), e1 = s->Pos(i1) - p0, e2 = s->Pos(i2) - p0;
            const float a11 = e1.DotProduct(e1), a12 = e1.DotProduct(e2), a22 = e2.DotProduct(e2);
            const float det = a11 * a22 - a12 * a12;
            const Vector3 n(q.nrm[0], q.nrm[1], q.nrm[2]);
            const int nc = q.n == 3 ? 3 : 4;
            int order[4] = {0, 1, 2, 3};
            // wound as the model's faces are against their inward normal (-n)
            // (its corners' own turn: a triangle, or a quad with three corners in a line)
            Vector3 turn(0.0f, 0.0f, 0.0f);
            for (int k = 0; k < nc; ++k)
            {
                const Vector3 a(q.p[k][0], q.p[k][1], q.p[k][2]);
                const Vector3 b(q.p[(k + 1) % nc][0], q.p[(k + 1) % nc][1], q.p[(k + 1) % nc][2]);
                turn = turn + a.CrossProduct(b);
            }
            if (turn.DotProduct(-n) * windSign < 0.0f)
                for (int k = 1; k < nc; ++k)
                    order[k] = nc - k;
            Poly p = src;
            p.AndSpecial(~IsHidden); // (its source face, rebuilt, is hidden by now)
            p.SetN(static_cast<VertexIndex>(nc));
            for (int k = 0; k < nc; ++k)
            {
                const int j = order[k];
                const Vector3 pos(q.p[j][0], q.p[j][1], q.p[j][2]);
                float u = s->U(i0), v = s->V(i0);
                if (std::fabs(det) > 1e-8f)
                {
                    const Vector3 r = Vector3(q.tp[j][0], q.tp[j][1], q.tp[j][2]) - p0;
                    const float b1 = r.DotProduct(e1), b2 = r.DotProduct(e2);
                    const float x = (b1 * a22 - b2 * a12) / det, y = (b2 * a11 - b1 * a12) / det;
                    u += x * (s->U(i1) - s->U(i0)) + y * (s->U(i2) - s->U(i0));
                    v += x * (s->V(i1) - s->V(i0)) + y * (s->V(i2) - s->V(i0));
                }
                p.Set(k, add->AddVertexFast(pos, -n, s->Clip(i0), u, v));
            }
            add->AddFace(p);
        };
        for (const ShellPier& pier : plan.piers)
        {
            ++st.piers;
            for (int c = 1; c <= pier.copies; ++c)
                for (const int fi : pier.band)
                {
                    copyFace(fi, -pier.step * static_cast<float>(c), false, 1e9f);
                    ++st.pierFaces;
                }
        }
        // the walls' inner sides (of what stays under each wall's cut)
        for (const int fi : plan.inner)
            if (cutAt[static_cast<std::size_t>(fi)] > -1e8f)
            {
                copyFace(fi, 0.0f, true, cutAt[static_cast<std::size_t>(fi)]);
                ++st.inner;
            }
        // the cut walls: their part under the cut stays, the face goes
        for (std::size_t i = 0; i < offs.size(); ++i)
            if (cutAt[i] < 1e8f)
            {
                if (cutAt[i] > -1e8f)
                    copyFace(static_cast<int>(i), 0.0f, false, cutAt[i]);
                s->Face(offs[i]).OrSpecial(IsHidden);
            }
        for (const ShellQuad& q : tops.quads)
            addQuad(q);
        st.masonryFaces += static_cast<int>(tops.quads.size());
        if (add->NFaces() > 0)
            s->MergeFast(add, MIdentity);
        s->FindSections();
        s->RefreshPlanes();
        s->CalculateHints();
        s->InvalidateBuffer();
        if (st.eavesY == 0.0f && st.levels == 1)
        {
            st.footY = plan.footY;
            st.eavesY = plan.eavesY;
            st.ridgeY = plan.ridgeY;
        }
    }
    if (st.levels > 0)
    {
        shape->LockAutoCenter(true);
        shape->CalculateHints();
        shape->CalculateMinMax(true);
        shape->OptimizeRendering();
    }
    return st;
}

Ref<LODShapeWithShadow> BurntShellShape(LODShapeWithShadow* src, bool church, int stage, std::uint32_t seed)
{
    if (!src)
        return nullptr;
    stage = std::clamp(stage, 0, 1);
    std::vector<ShellFamily>& fams = Families();
    // (families whose house shape is gone are dropped)
    fams.erase(std::remove_if(fams.begin(), fams.end(), [](const ShellFamily& f) { return !f.src.GetTypeRef(); }),
               fams.end());
    ShellFamily* fam = nullptr;
    for (ShellFamily& f : fams)
        if (f.src.GetTypeRef() == src || f.Has(src))
            fam = &f;
    if (!fam)
    {
        ShellFamily f;
        f.src = src;
        f.church = church;
        fams.push_back(f);
        fam = &fams.back();
    }
    if (fam->none)
        return nullptr;
    // (the stage-1 shells nobody wears any more are forgotten)
    fam->stage1.erase(
        std::remove_if(fam->stage1.begin(), fam->stage1.end(), [](const auto& s1) { return !s1.second.GetTypeRef(); }),
        fam->stage1.end());
    if (stage == 0)
    {
        if (LODShapeWithShadow* worn = fam->stage0.GetTypeRef())
            return worn;
    }
    else
        for (const auto& s1 : fam->stage1)
            if (s1.first == seed && s1.second.GetTypeRef())
                return s1.second.GetTypeRef();
    LODShapeWithShadow* from = fam->src.GetTypeRef();
    const auto t0 = std::chrono::steady_clock::now();
    Ref<LODShapeWithShadow> shell = new LODShapeWithShadow(*from);
    // (the copy constructor leaves the map type out: a copy reads as a tree)
    shell->CopyClassFrom(*from);
    const ShellStats st = AddBurntShellVisual(shell, fam->church, stage == 1, seed);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    LOG_INFO(Script,
             "UDFIRE;event=buildingshell;shape={};stage={};seed={};levels={};piers={};pierfaces={};gone={};inner={};"
             "contents={};walls={};gables={};stumps={};pockets={};masonry={};foot={:.2f};eaves={:.2f};ridge={:.2f};ms={"
             ":.1f}",
             (const char*)from->GetName(), stage, seed, st.levels, st.piers, st.pierFaces, st.gone, st.inner,
             st.contents, st.walls, st.gables, st.stumps, st.pockets, st.masonryFaces, st.footY, st.eavesY, st.ridgeY,
             ms);
    if (st.levels == 0)
    {
        fam->none = true;
        return nullptr;
    }
    if (stage == 0)
        fam->stage0 = shell.GetRef();
    else
        fam->stage1.emplace_back(seed, Link<LODShapeWithShadow>(shell.GetRef()));
    return shell;
}

} // namespace Poseidon::firespread
