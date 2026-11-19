#include <Poseidon/World/Simulation/FireShell.hpp>

#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Simulation/FireBuildingShell.hpp>
#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace Poseidon::firespread
{

// ---- Pure pieces ----

float ShellWallTopY(const std::vector<ShellFaceStat>& faces, float baseY, float topY)
{
    const float h = topY - baseY;
    if (h <= 0.0f)
        return topY;
    std::vector<std::pair<float, float>> roof; // (lowest point, area)
    float total = 0.0f;
    for (const ShellFaceStat& f : faces)
        if (f.ny > kShellRoofNy && f.meanY > baseY + kShellRoofFrom * h && f.area > 0.0f)
        {
            roof.emplace_back(f.minY, f.area);
            total += f.area;
        }
    if (roof.empty() || total <= 0.0f)
        return baseY + 0.6f * h; // (FireSpreadWorld.cpp kBuildingWallShare)
    std::sort(roof.begin(), roof.end());
    float acc = 0.0f;
    float eave = roof.back().first;
    for (const auto& [y, a] : roof)
    {
        acc += a;
        if (acc >= kShellEaveQuantile * total)
        {
            eave = y;
            break;
        }
    }
    return std::clamp(eave, baseY + 0.3f * h, topY);
}

namespace
{
float Hash01(int x, int z, std::uint32_t seed)
{
    std::uint32_t h =
        static_cast<std::uint32_t>(x) * 73856093U ^ static_cast<std::uint32_t>(z) * 19349663U ^ seed * 83492791U;
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return static_cast<float>(h >> 8) / 16777216.0f;
}
// Smooth value noise 0..1 on a unit lattice.
float ValueNoise(float x, float z, std::uint32_t seed)
{
    const float fx = std::floor(x), fz = std::floor(z);
    const int ix = static_cast<int>(fx), iz = static_cast<int>(fz);
    float tx = x - fx, tz = z - fz;
    tx = tx * tx * (3.0f - 2.0f * tx);
    tz = tz * tz * (3.0f - 2.0f * tz);
    const float a = Hash01(ix, iz, seed), b = Hash01(ix + 1, iz, seed);
    const float c = Hash01(ix, iz + 1, seed), d = Hash01(ix + 1, iz + 1, seed);
    return (a + (b - a) * tx) + ((c + (d - c) * tx) - (a + (b - a) * tx)) * tz;
}
float SmoothStep(float e0, float e1, float x)
{
    const float t = std::clamp((x - e0) / std::max(e1 - e0, 1e-6f), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
} // namespace

float ShellRubbleDepth(const ShellRubbleFrame& f, float x, float z)
{
    const float wallH = f.wallTopY - f.floorY;
    const float d = wallH > 4.5f ? 0.6f : 0.35f;
    const float lx = x - f.cx, lz = z - f.cz;
    // heaped along the foot of the walls and under the ridge line, low in patches
    const float toWall = std::min(f.hx - std::fabs(lx), f.hz - std::fabs(lz));
    const float wallBank = 1.0f - SmoothStep(0.0f, 1.2f, toWall);
    const float toRidge = f.ridgeAlongX ? std::fabs(lz) : std::fabs(lx);
    const float ridgeHalf = f.ridgeAlongX ? f.hz : f.hx;
    const float ridge = 1.0f - SmoothStep(0.0f, 0.35f * std::max(ridgeHalf, 0.5f), toRidge);
    const float n = ValueNoise(x / 1.3f, z / 1.3f, f.seed);
    const float n2 = ValueNoise(x / 0.45f + 17.0f, z / 0.45f - 5.0f, f.seed + 1U);
    float r = d * (0.45f + 0.75f * n + 0.3f * (n2 - 0.5f)) + 0.35f * d * wallBank + 0.3f * d * ridge;
    r = std::clamp(r, 0.15f, 1.1f);
    return std::max(0.05f, std::min(r, wallH - 0.3f));
}

void ShellRubbleInside(const ShellRubbleFrame& f, float rcx, float rcz, float rhx, float rhz, float& x, float& z)
{
    const float sx = rhx > 1e-3f ? std::clamp((f.hx - kShellRubbleInsetM) / rhx, 0.0f, 1.0f) : 1.0f;
    const float sz = rhz > 1e-3f ? std::clamp((f.hz - kShellRubbleInsetM) / rhz, 0.0f, 1.0f) : 1.0f;
    x = f.cx + (x - rcx) * sx;
    z = f.cz + (z - rcz) * sz;
}

// ---- The engine side ----

namespace
{
// A shell made: its house's own shape, its mode and stage, and whether its
// visual levels hide the roof (the geometry agent's rule found one).
struct ShellInfo
{
    Ref<LODShapeWithShadow> source;
    ShellMode mode = ShellMode::Walls;
    int stage = 0;
    bool hidesRoof = false;
};
struct ShellStore
{
    // by its house shape, mode, stage and (stage 1: its broken tops) the building's seed
    std::map<std::tuple<const LODShape*, int, int, std::uint32_t>, Ref<LODShapeWithShadow>> shells;
    std::unordered_map<const LODShape*, ShellInfo> info; // by the shell
    std::unordered_map<const LODShape*, float> wallTop;
};
ShellStore& Store()
{
    static ShellStore* s = new ShellStore; // never destroyed (the shapes outlive the GL context at exit)
    return *s;
}

bool Hidden(const Poly& p)
{
    return (p.Special() & (IsHidden | IsHiddenProxy)) != 0;
}

// A face's corners: its mean normal's up share, lowest and mean height, area.
ShellFaceStat FaceOf(const Shape& s, const Poly& p)
{
    ShellFaceStat f;
    Vector3 n = VZero;
    float sumY = 0.0f;
    f.minY = 1e9f;
    for (int k = 0; k < p.N(); ++k)
    {
        const int v = p.GetVertex(k);
        n += s.Norm(v);
        const float y = s.Pos(v).Y();
        f.minY = std::min(f.minY, y);
        sumY += y;
    }
    const float len = n.Size();
    // (a shape keeps its vertex normals turned in: the vertex buffer negates
    // them, EngineGL33_VertexBuffer.cpp; ny is the outward one's)
    f.ny = len > 1e-6f ? -n.Y() / len : 0.0f;
    f.meanY = p.N() > 0 ? sumY / static_cast<float>(p.N()) : 0.0f;
    f.area = p.GetArea(s);
    return f;
}

bool IsRoofFace(const Shape& s, const Poly& p, float cutY)
{
    if (p.N() < 3 || Hidden(p))
        return false;
    const ShellFaceStat f = FaceOf(s, p);
    return f.ny > kShellRoofNy && f.minY >= cutY - 0.3f;
}

// The frame of a shape's rubble: its walls' box (the upright faces below the
// cut), its roof's box (the roof faces) and the ridge's direction (its
// highest points), from its first visual level.
struct RubbleSetup
{
    ShellRubbleFrame frame;
    float rcx = 0.0f, rcz = 0.0f, rhx = 1.0f, rhz = 1.0f;
    bool ok = false;
};
RubbleSetup SetupOf(const LODShape* shape, float cutY, float depthScale)
{
    RubbleSetup r;
    const Shape* s = shape->NLevels() > 0 ? shape->Level(0) : nullptr;
    if (!s || s->NFaces() <= 0 || s->NNorm() != s->NPos())
        return r;
    float wx0 = 1e9f, wx1 = -1e9f, wz0 = 1e9f, wz1 = -1e9f;
    float rx0 = 1e9f, rx1 = -1e9f, rz0 = 1e9f, rz1 = -1e9f, top = -1e9f;
    for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
    {
        const Poly& p = s->Face(o);
        if (p.N() < 3 || Hidden(p))
            continue;
        const ShellFaceStat f = FaceOf(*s, p);
        const bool roof = f.ny > kShellRoofNy && f.minY >= cutY - 0.3f;
        const bool wall = std::fabs(f.ny) < 0.3f && f.minY < cutY - 0.5f;
        for (int k = 0; k < p.N(); ++k)
        {
            const Vector3 q = s->Pos(p.GetVertex(k));
            if (wall)
            {
                wx0 = std::min(wx0, q.X()), wx1 = std::max(wx1, q.X());
                wz0 = std::min(wz0, q.Z()), wz1 = std::max(wz1, q.Z());
            }
            if (roof)
            {
                rx0 = std::min(rx0, q.X()), rx1 = std::max(rx1, q.X());
                rz0 = std::min(rz0, q.Z()), rz1 = std::max(rz1, q.Z());
                top = std::max(top, q.Y());
            }
        }
    }
    if (wx1 <= wx0 || wz1 <= wz0 || rx1 <= rx0 || rz1 <= rz0)
        return r;
    // the ridge: the spread of the roof's points within 0.3 m of its top
    float hx0 = 1e9f, hx1 = -1e9f, hz0 = 1e9f, hz1 = -1e9f;
    for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
    {
        const Poly& p = s->Face(o);
        if (!IsRoofFace(*s, p, cutY))
            continue;
        for (int k = 0; k < p.N(); ++k)
        {
            const Vector3 q = s->Pos(p.GetVertex(k));
            if (q.Y() >= top - 0.3f)
            {
                hx0 = std::min(hx0, q.X()), hx1 = std::max(hx1, q.X());
                hz0 = std::min(hz0, q.Z()), hz1 = std::max(hz1, q.Z());
            }
        }
    }
    ShellRubbleFrame& f = r.frame;
    f.cx = 0.5f * (wx0 + wx1);
    f.cz = 0.5f * (wz0 + wz1);
    f.hx = 0.5f * (wx1 - wx0);
    f.hz = 0.5f * (wz1 - wz0);
    // The ground floor: the lowest floor face inside the walls (a house on a
    // slope stands on a plinth, its foot up to 2 m under its floor: the
    // geometry agent's survey); its foot when it has none.
    f.floorY = s->Min().Y();
    float floor = 1e9f;
    for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
    {
        const Poly& p = s->Face(o);
        if (p.N() < 3 || Hidden(p))
            continue;
        const ShellFaceStat ff = FaceOf(*s, p);
        if (ff.ny < 0.9f || ff.area < 1.0f || ff.meanY > cutY - 1.5f)
            continue;
        Vector3 c = VZero;
        for (int k = 0; k < p.N(); ++k)
            c += s->Pos(p.GetVertex(k));
        c = c / static_cast<float>(p.N());
        if (std::fabs(c.X() - f.cx) < f.hx - 0.3f && std::fabs(c.Z() - f.cz) < f.hz - 0.3f)
            floor = std::min(floor, ff.meanY);
    }
    if (floor < 1e8f)
        f.floorY = std::max(f.floorY, floor);
    f.wallTopY = f.floorY + (cutY - f.floorY) * depthScale;
    f.ridgeAlongX = (hx1 - hx0) >= (hz1 - hz0);
    std::uint32_t seed = 2166136261U;
    for (const char* c = shape->Name(); c && *c; ++c)
        seed = (seed ^ static_cast<unsigned char>(*c)) * 16777619U;
    f.seed = seed;
    r.rcx = 0.5f * (rx0 + rx1);
    r.rcz = 0.5f * (rz0 + rz1);
    r.rhx = 0.5f * (rx1 - rx0);
    r.rhz = 0.5f * (rz1 - rz0);
    r.ok = true;
    return r;
}

struct Corner
{
    Vector3 p;
    float u = 0.0f, v = 0.0f;
};
Corner Lerp(const Corner& a, const Corner& b, float t)
{
    Corner c;
    c.p = a.p + (b.p - a.p) * t;
    c.u = a.u + (b.u - a.u) * t;
    c.v = a.v + (b.v - a.v) * t;
    return c;
}

// The rubble of one visual level: its roof faces fallen in, each cut into
// cells of about cellM, every point brought inside the walls at the heap's
// height; the faces keep their texture, their properties and their winding
// (seen from above as the roof was); and `beams` charred beams, most leaning
// from a pocket in a long wall down into the heap, some lying on it (the
// eleventh answer: 5 to 15 in a house). Merged into the level.
int AddRubble(const Shape* level, Shape* dst, const RubbleSetup& rs, float cutY, float cellM, int beams)
{
    if (!level || !dst || level->NFaces() <= 0 || level->NNorm() != level->NPos() || dst->NNorm() != dst->NPos())
        return 0;
    Ref<Shape> rub = new Shape;
    Poly roofProps;
    bool haveRoof = false;
    float windSign = 1.0f; // the source's front winding against its outward normal
    float roofU = 0.0f, roofV = 0.0f;
    std::vector<int> ids;
    auto put = [&](const Corner& c, ClipFlags clip)
    {
        float x = c.p.X(), z = c.p.Z();
        ShellRubbleInside(rs.frame, rs.rcx, rs.rcz, rs.rhx, rs.rhz, x, z);
        const float y = rs.frame.floorY + ShellRubbleDepth(rs.frame, x, z);
        return rub->AddVertex(Vector3(x, y, z), VUp, clip, c.u, c.v);
    };
    int faces = 0;
    for (Offset o = level->BeginFaces(); o < level->EndFaces(); level->NextFace(o))
    {
        const Poly& src = level->Face(o);
        if (!IsRoofFace(*level, src, cutY))
            continue;
        const int n = std::min<int>(src.N(), MaxPoly);
        Corner c[MaxPoly];
        for (int k = 0; k < n; ++k)
        {
            const int v = src.GetVertex(k);
            c[k].p = level->Pos(v);
            c[k].u = level->U(v);
            c[k].v = level->V(v);
        }
        const ClipFlags clip = level->Clip(src.GetVertex(0)) & ~ClipLandMask;
        if (!haveRoof && n >= 3)
        {
            haveRoof = true;
            roofProps = src;
            roofU = c[0].u;
            roofV = c[0].v;
            Vector3 out = VZero;
            for (int k = 0; k < n; ++k)
                out -= level->Norm(src.GetVertex(k)); // (kept turned in)
            const Vector3 g = (c[1].p - c[0].p).CrossProduct(c[2].p - c[0].p);
            windSign = g * out >= 0.0f ? 1.0f : -1.0f;
        }
        auto emit = [&](const int* vi, int nv)
        {
            Poly f;
            f.Init();
            f.CopyProperties(src);
            f.SetN(static_cast<VertexIndex>(nv));
            for (int k = 0; k < nv; ++k)
                f.Set(k, static_cast<VertexIndex>(vi[k]));
            rub->AddFace(f);
            ++faces;
        };
        float longest = 0.0f;
        for (int k = 0; k < n; ++k)
            longest = std::max(longest, c[k].p.Distance(c[(k + 1) % n].p));
        const int cells = std::clamp(static_cast<int>(std::ceil(longest / cellM)), 1, 8);
        if (n == 4)
        {
            // bilinear cells, in the quad's own order (its winding kept)
            std::vector<int> grid(static_cast<std::size_t>((cells + 1) * (cells + 1)));
            for (int j = 0; j <= cells; ++j)
                for (int i = 0; i <= cells; ++i)
                {
                    const float s = static_cast<float>(i) / cells, t = static_cast<float>(j) / cells;
                    const Corner a = Lerp(c[0], c[1], s), b = Lerp(c[3], c[2], s);
                    grid[static_cast<std::size_t>(j * (cells + 1) + i)] = put(Lerp(a, b, t), clip);
                }
            for (int j = 0; j < cells; ++j)
                for (int i = 0; i < cells; ++i)
                {
                    const int q[4] = {grid[static_cast<std::size_t>(j * (cells + 1) + i)],
                                      grid[static_cast<std::size_t>(j * (cells + 1) + i + 1)],
                                      grid[static_cast<std::size_t>((j + 1) * (cells + 1) + i + 1)],
                                      grid[static_cast<std::size_t>((j + 1) * (cells + 1) + i)]};
                    emit(q, 4);
                }
        }
        else
        {
            // a triangle (a larger polygon as a fan): barycentric cells
            for (int k = 1; k + 1 < n; ++k)
            {
                const Corner t0 = c[0], t1 = c[k], t2 = c[k + 1];
                std::vector<int> g(static_cast<std::size_t>((cells + 1) * (cells + 2) / 2));
                auto at = [cells](int i, int j)
                { return static_cast<std::size_t>(j * (cells + 1) - j * (j - 1) / 2 + i); };
                for (int j = 0; j <= cells; ++j)
                    for (int i = 0; i + j <= cells; ++i)
                    {
                        const float s = static_cast<float>(i) / cells, t = static_cast<float>(j) / cells;
                        Corner q;
                        q.p = t0.p + (t1.p - t0.p) * s + (t2.p - t0.p) * t;
                        q.u = t0.u + (t1.u - t0.u) * s + (t2.u - t0.u) * t;
                        q.v = t0.v + (t1.v - t0.v) * s + (t2.v - t0.v) * t;
                        g[at(i, j)] = put(q, clip);
                    }
                for (int j = 0; j < cells; ++j)
                    for (int i = 0; i + j < cells; ++i)
                    {
                        const int a[3] = {g[at(i, j)], g[at(i + 1, j)], g[at(i, j + 1)]};
                        emit(a, 3);
                        if (i + j + 1 < cells)
                        {
                            const int b[3] = {g[at(i + 1, j)], g[at(i + 1, j + 1)], g[at(i, j + 1)]};
                            emit(b, 3);
                        }
                    }
            }
        }
    }
    if (faces == 0)
        return 0;
    const int heapVerts = rub->NPos();
    // The beams: boxes, each face its own four points with its own normal.
    if (beams > 0 && haveRoof)
    {
        const ShellRubbleFrame& f = rs.frame;
        const ClipFlags clip = 0;
        auto quad = [&](const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& out)
        {
            Vector3 q[4] = {a, b, c, d};
            if (((q[1] - q[0]).CrossProduct(q[2] - q[0]) * out) * windSign < 0.0f)
                std::swap(q[1], q[3]);
            Poly pf;
            pf.Init();
            pf.CopyProperties(roofProps);
            pf.SetN(4);
            for (int k = 0; k < 4; ++k)
                pf.Set(k, static_cast<VertexIndex>(rub->AddVertexFast(q[k], -out, clip, roofU + 0.002f * k, roofV)));
            rub->AddFace(pf);
            ++faces;
        };
        auto beam = [&](const Vector3& a, const Vector3& b, float w, float h)
        {
            Vector3 ax = b - a;
            const float len = ax.Size();
            if (len < 0.3f)
                return;
            ax = ax / len;
            Vector3 side = ax.CrossProduct(VUp);
            side = side.Size() > 1e-3f ? side / side.Size() : Vector3(1.0f, 0.0f, 0.0f);
            Vector3 up = side.CrossProduct(ax);
            up = up / std::max(up.Size(), 1e-6f);
            if (up.Y() < 0.0f)
                up = -up;
            const Vector3 sw = side * (0.5f * w), uh = up * (0.5f * h);
            const Vector3 a00 = a - sw - uh, a10 = a + sw - uh, a11 = a + sw + uh, a01 = a - sw + uh;
            const Vector3 b00 = b - sw - uh, b10 = b + sw - uh, b11 = b + sw + uh, b01 = b - sw + uh;
            quad(a01, a11, b11, b01, up);
            quad(a00, b00, b10, a10, -up);
            quad(a10, b10, b11, a11, side);
            quad(a00, a01, b01, b00, -side);
            quad(a00, a10, a11, a01, -ax);
            quad(b00, b01, b11, b10, ax);
        };
        for (int k = 0; k < beams; ++k)
        {
            const float r1 = Hash01(k, 11, f.seed), r2 = Hash01(k, 12, f.seed), r3 = Hash01(k, 13, f.seed);
            const float r4 = Hash01(k, 14, f.seed), r5 = Hash01(k, 15, f.seed), r6 = Hash01(k, 16, f.seed);
            const float sd = r1 < 0.5f ? -1.0f : 1.0f;
            // a pocket in a long wall (one of the two under the eaves)
            Vector3 start, dir, across;
            if (f.ridgeAlongX)
            {
                start = Vector3(f.cx + (2.0f * r2 - 1.0f) * (f.hx - 0.7f), 0.0f, f.cz + sd * (f.hz - 0.2f));
                dir = Vector3(0.0f, 0.0f, -sd);
                across = Vector3(1.0f, 0.0f, 0.0f);
            }
            else
            {
                start = Vector3(f.cx + sd * (f.hx - 0.2f), 0.0f, f.cz + (2.0f * r2 - 1.0f) * (f.hz - 0.7f));
                dir = Vector3(-sd, 0.0f, 0.0f);
                across = Vector3(0.0f, 0.0f, 1.0f);
            }
            dir = dir + across * ((r6 - 0.5f) * 0.6f);
            dir = dir / dir.Size();
            const float len = 2.0f + 1.8f * r5;
            Vector3 end;
            if (k % 3 == 2)
            {
                // lying on the heap
                start = start + dir * 0.6f;
                start[1] = f.floorY + ShellRubbleDepth(f, start.X(), start.Z()) + 0.08f;
                end = start + dir * len;
                end[1] = f.floorY + ShellRubbleDepth(f, end.X(), end.Z()) + 0.08f;
            }
            else
            {
                // from its pocket down into the heap, 30 to 55 degrees
                start[1] = f.floorY + (f.wallTopY - f.floorY) * (0.6f + 0.35f * r3);
                const float tilt = (30.0f + 25.0f * r4) * 0.0174533f;
                end = start + dir * (len * std::cos(tilt));
                end[1] =
                    std::max(start.Y() - len * std::sin(tilt), f.floorY + 0.6f * ShellRubbleDepth(f, end.X(), end.Z()));
            }
            beam(start, end, 0.16f, 0.2f);
        }
    }
    // Normals: each heap point the mean of its faces', turned up (a heap seen
    // from above); the beams keep their own.
    std::vector<Vector3> acc(static_cast<std::size_t>(rub->NPos()), VZero);
    for (Offset o = rub->BeginFaces(); o < rub->EndFaces(); rub->NextFace(o))
    {
        const Poly& f = rub->Face(o);
        const Vector3 a = rub->Pos(f.GetVertex(0)), b = rub->Pos(f.GetVertex(1)), c = rub->Pos(f.GetVertex(2));
        Vector3 nrm = (b - a).CrossProduct(c - a);
        if (nrm.Y() < 0.0f)
            nrm = -nrm;
        for (int k = 0; k < f.N(); ++k)
            acc[static_cast<std::size_t>(f.GetVertex(k))] += nrm;
    }
    for (int i = 0; i < heapVerts; ++i)
    {
        Vector3 nrm = acc[static_cast<std::size_t>(i)];
        const float len = nrm.Size();
        rub->SetNorm(i) = len > 1e-6f ? -(nrm / len) : -VUp; // (kept turned in, as every shape's)
    }
    // its textures as the level has them
    for (Offset o = rub->BeginFaces(); o < rub->EndFaces(); rub->NextFace(o))
    {
        Texture* tex = rub->Face(o).GetTexture();
        if (tex && rub->GetTextureIndex(tex) < 0)
        {
            const int li = level->GetTextureIndex(tex);
            rub->RegisterTexture(tex, li >= 0 ? level->GetAreaOTex(li) : 1.0f);
        }
    }
    rub->CalculateHints();
    rub->FindSections();
    rub->CalculateMinMax();
    dst->Merge(rub, MIdentity);
    dst->RefreshPlanes();
    dst->CalculateHints();
    dst->InvalidateBuffer();
    return faces;
}

// The cut of a geometry level: every point above it brought down onto it.
void CutLevel(Shape* s, float cutY)
{
    if (!s)
        return;
    for (int i = 0; i < s->NPos(); ++i)
    {
        Vector3& p = s->SetPos(i);
        p[1] = ShellSquashY(p[1], cutY);
    }
    s->InvalidateNormals();
    s->RecalculateNormals(true);
    s->CalculateMinMax();
}

// The components that lay wholly above the cut go.
void DropComponents(ConvexComponents* cc, float cutY)
{
    if (!cc)
        return;
    for (int i = cc->Size() - 1; i >= 0; --i)
        if (ShellComponentGoes((*cc)[i]->Min().Y(), cutY))
            cc->Delete(i);
}
} // namespace

float ShellWallTopOf(const LODShape* shape)
{
    if (!shape || shape->NLevels() <= 0)
        return 0.0f;
    ShellStore& st = Store();
    const auto it = st.wallTop.find(shape);
    if (it != st.wallTop.end())
        return it->second;
    const Shape* s = shape->Level(0);
    std::vector<ShellFaceStat> faces;
    if (s && s->NNorm() == s->NPos())
        for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
        {
            const Poly& p = s->Face(o);
            if (p.N() >= 3 && !Hidden(p))
                faces.push_back(FaceOf(*s, p));
        }
    const float baseY = s ? s->Min().Y() : shape->Min().Y();
    const float topY = s ? s->Max().Y() : shape->Max().Y();
    const float top = ShellWallTopY(faces, baseY, topY);
    st.wallTop.emplace(shape, top);
    return top;
}

LODShapeWithShadow* ShellShapeOf(LODShapeWithShadow* shape, ShellMode mode, int stage, std::uint32_t seed)
{
    if (!shape || shape->NLevels() <= 0)
        return nullptr;
    stage = std::clamp(stage, 0, 1);
    // (stage 0 keeps the house's own tops: one for every building of a shape)
    if (stage == 0 || mode == ShellMode::Stub)
        seed = 0;
    ShellStore& st = Store();
    const auto key = std::make_tuple(static_cast<const LODShape*>(shape), static_cast<int>(mode), stage, seed);
    const auto it = st.shells.find(key);
    if (it != st.shells.end())
        return it->second;
    const Shape* l0 = shape->Level(0);
    const float baseY = l0 ? l0->Min().Y() : shape->Min().Y();
    const float cutY = mode != ShellMode::Stub ? ShellWallTopOf(shape) : baseY + kShellStubM;
    const auto t0 = std::chrono::steady_clock::now();
    Ref<LODShapeWithShadow> c = new LODShapeWithShadow(*shape);
    // (the copy constructor leaves the map type out: a copy reads as a tree)
    c->CopyClassFrom(*shape);
    // What stands (the geometry agent's rule, FireBuildingShell.hpp): each
    // chimney carried down to the ground floor on a pier, the walls' inner
    // sides; at stage 1 what the fire took hidden (in the shadow pass too).
    // It plans on the house's own faces, so it runs first.
    ShellStats vis;
    if (mode != ShellMode::Stub)
        vis = AddBurntShellVisual(c, mode == ShellMode::Church, stage >= 1, seed);
    for (int l = 0; l < c->NLevels(); ++l)
        if (c->Resolution(l) <= 900.0f && c->Level(l))
            c->Level(l)->ReleaseVBuffer(); // (made again below, with the rubble)
    // Collision, fire and view: nothing above the cut.
    const int geo[3] = {c->FindGeometryLevel(), c->FindFireGeometryLevel(), c->FindViewGeometryLevel()};
    for (int k = 0; k < 3; ++k)
    {
        if (geo[k] < 0 || (k > 0 && geo[k] == geo[0]) || (k > 1 && geo[k] == geo[1]))
            continue;
        CutLevel(c->Level(geo[k]), cutY);
    }
    c->RebuildConvexComponents();
    const ConvexComponents* seen[3] = {nullptr, nullptr, nullptr};
    for (int k = 0; k < 3; ++k)
    {
        if (geo[k] < 0)
            continue;
        ConvexComponents* cc = c->GetConvexComponents(geo[k]);
        if (!cc || cc == seen[0] || cc == seen[1])
            continue;
        seen[k] = cc;
        DropComponents(cc, cutY);
        if (k == 2)
            cc->RecalculateEdges(c->Level(geo[k]));
    }
    // Its rubble in every visual level (coarser in the far ones), from the
    // house's own roof faces (the shell's may be hidden by now).
    const RubbleSetup rs = SetupOf(shape, cutY, 1.0f);
    int rubble = 0;
    if (rs.ok)
    {
        RubbleSetup use = rs;
        if (mode == ShellMode::Stub)
            use.frame.wallTopY = use.frame.floorY + 0.55f; // an ash bed 0.15 to 0.25 m (ShellRubbleDepth's floor)
        int visual = 0;
        for (int l = 0; l < c->NLevels(); ++l)
        {
            if (c->Resolution(l) > 900.0f)
                continue;
            const float cell = 0.7f * static_cast<float>(1 << std::min(visual, 3));
            const int beams = mode == ShellMode::Walls && visual < 2
                                  ? std::clamp(static_cast<int>(0.6f * (use.frame.hx + use.frame.hz)), 4, 10) >> visual
                                  : 0;
            rubble += AddRubble(shape->Level(l), c->Level(l), use, cutY, cell, beams);
            ++visual;
        }
    }
    c->CalculateMinMax(true);
    c->OptimizeRendering();
    LOG_INFO(Script,
             "UDFIRE;event=shellshape;shape={};mode={};stage={};cut={:.2f};base={:.2f};floor={:.2f};top={:.2f};"
             "walls={:.1f}x{:.1f};rubbleFaces={};levels={};piers={};gone={};inner={};seed={};tops={}/{};stumps={};"
             "tall={};pockets={};masonry={};ms={:.2f}",
             shape->Name(), static_cast<int>(mode), stage, cutY, baseY, rs.ok ? rs.frame.floorY : baseY,
             shape->Max().Y(), rs.ok ? 2.0f * rs.frame.hx : 0.0f, rs.ok ? 2.0f * rs.frame.hz : 0.0f, rubble, vis.levels,
             vis.piers, vis.gone, vis.inner, seed, vis.walls, vis.gables, vis.stumps, vis.gables - vis.stumps,
             vis.pockets, vis.masonryFaces,
             std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
    st.shells.emplace(key, c);
    ShellInfo inf;
    inf.source = shape;
    inf.mode = mode;
    inf.stage = stage;
    inf.hidesRoof = stage >= 1 && vis.levels > 0;
    st.info.emplace(c.GetRef(), inf);
    return c;
}

bool IsShellShape(const LODShape* shape)
{
    return shape && Store().info.count(shape) > 0;
}

int ShellStageOf(const LODShape* shape)
{
    const auto it = shape ? Store().info.find(shape) : Store().info.end();
    return it != Store().info.end() ? it->second.stage : -1;
}

const LODShape* ShellSourceOf(const LODShape* shape)
{
    const auto it = shape ? Store().info.find(shape) : Store().info.end();
    return it != Store().info.end() ? it->second.source.GetRef() : shape;
}

bool ShellHidesRoof(const LODShape* shape)
{
    const auto it = shape ? Store().info.find(shape) : Store().info.end();
    return it != Store().info.end() && it->second.hidesRoof;
}

namespace
{
// obj takes `shape`: the scene keeps the shape it met an object with while
// the object stays in its draw list (the draw, the shadow pass and the
// instanced runs read it: the geometry agent's finding), and the shadow
// cache is told.
void Wear(Object* obj, LODShapeWithShadow* shape)
{
    obj->SetShape(shape);
    if (SortObject* so = obj->GetInList())
        so->shape = shape;
    if (GScene)
        GScene->GetShadowCache().ShadowChanged(obj);
}
} // namespace

void ShellWear(Object* obj, int stage)
{
    if (!obj || !Enabled() || !obj->IsGutted() || !obj->GetShape())
        return;
    LODShapeWithShadow* cur = obj->GetShape();
    const auto it = Store().info.find(cur);
    if (it != Store().info.end() && it->second.stage == stage)
        return;
    LODShapeWithShadow* own = it != Store().info.end() ? it->second.source.GetRef() : cur;
    LODShapeWithShadow* shell = ShellShapeOf(own, ShellModeOf(own), stage, ShellSeedOf(obj));
    if (shell && shell != cur)
        Wear(obj, shell);
}

void ShellGut(Object* obj)
{
    if (!obj || !Enabled() || !obj->IsGutted() || !obj->GetShape())
        return;
    if (!IsShellShape(obj->GetShape()))
        ShellWear(obj, ShellStageNow(obj));
    NotifyGutted(obj);
}

void ShellUngut(Object* obj)
{
    if (!obj)
        return;
    const auto it = Store().info.find(obj->GetShape());
    if (it != Store().info.end())
        Wear(obj, it->second.source);
    if (Enabled())
        NotifyUngutted(obj);
}

void ShellClear()
{
    ShellStore& st = Store();
    st.shells.clear();
    st.info.clear();
    st.wallTop.clear();
}

} // namespace Poseidon::firespread
