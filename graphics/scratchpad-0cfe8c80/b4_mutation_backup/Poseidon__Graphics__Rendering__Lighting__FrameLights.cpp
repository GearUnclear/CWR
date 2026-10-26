#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::lighting
{

namespace
{

float Importance(const FrameLight& l)
{
    const float d2 = l.pos[0] * l.pos[0] + l.pos[1] * l.pos[1] + l.pos[2] * l.pos[2];
    return l.sortBrightness / std::max(d2, 1e-6f);
}

// The view-space slopes (a / z) of the two tangents from the eye to the circle
// of radius r around (a, z), for z > r: the reach's extent along one screen
// axis.
void TangentSlopes(float a, float z, float r, float& lo, float& hi)
{
    const float t = std::sqrt(std::max(a * a + z * z - r * r, 0.0f));
    // Both denominators are positive for z > r: (zt)^2 - (ra)^2 = (z^2 - r^2)(a^2 + z^2).
    lo = (a * t - r * z) / (z * t + r * a);
    hi = (a * t + r * z) / (z * t - r * a);
}

} // namespace

const char* FrameLightKindName(FrameLightKind kind)
{
    switch (kind)
    {
        case FrameLightKind::Classic:
            return "classic";
        case FrameLightKind::Lamp:
            return "lamp";
        case FrameLightKind::Spot:
            return "spot";
        case FrameLightKind::Fire:
            return "fire";
        case FrameLightKind::Combat:
            return "combat";
    }
    return "unknown";
}

bool LightIsDark(const FrameLight& light)
{
    for (int i = 0; i < 3; ++i)
    {
        if (light.diffuse[i] > 0.0f || light.ambient[i] > 0.0f)
            return false;
    }
    return true;
}

LightSphere LightBounds(const FrameLight& light)
{
    const float r = light.hdr ? std::fabs(light.range) : kRangeFactor * std::fabs(light.start);
    return LightSphere{{light.pos[0], light.pos[1], light.pos[2]}, r};
}

void RankFrameLights(std::vector<FrameLight>& lights, int capacity)
{
    lights.erase(std::remove_if(lights.begin(), lights.end(), LightIsDark), lights.end());
    std::stable_sort(lights.begin(), lights.end(),
                     [](const FrameLight& a, const FrameLight& b) { return Importance(a) > Importance(b); });
    if (capacity >= 0 && static_cast<int>(lights.size()) > capacity)
        lights.resize(static_cast<size_t>(capacity));
}

int MergeCombatFrameLights(std::vector<FrameLight>& classicRanked, const std::vector<FrameLight>& combat)
{
    std::vector<FrameLight> merged;
    merged.reserve(std::min<size_t>(combat.size() + classicRanked.size(), kMaxFrameLights));
    int n = 0;
    for (const FrameLight& l : combat)
    {
        if (n >= kMaxCombatFrameLights)
            break;
        if (LightIsDark(l))
            continue;
        merged.push_back(l);
        merged.back().hdr = true;
        ++n;
    }
    for (const FrameLight& l : classicRanked)
    {
        if (static_cast<int>(merged.size()) >= kMaxFrameLights)
            break;
        merged.push_back(l);
    }
    classicRanked.swap(merged);
    return n;
}

void ResizeTileGrid(TileGrid& grid, int targetW, int targetH, int tileSize)
{
    grid.tileSize = tileSize > 0 ? tileSize : kTileSize;
    grid.tilesX = targetW > 0 ? (targetW + grid.tileSize - 1) / grid.tileSize : 0;
    grid.tilesY = targetH > 0 ? (targetH + grid.tileSize - 1) / grid.tileSize : 0;
    grid.indices.assign(static_cast<size_t>(grid.tilesX) * grid.tilesY * kMaxLightsPerTile, 0);
    grid.refs = 0;
    grid.dropped = 0;
    grid.litTiles = 0;
}

bool LightScreenRect(const FrameLight& light, const TileView& view, float rect[4])
{
    const float vx0 = view.viewport[0], vy0 = view.viewport[1];
    const float vw = view.viewport[2], vh = view.viewport[3];
    if (vw <= 0.0f || vh <= 0.0f)
        return false;
    const LightSphere s = LightBounds(light);
    float v[3];
    for (int r = 0; r < 3; ++r)
        v[r] = view.rot[r][0] * s.c[0] + view.rot[r][1] * s.c[1] + view.rot[r][2] * s.c[2];
    const float r = s.r;
    // Behind the eye, or wholly outside one side of the view frustum (the four
    // side planes pass through the eye: |x| = z / p11, |y| = z / p22).
    if (v[2] + r <= 0.0f)
        return false;
    const float ix = 1.0f / std::fabs(view.p11), iy = 1.0f / std::fabs(view.p22);
    const float nx = std::sqrt(1.0f + ix * ix), ny = std::sqrt(1.0f + iy * iy);
    if ((v[0] - v[2] * ix) / nx >= r || (-v[0] - v[2] * ix) / nx >= r || (v[1] - v[2] * iy) / ny >= r ||
        (-v[1] - v[2] * iy) / ny >= r)
        return false;

    float x0 = vx0, x1 = vx0 + vw, y0 = vy0, y1 = vy0 + vh;
    if (v[2] - r >= view.nearZ)
    {
        float lo, hi;
        TangentSlopes(v[0], v[2], r, lo, hi);
        float a = view.p11 * lo, b = view.p11 * hi;
        x0 = vx0 + (std::min(a, b) * 0.5f + 0.5f) * vw;
        x1 = vx0 + (std::max(a, b) * 0.5f + 0.5f) * vw;
        TangentSlopes(v[1], v[2], r, lo, hi);
        a = view.p22 * lo;
        b = view.p22 * hi;
        y0 = vy0 + (std::min(a, b) * 0.5f + 0.5f) * vh;
        y1 = vy0 + (std::max(a, b) * 0.5f + 0.5f) * vh;
        // One pixel of slack for the GPU's rounding of the same projection.
        x0 -= 1.0f;
        y0 -= 1.0f;
        x1 += 1.0f;
        y1 += 1.0f;
    }
    // Clip to the viewport's pixels (inclusive bounds).
    x0 = std::max(x0, vx0);
    y0 = std::max(y0, vy0);
    x1 = std::min(x1, vx0 + vw - 1.0f);
    y1 = std::min(y1, vy0 + vh - 1.0f);
    if (x1 < x0 || y1 < y0)
        return false;
    rect[0] = x0;
    rect[1] = y0;
    rect[2] = x1;
    rect[3] = y1;
    return true;
}

void BinLightsToTiles(const std::vector<FrameLight>& lights, const TileView& view, TileGrid& grid)
{
    std::fill(grid.indices.begin(), grid.indices.end(), std::uint8_t{0});
    grid.refs = 0;
    grid.dropped = 0;
    grid.litTiles = 0;
    if (grid.tilesX <= 0 || grid.tilesY <= 0)
        return;
    const int n = std::min(static_cast<int>(lights.size()), kMaxFrameLights);
    const float ts = static_cast<float>(grid.tileSize);
    for (int i = 0; i < n; ++i)
    {
        float rect[4];
        if (!LightScreenRect(lights[static_cast<size_t>(i)], view, rect))
            continue;
        const int tx0 = std::clamp(static_cast<int>(std::floor(rect[0] / ts)), 0, grid.tilesX - 1);
        const int ty0 = std::clamp(static_cast<int>(std::floor(rect[1] / ts)), 0, grid.tilesY - 1);
        const int tx1 = std::clamp(static_cast<int>(std::floor(rect[2] / ts)), 0, grid.tilesX - 1);
        const int ty1 = std::clamp(static_cast<int>(std::floor(rect[3] / ts)), 0, grid.tilesY - 1);
        for (int ty = ty0; ty <= ty1; ++ty)
        {
            for (int tx = tx0; tx <= tx1; ++tx)
            {
                std::uint8_t* tile =
                    grid.indices.data() + (static_cast<size_t>(ty) * grid.tilesX + tx) * kMaxLightsPerTile;
                int k = 0;
                while (k < kMaxLightsPerTile && tile[k] != 0)
                    ++k;
                if (k == kMaxLightsPerTile)
                {
                    ++grid.dropped;
                    continue;
                }
                if (k == 0)
                    ++grid.litTiles;
                tile[k] = static_cast<std::uint8_t>(i + 1);
                ++grid.refs;
            }
        }
    }
}

int TileLightsAt(const TileGrid& grid, int px, int py, int out[kMaxLightsPerTile])
{
    if (px < 0 || py < 0 || grid.tileSize <= 0)
        return 0;
    const int tx = px / grid.tileSize, ty = py / grid.tileSize;
    if (tx >= grid.tilesX || ty >= grid.tilesY)
        return 0;
    const std::uint8_t* tile = grid.indices.data() + (static_cast<size_t>(ty) * grid.tilesX + tx) * kMaxLightsPerTile;
    int n = 0;
    while (n < kMaxLightsPerTile && tile[n] != 0)
    {
        out[n] = tile[n] - 1;
        ++n;
    }
    return n;
}

void PackFrameLights(const std::vector<FrameLight>& lights, std::vector<float>& texels)
{
    texels.assign(lights.size() * kTexelsPerLight * 4, 0.0f);
    float* t = texels.data();
    for (const FrameLight& l : lights)
    {
        t[0] = l.pos[0];
        t[1] = l.pos[1];
        t[2] = l.pos[2];
        t[3] = l.start;
        t[4] = l.diffuse[0];
        t[5] = l.diffuse[1];
        t[6] = l.diffuse[2];
        t[7] = l.hdr ? 2.0f : (l.IsSpot() ? 1.0f : 0.0f);
        if (l.hdr)
        {
            t[8] = l.range;
            t[9] = static_cast<float>(l.shadowSlot + 1);
            t[10] = 0.0f;
            t[11] = 0.0f;
        }
        else
        {
            t[8] = l.ambient[0];
            t[9] = l.ambient[1];
            t[10] = l.ambient[2];
            t[11] = l.sortBrightness;
        }
        t[12] = l.dir[0];
        t[13] = l.dir[1];
        t[14] = l.dir[2];
        t[15] = static_cast<float>(static_cast<int>(l.kind));
        t += kTexelsPerLight * 4;
    }
}

float ClassicImportance(const FrameLight& light, const float p[3])
{
    const LightSphere s = LightBounds(light);
    float d2 = 0.0f, c2 = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        d2 += (light.pos[i] - p[i]) * (light.pos[i] - p[i]);
        c2 += (s.c[i] - p[i]) * (s.c[i] - p[i]);
    }
    const float r2 = s.r * s.r;
    const float t = std::clamp((c2 - 0.5f * r2) / std::max(0.5f * r2, 1e-12f), 0.0f, 1.0f);
    const float fade = 1.0f - t * t * (3.0f - 2.0f * t); // 1 - smoothstep(0.5 r2, r2, c2)
    return fade * light.sortBrightness / std::max(d2, 1e-4f);
}

void ClassicBudgetWeights(const float* importance, int n, float* weight)
{
    const int budget = kClassicLightsPerSurface;
    float edgeIn = 0.0f, edgeOut = 0.0f;
    if (n > budget)
    {
        // The strongest budget + 1 kept sorted, as the shader keeps them in
        // registers: the last two are the 7th and the 8th.
        float top[kClassicLightsPerSurface + 1] = {};
        for (int i = 0; i < n; ++i)
        {
            float v = importance[i];
            for (float& t : top)
            {
                const float m = std::max(t, v);
                v = std::min(t, v);
                t = m;
            }
        }
        edgeIn = top[budget - 1];
        edgeOut = top[budget];
    }
    const float mid = 0.5f * (edgeIn + edgeOut);
    const float band = std::max(edgeIn - edgeOut, kClassicBudgetSoftness * edgeIn);
    for (int k = 0; k < n; ++k)
        weight[k] = n > budget ? std::clamp(0.5f + (importance[k] - mid) / band, 0.0f, 1.0f) : 1.0f;
}

} // namespace Poseidon::lighting
