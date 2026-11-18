#include <Poseidon/Graphics/Remaster/RainXp.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace Poseidon::remaster
{

namespace
{
bool EnvOn(const char* name)
{
    const char* v = std::getenv(name);
    return v && *v && std::strcmp(v, "0") != 0;
}
} // namespace

RainXpMode RainXpModeFromString(const char* value)
{
    if (!value || !*value)
        return RainXpMode::Off;
    if (std::strcmp(value, "1") == 0 || std::strcmp(value, "late") == 0)
        return RainXpMode::Late;
    if (std::strcmp(value, "2") == 0 || std::strcmp(value, "world") == 0)
        return RainXpMode::World;
    return RainXpMode::Off;
}

RainXpMode RainXpModeFromEnv()
{
    // Unset or empty: World, the shipped look (owner verdict 2026-11-12, "Rain
    // looks right"). POSEIDON_XP_RAIN=0 gives today's overlay, =late the
    // overlay after the TAA resolve.
    static const RainXpMode mode = RainXpModeFromEnvValue(std::getenv("POSEIDON_XP_RAIN"));
    return mode;
}

RainXpMode RainXpModeFromEnvValue(const char* value)
{
    return (value && *value) ? RainXpModeFromString(value) : RainXpMode::World;
}

bool RainXpRound1FromEnv()
{
    static const bool on = EnvOn("POSEIDON_XP_RAIN_R1");
    return on;
}

bool RainXpProbeFromEnv()
{
    static const bool on = EnvOn("POSEIDON_XP_RAIN_PROBE");
    return on;
}

void RainWrapShift(const double camPos[3], const double travel[3], const float boxMin[3], const float size[3],
                   float out[3])
{
    for (int i = 0; i < 3; ++i)
    {
        const double s = std::max(static_cast<double>(size[i]), 1e-3);
        double v = std::fmod(travel[i] - camPos[i] - static_cast<double>(boxMin[i]), s);
        if (v < 0.0)
            v += s;
        if (v >= s)
            v = 0.0;
        out[i] = static_cast<float>(v);
    }
}

void RainDriftStep(RainXpDrift& drift, double t, float density, const float wind[3], const double camPos[3],
                   RainXpFrame& out)
{
    const float fallNow = RainFallSpeed(density);
    // The wind as the rain takes it: level, and held to a gale.
    float windNow[3] = {wind[0], 0.0f, wind[2]};
    const float w2 = windNow[0] * windNow[0] + windNow[2] * windNow[2];
    if (w2 > kRainWindMax * kRainWindMax)
    {
        const float k = kRainWindMax / std::sqrt(w2);
        windNow[0] *= k;
        windNow[2] *= k;
    }
    double dt = t - drift.time;
    const bool restart = !drift.valid || dt < 0.0 || dt > 0.5;
    if (restart)
    {
        dt = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            drift.wind[i] = windNow[i];
            drift.camVel[i] = 0.0f;
            drift.r1Speed[i] = 0.0f;
        }
        drift.fall = fallNow;
    }
    else if (dt > 0.0)
    {
        // The camera's velocity over the step (none over a cut), eased.
        double v[3];
        double v2 = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            v[i] = (camPos[i] - drift.camPos[i]) / dt;
            v2 += v[i] * v[i];
        }
        const bool cut = v2 >= static_cast<double>(kRainCutSpeed) * kRainCutSpeed;
        const float kc = 1.0f - std::exp(-static_cast<float>(dt) / kRainCamEase);
        const float kw = 1.0f - std::exp(-static_cast<float>(dt) / kRainWindEase);
        for (int i = 0; i < 3; ++i)
        {
            if (cut)
                drift.camVel[i] = 0.0f;
            else
                drift.camVel[i] += (static_cast<float>(v[i]) - drift.camVel[i]) * kc;
            drift.wind[i] += (windNow[i] - drift.wind[i]) * kw;
        }
        drift.fall += (fallNow - drift.fall) * kw;
    }
    // Round 1's placement (the probe): the velocity of the moment times the
    // whole clock.
    {
        const double vel[3] = {windNow[0], -fallNow, windNow[2]};
        for (int i = 0; i < 3; ++i)
        {
            const double place = vel[i] * t;
            if (!restart && dt > 0.0)
                drift.r1Speed[i] = static_cast<float>((place - drift.r1Place[i]) / dt);
            drift.r1Place[i] = place;
            drift.r1Vel[i] = static_cast<float>(vel[i]);
        }
    }
    for (int l = 0; l < kRainLayers; ++l)
    {
        const float fall = drift.fall * kRainLayerFall[l];
        // The drops' velocity relative to the camera, held so they keep
        // falling on screen.
        float rel[3] = {drift.wind[0] * kRainLayerWind[l] - drift.camVel[0], -fall - drift.camVel[1],
                        drift.wind[2] * kRainLayerWind[l] - drift.camVel[2]};
        rel[1] = std::min(rel[1], -kRainKeepFall * fall);
        const float side = std::sqrt(rel[0] * rel[0] + rel[2] * rel[2]);
        const float sideMax = kRainMaxSlant * fall;
        if (side > sideMax)
        {
            rel[0] *= sideMax / side;
            rel[2] *= sideMax / side;
        }
        for (int i = 0; i < 3; ++i)
        {
            // In the world the drops move at the held relative velocity plus
            // the camera's.
            double tr = drift.travel[l][i] + (static_cast<double>(rel[i]) + drift.camVel[i]) * dt;
            tr = std::fmod(tr, kRainTravelFold);
            drift.travel[l][i] = tr;
            out.travel[l][i] = tr;
            out.relVel[l][i] = rel[i];
        }
    }
    drift.valid = true;
    drift.time = t;
    for (int i = 0; i < 3; ++i)
    {
        drift.camPos[i] = camPos[i];
        out.camPos[i] = camPos[i];
    }
    out.density = density;
}

