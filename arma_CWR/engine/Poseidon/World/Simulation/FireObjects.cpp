#include <Poseidon/World/Simulation/FireObjects.hpp>

namespace Poseidon::firespread
{

const char* WreckClassName(WreckClass c)
{
    switch (c)
    {
        case WreckClass::Truck:
            return "truck";
        case WreckClass::Tanker:
            return "tanker";
        case WreckClass::Armour:
            return "armour";
        default:
            return "car";
    }
}

namespace
{
float Smooth(float e0, float e1, float x)
{
    if (e1 <= e0)
        return x < e0 ? 0.0f : 1.0f;
    const float k = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return k * k * (3.0f - 2.0f * k);
}
} // namespace

WreckTimeline WreckTimelineOf(const WreckConfig& c, std::uint32_t seed)
{
    ObjRand r(seed * 2654435761U + 0x51ed27U);
    auto U = [&](float a, float b) { return a + (b - a) * r.U(); };
    WreckTimeline w;
    w.cls = c.cls;
    w.fuelBurning = c.fuelBurning;
    w.ammo = c.ammo && c.cls == WreckClass::Armour;
    w.sizeK = std::clamp(c.sizeK, 0.3f, 1.0f);
    switch (c.cls)
    {
        case WreckClass::Truck:
        case WreckClass::Tanker:
            if (c.fuelBurning)
            {
                w.fuelSec = U(180.0f, 360.0f);
                w.fuelFlameM = U(4.0f, 6.0f);
                w.emberFuelSec = std::min(w.fuelSec, U(120.0f, 180.0f));
                w.poolAreaM2 = U(10.0f, 25.0f);
                w.poolFlameM = U(0.6f, 1.2f);
                w.poolSec = w.emberFuelSec;
            }
            w.bodyPeakSec = U(600.0f, 1200.0f);
            w.bodyFlameM = U(5.0f, 7.0f);
            w.flameEndSec = U(3600.0f, 5400.0f);
            w.bodyHoldSec = w.flameEndSec * U(0.55f, 0.7f);
            w.tyreSec = w.flameEndSec - U(600.0f, 900.0f);
            w.tyreFlameM = U(3.0f, 4.0f);
            w.smoulderEndSec = w.flameEndSec + U(900.0f, 2400.0f);
            w.smokeEndSec = U(7200.0f, 14400.0f);
            w.hotEndSec = U(14400.0f, 21600.0f);
            w.glowEndSec = U(1800.0f, 2700.0f);
            w.emberK = 2.0f;
            w.heatReachM = U(3.0f, 4.0f);
            if (c.cls == WreckClass::Tanker)
            {
                // The tank opened: what the fireball did not take pours out
                // and burns as a pool (petrol at about 2.4 MW a square metre,
                // its level falling 4 to 5 mm a minute).
                const float loadM3 = std::max(c.fuelCargoL, 0.0f) / 1000.0f;
                const float ballM3 =
                    std::min(kTankerBallMaxKg, std::max(kTankerBallMinKg, 0.2f * loadM3 * 740.0f)) / 740.0f;
                const float poolM3 = std::max(loadM3 - ballM3, 0.3f * loadM3);
                const float area = std::max(kPoolAreaMinM2, 100.0f * std::clamp(poolM3 / kPoolFullM3, 0.0f, 1.0f));
                w.poolAreaM2 = area;
                const float d = std::sqrt(4.0f * area / 3.14159265f);
                const float q = 2400.0f * area; // kW
                w.poolFlameM = std::max(2.0f, (0.235f * std::pow(q, 0.4f) - 1.02f * d) * U(0.92f, 1.15f));
                const float minutes = poolM3 / (area * 0.0045f);
                w.poolSec = std::clamp(60.0f * minutes * U(0.85f, 1.15f), 300.0f, 900.0f);
                w.poolReachM = U(15.0f, 25.0f) * std::sqrt(area / 100.0f);
            }
            break;
        case WreckClass::Armour:
            if (c.fuelBurning)
            {
                w.fuelSec = U(480.0f, 720.0f);
                w.fuelFlameM = U(4.0f, 6.0f);
                w.emberFuelSec = U(120.0f, 180.0f);
            }
            w.bodyPeakSec = std::max(w.fuelSec, U(300.0f, 600.0f));
            w.bodyFlameM = U(3.0f, 5.0f);
            w.flameEndSec = U(3600.0f, 10800.0f);
            w.bodyHoldSec = w.flameEndSec * U(0.55f, 0.7f);
            w.tyreSec = w.flameEndSec * U(0.78f, 0.88f);
            w.tyreFlameM = U(1.5f, 2.5f);
            w.smoulderEndSec = w.flameEndSec + U(1200.0f, 3600.0f);
            w.smokeEndSec = w.flameEndSec + U(3600.0f, 7200.0f);
            w.hotEndSec = U(21600.0f, 43200.0f);
            w.glowEndSec = U(2400.0f, 3600.0f);
            w.emberK = 1.0f;
            w.heatReachM = U(3.0f, 4.0f);
            if (w.ammo)
            {
                // Propellant charges flaring from the hatches (white-yellow,
                // 5 to 20 m, 2 to 10 s each, several), and now and then a
                // high-explosive round going off (0 to 2).
                w.jets = 3 + static_cast<int>(r.U() * 4.0f);
                for (int i = 0; i < w.jets; ++i)
                {
                    w.jetAt[i] = U(120.0f, 1800.0f);
                    w.jetSec[i] = U(2.0f, 10.0f);
                    w.jetM[i] = U(5.0f, 20.0f);
                }
                for (int i = 1; i < w.jets; ++i)
                    for (int k = i; k > 0 && w.jetAt[k] < w.jetAt[k - 1]; --k)
                    {
                        std::swap(w.jetAt[k], w.jetAt[k - 1]);
                        std::swap(w.jetSec[k], w.jetSec[k - 1]);
                        std::swap(w.jetM[k], w.jetM[k - 1]);
                    }
                // one jet never overlaps the next (they are separate flares)
                for (int i = 1; i < w.jets; ++i)
                    w.jetAt[i] = std::max(w.jetAt[i], w.jetAt[i - 1] + w.jetSec[i - 1] + 5.0f);
                const float b = r.U();
                w.blasts = b < 0.3f ? 0 : (b < 0.8f ? 1 : 2);
                for (int i = 0; i < w.blasts; ++i)
                    w.blastAt[i] = U(120.0f, 1800.0f);
            }
            break;
        default: // a car or jeep
            if (c.fuelBurning)
            {
                w.fuelSec = U(120.0f, 300.0f);
                w.fuelFlameM = U(3.0f, 5.0f);
                w.emberFuelSec = std::min(w.fuelSec, U(120.0f, 180.0f));
                w.poolAreaM2 = U(5.0f, 15.0f);
                w.poolFlameM = U(0.5f, 1.0f);
                w.poolSec = w.emberFuelSec;
                w.bodyPeakSec = w.fuelSec + U(60.0f, 300.0f);
            }
            else
                w.bodyPeakSec = U(600.0f, 1200.0f);
            w.bodyFlameM = U(2.0f, 3.0f);
            w.bodyHoldSec = std::max(w.bodyPeakSec + 60.0f, U(900.0f, 1100.0f));
            w.tyreSec = std::max(w.bodyHoldSec + 60.0f, U(1100.0f, 1400.0f));
            w.tyreFlameM = U(0.8f, 1.4f);
            w.flameEndSec = std::max(w.tyreSec + 300.0f, U(1800.0f, 2700.0f));
            w.smoulderEndSec = w.flameEndSec + U(600.0f, 1800.0f);
            w.smokeEndSec = U(3600.0f, 7200.0f);
            w.hotEndSec = U(7200.0f, 14400.0f);
            w.glowEndSec = U(1200.0f, 1800.0f);
            w.emberK = 1.0f;
            w.heatReachM = U(1.5f, 2.5f);
            break;
    }
    w.smokeEndSec = std::max(w.smokeEndSec, w.smoulderEndSec + 600.0f);
    // (followups) its tyres: they catch some minutes after its fuel phase
    // (drawn after every other draw, so the rest of its timeline is as it was)
    {
        const float start = w.fuelSec > 0.0f ? w.fuelSec : 0.5f * w.bodyPeakSec;
        const float u = U(kTyreCatchMinSec, kTyreCatchMaxSec);
        switch (c.cls)
        {
            case WreckClass::Truck:
            case WreckClass::Tanker:
                w.tyreCatchSec = std::min(start + 0.7f * u, w.tyreSec - 300.0f);
                w.tyreSmokeShare = kTyreSmokeTruck;
                break;
            case WreckClass::Armour:
                w.tyreCatchSec = std::min(start + 2.0f * u, w.tyreSec - 600.0f);
                w.tyreSmokeShare = kTyreSmokeArmour;
                break;
            default:
                w.tyreCatchSec = std::min(start + u, w.tyreSec - 120.0f);
                w.tyreSmokeShare = kTyreSmokeCar;
                break;
        }
        w.tyreCatchSec = std::max(w.tyreCatchSec, 0.0f);
    }
    if (w.sizeK < 1.0f)
    {
        // A motorcycle: a small car (its 15 to 20 l, less trim, two tyres).
        const float f = 0.5f + 0.5f * w.sizeK, k = w.sizeK;
        w.fuelFlameM *= f;
        w.bodyFlameM *= f;
        w.tyreFlameM *= f;
        w.poolAreaM2 *= k;
        w.heatReachM *= f;
        for (float* t :
             {&w.fuelSec, &w.emberFuelSec, &w.poolSec, &w.bodyPeakSec, &w.bodyHoldSec, &w.tyreSec, &w.flameEndSec,
              &w.smoulderEndSec, &w.smokeEndSec, &w.hotEndSec, &w.glowEndSec, &w.tyreCatchSec})
            *t *= k;
    }
    // (followups) its char's dose: its flames over their peak (a tanker's
    // pool engulfing it), integrated to its flames' end
    {
        const float peakM = std::max({w.fuelFlameM, w.bodyFlameM, 0.3f});
        const float step = std::max(w.flameEndSec, 1.0f) / static_cast<float>(kWreckCharSteps);
        constexpr int kSub = 8;
        float dose = 0.0f;
        w.charDose[0] = 0.0f;
        for (int i = 0; i < kWreckCharSteps; ++i)
        {
            for (int j = 0; j < kSub; ++j)
            {
                const float t = (static_cast<float>(i) + (static_cast<float>(j) + 0.5f) / kSub) * step;
                float f = std::min(WreckBodyFlameAt(w, t) / peakM, 1.0f);
                if (w.cls == WreckClass::Tanker && t < w.poolSec)
                    f = 1.0f;
                dose += f * step / kSub;
            }
            w.charDose[i + 1] = dose;
        }
    }
    return w;
}

float WreckBodyFlameAt(const WreckTimeline& w, float t)
{
    if (t < 0.0f)
        return 0.0f;
    // The fuel phase (from the first second: the fuel is alight from the
    // fireball on), easing into the body's own fire over its last 30%; the
    // body rising to its peak, holding (a slow wander), falling to the
    // tyres', which die out over the last 3 min of flaming.
    auto bodyCurve = [&](float tt)
    {
        const float start = w.fuelSec > 0.0f ? w.fuelSec : 0.0f;
        const float base = w.fuelSec > 0.0f ? 0.6f * w.bodyFlameM : 0.15f * w.bodyFlameM;
        float L = 0.0f;
        if (tt < w.bodyPeakSec)
            L = base + (w.bodyFlameM - base) * Smooth(start, w.bodyPeakSec, tt);
        else if (tt < w.bodyHoldSec)
            L = w.bodyFlameM;
        else if (tt < w.tyreSec)
            L = w.bodyFlameM + (w.tyreFlameM - w.bodyFlameM) * Smooth(w.bodyHoldSec, w.tyreSec, tt);
        else
            L = w.tyreFlameM;
        L *= 1.0f - Smooth(w.flameEndSec - 180.0f, w.flameEndSec, tt);
        return L;
    };
    float body = 0.0f;
    if (w.fuelSec > 0.0f && t < w.fuelSec)
    {
        const float rise = Smooth(0.0f, 12.0f, t);
        body = w.fuelFlameM * rise;
        const float k = Smooth(0.7f * w.fuelSec, w.fuelSec, t);
        body += (bodyCurve(w.fuelSec) - body) * k;
    }
    else if (t < w.flameEndSec)
        body = bodyCurve(t);
    return body;
}

WreckState WreckStateAt(const WreckTimeline& w, float t, float diameterM)
{
    WreckState s;
    if (t < 0.0f)
        return s;
    const float D = std::max(diameterM, 0.5f);
    // The body's flames (WreckBodyFlameAt); then small flames flickering at
    // the tyres. (followups) The body's own smoke is its fuel's, then its
    // trim's and what is left's; its tyres' is their own (below).
    float body = WreckBodyFlameAt(w, t);
    const float smokePerMW = w.fuelSec > 0.0f && t < w.fuelSec ? kWreckSmokeFuel : kWreckSmokeBody;
    // A slow wander of the flames (a door burning through, a tyre going): up
    // to 10% over a minute or two.
    if (body > 0.0f)
        body *= 1.0f + 0.08f * std::sin(t * 0.061f + 1.3f) * std::sin(t * 0.017f + 0.4f);
    const float peakM = std::max({w.fuelFlameM, w.bodyFlameM, 0.3f});
    if (t >= w.flameEndSec && t < w.smoulderEndSec)
    {
        // small flames at the tyres, coming and going (their smoke is the tyres', below)
        const float k = 1.0f - Smooth(w.flameEndSec, w.smoulderEndSec, t);
        const float on = 0.5f + 0.5f * std::sin(t * 0.21f + 0.7f) * std::sin(t * 0.047f);
        body = 0.3f * w.sizeK * k * std::max(on, 0.0f);
    }
    s.bodyFlameM = body;
    s.bodyHeat = body > 0.01f ? std::clamp(0.45f + 0.55f * body / peakM, 0.0f, 1.0f) : 0.0f;
    s.bodyMW = body > 0.01f ? HeskestadMW(body, D) * w.sizeK : 0.0f;
    const float peakMW = std::max(HeskestadMW(peakM, D) * w.sizeK, 0.05f);
    s.heatShare = std::clamp(s.bodyMW / peakMW, 0.0f, 1.0f);
    // Its pool: a car's thin spill under and round it (its heat is in the
    // body's: a spill burns far slower than a deep pool), a tanker's deep
    // pool burning at 2.4 MW a square metre, shrinking over its last minute.
    if (w.poolAreaM2 > 0.0f && t < w.poolSec)
    {
        const float on = Smooth(0.0f, w.cls == WreckClass::Tanker ? 6.0f : 3.0f, t);
        const float off = 1.0f - Smooth(w.poolSec - 60.0f, w.poolSec, t);
        s.poolFlameM = w.poolFlameM * on * (0.35f + 0.65f * off);
        s.poolAreaM2 = w.poolAreaM2 * (0.4f + 0.6f * off);
        if (w.cls == WreckClass::Tanker)
            s.poolMW = 2.4f * s.poolAreaM2 * on * off;
    }
    // Smoke (followups: the consult's timeline): the flaming body's by what
    // burns, its tyres' from their catch (the densest and blackest), the
    // shell's after flaming dying away (black going grey), the pool's.
    const float peakSmoke = HeskestadMW(peakM, D) * w.sizeK * kWreckSmokeBody;
    float tyre = 0.0f;
    if (w.tyreSmokeShare > 0.0f && t >= w.tyreCatchSec)
    {
        if (t < w.flameEndSec)
            tyre = Smooth(w.tyreCatchSec, std::max(w.tyreSec, w.tyreCatchSec + 60.0f), t) *
                   (1.0f - (1.0f - kTyreSmoulderShare) * Smooth(w.flameEndSec - 180.0f, w.flameEndSec, t));
        else
            tyre = kTyreSmoulderShare * (1.0f - Smooth(w.flameEndSec, w.smoulderEndSec, t));
    }
    const float tyreFull = w.tyreSmokeShare * peakSmoke;
    float smoke = s.bodyMW * smokePerMW + tyre * tyreFull;
    float black = t < w.tyreCatchSec ? 0.9f : 0.9f + 0.1f * std::min(tyre, 1.0f);
    if (t >= w.flameEndSec && t < w.smokeEndSec)
    {
        const float k = 1.0f - Smooth(w.flameEndSec, w.smokeEndSec, t);
        smoke = std::max(smoke, kShellSmokeShare * tyreFull * k);
        black = 0.25f + 0.65f * k;
    }
    s.smoke = smoke + s.poolMW * kWreckSmokePool;
    s.black = s.poolMW > 0.0f ? 1.0f : black;
    // Its embers: the build's rate through the fuel phase, then a fourteenth
    // of it while it flames, none after.
    if (w.fuelSec > 0.0f && t < w.emberFuelSec)
    {
        s.emberRate = kEmberFuelRate * w.emberK;
        s.emberFuel = true;
    }
    else if (t < w.flameEndSec)
        s.emberRate = kEmberAfterRate * w.emberK;
    // Its shell: thin bodywork glowing dull red (by night) once its flames are
    // low (its glow points and light; the hot shell after its flames is the
    // renderer's, shellGlow below: a point light left inside the body lit
    // its panels in orange blotches).
    if (t < w.glowEndSec)
        s.glow = (1.0f - Smooth(0.6f * w.glowEndSec, w.glowEndSec, t)) * (1.0f - 0.6f * s.heatShare);
    // (followups) for the renderer: its char by the dose of flame it has
    // taken, and its shell's heat glow, hot in its flames, cooling to its
    // hot time's end
    {
        const float span = std::max(w.flameEndSec, 1.0f);
        const float whole = w.charDose[kWreckCharSteps];
        if (t >= w.flameEndSec)
            s.charK = 1.0f;
        else if (whole > 0.0f)
        {
            const float at = t / span * static_cast<float>(kWreckCharSteps);
            const int i = std::clamp(static_cast<int>(at), 0, kWreckCharSteps - 1);
            const float dose = w.charDose[i] + (w.charDose[i + 1] - w.charDose[i]) * (at - static_cast<float>(i));
            const float tau = whole / kWreckCharK;
            s.charK = std::clamp((1.0f - std::exp(-dose / tau)) / (1.0f - std::exp(-kWreckCharK)), 0.0f, 1.0f);
        }
        if (t < w.flameEndSec)
            s.shellGlow =
                Smooth(0.0f, kShellGlowInSec, t) * (kShellGlowFlaming + (1.0f - kShellGlowFlaming) * s.heatShare);
        else if (t < w.hotEndSec)
        {
            const float cool = std::max(w.hotEndSec - w.flameEndSec, 1.0f), a = t - w.flameEndSec;
            s.shellGlow =
                kShellGlowEnd * std::exp(-a / (kShellGlowFall * cool)) * (1.0f - Smooth(0.7f * cool, cool, a));
        }
    }
    // A propellant jet.
    for (int i = 0; i < w.jets; ++i)
    {
        const float a = t - w.jetAt[i];
        if (a < 0.0f || a > w.jetSec[i])
            continue;
        const float env = Smooth(0.0f, 0.3f, a) * (1.0f - Smooth(w.jetSec[i] - 0.6f, w.jetSec[i], a));
        // its strength surges: a charge's burning is uneven
        const float surge = 0.8f + 0.2f * std::sin(a * 7.3f + static_cast<float>(i));
        s.jetM = w.jetM[i] * env * surge;
        s.jet = i;
        break;
    }
    s.flaming = s.bodyFlameM > 0.01f || s.poolFlameM > 0.01f || s.jetM > 0.01f;
    s.smoking = s.smoke > 0.01f;
    s.done = t >= w.smokeEndSec && t >= w.smoulderEndSec && t >= w.glowEndSec && t >= w.hotEndSec;
    return s;
}

WreckDay WreckDayOf(float m1h)
{
    WreckDay d;
    d.pig = EmberPig(m1h);
    d.heatMeanSec = DayTime(kHeatMeanTypSec, kHeatMeanWorstSec, m1h);
    d.poolMeanSec = DayTime(kPoolMeanTypSec, kPoolMeanWorstSec, m1h);
    const float s = std::clamp(DayShare(m1h), 0.0f, 1.0f);
    auto logMix = [&](float a, float b) { return std::exp(std::log(a) + (std::log(b) - std::log(a)) * s); };
    d.fuelEmberMedianM = logMix(kFuelEmberMedianTypM, kFuelEmberMedianWorstM);
    d.bodyEmberMedianM = logMix(kBodyEmberMedianTypM, kBodyEmberMedianWorstM);
    d.fuelEmberMaxM = kFuelEmberMaxTypM + (kEmberMaxWorstM - kFuelEmberMaxTypM) * s;
    d.bodyEmberMaxM = kBodyEmberMaxTypM + (kEmberMaxWorstM - kBodyEmberMaxTypM) * s;
    d.emberSigma = kEmberSigmaTyp + (kEmberSigmaWorst - kEmberSigmaTyp) * s;
    d.fireballFull = DayChance(kFireballFullTyp, kFireballFullWorst, m1h);
    d.debris = DayChance(kDebrisTyp, kDebrisWorst, m1h);
    d.debrisDelaySec = DayTime(kDebrisDelayTypSec, kDebrisDelayWorstSec, m1h);
    d.smoulderSec = DayTime(kSmoulderTypSec, kSmoulderWorstSec, m1h);
    d.tankerRing = kTankerRingTyp + (kTankerRingWorst - kTankerRingTyp) * s;
    d.rain = DayChance(kRainTyp, kRainWorst, m1h);
    d.frag = DayChance(kFragTyp, kFragWorst, m1h);
    d.blastFrag = DayChance(kBlastFragTyp, kBlastFragWorst, m1h);
    // Past the grass's moisture of extinction nothing lights.
    if (m1h >= 0.12f)
    {
        d.pig = d.fireballFull = d.debris = d.rain = d.frag = d.blastFrag = 0.0f;
        d.heatMeanSec = d.poolMeanSec = d.tankerRing = 0.0f;
    }
    return d;
}

} // namespace Poseidon::firespread
