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

LightSphere LightBounds(const FrameLight& light, const PhysicalLightParams* physical)
{
    const float r = light.hdr                                         ? std::fabs(light.range)
                    : physical && UsesPhysicalModel(light.kind) ? PhysicalReach(light, *physical)
                                                                      : kRangeFactor * std::fabs(light.start);
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

bool LightScreenRect(const FrameLight& light, const TileView& view, float rect[4], const PhysicalLightParams* physical)
{
    const float vx0 = view.viewport[0], vy0 = view.viewport[1];
    const float vw = view.viewport[2], vh = view.viewport[3];
    if (vw <= 0.0f || vh <= 0.0f)
        return false;
    const LightSphere s = LightBounds(light, physical);
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

void BinLightsToTiles(const std::vector<FrameLight>& lights, const TileView& view, TileGrid& grid,
                      const PhysicalLightParams* physical)
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
        if (!LightScreenRect(lights[static_cast<size_t>(i)], view, rect, physical))
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

bool UsesPhysicalModel(FrameLightKind kind)
{
    return kind == FrameLightKind::Lamp || kind == FrameLightKind::Spot;
}

namespace
{

float Smoothstep(float e0, float e1, float x)
{
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float Dot(const float a[3], const float b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

} // namespace

float LowBeamPattern(const float toPoint[3], const float beam[3])
{
    const float c = Dot(toPoint, beam);
    if (c <= 0.0f)
        return 0.0f;
    // The beam's frame: right = up x beam, u = beam x right (y up).
    float right[3] = {beam[2], 0.0f, -beam[0]};
    const float rl = std::sqrt(std::max(right[0] * right[0] + right[2] * right[2], 1e-12f));
    right[0] /= rl;
    right[2] /= rl;
    const float u[3] = {beam[1] * right[2] - beam[2] * right[1], beam[2] * right[0] - beam[0] * right[2],
                        beam[0] * right[1] - beam[1] * right[0]};
    const float ah = std::atan2(Dot(toPoint, right), c);
    const float av = std::atan2(Dot(toPoint, u), c);
    const float cut = 1.0f - Smoothstep(kLowBeamCutLow, kLowBeamCutHigh, av);
    const float hv = (av - kLowBeamHotBand) / kLowBeamHotWidth;
    const float vert = std::exp(-hv * hv) + kLowBeamFloor * Smoothstep(kLowBeamFloorEnd, kLowBeamFloorStart, av);
    const float h1 = ah / kLowBeamCoreWidth;
    const float h2 = ah / kLowBeamSpillWidth;
    const float horiz = std::exp(-h1 * h1) + kLowBeamSpill * std::exp(-h2 * h2);
    return vert * horiz * cut;
}

void LowBeamAim(const float lampDir[3], const float bodyForward[3], float out[3])
{
    float hx = lampDir[0], hz = lampDir[2];
    float hl = std::sqrt(hx * hx + hz * hz);
    const float fl = std::sqrt(bodyForward[0] * bodyForward[0] + bodyForward[2] * bodyForward[2]);
    if (hl < 1e-4f)
    {
        // A lamp pointing straight up or down: take the body's heading.
        hx = bodyForward[0];
        hz = bodyForward[2];
        hl = fl;
    }
    if (hl < 1e-4f || fl < 1e-4f)
    {
        // No heading at all (a body pointing straight up): keep the model's axis.
        const float l = std::sqrt(std::max(Dot(lampDir, lampDir), 1e-12f));
        for (int i = 0; i < 3; ++i)
            out[i] = lampDir[i] / l;
        return;
    }
    hx /= hl;
    hz /= hl;
    // The body's slope along the lamp's heading (its forward slope projected
    // onto that heading), then the tilt under it.
    const float along = (bodyForward[0] * hx + bodyForward[2] * hz) / fl;
    const float slope = bodyForward[1] / fl * along - kLowBeamTilt;
    const float l = std::sqrt(1.0f + slope * slope);
    out[0] = hx / l;
    out[1] = slope / l;
    out[2] = hz / l;
}

float PhysicalReach(const FrameLight& light, const PhysicalLightParams& params)
{
    const float peak = light.IsSpot() ? params.spotPeak : params.lampPerStart * light.start;
    const float classic2 = kRangeFactor * kRangeFactor * light.start * light.start;
    return std::sqrt(std::min(classic2, std::max(peak, 0.0f) / kPhysicalMinIrradiance));
}

void PhysicalIrradiance(const FrameLight& light, const float p[3], const PhysicalLightParams& params, float out[3])
{
    out[0] = out[1] = out[2] = 0.0f;
    if (!UsesPhysicalModel(light.kind))
        return;
    const float rel[3] = {p[0] - light.pos[0], p[1] - light.pos[1], p[2] - light.pos[2]};
    const float d2 = Dot(rel, rel);
    const float reach = PhysicalReach(light, params);
    const float end2 = reach * reach;
    if (d2 >= end2)
        return;
    const float d = std::sqrt(std::max(d2, 1e-12f));
    float intensity = 0.0f;
    if (light.IsSpot())
    {
        const float toPoint[3] = {rel[0] / d, rel[1] / d, rel[2] / d};
        intensity = params.spotPeak * LowBeamPattern(toPoint, light.dir);
    }
    else
    {
        intensity = params.lampPerStart * light.start;
    }
    intensity *= (1.0f - Smoothstep(0.5f * end2, end2, d2)) * Smoothstep(kPhysicalNearStart, kPhysicalNearEnd, d);
    const float e = intensity / (d2 + kPhysicalSoftening);
    float colour[3];
    PhysicalColour(light, colour);
    for (int i = 0; i < 3; ++i)
        out[i] = colour[i] * e;
}

void PhysicalColour(const FrameLight& light, float out[3])
{
    for (int i = 0; i < 3; ++i)
        out[i] = std::pow(std::max(light.diffuse[i], 0.0f), 2.2f);
    if (!light.IsSpot())
        return;
    // The shaders' kLuma (Rec. 709).
    const float y = 0.2126f * out[0] + 0.7152f * out[1] + 0.0722f * out[2];
    out[0] = out[1] = out[2] = y;
}

void ParticleLight(const std::vector<FrameLight>& lights, const float p[3], const PhysicalLightParams& params,
                   float out[3])
{
    out[0] = out[1] = out[2] = 0.0f;
    for (const FrameLight& l : lights)
    {
        float e[3];
        PhysicalIrradiance(l, p, params, e);
        for (int i = 0; i < 3; ++i)
            out[i] += e[i];
    }
}

} // namespace Poseidon::lighting