void RainDriftAsRound1(const RainXpDrift& drift, RainXpFrame& out)
{
    for (int l = 0; l < kRainLayers; ++l)
    {
        for (int i = 0; i < 3; ++i)
        {
            out.travel[l][i] = drift.r1Place[i];
            out.relVel[l][i] = drift.r1Vel[i] - drift.camVel[i];
        }
    }
}

float RainFallSpeed(float density)
{
    const float d = std::min(std::max(density, 0.0f), 1.0f);
    return kRainFallSlow + (kRainFallFast - kRainFallSlow) * d;
}

void RainCoverGrid::Clear()
{
    std::fill(std::begin(clear), std::end(clear), 1.0f);
    std::fill(std::begin(depth), std::end(depth), 0.0f);
    std::fill(std::begin(weight), std::end(weight), 0.0f);
    puffs = 0;
}

void RainCoverGrid::Add(float x, float y, float sx, float sy, float w, float h, float opacity, float depthM)
{
    if (!(w > 0.0f) || !(h > 0.0f) || !(sx > 0.0f) || !(sy > 0.0f) || !(opacity > 0.004f) || !(depthM > 0.0f))
        return;
    // In cells.
    const float cx = x / w * kRainCoverW;
    const float cy = y / h * kRainCoverH;
    const float rx = sx / w * kRainCoverW;
    const float ry = sy / h * kRainCoverH;
    const int x0 = std::max(0, static_cast<int>(std::floor(cx - rx)));
    const int x1 = std::min(kRainCoverW - 1, static_cast<int>(std::floor(cx + rx)));
    const int y0 = std::max(0, static_cast<int>(std::floor(cy - ry)));
    const int y1 = std::min(kRainCoverH - 1, static_cast<int>(std::floor(cy + ry)));
    if (x0 > x1 || y0 > y1)
        return;
    ++puffs;
    // A puff's texture is a soft blob: about 0.7 of its opacity at the centre,
    // none at the rim. One smaller than a cell covers its share of the cell.
    constexpr float kCentre = 0.7f;
    const float small = std::min(1.0f, 4.0f * rx * ry);
    const float irx = 1.0f / std::max(rx, 0.5f);
    const float iry = 1.0f / std::max(ry, 0.5f);
    for (int j = y0; j <= y1; ++j)
    {
        const float dy = (static_cast<float>(j) + 0.5f - cy) * iry;
        for (int i = x0; i <= x1; ++i)
        {
            const float dx = (static_cast<float>(i) + 0.5f - cx) * irx;
            const float r2 = dx * dx + dy * dy;
            if (r2 >= 1.0f)
                continue;
            const float a = std::min(opacity, 1.0f) * kCentre * (1.0f - r2) * small;
            const int k = j * kRainCoverW + i;
            clear[k] *= 1.0f - a;
            depth[k] += a * depthM;
            weight[k] += a;
        }
    }
}

void RainCoverGrid::Cell(int i, float& cover, float& depthM) const
{
    cover = 1.0f - clear[i];
    depthM = weight[i] > 1e-6f ? depth[i] / weight[i] : 0.0f;
}

} // namespace Poseidon::remaster
