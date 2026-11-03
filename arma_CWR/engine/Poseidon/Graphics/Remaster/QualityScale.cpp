#include <Poseidon/Graphics/Remaster/QualityScale.hpp>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace Poseidon::remaster
{

QualityProfile ProfileFor(QualityLevel level)
{
    QualityProfile p;
    p.level = level;
    switch (level)
    {
        case QualityLevel::Low:
            p.identity = false;
            p.renderObjectCap = 1200.0f;
            p.farLodK = 3.0f;
            p.farLodNear = 300.0f;
            p.farLodFar = 1000.0f;
            p.shadowResolution = 1024;
            p.shadowFarCap = 150.0f;
            p.fogQuality = 0.0f;
            p.ao = false;
            p.softShadows = false;
            p.canopyShadows = false;
            p.airBeams = false;
            p.ssr = false;
            break;
        case QualityLevel::Medium:
            p.identity = false;
            p.renderObjectCap = 2000.0f;
            p.farLodK = 2.0f;
            p.farLodNear = 600.0f;
            p.farLodFar = 1500.0f;
            p.shadowFarCap = 300.0f;
            p.fogQuality = 0.0f;
            break;
        case QualityLevel::High:
            break;
        case QualityLevel::Ultra:
            p.identity = false;
            p.renderObjectViewRatio = 0.8f;
            p.renderObjectCap = 4000.0f;
            p.shadowResolution = 4096;
            p.shadowViewRatio = 250.0f / 900.0f;
            p.shadowFarCap = 800.0f;
            p.fogQuality = 2.0f;
            break;
    }
    return p;
}

const char* QualityLevelName(QualityLevel level)
{
    switch (level)
    {
        case QualityLevel::Low:
            return "low";
        case QualityLevel::Medium:
            return "medium";
        case QualityLevel::High:
            return "high";
        case QualityLevel::Ultra:
            return "ultra";
    }
    return "high";
}

bool ParseQualityLevel(const char* text, QualityLevel& out)
{
    if (!text || !*text)
        return false;
    char buf[16] = {};
    for (int i = 0; i < 15 && text[i]; ++i)
        buf[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
    struct Name
    {
        const char* a;
        const char* b;
        QualityLevel level;
    };
    static const Name kNames[] = {{"low", "0", QualityLevel::Low},
                                  {"medium", "1", QualityLevel::Medium},
                                  {"high", "2", QualityLevel::High},
                                  {"ultra", "3", QualityLevel::Ultra}};
    for (const Name& n : kNames)
    {
        if (std::strcmp(buf, n.a) == 0 || std::strcmp(buf, n.b) == 0)
        {
            out = n.level;
            return true;
        }
    }
    return false;
}

float FarLodFactor(const QualityProfile& p, float distance)
{
    if (p.farLodK == 1.0f || distance <= p.farLodNear)
        return 1.0f;
    if (distance >= p.farLodFar || p.farLodFar <= p.farLodNear)
        return p.farLodK;
    float t = (distance - p.farLodNear) / (p.farLodFar - p.farLodNear);
    t = t * t * (3.0f - 2.0f * t); // smoothstep: no kink where the ramp starts
    return 1.0f + (p.farLodK - 1.0f) * t;
}

float RenderObjectDistance(const QualityProfile& p, float objectsZ, float viewZ)
{
    if (p.identity)
        return objectsZ;
    float d = p.renderObjectViewRatio > 0.0f ? std::max(objectsZ, viewZ * p.renderObjectViewRatio) : objectsZ;
    if (p.renderObjectCap > 0.0f)
        d = std::min(d, std::max(p.renderObjectCap, p.renderObjectViewRatio > 0.0f ? objectsZ : 0.0f));
    // Never past the terrain's own far plane.
    return std::min(d, std::max(viewZ, objectsZ));
}

float ShadowReach(const QualityProfile& p, float shadowsZ, float viewZ, float objectsZ)
{
    if (p.identity)
        return shadowsZ;
    float reach = shadowsZ;
    if (p.shadowViewRatio > 0.0f)
    {
        reach = std::max(shadowsZ, viewZ * p.shadowViewRatio);
        if (p.shadowFarCap > 0.0f)
            reach = std::min(reach, std::max(p.shadowFarCap, shadowsZ));
        reach = std::min(reach, std::max(RenderObjectDistance(p, objectsZ, viewZ), shadowsZ));
    }
    else if (p.shadowFarCap > 0.0f)
    {
        reach = std::min(reach, p.shadowFarCap);
    }
    return reach;
}

void ApplyQualityToTuning(const QualityProfile& p, LightingTuning& t)
{
    if (p.identity)
        return;
    if (p.fogQuality >= 0.0f)
        t.fogQuality = p.fogQuality;
    if (!p.ao)
        t.ambientOcclusion = 0.0f;
    if (!p.softShadows)
        t.features.softShadows = 0.0f;
    if (!p.canopyShadows)
        t.features.canopyShadows = 0.0f;
    if (!p.airBeams)
        t.features.airBeams = 0.0f;
    if (!p.ssr)
        t.features.ssr = 0.0f;
}

namespace
{
QualityProfile s_profile;
std::atomic<bool> s_active{false};
std::atomic<bool> s_change{false};
bool s_envForced = false;

bool InitFromEnv()
{
    QualityLevel level;
    if (ParseQualityLevel(std::getenv("POSEIDON_XP_PRESET"), level))
    {
        s_profile = ProfileFor(level);
        s_envForced = true;
        s_change.store(!s_profile.identity);
        s_active.store(true);
    }
    return true;
}

void EnsureInit()
{
    static const bool once = InitFromEnv();
    (void)once;
}
} // namespace

const QualityProfile* ActiveQuality()
{
    EnsureInit();
    return s_active.load(std::memory_order_acquire) ? &s_profile : nullptr;
}

const QualityProfile* ActiveQualityChange()
{
    EnsureInit();
    return s_change.load(std::memory_order_acquire) ? &s_profile : nullptr;
}

void SetActiveQualityLevel(const QualityLevel* level)
{
    EnsureInit();
    if (s_envForced)
        return;
    s_active.store(false);
    s_change.store(false);
    if (!level)
        return;
    s_profile = ProfileFor(*level);
    s_change.store(!s_profile.identity, std::memory_order_release);
    s_active.store(true, std::memory_order_release);
}

bool QualityLevelForcedByEnv()
{
    EnsureInit();
    return s_envForced;
}

float ActiveRenderObjectDistance(float objectsZ, float viewZ)
{
    const QualityProfile* p = ActiveQualityChange();
    return p ? RenderObjectDistance(*p, objectsZ, viewZ) : objectsZ;
}

float ActiveShadowReach(float shadowsZ, float viewZ, float objectsZ)
{
    const QualityProfile* p = ActiveQualityChange();
    return p ? ShadowReach(*p, shadowsZ, viewZ, objectsZ) : shadowsZ;
}

} // namespace Poseidon::remaster
