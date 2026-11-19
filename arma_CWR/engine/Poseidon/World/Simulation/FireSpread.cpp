#include <Poseidon/World/Simulation/FireSpread.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::firespread
{

namespace
{
// Rothermel's constants (US customary, as published).
constexpr float kRhoP = 32.0f;      // oven-dry particle density (lb/ft3)
constexpr float kHeat = 8000.0f;    // low heat content (Btu/lb)
constexpr float kMineral = 0.0555f; // total mineral content
constexpr float kEtaS = 0.4174f;    // mineral damping at effective mineral content 0.010 (0.174 Se^-0.19)
constexpr float kTonsAcreToLbFt2 = 2000.0f / 43560.0f;
constexpr float kSav10h = 109.0f, kSav100h = 30.0f, kSavLive = 1500.0f;
constexpr float kMpsToFtMin = 196.850394f;
constexpr float kFtMinToMps = 0.00508f;
constexpr float kBtuFtSToKwM = 3.46139f;
constexpr float kMpsToMph = 2.2369363f;

float MoistureDamping(float m, float mx)
{
    if (mx <= 0.0f)
        return 0.0f;
    const float r = std::min(m / mx, 1.0f);
    return std::max(0.0f, 1.0f - 2.59f * r + 5.11f * r * r - 3.52f * r * r * r);
}
} // namespace

const FuelModel& Model(int fm)
{
    // Anderson (1982), "Aids to determining fuel models for estimating fire
    // behavior", INT-122: 1 h / 10 h / 100 h / live loads (tons/acre), 1 h
    // surface-to-volume (1/ft), depth (ft), dead extinction moisture. Wind
    // adjustment factors for unsheltered fuel from the same models' fuel bed
    // depths (Albini and Baughman 1979 table, as BehavePlus uses).
    static const FuelModel models[kFuelModels] = {
        {"none", 0, 0, 0, 0, 1, 1, 0, 0},
        {"short grass", 0.74f, 0.00f, 0.00f, 0.00f, 3500, 1.0f, 0.12f, 0.36f},
        {"timber grass and understory", 2.00f, 1.00f, 0.50f, 0.50f, 3000, 1.0f, 0.15f, 0.36f},
        {"tall grass", 3.01f, 0.00f, 0.00f, 0.00f, 1500, 2.5f, 0.25f, 0.44f},
        {"chaparral", 5.01f, 4.01f, 2.00f, 5.01f, 2000, 6.0f, 0.20f, 0.55f},
        {"brush", 1.00f, 0.50f, 0.00f, 2.00f, 2000, 2.0f, 0.20f, 0.42f},
        {"dormant brush", 1.50f, 2.50f, 2.00f, 0.00f, 1750, 2.5f, 0.25f, 0.44f},
        {"southern rough", 1.13f, 1.87f, 1.50f, 0.37f, 1750, 2.5f, 0.40f, 0.44f},
        {"closed timber litter", 1.50f, 1.00f, 2.50f, 0.00f, 2000, 0.2f, 0.30f, 0.28f},
        {"hardwood litter", 2.92f, 0.41f, 0.15f, 0.00f, 2500, 0.2f, 0.25f, 0.28f},
        {"timber litter and understory", 3.01f, 2.00f, 5.01f, 2.00f, 2000, 1.0f, 0.25f, 0.36f},
        {"light logging slash", 1.50f, 4.51f, 5.51f, 0.00f, 1500, 1.0f, 0.15f, 0.36f},
        {"medium logging slash", 4.01f, 14.03f, 16.53f, 0.00f, 1500, 2.3f, 0.20f, 0.43f},
        {"heavy logging slash", 7.01f, 23.04f, 28.05f, 0.00f, 1500, 3.0f, 0.25f, 0.46f},
    };
    return models[(fm >= 0 && fm < kFuelModels) ? fm : 0];
}

SurfaceFire SolveSurface(int fm, const Moisture& mo)
{
    SurfaceFire s;
    if (fm <= 0 || fm >= kFuelModels)
        return s;
    const FuelModel& m = Model(fm);
    struct P
    {
        float w0, sav, mf;
    };
    const P dead[3] = {{m.w1h * kTonsAcreToLbFt2, m.sav1h, mo.m1h},
                       {m.w10h * kTonsAcreToLbFt2, kSav10h, mo.m10h},
                       {m.w100h * kTonsAcreToLbFt2, kSav100h, mo.m100h}};
    const P live = {m.wLive * kTonsAcreToLbFt2, kSavLive, mo.mLive};

    float aDead = 0.0f;
    for (const P& p : dead)
        aDead += p.sav * p.w0 / kRhoP;
    const float aLive = live.sav * live.w0 / kRhoP;
    const float aTotal = aDead + aLive;
    if (aDead <= 0.0f)
        return s;

    // Category values weighted by surface area (Rothermel 1972).
    float sigDead = 0.0f, wnDead = 0.0f, mDead = 0.0f, sinkDead = 0.0f;
    for (const P& p : dead)
    {
        const float f = (p.sav * p.w0 / kRhoP) / aDead;
        sigDead += f * p.sav;
        wnDead += f * p.w0 * (1.0f - kMineral);
        mDead += f * p.mf;
        if (p.w0 > 0.0f)
            sinkDead += f * std::exp(-138.0f / p.sav) * (250.0f + 1116.0f * p.mf);
    }
    float sigLive = 0.0f, wnLive = 0.0f, sinkLive = 0.0f, mxLive = m.mxDead;
    if (aLive > 0.0f)
    {
        sigLive = live.sav;
        wnLive = live.w0 * (1.0f - kMineral);
        sinkLive = std::exp(-138.0f / live.sav) * (250.0f + 1116.0f * live.mf);
        // Live extinction moisture (Albini 1976): from the dead to live fine fuel ratio.
        float deadFine = 0.0f, deadFineM = 0.0f;
        for (const P& p : dead)
        {
            const float k = p.w0 * std::exp(-138.0f / p.sav);
            deadFine += k;
            deadFineM += k * p.mf;
        }
        const float liveFine = live.w0 * std::exp(-500.0f / live.sav);
        const float mfDeadFine = deadFine > 0.0f ? deadFineM / deadFine : 0.0f;
        const float wRatio = liveFine > 0.0f ? deadFine / liveFine : 0.0f;
        mxLive = std::max(m.mxDead, 2.9f * wRatio * (1.0f - mfDeadFine / m.mxDead) - 0.226f);
    }
    const float fDead = aDead / aTotal, fLive = aLive / aTotal;
    const float sigma = fDead * sigDead + fLive * sigLive;

    const float w0Total = dead[0].w0 + dead[1].w0 + dead[2].w0 + live.w0;
    const float rhoB = w0Total / m.depthFt;
    const float beta = rhoB / kRhoP;
    const float betaOp = 3.348f * std::pow(sigma, -0.8189f);
    const float ratio = beta / betaOp;
    const float s15 = std::pow(sigma, 1.5f);
    const float gammaMax = s15 / (495.0f + 0.0594f * s15);
    const float a = 133.0f * std::pow(sigma, -0.7913f);
    const float gamma = gammaMax * std::pow(ratio, a) * std::exp(a * (1.0f - ratio));

    const float ir =
        gamma * kHeat * kEtaS * (wnDead * MoistureDamping(mDead, m.mxDead) + wnLive * MoistureDamping(live.mf, mxLive));
    const float xi = std::exp((0.792f + 0.681f * std::sqrt(sigma)) * (beta + 0.1f)) / (192.0f + 0.2595f * sigma);
    const float sink = rhoB * (fDead * sinkDead + fLive * sinkLive);

    s.irBtu = ir;
    s.sigma = sigma;
    s.beta = beta;
    s.betaOp = betaOp;
    s.r0Ftmin = sink > 0.0f ? ir * xi / sink : 0.0f;
    s.burns = s.r0Ftmin > 1e-4f;
    s.windC = 7.47f * std::exp(-0.133f * std::pow(sigma, 0.55f));
    s.windB = 0.02526f * std::pow(sigma, 0.54f);
    s.windE = 0.715f * std::exp(-3.59e-4f * sigma);
    s.slopeK = 5.275f * std::pow(beta, -0.3f);
    s.residenceSec = 60.0f * 384.0f / sigma;
    return s;
}

float LengthToBreadth(float wind10Mps, bool grass)
{
    // U the 10 m open wind in km/h. Forest and shrub: Forestry Canada Fire
    // Danger Group (1992), ST-X-3 eq. 79, fitted to Alexander's (1985) data
    // (5.0 at 40 km/h; Rothermel's 1991 midflame form gave 3.2 there). Grass:
    // ST-X-3 eq. 80 as corrected by Wotton, Alexander and Taylor (2009,
    // GLC-X-10), 1.1 U^0.464 from 1 km/h (6.06 at 40 km/h); both confirmed in
    // the Canadian Forest Service's cffdrs package (the fire science consult).
    const float u = std::max(0.0f, wind10Mps) * 3.6f;
    if (grass)
        return u < 1.0f ? 1.0f : std::min(kGrassMaxLB, 1.1f * std::pow(u, 0.464f));
    return 1.0f + 8.729f * std::pow(1.0f - std::exp(-0.030f * u), 2.155f);
}

HeadFire SolveHead(const SurfaceFire& s, float windX, float windZ, float slopeX, float slopeZ, float waf, bool grass)
{
    HeadFire h;
    if (!s.burns)
        return h;
    const float ratioE = std::pow(s.beta / s.betaOp, -s.windE);
    const float u = std::sqrt(windX * windX + windZ * windZ);
    const float uFt = u * kMpsToFtMin;
    const float phiW = uFt > 0.0f ? s.windC * std::pow(uFt, s.windB) * ratioE : 0.0f;
    const float g = std::sqrt(slopeX * slopeX + slopeZ * slopeZ);
    const float phiS = s.slopeK * g * g;
    // Wind and slope as vectors (BEHAVE).
    float vx = (u > 1e-4f ? phiW * windX / u : 0.0f) + (g > 1e-5f ? phiS * slopeX / g : 0.0f);
    float vz = (u > 1e-4f ? phiW * windZ / u : 0.0f) + (g > 1e-5f ? phiS * slopeZ / g : 0.0f);
    float phiE = std::sqrt(vx * vx + vz * vz);
    if (phiE > 1e-6f)
    {
        h.dirX = vx / phiE;
        h.dirZ = vz / phiE;
    }
    // Effective wind speed, capped by Rothermel's wind limit (0.9 IR, ft/min).
    float ueFt = phiE > 0.0f ? std::pow(phiE / (s.windC * ratioE), 1.0f / s.windB) : 0.0f;
    const float limit = 0.9f * s.irBtu;
    if (ueFt > limit)
    {
        ueFt = limit;
        phiE = s.windC * std::pow(ueFt, s.windB) * ratioE;
    }
    const float headFtMin = s.r0Ftmin * (1.0f + phiE);
    h.rateMps = headFtMin * kFtMinToMps;
    if (grass)
        h.rateMps = std::min(h.rateMps, kGrassMaxRateMps);
    // The effective (wind and slope) midflame wind, back to the 10 m wind.
    h.lengthToBreadth = LengthToBreadth(ueFt / kMpsToFtMin / std::max(waf, 0.05f), grass);
    const float lb = h.lengthToBreadth;
    h.eccentricity = lb > 1.0f ? std::sqrt(lb * lb - 1.0f) / lb : 0.0f;
    h.intensityKwM = IntensityAt(s, h.rateMps);
    h.flameM = FlameLengthM(h.intensityKwM);
    return h;
}

float RateToward(const HeadFire& h, float dirX, float dirZ)
{
    // Elliptical front with the ignition at the rear focus.
    const float c = dirX * h.dirX + dirZ * h.dirZ;
    const float e = h.eccentricity;
    return h.rateMps * (1.0f - e) / std::max(1e-4f, 1.0f - e * c);
}

float IntensityAt(const SurfaceFire& s, float rateMps)
{
    // Byram intensity = reaction intensity x flame depth = IR x R x t_r (Btu/ft/s).
    const float rFtMin = rateMps / kFtMinToMps;
    const float ib = s.irBtu * rFtMin * (s.residenceSec / 60.0f) / 60.0f;
    return ib * kBtuFtSToKwM;
}

float FlameLengthM(float intensityKwM)
{
    // Byram (1959): L = 0.0775 I^0.46 (m, kW/m).
    return intensityKwM > 0.0f ? 0.0775f * std::pow(intensityKwM, 0.46f) : 0.0f;
}

float TorchingIntensityKwM(float canopyBaseM, float foliarMoisture)
{
    // Van Wagner (1977): I0 = (0.010 CBH (460 + 25.9 FMC))^1.5, FMC in percent.
    const float v = 0.010f * canopyBaseM * (460.0f + 25.9f * foliarMoisture * 100.0f);
    return std::pow(std::max(0.0f, v), 1.5f);
}

float FlameTiltRad(float windMps, float flameM)
{
    const float u = std::max(windMps, 0.0f);
    return std::min(kFlameTiltMax, std::atan(kFlameTiltK * u / std::sqrt(9.81f * std::max(flameM, 0.05f))));
}

float TorchEnvelope(float t)
{
    const float a = t - kTorchDelaySec;
    if (a <= 0.0f || a >= kTorchSec)
        return 0.0f;
    auto smooth = [](float x)
    {
        x = std::clamp(x, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    };
    return smooth(a / kTorchRiseSec) * smooth((kTorchSec - a) / kTorchFallSec);
}

float TorchRatio(int treesTorching)
{
    return std::min(kTorchRatioMax, kTorchRatio * std::pow(static_cast<float>(std::max(treesTorching, 1)), 0.4f));
}

float BuildUp(float ageSec, float perMin)
{
    if (perMin <= 0.0f)
        return 1.0f;
    return 1.0f - std::exp(-perMin / 60.0f * std::max(ageSec, 0.0f));
}

float BuildUpRun(float age0Sec, float elapsedSec, float perMin)
{
    const float e = std::max(elapsedSec, 0.0f);
    if (perMin <= 0.0f)
        return e;
    // The integral of 1 - exp(-k (age0 + t)) over t from 0 to e.
    const float k = perMin / 60.0f;
    return e - std::exp(-k * std::max(age0Sec, 0.0f)) * (1.0f - std::exp(-k * e)) / k;
}

float AlbiniTorchSpotMaxM(float treeM, float wind10Mps)
{
    // Albini (1979), "Spot fire distance from burning trees: a predictive
    // model", USDA FS GTR INT-56, in BehavePlus's form (Spot module,
    // torching trees), US units. One tree torching (n = 1).
    if (treeM <= 0.0f || wind10Mps <= 0.0f)
        return 0.0f;
    const float htFt = treeM / 0.3048f;
    // The tree's diameter at breast height from its height: open-grown
    // conifers of 8 to 15 m stand about 2.2 cm of DBH a metre of height (our
    // allometry, a judgement: the game knows only the shape's height).
    const float dbhIn = std::max(2.2f * treeM / 2.54f, 2.0f);
    // Steady flame height (ft) and duration of a torching tree: Albini's
    // coefficients for pines (ponderosa and lodgepole: 12.9, 0.453; 12.6,
    // -0.256; the islands' conifers are mostly pines).
    const float zf = 12.9f * std::pow(dbhIn, 0.453f);
    const float tf = 12.6f * std::pow(dbhIn, -0.256f);
    // The firebrand's lofted height (ft), by the tree's height over the flame's.
    const float ratio = htFt / zf;
    float a = 4.70f, b = 0.0f;
    if (ratio >= 1.0f)
    {
        a = 4.24f;
        b = 0.332f;
    }
    else if (ratio >= 0.5f)
    {
        a = 3.64f;
        b = 0.391f;
    }
    else if (tf < 3.5f)
    {
        // (a tree under half its flame's height: by the flame's duration,
        // as BehavePlus's spot.cpp selects it)
        a = 2.78f;
        b = 0.418f;
    }
    const float z = a * std::pow(tf, b) * zf + 0.5f * htFt;
    // Its drift down a logarithmic wind profile over flat ground: the
    // downwind cover (open: half its height; the torching tree's height
    // standing for the stand's) and the 20 ft wind (the 10 m wind / 1.15).
    const float hc = 0.5f * htFt;
    if (z <= hc)
        return 0.0f;
    const float u20Mph = wind10Mps / 1.15f * kMpsToMph;
    const float zr = z / hc;
    const float miles = 0.000718f * u20Mph * std::sqrt(hc) * (0.362f + 0.5f * std::sqrt(zr) * std::log(zr));
    return std::max(0.0f, miles * 1609.344f);
}

float SpotDistanceM(float capM, float u1, float u2)
{
    if (capM <= 0.0f)
        return 0.0f;
    // Box-Muller: a standard normal from two uniforms.
    const float n = std::sqrt(-2.0f * std::log(std::clamp(u1, 1e-6f, 1.0f))) * std::cos(6.2831853f * u2);
    return std::min(capM, kSpotMedianShare * capM * std::exp(kSpotLogSigma * n));
}

namespace
{
// Where along the front's way across a cell (0 at the edge it came in by, 1
// at the far one) the point (ox, oz) off its centre lies.
float AlongFront(float crossSec, float frontX, float frontZ, float ox, float oz, float cellM)
{
    return crossSec > 0.0f ? std::clamp((ox * frontX + oz * frontZ) / cellM + 0.5f, 0.0f, 1.0f) : 0.0f;
}

std::uint32_t HashU(std::uint32_t h)
{
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

// 0..1 from a lattice point and a salt.
float Lattice01(std::int32_t i, std::int32_t j, std::uint32_t salt)
{
    const std::uint32_t h =
        HashU(static_cast<std::uint32_t>(i) * 0x8da6b343u ^ static_cast<std::uint32_t>(j) * 0xd8163841u ^ salt);
    return static_cast<float>(h >> 8) / 16777216.0f;
}

// Smooth value noise (-1..1) at (x, z) in lattice units.
float ValueNoise(float x, float z, std::uint32_t salt)
{
    const float fx = std::floor(x), fz = std::floor(z);
    const std::int32_t i = static_cast<std::int32_t>(fx), j = static_cast<std::int32_t>(fz);
    float u = x - fx, w = z - fz;
    u = u * u * (3.0f - 2.0f * u);
    w = w * w * (3.0f - 2.0f * w);
    const float a = Lattice01(i, j, salt), b = Lattice01(i + 1, j, salt);
    const float c = Lattice01(i, j + 1, salt), d = Lattice01(i + 1, j + 1, salt);
    return 2.0f * ((a + (b - a) * u) * (1.0f - w) + (c + (d - c) * u) * w) - 1.0f;
}

// Two smooth value noises (-1..1) at once from one hash a lattice point
// (its high and low 16 bits): half the hashing of two ValueNoise calls.
void ValueNoise2(float x, float z, std::uint32_t salt, float& n0, float& n1)
{
    const float fx = std::floor(x), fz = std::floor(z);
    const std::int32_t i = static_cast<std::int32_t>(fx), j = static_cast<std::int32_t>(fz);
    float u = x - fx, w = z - fz;
    u = u * u * (3.0f - 2.0f * u);
    w = w * w * (3.0f - 2.0f * w);
    auto h = [salt](std::int32_t a, std::int32_t b)
    { return HashU(static_cast<std::uint32_t>(a) * 0x8da6b343u ^ static_cast<std::uint32_t>(b) * 0xd8163841u ^ salt); };
    const std::uint32_t ha = h(i, j), hb = h(i + 1, j), hc = h(i, j + 1), hd = h(i + 1, j + 1);
    auto mix = [u, w](float a, float b, float c, float dd)
    { return 2.0f * ((a + (b - a) * u) * (1.0f - w) + (c + (dd - c) * u) * w) - 1.0f; };
    constexpr float k = 1.0f / 65535.0f;
    n0 = mix((ha >> 16) * k, (hb >> 16) * k, (hc >> 16) * k, (hd >> 16) * k);
    n1 = mix((ha & 0xffffu) * k, (hb & 0xffffu) * k, (hc & 0xffffu) * k, (hd & 0xffffu) * k);
}
} // namespace

float Gauge(float px, float pz, float rate, float hx, float hz, float ecc)
{
    if (rate <= 1e-6f)
        return 1e9f;
    const float r = std::sqrt(px * px + pz * pz);
    return (r - ecc * (px * hx + pz * hz)) / (rate * (1.0f - ecc));
}

float SegmentGauge(float wx, float wz, float ux, float uz, float halfM, float rate, float hx, float hz, float ecc,
                   float* qx, float* qz)
{
    float s = 0.0f;
    if (halfM > 0.0f)
    {
        // The nearest point in travel time: along the segment where the ray
        // to the point leaves it at the angle whose cosine is e (u.h)
        // (the minimum of |w - s u| - e (w - s u).h, a convex function of s).
        const float along = wx * ux + wz * uz;
        const float px = wx - along * ux, pz = wz - along * uz;
        const float perp = std::sqrt(px * px + pz * pz);
        const float k = ecc * (ux * hx + uz * hz);
        s = std::clamp(along - k * perp / std::sqrt(std::max(1.0f - k * k, 1e-6f)), -halfM, halfM);
    }
    if (qx)
        *qx = s * ux;
    if (qz)
        *qz = s * uz;
    return Gauge(wx - s * ux, wz - s * uz, rate, hx, hz, ecc);
}

float AnchorGauge(const Anchor& a, float wx, float wz, float rate, float hx, float hz, float ecc, float* qx, float* qz)
{
    float sx = 0.0f, sz = 0.0f;
    float g = 0.0f;
    if (a.radiusM > 0.0f)
    {
        // An area: inside it, now; else the soonest of its widest chord
        // square to the head and its foremost and hindmost points (the front
        // of a disc in a narrow ellipse is those three's, near enough).
        if (wx * wx + wz * wz <= a.radiusM * a.radiusM)
            g = 0.0f;
        else
        {
            g = SegmentGauge(wx, wz, -hz, hx, a.radiusM, rate, hx, hz, ecc, &sx, &sz);
            for (float k : {1.0f, -1.0f})
            {
                const float fx = k * a.radiusM * hx, fz = k * a.radiusM * hz;
                const float gf = Gauge(wx - fx, wz - fz, rate, hx, hz, ecc);
                if (gf < g)
                {
                    g = gf;
                    sx = fx;
                    sz = fz;
                }
            }
        }
    }
    else
        g = SegmentGauge(wx, wz, a.ux, a.uz, a.halfM, rate, hx, hz, ecc, &sx, &sz);
    if (qx)
        *qx = sx;
    if (qz)
        *qz = sz;
    return g;
}

float FrontSpeed(float rate, float hx, float hz, float ecc, float nx, float nz)
{
    // The support function of the rear-focus ellipse (semi-major R / (1 + e)
    // about a centre e of it ahead of the focus).
    const float c = std::clamp(nx * hx + nz * hz, -1.0f, 1.0f);
    return rate / (1.0f + ecc) * (ecc * c + std::sqrt(c * c + (1.0f - ecc * ecc) * (1.0f - c * c)));
}

float BuildUpTime(float age0Sec, float runSec, float perMin)
{
    if (runSec <= 0.0f)
        return 0.0f;
    if (perMin <= 0.0f || age0Sec >= kMatureAgeSec)
        return runSec;
    const float k = perMin / 60.0f;
    const float a0 = std::max(age0Sec, 0.0f);
    // Newton on BuildUpRun(a0, t) = run from above (it is convex in t).
    float t = std::max(runSec, std::sqrt(2.0f * runSec / k)) + runSec;
    for (int it = 0; it < 12; ++it)
    {
        const float f = BuildUpRun(a0, t, perMin) - runSec;
        const float d = std::max(BuildUp(a0 + t, perMin), 1e-4f);
        const float nt = std::max(0.0f, t - f / d);
        if (std::fabs(nt - t) < 1e-3f)
            return nt;
        t = nt;
    }
    return t;
}

float SquareEntryGauge(const Anchor& src, float cx, float cz, float qx, float qz, float half, float rate, float hx,
                       float hz, float ecc, float& ex, float& ez)
{
    // The source's point nearest the centre, off the centre.
    const float px = src.cx + qx - cx, pz = src.cz + qz - cz;
    if (std::fabs(px) <= half && std::fabs(pz) <= half)
    {
        ex = px;
        ez = pz;
        return 0.0f;
    }
    float best = 1e30f;
    // Its corners from the whole source (a straight front's soonest).
    for (int k = 0; k < 4; ++k)
    {
        const float kx = k & 1 ? half : -half, kz = k & 2 ? half : -half;
        const float g = AnchorGauge(src, cx + kx - src.cx, cz + kz - src.cz, rate, hx, hz, ecc);
        if (g < best)
        {
            best = g;
            ex = kx;
            ez = kz;
        }
    }
    // Its edges from that point: the gauge to a point on an edge is the
    // gauge from the edge to the point with the head reversed, whose minimum
    // along the edge has the closed form.
    for (int k = 0; k < 4; ++k)
    {
        const bool alongZ = k < 2;
        const float ecx = alongZ ? (k == 0 ? half : -half) : 0.0f, ecz = alongZ ? 0.0f : (k == 2 ? half : -half);
        const float eux = alongZ ? 0.0f : 1.0f, euz = alongZ ? 1.0f : 0.0f;
        float sx = 0.0f, sz = 0.0f;
        const float g = SegmentGauge(px - ecx, pz - ecz, eux, euz, half, rate, -hx, -hz, ecc, &sx, &sz);
        if (g < best)
        {
            best = g;
            ex = ecx + sx;
            ez = ecz + sz;
        }
    }
    return best;
}

float FrontRadiusM(float px, float pz, float hx, float hz, float ecc)
{
    // The level set of the gauge: its curvature is t.H.t / |grad| with the
    // Hessian of |p| (the linear part has none): (1 - (t.p^)^2) / (|p| |p^ - e h|).
    const float r = std::sqrt(px * px + pz * pz);
    if (r < 1e-3f)
        return 0.0f;
    const float ux = px / r, uz = pz / r;
    const float gx = ux - ecc * hx, gz = uz - ecc * hz;
    const float gl = std::sqrt(gx * gx + gz * gz);
    if (gl < 1e-6f)
        return 1e9f;
    const float tp = (-gz * ux + gx * uz) / gl;
    const float k = (1.0f - tp * tp) / (r * gl);
    return k > 1e-9f ? 1.0f / k : 1e9f;
}

Anchor FrontPiece(const Anchor& old, float qx, float qz, float px, float pz, float nx, float nz, float hx, float hz,
                  float ecc, float t, float age, float maxHalf)
{
    Anchor a;
    a.cx = px;
    a.cz = pz;
    a.t = t;
    a.age = age;
    a.radiusM = 0.0f;
    const float s = qx * old.ux + qz * old.uz; // the nearest point's place along old
    const float spanLo = std::min(old.spanLo, -old.halfM), spanHi = std::max(old.spanHi, old.halfM);
    if (old.halfM > 0.0f && s >= spanLo - 1e-3f && s <= spanHi + 1e-3f)
    {
        // Inside old's straight front the front is old carried on: the piece
        // lies along it, over that front's span within maxHalf of the point
        // (the span carried with it).
        const float lo = std::max(-maxHalf, spanLo - s), hi = std::min(maxHalf, spanHi - s);
        const float mid = 0.5f * (lo + hi);
        a.ux = old.ux;
        a.uz = old.uz;
        a.cx += old.ux * mid;
        a.cz += old.uz * mid;
        a.halfM = std::max(0.0f, 0.5f * (hi - lo));
        a.spanLo = spanLo - s - mid;
        a.spanHi = spanHi - s - mid;
        return a;
    }
    // Else the front is the ellipse from that point: square to its normal,
    // as wide as its curvature allows.
    const float nl = std::sqrt(nx * nx + nz * nz);
    a.ux = nl > 1e-6f ? -nz / nl : 1.0f;
    a.uz = nl > 1e-6f ? nx / nl : 0.0f;
    const float rho = FrontRadiusM(px - (old.cx + qx), pz - (old.cz + qz), hx, hz, ecc);
    a.halfM = nl > 1e-6f ? std::min(maxHalf, std::sqrt(2.0f * kAnchorBulgeM * rho)) : 0.0f;
    a.spanLo = -a.halfM;
    a.spanHi = a.halfM;
    return a;
}

void EdgeWarp(float x, float z, float& wx, float& wz)
{
    // (one octave: the char's finer raggedness is the ground shader's own warp)
    const float a = 1.0f / kEdgeNoiseCellM;
    float x0 = 0.0f, z0 = 0.0f;
    ValueNoise2(x * a, z * a, 0x1234567u, x0, z0);
    wx = kEdgeNoiseM * x0;
    wz = kEdgeNoiseM * z0;
}

float WindWanderRad(double t, std::uint32_t seed, float sigmaDeg)
{
    if (sigmaDeg <= 0.0f)
        return 0.0f;
    // Three swings of equal share: each sine's standard deviation is its
    // amplitude over the root of two.
    const float amp = sigmaDeg * 0.0174532925f * std::sqrt(2.0f / 3.0f);
    float turn = 0.0f;
    for (int k = 0; k < 3; ++k)
    {
        const float phase =
            6.2831853f * static_cast<float>(HashU(seed * 2654435761u + 977u * (k + 1)) >> 8) / 16777216.0f;
        turn += amp * static_cast<float>(std::sin(6.283185307179586 * t / kWanderPeriodsSec[k] + phase));
    }
    return turn;
}

float FuelNoiseFactor(std::int32_t i, std::int32_t j, std::uint32_t seed, float scale)
{
    if (scale <= 0.0f)
        return 1.0f;
    const float smooth = ValueNoise(static_cast<float>(i) / kFuelNoiseCells, static_cast<float>(j) / kFuelNoiseCells,
                                    seed ^ 0x51ed270bu);
    const float own = 2.0f * Lattice01(i, j, seed ^ 0x2b8f1a3du) - 1.0f;
    return std::exp(scale * (0.22f * smooth + 0.12f * own));
}

float RoadEmberRate(float intensityKwM, float wind10Mps, float gapEffM, float reachM, float pIgnite)
{
    if (wind10Mps < kRoadEmberMinWindMps || intensityKwM <= 0.0f)
        return 0.0f;
    return kRoadEmberK * (intensityKwM / 2000.0f) * std::pow(wind10Mps / 11.0f, 1.5f) *
           std::exp(-std::max(gapEffM - reachM, 0.0f) / kRoadEmberDecayM) * std::max(pIgnite, 0.0f);
}

bool BareAt(const BurningView& v, float ox, float oz)
{
    if (v.roadHalf <= 0.0f)
        return false;
    // Its signed distance from the centre line, positive on the cell's side.
    const float s = ((ox - v.roadX) * v.roadDZ - (oz - v.roadZ) * v.roadDX) * v.roadSide;
    return s < v.roadHalf;
}

namespace
{
// The front's age at the point (px, pz) off the cell's centre (warped or not).
float AgeAtPoint(const BurningView& v, float px, float pz, float cellM, float* speedMps);
} // namespace

float FrontAgeAt(const BurningView& v, float ox, float oz, float cellM, float* speedMps)
{
    if (speedMps)
        *speedMps = std::max(v.rateMps, 0.004f);
    if (BareAt(v, ox, oz))
        return -1e9f;
    if (!v.exact && !v.grid && !v.planar)
        return v.age - AlongFront(v.crossSec, v.frontX, v.frontZ, ox, oz, cellM) * v.crossSec;
    float wx = 0.0f, wz = 0.0f;
    EdgeWarp(v.x + ox, v.z + oz, wx, wz);
    return AgeAtPoint(v, ox + wx, oz + wz, cellM, speedMps);
}

float FrontAgeField(const BurningView& v, float ox, float oz, float cellM)
{
    if (!v.exact && !v.grid && !v.planar)
        return v.age - AlongFront(v.crossSec, v.frontX, v.frontZ, ox, oz, cellM) * v.crossSec;
    float wx = 0.0f, wz = 0.0f;
    EdgeWarp(v.x + ox, v.z + oz, wx, wz);
    return AgeAtPoint(v, ox + wx, oz + wz, cellM, nullptr);
}

namespace
{
// FrontDepthAt's directions, round the circle.
struct DepthDirs
{
    float x[kFrontDepthDirs], z[kFrontDepthDirs];
    DepthDirs()
    {
        for (int k = 0; k < kFrontDepthDirs; ++k)
        {
            const float ang = 6.2831853f * static_cast<float>(k) / static_cast<float>(kFrontDepthDirs);
            x[k] = std::cos(ang);
            z[k] = std::sin(ang);
        }
    }
};
const DepthDirs& Dirs()
{
    static const DepthDirs d;
    return d;
}
} // namespace

FrontDepthSupport FrontDepthSupportOf(const BurningView& v)
{
    // The burnt area now: {g < gRun}, the seed's segment or disc grown by
    // gRun times the ellipse (support FrontSpeed), about the anchor.
    FrontDepthSupport sup;
    sup.on = v.exact;
    if (!sup.on)
        return sup;
    const DepthDirs& d = Dirs();
    for (int k = 0; k < kFrontDepthDirs; ++k)
    {
        const float seed =
            v.ancRadius > 0.0f ? v.ancRadius : v.ancHalf * std::fabs(v.ancUX * d.x[k] + v.ancUZ * d.z[k]);
        sup.s[k] = v.gRun * FrontSpeed(v.gRate, v.gDirX, v.gDirZ, v.gEcc, d.x[k], d.z[k]) + seed;
    }
    return sup;
}

float FrontDepthAt(const BurningView& v, const FrontDepthSupport& sup, float ox, float oz, float ageSec, float speedMps)
{
    const float flat = std::max(ageSec, 0.0f) * std::max(speedMps, 0.0f);
    if (!sup.on || ageSec <= 0.0f)
        return flat;
    // From a point inside it the nearest edge is the least of that support
    // less the point's reach, over the directions (through the same edge
    // noise as the field).
    float wx = 0.0f, wz = 0.0f;
    EdgeWarp(v.x + ox, v.z + oz, wx, wz);
    const float px = ox + wx - v.ancX, pz = oz + wz - v.ancZ;
    const DepthDirs& d = Dirs();
    float depth = flat;
    for (int k = 0; k < kFrontDepthDirs; ++k)
        depth = std::min(depth, sup.s[k] - (px * d.x[k] + pz * d.z[k]));
    return std::max(depth, 0.0f);
}

float FrontDepthAt(const BurningView& v, float ox, float oz, float ageSec, float speedMps)
{
    return FrontDepthAt(v, FrontDepthSupportOf(v), ox, oz, ageSec, speedMps);
}

namespace
{
float AgeAtPoint(const BurningView& v, float px, float pz, float cellM, float* speedMps)
{
    // The cell's own source (its anchor, through the cell's ellipse now,
    // matched to the centre's arrival): exact near a seed (a young fire round,
    // a line two-sided), and a narrow head's true width anywhere.
    float best = -1e30f, speed = std::max(v.rateMps, 0.004f);
    if (v.exact || v.ancHalf < kNarrowSourceM || !v.grid)
    {
        const float ax = px - v.ancX, az = pz - v.ancZ;
        float qx = 0.0f, qz = 0.0f;
        Anchor a;
        a.ux = v.ancUX;
        a.uz = v.ancUZ;
        a.halfM = v.ancHalf;
        a.radiusM = v.ancRadius;
        const float g = AnchorGauge(a, ax, az, v.gRate, v.gDirX, v.gDirZ, v.gEcc, &qx, &qz);
        // (the dance fix) behind the centre the build-up's share when the
        // front passed it, ahead of it the share now (FillFront)
        const float ahead = std::max(v.gGrownAhead > 0.0f ? v.gGrownAhead : v.gGrown, 0.02f);
        const float behind = std::max(v.gGrown, 0.02f);
        const float gCentre = v.gRun - v.ageC * (v.ageC > 0.0f ? ahead : behind);
        const float grown = g > gCentre ? ahead : behind;
        best = v.ageC + (gCentre - g) / grown;
        const float rx = ax - qx, rz = az - qz, r = speedMps ? std::sqrt(rx * rx + rz * rz) : 0.0f;
        if (r > 1e-3f)
        {
            const float gx = rx / r - v.gEcc * v.gDirX, gz = rz / r - v.gEcc * v.gDirZ;
            speed = std::max(v.gRate * (1.0f - v.gEcc) / std::max(std::sqrt(gx * gx + gz * gz), 1e-4f) * grown, 0.004f);
        }
    }
    if (!v.exact)
    {
        // Elsewhere the front is the union of its sources: the arrival field
        // between the 3 x 3 centres (bilinear in the quarter of the block the
        // point lies in) where it came sooner, so the cells draw one front.
        float a = -1e30f, sp = speed;
        if (v.grid)
        {
            const float u = std::clamp(px / cellM + 1.0f, 0.0f, 2.0f);
            const float w = std::clamp(pz / cellM + 1.0f, 0.0f, 2.0f);
            const int i0 = std::min(static_cast<int>(u), 1), j0 = std::min(static_cast<int>(w), 1);
            const float fu = u - static_cast<float>(i0), fw = w - static_cast<float>(j0);
            const float a00 = v.nbAge[j0 * 3 + i0], a10 = v.nbAge[j0 * 3 + i0 + 1];
            const float a01 = v.nbAge[(j0 + 1) * 3 + i0], a11 = v.nbAge[(j0 + 1) * 3 + i0 + 1];
            a = (a00 + (a10 - a00) * fu) * (1.0f - fw) + (a01 + (a11 - a01) * fu) * fw;
            if (speedMps)
            {
                const float gx = ((a10 - a00) * (1.0f - fw) + (a11 - a01) * fw) / cellM;
                const float gz = ((a01 - a00) * (1.0f - fu) + (a11 - a10) * fu) / cellM;
                sp = std::clamp(1.0f / std::max(std::sqrt(gx * gx + gz * gz), 1e-4f), 0.004f, 10.0f);
            }
        }
        else
            a = v.ageC - (px * v.frontX + pz * v.frontZ) / std::max(v.rateMps, 0.004f);
        // (a wide source's front is straight, which the field draws exactly:
        // only a point-like one, a narrow head, joins it)
        if (a > best)
        {
            best = a;
            speed = sp;
        }
    }
    if (speedMps)
        *speedMps = speed;
    return best;
}
} // namespace

void AgeView(BurningView& v, float newAge)
{
    const float d = newAge - v.age;
    // (its run to the front: behind its centre's share, past it the share ahead)
    const float ahead = std::max(v.gGrownAhead > 0.0f ? v.gGrownAhead : v.gGrown, 0.02f);
    const float behind = std::max(v.gGrown, 0.02f);
    const float gCentre = v.gRun - v.ageC * (v.ageC > 0.0f ? ahead : behind);
    v.ageC += d;
    v.gRun = gCentre + v.ageC * (v.ageC > 0.0f ? ahead : behind);
    for (float& a : v.nbAge)
        a += d;
    v.age = newAge;
}

PartFlame SurfacePart(const BurningView& v, float ox, float oz, float cellM)
{
    PartFlame out;
    const float tr = std::max(v.residenceSec, 0.5f);
    const float a = FrontAgeAt(v, ox, oz, cellM);
    if (a <= 0.0f || a >= tr)
        return out;
    const float left = 1.0f - a / tr; // the share of this point's fuel still to burn
    const float in = std::min(1.0f, a / std::min(kAttachSec, 0.3f * tr));
    const float formed = in * in * (3.0f - 2.0f * in);
    out.heat = formed * left;
    out.length = formed * std::pow(left, 0.46f);
    return out;
}

// ---------------------------------------------------------------------------

FireGrid::FireGrid(const Params& params, FuelQuery fuel, HeightQuery height)
    : _p(params), _fuel(std::move(fuel)), _height(std::move(height)), _rng(params.seed)
{
    SetMoisture(_p.moisture);
}

FireGrid::~FireGrid()
{
    Clear();
}

void FireGrid::SetMoisture(const Moisture& m)
{
    _p.moisture = m;
    for (int fm = 0; fm < kFuelModels; ++fm)
        _surface[fm] = SolveSurface(fm, m);
}

void FireGrid::Clear()
{
    for (auto& kv : _tiles)
        delete kv.second;
    _tiles.clear();
    _front.clear();
    _flaming.clear();
    _offers.clear();
    _caught.clear();
    _torches.clear();
    _pending.clear();
    _seedFix.clear();
    _cursor = 0;
    _acc = 0.0f;
    _stats = Stats();
    _haveBounds = false;
}

std::int32_t FireGrid::IndexOf(float v) const
{
    return static_cast<std::int32_t>(std::floor(v / _p.cellM));
}

static std::int32_t FloorDiv(std::int32_t a, std::int32_t b)
{
    return (a >= 0) ? a / b : -((-a + b - 1) / b);
}

Cell* FireGrid::CellAt(std::int32_t i, std::int32_t j, bool create)
{
    const std::int32_t ti = FloorDiv(i, kTile), tj = FloorDiv(j, kTile);
    const std::uint64_t key = TileKey(ti, tj);
    auto it = _tiles.find(key);
    Tile* tile = nullptr;
    if (it == _tiles.end())
    {
        if (!create)
            return nullptr;
        tile = new Tile();
        _tiles.emplace(key, tile);
        _stats.tiles = static_cast<int>(_tiles.size());
    }
    else
    {
        tile = it->second;
    }
    Cell& c = tile->cells[(j - tj * kTile) * kTile + (i - ti * kTile)];
    if (c.state == CellState::Unknown && create)
        Classify(c, i, j);
    return &c;
}

const Cell* FireGrid::CellAtConst(std::int32_t i, std::int32_t j) const
{
    const std::int32_t ti = FloorDiv(i, kTile), tj = FloorDiv(j, kTile);
    auto it = _tiles.find(TileKey(ti, tj));
    if (it == _tiles.end())
        return nullptr;
    return &it->second->cells[(j - tj * kTile) * kTile + (i - ti * kTile)];
}

void FireGrid::Classify(Cell& c, std::int32_t i, std::int32_t j)
{
    const float x = CentreX(i), z = CentreZ(j);
    const Fuel f = _fuel ? _fuel(x, z) : Fuel();
    c.model = f.model < kFuelModels ? f.model : 0;
    c.canopy = f.canopy;
    c.sheltered = f.sheltered;
    c.conifer = f.conifer;
    c.olive = f.olive;
    c.canopyBaseM = f.canopyBaseM;
    // Only a tree standing in the cell torches here (one just outside it
    // shelters the cell's grass but torches in its own cell).
    c.treeH = f.canopy ? f.treeHeightM : 0.0f;
    c.treeOX = f.treeOffX;
    c.treeOZ = f.treeOffZ;
    c.shrubH = f.shrubHeightM;
    c.fuelK = FuelNoiseFactor(i, j, _p.seed, _p.fuelNoise);
    c.roadHalfM = f.roadHalfM;
    c.built = f.built;
    if (f.roadHalfM > 0.0f)
    {
        const float dl = std::sqrt(f.roadDirX * f.roadDirX + f.roadDirZ * f.roadDirZ);
        c.roadX = x + f.roadOffX;
        c.roadZ = z + f.roadOffZ;
        c.roadDX = dl > 1e-6f ? f.roadDirX / dl : 0.0f;
        c.roadDZ = dl > 1e-6f ? f.roadDirZ / dl : 1.0f;
    }
    c.heightM = _height ? _height(x, z) : 0.0f;
    if (_height)
    {
        const float h = 0.5f * _p.cellM;
        c.gx = (_height(x + h, z) - _height(x - h, z)) / _p.cellM;
        c.gz = (_height(x, z + h) - _height(x, z - h)) / _p.cellM;
    }
    c.windStamp = 0;
    c.state = CellState::Unburnt;
    _stats.known++;
    if (!_haveBounds)
    {
        _minI = _maxI = i;
        _minJ = _maxJ = j;
        _haveBounds = true;
    }
    else
    {
        _minI = std::min(_minI, i);
        _maxI = std::max(_maxI, i);
        _minJ = std::min(_minJ, j);
        _maxJ = std::max(_maxJ, j);
    }
}

float FireGrid::Rand()
{
    _rng = _rng * 1664525u + 1013904223u;
    return static_cast<float>(_rng >> 8) / static_cast<float>(1u << 24);
}

void FireGrid::UpdateHead(Cell& c, float wind10X, float wind10Z)
{
    if (c.windStamp == _windStamp)
        return;
    c.windStamp = _windStamp;
    // The cell's fuel, its midflame wind and its slope set the ellipse.
    const float waf = c.sheltered ? 0.15f : Model(c.model).waf;
    const HeadFire h =
        SolveHead(_surface[c.model], wind10X * waf, wind10Z * waf, c.gx, c.gz, waf, c.model >= 1 && c.model <= 3);
    c.hRate = h.rateMps * c.fuelK;
    c.hDirX = h.dirX;
    c.hDirZ = h.dirZ;
    c.hEcc = h.eccentricity;
    c.hLB = h.lengthToBreadth;
    c.hCrown = 0.0f;
    // (only a conifer forest carries a crown fire: deciduous broadleaf
    // forest practically never crowns in summer)
    if (c.sheltered && c.conifer)
    {
        // Active crown fire: Rothermel (1991), 3.34 x fuel model 10 at a 0.4 wind
        // adjustment, when it beats Van Wagner's critical rate 3 / CBD (m/min).
        const HeadFire h10 = SolveHead(_surface[10], wind10X * 0.4f, wind10Z * 0.4f, c.gx, c.gz);
        const float crown = 3.34f * h10.rateMps;
        if (crown * 60.0f >= 3.0f / 0.12f)
            c.hCrown = crown;
    }
}

bool FireGrid::Admit(Cell& c)
{
    if (c.state != CellState::Unburnt || c.model == 0 || !_surface[c.model].burns)
        return false;
    if (static_cast<int>(_front.size() + _caught.size()) >= _p.maxFront)
    {
        _stats.deferred++;
        return false;
    }
    c.state = CellState::Burning;
    c.gapDrawn = false;
    c.hasBack = false;
    c.torching = false;
    return true;
}

void FireGrid::DecideTorch(std::int32_t i, std::int32_t j, Cell& c)
{
    if (c.torching)
        return;
    // Only a tree standing in the cell torches (the canopy flag, any tree
    // within 9 m, shelters the grass but has no crown here), only a conifer
    // (Van Wagner 1977 was fitted to conifer stands near 100% foliar
    // moisture; summer broadleaf foliage at 150 to 250% scorches rather than
    // torches), and only when the surface fire beats its critical intensity.
    // An olive only partly: from kOliveTorchKwM, about kOliveTorchShare of them.
    const std::uint32_t oh = static_cast<std::uint32_t>(i) * 73856093u ^ static_cast<std::uint32_t>(j) * 19349663u;
    const bool oliveTorch = c.olive && c.intensityKwM >= kOliveTorchKwM &&
                            static_cast<float>((oh * 2654435761u) >> 8) / 16777216.0f < kOliveTorchShare;
    if (c.canopy && c.treeH > 0.0f &&
        ((c.conifer && c.intensityKwM >= TorchingIntensityKwM(c.canopyBaseM, _p.moisture.foliar)) || oliveTorch))
    {
        // The crown torches (its flame: TorchFlameM, TorchRatio x the
        // tree's height) once the front reaches its tree, and the cell flames
        // until it has burnt out. The surface flame keeps Byram's length.
        c.torching = true;
        const float treeAt = AlongFront(c.crossSec, c.frontX, c.frontZ, c.treeOX, c.treeOZ, _p.cellM) * c.crossSec;
        c.flameSec = std::max(c.flameSec, treeAt + kTorchDelaySec + kTorchSec + _p.stepSec);
        _stats.torching++;
        _torches.push_back({i, j});
    }
}

void FireGrid::Light(std::int32_t i, std::int32_t j, Cell& c)
{
    // (the dance fix) its drawn arrival as it catches (DrawnArrivalOf)
    c.drawnArrival = std::min(c.drawnArrival, c.centreTime);
    c.drawnCaughtT = static_cast<float>(_stats.simTime);
    _caught.push_back({i, j});
    _stats.ignitions++;
    const float cx = CentreX(i), cz = CentreZ(j);
    _stats.extMinX = std::min(_stats.extMinX, cx);
    _stats.extMaxX = std::max(_stats.extMaxX, cx);
    _stats.extMinZ = std::min(_stats.extMinZ, cz);
    _stats.extMaxZ = std::max(_stats.extMaxZ, cz);
}

namespace
{
// The cell's ellipse at a build-up share (its length to breadth grows from 1
// with it: FBP eq. 81).
float EccAt(const Cell& c, float grown)
{
    return EccentricityOf(1.0f + (c.hLB - 1.0f) * std::clamp(grown, 0.0f, 1.0f));
}
// Its square's half extent along a unit direction.
float HalfExtent(float cellM, float nx, float nz)
{
    return 0.5f * cellM * (std::fabs(nx) + std::fabs(nz));
}
// (the dance fix) The head its views draw with (FillFront): its head now, as
// it catches (its fields and its window are made with it too).
void FreezeDrawnHead(Cell& c)
{
    c.drawRate = c.hRate;
    c.drawDirX = c.hDirX;
    c.drawDirZ = c.hDirZ;
    c.drawLB = c.hLB;
}
// (the dance fix) A cell offered a front this step (its centre reached at
// eta): its drawn arrival the sooner of that and the one before run on by
// the step, so it comes sooner at once but later only with the clock.
void DrawnArrivalOffer(Cell& c, float eta, float stepSec)
{
    c.drawnArrival = c.drawnArrival < kNoDrawnArrival ? std::min(eta, c.drawnArrival + stepSec) : eta;
}
// Its arrival as its neighbours' views draw it (FillFront's arrival field):
// offered, its drawn arrival; caught, its centre's time, but no later than
// its drawn arrival when it caught run on with the clock (its neighbours
// drew it sooner, and their fronts must not run back). False when neither.
bool DrawnArrivalOf(const Cell& k, double now, double& t)
{
    if (k.state == CellState::Burning || k.state == CellState::Burnt)
    {
        t = std::min(static_cast<double>(k.centreTime),
                     static_cast<double>(k.drawnArrival) + std::max(0.0, now - static_cast<double>(k.drawnCaughtT)));
        return true;
    }
    if (k.state == CellState::Unburnt && k.model != 0 && k.drawnArrival < kNoDrawnArrival)
    {
        t = static_cast<double>(k.drawnArrival);
        return true;
    }
    return false;
}
} // namespace

void FireGrid::SeedFields(std::int32_t i, std::int32_t j, Cell& c)
{
    // (with the wind of the step it is made in: an ignition between steps
    // takes the next step's)
    UpdateHead(c, _windX, _windZ);
    FreezeDrawnHead(c);
    const SurfaceFire& s = _surface[c.model];
    const Anchor& a = c.anchor;
    const float t0 = a.t, age = a.age;
    const float cx = CentreX(i), cz = CentreZ(j);
    const float grown = BuildUp(age, _p.buildUpPerMin), ecc = EccAt(c, grown);
    const float rate = std::max(c.hRate, 1e-3f);
    c.fireAge0 = age;
    c.entryTime = t0;
    c.centreTime =
        t0 + BuildUpTime(age, AnchorGauge(a, cx - a.cx, cz - a.cz, rate, c.hDirX, c.hDirZ, ecc), _p.buildUpPerMin);
    c.drawnArrival = std::min(c.drawnArrival, c.centreTime); // (the dance fix) redone with the step's wind
    c.hasBack = false;
    if (a.halfM <= 0.0f)
    {
        // A point (or an area) starts as a flank-strength fire (the no-wind
        // rate) and lights its cell from itself (no planar front: crossSec 0);
        // its window lasts until the front has crossed to the far corner and
        // burnt there.
        const float r0 = std::max(s.r0Ftmin * kFtMinToMps * c.fuelK, 0.004f);
        c.rateMps = r0;
        c.intensityKwM = IntensityAt(s, r0);
        c.flameM = FlameLengthM(c.intensityKwM);
        c.frontX = c.frontZ = 0.0f;
        c.crossSec = 0.0f;
        c.igniteTime = t0;
        const float h = 0.5f * _p.cellM;
        float far = 0.0f;
        for (int k = 0; k < 4; ++k)
            far = std::max(far, AnchorGauge(a, cx + (k & 1 ? h : -h) - a.cx, cz + (k & 2 ? h : -h) - a.cz, rate,
                                            c.hDirX, c.hDirZ, ecc));
        c.flameSec = std::clamp(BuildUpTime(age, far + kEdgeNoiseM / rate, _p.buildUpPerMin) + s.residenceSec,
                                s.residenceSec + 2.0f, 900.0f);
        return;
    }
    // A line: a planar front leaves it on each side, the head phase on the
    // side the head runs to (window: its crossing to the far edge and the
    // residence), then the backing phase on the other.
    float nx = -a.uz, nz = a.ux;
    if (nx * c.hDirX + nz * c.hDirZ < 0.0f)
    {
        nx = -nx;
        nz = -nz;
    }
    const float l = (a.cx - cx) * nx + (a.cz - cz) * nz; // the line's offset along the head normal
    const float half = HalfExtent(_p.cellM, nx, nz);
    auto phase = [&](float px, float pz, float off, float& cross, float& igniteAt, float& window, float& rateMps,
                     float& intensity, float& flame)
    {
        const float veq = std::max(FrontSpeed(rate, c.hDirX, c.hDirZ, ecc, px, pz), 1e-4f);
        const float d = std::max(half - off, 0.0f);
        const float tFar = BuildUpTime(age, d / veq, _p.buildUpPerMin);
        const float v = std::max(d > 0.05f && tFar > 1e-3f ? d / tFar : veq * std::max(grown, 0.02f), 0.004f);
        rateMps = std::max(veq * BuildUp(age + 0.5f * tFar, _p.buildUpPerMin), 0.004f);
        intensity = IntensityAt(s, rateMps);
        flame = FlameLengthM(intensity);
        cross = std::min(_p.cellM / v, 900.0f);
        igniteAt = t0 + (-0.5f * _p.cellM - off) / v;
        window = std::min(900.0f, t0 + tFar + std::min(kEdgeNoiseM / v, kEntryNoiseMaxSec) + s.residenceSec - igniteAt);
    };
    c.frontX = nx;
    c.frontZ = nz;
    phase(nx, nz, l, c.crossSec, c.igniteTime, c.flameSec, c.rateMps, c.intensityKwM, c.flameM);
    c.hasBack = true;
    c.backX = -nx;
    c.backZ = -nz;
    phase(-nx, -nz, -l, c.backCross, c.backIgnite, c.backFlameSec, c.backRate, c.backIntensityKwM, c.backFlameM);
    // (the backing phase begins once the head phase's window is over; none
    // when the line lies on the cell's back edge)
    if (c.backIgnite + c.backFlameSec <= c.igniteTime + c.flameSec + 0.5f)
        c.hasBack = false;
}

bool FireGrid::CatchPoint(std::int32_t i, std::int32_t j, Cell& c, float px, float pz, float ageSec, float radiusM)
{
    if (!Admit(c))
        return false;
    const float now = static_cast<float>(_stats.simTime);
    c.anchor = Anchor{px, pz, 1.0f, 0.0f, 0.0f, now, std::max(ageSec, 0.0f), ++_anchorIds, true, radiusM};
    SeedFields(i, j, c);
    Light(i, j, c);
    _seedFix.push_back({i, j});
    return true;
}

bool FireGrid::CatchOffer(std::int32_t i, std::int32_t j, Cell& c, const Offer& o)
{
    if (!Admit(c))
        return false;
    const SurfaceFire& s = _surface[c.model];
    const float now = static_cast<float>(_stats.simTime);
    const float late = std::clamp(now - o.eta, 0.0f, _p.stepSec);
    c.entryTime = now - late;
    c.fireAge0 = std::max(0.0f, o.ageNow - late);
    c.centreTime = std::max(o.etaCentre, c.entryTime);
    // The front's normal and normal speed at the centre: the arrival field
    // it came from (the neighbours it reached earlier, a plane fitted through
    // their centre times and its own, the nearer weighing more) where that
    // agrees with its source's (within 60 degrees); else the source's own (a
    // seed's first cells, a narrow head). Within the ellipse's own back and
    // head rates; the intensity and flames from it (a flank's are a flank's).
    float nx = o.nx, nz = o.nz;
    float v = o.vn * std::max(o.grown, 0.0f);
    {
        float sxx = 0.0f, sxz = 0.0f, szz = 0.0f, bx = 0.0f, bz = 0.0f;
        int count = 0;
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
            {
                const Cell* k = (di || dj) ? CellAtConst(i + di, j + dj) : nullptr;
                if (!k || (k->state != CellState::Burning && k->state != CellState::Burnt) ||
                    k->centreTime >= c.centreTime - 0.05f)
                    continue;
                const float dx = static_cast<float>(di) * _p.cellM, dz = static_cast<float>(dj) * _p.cellM;
                const float w = 1.0f / (dx * dx + dz * dz), dt = k->centreTime - c.centreTime;
                sxx += w * dx * dx;
                sxz += w * dx * dz;
                szz += w * dz * dz;
                bx += w * dx * dt;
                bz += w * dz * dt;
                count++;
            }
        const float det = sxx * szz - sxz * sxz;
        if (count >= 2 && det > 1e-3f * (sxx + szz) * (sxx + szz))
        {
            const float gx = (szz * bx - sxz * bz) / det, gz = (sxx * bz - sxz * bx) / det;
            const float gl = std::sqrt(gx * gx + gz * gz);
            if (gl > 1e-6f && gx * o.nx + gz * o.nz > 0.5f * gl)
            {
                nx = gx / gl;
                nz = gz / gl;
                v = 1.0f / gl;
            }
        }
        const float ecc = o.ecc, rate = std::max(c.hRate, 1e-3f), g = std::max(o.grown, 0.02f);
        v = std::clamp(v, rate * (1.0f - ecc) / (1.0f + ecc) * g, rate * g);
    }
    v = std::max(v, 0.004f);
    c.rateMps = v;
    c.intensityKwM = IntensityAt(s, v);
    c.flameM = FlameLengthM(c.intensityKwM);
    const float nl = std::sqrt(nx * nx + nz * nz);
    c.frontX = nl > 1e-4f ? nx / nl : 0.0f;
    c.frontZ = nl > 1e-4f ? nz / nl : 0.0f;
    // The planar front across the square: it passes the centre at the
    // centre's arrival, the near edge half a crossing before; the window runs
    // until the far corner (and its edge noise) has burnt for the residence.
    c.crossSec = std::min(_p.cellM / v, 900.0f);
    c.igniteTime = c.centreTime - 0.5f * c.crossSec;
    const float far = std::max(c.centreTime + HalfExtent(_p.cellM, c.frontX, c.frontZ) / v, o.etaLast) +
                      std::min(kEdgeNoiseM / v, kEntryNoiseMaxSec);
    c.flameSec = std::min(900.0f, far + s.residenceSec - c.igniteTime);
    if (o.slide)
    {
        // A new anchor on the same ray, kAnchorSlideM behind the cell: the
        // front there (its time from the old anchor's).
        const float dt = BuildUpTime(o.src.age, o.slideRun, _p.buildUpPerMin);
        c.anchor = FrontPiece(o.src, o.qx, o.qz, o.slideX, o.slideZ, c.frontX, c.frontZ, o.hx, o.hz, o.ecc,
                              o.src.t + dt, o.src.age + dt, kAnchorSlideHalfM);
        c.anchor.id = ++_anchorIds;
        _stats.slides++;
    }
    else
        c.anchor = o.src;
    FreezeDrawnHead(c);
    DecideTorch(i, j, c);
    Light(i, j, c);
    return true;
}

bool FireGrid::Ignite(float x, float z)
{
    const std::int32_t i = IndexOf(x), j = IndexOf(z);
    Cell* c = CellAt(i, j, true);
    const bool ok = c && c->model && CatchPoint(i, j, *c, x, z, 0.0f);
    for (const Key& k : _caught)
    {
        _front.push_back(k);
        _flaming.push_back(k);
    }
    _caught.clear();
    return ok;
}

int FireGrid::IgniteArea(float x, float z, float radiusM)
{
    // Its cells all at once, all of one fire from the disc (its edge burning now).
    int n = 0;
    const std::int32_t r = static_cast<std::int32_t>(std::ceil(radiusM / _p.cellM));
    const std::int32_t ci = IndexOf(x), cj = IndexOf(z);
    const float age = 2.0f * radiusM >= kMatureLineM ? kMatureAgeSec : 0.0f;
    for (std::int32_t j = cj - r; j <= cj + r; ++j)
        for (std::int32_t i = ci - r; i <= ci + r; ++i)
        {
            const float dx = CentreX(i) - x, dz = CentreZ(j) - z;
            if (dx * dx + dz * dz > radiusM * radiusM + 0.25f * _p.cellM * _p.cellM)
                continue;
            Cell* c = CellAt(i, j, true);
            if (c && c->model && CatchPoint(i, j, *c, x, z, age, radiusM))
                n++;
        }
    for (const Key& k : _caught)
    {
        _front.push_back(k);
        _flaming.push_back(k);
    }
    _caught.clear();
    return n;
}

int FireGrid::IgniteLine(float x0, float z0, float x1, float z1)
{
    const float len = std::sqrt((x1 - x0) * (x1 - x0) + (z1 - z0) * (z1 - z0));
    const float age = len >= kMatureLineM ? kMatureAgeSec : 0.0f;
    const int steps = std::max(1, static_cast<int>(std::ceil(len / (0.5f * _p.cellM))));
    const float now = static_cast<float>(_stats.simTime);
    const float ux = len > 1e-3f ? (x1 - x0) / len : 1.0f, uz = len > 1e-3f ? (z1 - z0) / len : 0.0f;
    // The line's runs of burnable cells: each run's piece of the line is the
    // anchor of its cells (a bare cell or a road breaks it).
    struct Run
    {
        std::vector<Key> cells;
        float s0 = 0.0f, s1 = 0.0f;
    };
    std::vector<Run> runs;
    Key last{0, 0};
    bool haveLast = false, open = false;
    for (int k = 0; k <= steps; ++k)
    {
        const float s = len * static_cast<float>(k) / static_cast<float>(steps);
        const std::int32_t i = IndexOf(x0 + ux * s), j = IndexOf(z0 + uz * s);
        if (haveLast && i == last.i && j == last.j)
        {
            if (open)
                runs.back().s1 = s;
            continue;
        }
        last = {i, j};
        haveLast = true;
        Cell* c = CellAt(i, j, true);
        const bool burns = c && c->state == CellState::Unburnt && c->model && _surface[c->model].burns;
        if (!burns)
        {
            open = false;
            continue;
        }
        if (!open)
        {
            runs.push_back(Run());
            runs.back().s0 = s;
        }
        open = true;
        runs.back().cells.push_back({i, j});
        runs.back().s1 = s;
    }
    const float step = len / static_cast<float>(steps);
    int n = 0;
    for (Run& run : runs)
    {
        const float s0 = std::max(0.0f, run.s0 - 0.5f * step), s1 = std::min(len, run.s1 + 0.5f * step);
        const float sc = 0.5f * (s0 + s1);
        const Anchor line{x0 + ux * sc, z0 + uz * sc, ux, uz, 0.5f * (s1 - s0), now, age, ++_anchorIds, true, 0.0f};
        for (const Key& key : run.cells)
        {
            Cell* c = CellAt(key.i, key.j, true);
            if (!c || !Admit(*c))
                continue;
            c->anchor = line;
            SeedFields(key.i, key.j, *c);
            Light(key.i, key.j, *c);
            _seedFix.push_back(key);
            n++;
        }
    }
    for (const Key& k : _caught)
    {
        _front.push_back(k);
        _flaming.push_back(k);
    }
    _caught.clear();
    return n;
}

float FireGrid::TorchAgeOf(const Cell& c) const
{
    return TorchAgeAt(c, DisplayTime());
}

float FireGrid::TorchShape(const Cell& c, float torchAge) const
{
    if (!c.olive)
        return TorchEnvelope(torchAge);
    // the olive's briefer torch: the same shape over kOliveTorchSec
    return TorchEnvelope(kTorchDelaySec + (torchAge - kTorchDelaySec) * (kTorchSec / kOliveTorchSec));
}

float FireGrid::TorchFlameM(const Cell& c, std::int32_t i, std::int32_t j, float* maxM) const
{
    if (!c.torching || c.treeH <= 0.0f)
    {
        if (maxM)
            *maxM = 0.0f;
        return 0.0f;
    }
    float ratio = kOliveTorchRatio;
    if (!c.olive)
    {
        // The trees torching together round it (its 8 neighbours' torches
        // burning now) flame higher (Albini's N^0.4).
        int n = 1;
        const double t = DisplayTime();
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
            {
                if (di == 0 && dj == 0)
                    continue;
                const Cell* o = CellAtConst(i + di, j + dj);
                if (o && o->torching && o->treeH > 0.0f && TorchShape(*o, TorchAgeAt(*o, t)) > 0.0f)
                    n++;
            }
        ratio = TorchRatio(n);
    }
    const float full = std::max(0.0f, ratio - kTorchFootShare) * c.treeH;
    if (maxM)
        *maxM = full;
    return full * TorchShape(c, TorchAgeOf(c));
}

float FireGrid::TorchAgeAt(const Cell& c, double t) const
{
    const float age = static_cast<float>(t - static_cast<double>(c.igniteTime));
    return age - AlongFront(c.crossSec, c.frontX, c.frontZ, c.treeOX, c.treeOZ, _p.cellM) * c.crossSec;
}

float FireGrid::StrengthOf(const Cell& c) const
{
    if (c.state != CellState::Burning || c.flameSec <= 0.0f)
        return 0.0f;
    // On the display clock, over the window the views show (it ends a step
    // before the cell lies burnt, so nothing is cut off while it still
    // shows): from when the front entered its square to the end of its
    // window, then (an ignition line's cell) over its backing phase's.
    const double now = DisplayTime();
    double from = c.entryTime, to = static_cast<double>(c.igniteTime) + c.flameSec - _p.stepSec;
    if (c.hasBack && now >= static_cast<double>(c.igniteTime) + c.flameSec)
    {
        from = static_cast<double>(c.igniteTime) + c.flameSec;
        to = static_cast<double>(c.backIgnite) + c.backFlameSec - _p.stepSec;
    }
    const float age = static_cast<float>(now - from);
    if (age <= 0.0f)
        return 0.0f;
    const float life = std::max(static_cast<float>(to - from), 0.5f);
    const float in = std::min(1.0f, age / std::max(0.5f, std::min(3.0f, 0.2f * life)));
    const float out = std::clamp((life - age) / (0.4f * life), 0.0f, 1.0f);
    return in * out;
}

float PartStrength(const BurningView& v, float ox, float oz, float cellM)
{
    // The point's own age (since the front reached it) and window.
    const float a = FrontAgeAt(v, ox, oz, cellM);
    if (a <= 0.0f)
        return 0.0f;
    const float d = std::max(v.life - v.crossSec, 0.5f);
    const float in = std::min(1.0f, a / std::max(0.5f, std::min(3.0f, 0.2f * d)));
    const float out = std::clamp((d - a) / (0.4f * d), 0.0f, 1.0f);
    return in * out;
}

void FireGrid::Tick(float dt, float wind10X, float wind10Z)
{
    if (!Active())
    {
        _acc = 0.0f;
        return;
    }
    _acc += dt;
    int steps = 0;
    while (_acc >= _p.stepSec && steps < _p.maxStepsPerTick)
    {
        // The wind this step: the given wind turned by the wander at the
        // step's own sim time (never the frame's).
        const float turn = WanderRad(_stats.simTime + _p.stepSec);
        const float c = std::cos(turn), sn = std::sin(turn);
        Step(wind10X * c - wind10Z * sn, wind10X * sn + wind10Z * c);
        _acc -= _p.stepSec;
        steps++;
    }
    if (_acc > _p.stepSec)
        _acc = _p.stepSec;
}

void FireGrid::Step(float wind10X, float wind10Z)
{
    _stats.steps++;
    _stats.simTime += _p.stepSec;
    const float now = static_cast<float>(_stats.simTime);
    _windX = wind10X;
    _windZ = wind10Z;

    // The 8 adjacent cells: the 4 sides (+x, -x, +z, -z), then the corners.
    // No knight's moves: the anchors give every direction, and a step never
    // jumps a cell (the lattice's free crossing of a one-cell gap is gone).
    static const int kDi[8] = {1, -1, 0, 0, 1, -1, -1, 1};
    static const int kDj[8] = {0, 0, 1, -1, 1, 1, -1, -1};

    const std::size_t n = _front.size();
    // (Past maxFrontPerStep the front is visited round robin: the travel
    // times run on the clock, so a cell visited late loses no time.)
    const std::size_t todo = std::min<std::size_t>(n, static_cast<std::size_t>(_p.maxFrontPerStep));
    if (_cursor >= n)
        _cursor = 0;

    // The cached head fires go stale when the wind has moved by a tenth of a
    // metre a second (half a degree at 11 m/s: a wandering wind turns them smoothly).
    if (std::fabs(wind10X - _stampWindX) + std::fabs(wind10Z - _stampWindZ) > 0.1f)
    {
        _windStamp++;
        _stampWindX = wind10X;
        _stampWindZ = wind10Z;
    }
    // The cells lit since the last step take this step's wind (their fronts, windows and torches).
    for (const Key& k : _seedFix)
    {
        Cell* c = CellAt(k.i, k.j, false);
        if (c && c->state == CellState::Burning)
        {
            SeedFields(k.i, k.j, *c);
            DecideTorch(k.i, k.j, *c);
        }
    }
    _seedFix.clear();
    _keep.assign(n, 1);
    std::vector<std::uint8_t>& keep = _keep;

    for (std::size_t k = 0; k < todo; ++k)
    {
        const std::size_t idx = (_cursor + k) % n;
        const Key key = _front[idx];
        Cell* c = CellAt(key.i, key.j, false);
        if (!c)
        {
            keep[idx] = 0;
            continue;
        }
        const float gx = CentreX(key.i), gz = CentreZ(key.j);
        bool open = false;
        std::uint8_t roadSides = 0, bareSides = 0, builtSides = 0;
        for (int d = 0; d < 8; ++d)
        {
            const std::int32_t ni = key.i + kDi[d], nj = key.j + kDj[d];
            Cell* nc = CellAt(ni, nj, true);
            if (!nc)
                continue;
            if (d < 4 && nc->model == 0 && nc->roadHalfM <= 0.0f)
                bareSides = static_cast<std::uint8_t>(bareSides | (1u << d));
            // (buildings builder) a building's bare cell (not a road's): the
            // drawing runs on into it; the spread is the bare cell's as before
            if (d < 4 && nc->model == 0 && nc->roadHalfM <= 0.0f && nc->built)
                builtSides = static_cast<std::uint8_t>(builtSides | (1u << d));
            if (nc->model == 0 && nc->roadHalfM > 0.0f)
            {
                // A bare road cell beside it: its ground runs on to the bare
                // edge there; the road's far side is the cell beyond, crossed
                // by flame contact within reach, else only by embers.
                if (d < 4)
                    roadSides = static_cast<std::uint8_t>(roadSides | (1u << d));
                if (c->state != CellState::Burning)
                    continue;
                float gapEff = 0.0f, reach = 0.0f;
                GapReach(*c, *nc, wind10X, wind10Z, gapEff, reach);
                if (gapEff > reach)
                {
                    RoadEmbers(*c, key.i, key.j, *nc, wind10X, wind10Z);
                    continue;
                }
                const std::int32_t fi = key.i + 2 * kDi[d], fj = key.j + 2 * kDj[d];
                Cell* fc = CellAt(fi, fj, true);
                if (fc && fc->state == CellState::Unburnt && fc->model && _surface[fc->model].burns)
                {
                    UpdateHead(*fc, wind10X, wind10Z);
                    open = true;
                    OfferTo(*c, key.i, key.j, *fc, fi, fj, wind10X, wind10Z, true);
                }
                continue;
            }
            if (nc->state != CellState::Unburnt || nc->model == 0)
                continue;
            const SurfaceFire& s = _surface[nc->model];
            if (!s.burns)
                continue;
            UpdateHead(*nc, wind10X, wind10Z);
            // A road between them: flame contact, or embers only.
            bool contact = false;
            if (const Cell* road = RoadBetween(*c, *nc, gx, gz, CentreX(ni), CentreZ(nj)))
            {
                float gapEff = 0.0f, reach = 0.0f;
                GapReach(*c, *road, wind10X, wind10Z, gapEff, reach);
                if (gapEff > reach)
                {
                    if (c->state == CellState::Burning)
                        RoadEmbers(*c, key.i, key.j, *road, wind10X, wind10Z);
                    continue;
                }
                contact = true;
            }
            open = true;
            OfferTo(*c, key.i, key.j, *nc, ni, nj, wind10X, wind10Z, contact);
        }
        c->roadSides = roadSides;
        c->bareSides = bareSides;
        c->builtSides = builtSides;
        if (!open)
            keep[idx] = 0;
    }
    _cursor = n > 0 ? (_cursor + todo) % n : 0;

    // Embers from the torching trees, and the road embers landing now.
    Torches(wind10X, wind10Z);
    Pending();

    // Offers: each cell catches once the soonest front offered it has entered its square.
    for (const Offer& o : _offers)
    {
        Cell* c = CellAt(o.key.i, o.key.j, false);
        if (!c)
            continue;
        c->offered = false;
        c->offer = -1;
        if (c->state != CellState::Unburnt)
            continue;
        DrawnArrivalOffer(*c, o.etaCentre, _p.stepSec);
        if (o.eta > now)
            continue;
        CatchOffer(o.key.i, o.key.j, *c, o);
    }
    _offers.clear();

    // The front: drop the closed cells, join the newly caught.
    std::size_t w = 0;
    for (std::size_t r = 0; r < n; ++r)
        if (keep[r])
            _front[w++] = _front[r];
    _front.resize(w);
    for (const Key& k : _caught)
    {
        _front.push_back(k);
        _flaming.push_back(k);
    }
    _caught.clear();
    if (_cursor >= _front.size())
        _cursor = 0;

    // Flaming cells whose time is up lie burnt (an ignition line's cell once
    // its backing phase is over too); the rest keep their head fires fresh
    // (their views draw the front from them).
    w = 0;
    for (std::size_t r = 0; r < _flaming.size(); ++r)
    {
        Cell* c = CellAt(_flaming[r].i, _flaming[r].j, false);
        if (!c || c->state != CellState::Burning)
            continue;
        const bool over =
            (now - c->igniteTime >= c->flameSec && (!c->hasBack || now - c->backIgnite >= c->backFlameSec)) ||
            InteriorOut(*c, _flaming[r].i, _flaming[r].j, static_cast<float>(now));
        if (over)
        {
            c->state = CellState::Burnt;
            _stats.burnt++;
            continue;
        }
        UpdateHead(*c, wind10X, wind10Z);
        _flaming[w++] = _flaming[r];
    }
    _flaming.resize(w);
    _stats.burning = static_cast<int>(_flaming.size());
    _stats.front = static_cast<int>(_front.size());
}

bool FireGrid::InteriorOut(const Cell& c, std::int32_t i, std::int32_t j, float now) const
{
    // A cell whose source said its last corner burns late (a narrow head's
    // far side, reached backwards from it) is often reached sooner by its
    // neighbours' fronts. Once every neighbour is caught (or has no fuel),
    // the drawn front across the square is the arrival between their
    // centres: the square is crossed when its latest corner (the mean of the
    // four centres round it) is, and the cell is out its residence and edge
    // noise after that. (Not an ignition line's cell, nor one drawing its
    // seed's own field, near it: FillFront's exact; not a torching crown.)
    if (c.hasBack || c.torching)
        return false;
    if (c.anchor.seed)
    {
        const float px = CentreX(i) - c.anchor.cx, pz = CentreZ(j) - c.anchor.cz;
        const float s = std::clamp(px * c.anchor.ux + pz * c.anchor.uz, -c.anchor.halfM, c.anchor.halfM);
        const float dx = px - s * c.anchor.ux, dz = pz - s * c.anchor.uz;
        if (std::sqrt(dx * dx + dz * dz) <= kSeedExactM + c.anchor.radiusM + _p.cellM)
            return false;
    }
    float t[9];
    for (int dj = -1; dj <= 1; ++dj)
        for (int di = -1; di <= 1; ++di)
        {
            const Cell* k = (di || dj) ? CellAtConst(i + di, j + dj) : &c;
            if (!k)
                return false;
            if (k->state == CellState::Burning || k->state == CellState::Burnt)
                t[(dj + 1) * 3 + di + 1] = k->centreTime;
            else if (k->model == 0)
                t[(dj + 1) * 3 + di + 1] = c.centreTime;
            else
                return false;
        }
    float latest = -1e30f;
    for (int sz = 0; sz <= 1; ++sz)
        for (int sx = 0; sx <= 1; ++sx)
            latest = std::max(latest, 0.25f * (t[sz * 3 + sx] + t[sz * 3 + sx + 1] + t[(sz + 1) * 3 + sx] +
                                               t[(sz + 1) * 3 + sx + 1]));
    const float v = std::max(c.rateMps, 1e-3f);
    return now >= latest + _surface[c.model].residenceSec + std::min(kEdgeNoiseM / v, kEntryNoiseMaxSec);
}

Anchor FireGrid::LocalAnchor(const Cell& g, std::int32_t gi, std::int32_t gj)
{
    // A point or a line ignition spreads from itself.
    if (g.hasBack || (g.frontX == 0.0f && g.frontZ == 0.0f))
        return g.anchor;
    // Any other cell from the piece of front through its centre at its
    // centre's time (as straight as its own anchor says the front is there).
    const Anchor& a = g.anchor;
    const float cx = CentreX(gi), cz = CentreZ(gj);
    const float now = static_cast<float>(_stats.simTime);
    const float ecc = EccAt(g, BuildUp(a.age + std::max(0.0f, now - a.t), _p.buildUpPerMin));
    float qx = 0.0f, qz = 0.0f;
    AnchorGauge(a, cx - a.cx, cz - a.cz, std::max(g.hRate, 1e-3f), g.hDirX, g.hDirZ, ecc, &qx, &qz);
    Anchor l = FrontPiece(a, qx, qz, cx, cz, g.frontX, g.frontZ, g.hDirX, g.hDirZ, ecc, g.centreTime,
                          g.fireAge0 + std::max(0.0f, g.centreTime - g.entryTime), kLocalHalfM);
    l.id = ++_anchorIds;
    return l;
}

void FireGrid::OfferTo(const Cell& g, std::int32_t gi, std::int32_t gj, Cell& n, std::int32_t ni, std::int32_t nj,
                       float wind10X, float wind10Z, bool contact)
{
    (void)wind10X;
    (void)wind10Z;
    const float now = static_cast<float>(_stats.simTime);
    const float nx = CentreX(ni), nz = CentreZ(nj);
    // Crown to crown, once the giving cell torches and the crowns can carry it.
    const float crown = g.torching && n.hCrown > 0.0f ? n.hCrown : 0.0f;
    if (!contact && crown <= 0.0f)
    {
        // The giver's anchor: the ray from its nearest point (in travel time)
        // to the cell's centre, through each cell's own ellipse.
        const Anchor& a = g.anchor;
        const float grown = BuildUp(a.age + std::max(0.0f, now - a.t), _p.buildUpPerMin);
        const float ecc = EccAt(n, grown);
        float qx = 0.0f, qz = 0.0f;
        AnchorGauge(a, nx - a.cx, nz - a.cz, std::max(n.hRate, 1e-3f), n.hDirX, n.hDirZ, ecc, &qx, &qz);
        const float px = a.cx + qx, pz = a.cz + qz;
        const float dx = nx - px, dz = nz - pz;
        const float dist = std::sqrt(dx * dx + dz * dz);
        if (dist <= kAnchorMaxM + 1.5f * _p.cellM)
        {
            // Past kAnchorMaxM it slides: a new anchor kAnchorSlideM behind the cell on this ray.
            const bool slide = dist > kAnchorMaxM;
            float run = 0.0f, runNear = 0.0f;
            if (RayRun(nx, nz, px, pz, grown, slide ? kAnchorSlideM : 0.0f, run, runNear))
            {
                const float sx = slide ? nx - dx * (kAnchorSlideM / dist) : 0.0f;
                const float sz = slide ? nz - dz * (kAnchorSlideM / dist) : 0.0f;
                Consider(n, ni, nj, a, run, qx, qz, slide, run - runNear, sx, sz, 0.0f);
                return;
            }
        }
    }
    // Its own piece of front (and a crown's rate, or a gap crossed by flame contact).
    const Anchor l = LocalAnchor(g, gi, gj);
    const float grown = BuildUp(l.age + std::max(0.0f, now - l.t), _p.buildUpPerMin);
    const float ecc = EccAt(n, grown);
    const float rate = std::max(std::max(n.hRate, crown), 1e-3f);
    float qx = 0.0f, qz = 0.0f;
    const float gc = AnchorGauge(l, nx - l.cx, nz - l.cz, rate, n.hDirX, n.hDirZ, ecc, &qx, &qz);
    _stats.localOffers++;
    Consider(n, ni, nj, l, gc, qx, qz, false, 0.0f, 0.0f, 0.0f, crown);
}

void FireGrid::Consider(Cell& n, std::int32_t ni, std::int32_t nj, const Anchor& src, float gCentre, float qx, float qz,
                        bool slide, float slideRun, float slideX, float slideZ, float crownRate)
{
    const float now = static_cast<float>(_stats.simTime);
    // (a source still ahead in time, a cell caught as the front entered its
    // square whose centre it has not reached: its run is negative until then)
    const float since = now - src.t;
    const float ageNow = src.age + std::max(0.0f, since);
    const float grown = BuildUp(ageNow, _p.buildUpPerMin);
    const float run = since >= 0.0f ? BuildUpRun(src.age, since, _p.buildUpPerMin)
                                    : since * std::max(BuildUp(src.age, _p.buildUpPerMin), 0.02f);
    // The front at the cell's centre: its normal (the gauge's gradient
    // through the cell's own ellipse) and normal speed.
    const float ecc = EccAt(n, grown);
    const float rate = std::max(std::max(n.hRate, crownRate), 1e-3f);
    const float px = CentreX(ni) - (src.cx + qx), pz = CentreZ(nj) - (src.cz + qz);
    const float r = std::sqrt(px * px + pz * pz);
    float gx = n.hDirX, gz = n.hDirZ;
    if (r > 1e-3f)
    {
        gx = px / r - ecc * n.hDirX;
        gz = pz / r - ecc * n.hDirZ;
    }
    const float gl = std::sqrt(gx * gx + gz * gz);
    const float nx = gl > 1e-6f ? gx / gl : n.hDirX, nz = gl > 1e-6f ? gz / gl : n.hDirZ;
    const float vn = std::max(FrontSpeed(rate, n.hDirX, n.hDirZ, ecc, nx, nz), 1e-4f);
    // When it enters the square: the soonest of its corners from the source
    // (a straight front reaches one first) and of its edges from the source's
    // point nearest the centre (a narrow head can reach an edge between
    // corners), carried by the ray's own time, less the edge noise's reach
    // there; and when it reaches the centre.
    const float single = AnchorGauge(src, CentreX(ni) - src.cx, CentreZ(nj) - src.cz, rate, n.hDirX, n.hDirZ, ecc);
    float ex = 0.0f, ez = 0.0f;
    const float sq =
        SquareEntryGauge(src, CentreX(ni), CentreZ(nj), qx, qz, 0.5f * _p.cellM, rate, n.hDirX, n.hDirZ, ecc, ex, ez);
    // (the front's speed there: from the source's own nearest point to it)
    float eqx = 0.0f, eqz = 0.0f;
    AnchorGauge(src, CentreX(ni) + ex - src.cx, CentreZ(nj) + ez - src.cz, rate, n.hDirX, n.hDirZ, ecc, &eqx, &eqz);
    const float ux = CentreX(ni) + ex - (src.cx + eqx), uz = CentreZ(nj) + ez - (src.cz + eqz);
    const float ul = std::sqrt(ux * ux + uz * uz);
    float vEntry = vn;
    if (ul > 1e-3f)
    {
        const float ax = ux / ul - ecc * n.hDirX, az = uz / ul - ecc * n.hDirZ;
        vEntry = rate * (1.0f - ecc) / std::max(std::sqrt(ax * ax + az * az), 1e-4f);
    }
    const float gEntry =
        std::max(0.0f, sq + (gCentre - single) - std::min(kEdgeNoiseM / std::max(vEntry, 1e-4f), kEntryNoiseMaxSec));
    // And when it reaches the square's last corner (the window's end).
    float gLast = 0.0f;
    for (int k = 0; k < 4; ++k)
    {
        const float kx = (k & 1 ? 0.5f : -0.5f) * _p.cellM, kz = (k & 2 ? 0.5f : -0.5f) * _p.cellM;
        gLast = std::max(
            gLast, AnchorGauge(src, CentreX(ni) + kx - src.cx, CentreZ(nj) + kz - src.cz, rate, n.hDirX, n.hDirZ, ecc));
    }
    gLast += gCentre - single;
    const float speed = std::max(grown, 0.02f);
    const float eta = now + (gEntry - run) / speed;
    if (!n.offered)
    {
        n.offered = true;
        n.offer = static_cast<std::int32_t>(_offers.size());
        _offers.push_back(Offer());
        _offers.back().key = {ni, nj};
    }
    Offer& o = _offers[static_cast<std::size_t>(n.offer)];
    // (a near tie goes to the wider source: a straight front stays a segment)
    const float rank = eta - 0.01f * std::min(src.halfM, kAnchorSlideHalfM);
    if (rank >= o.rank)
        return;
    o.rank = rank;
    o.eta = eta;
    o.etaCentre = now + (gCentre - run) / speed;
    o.etaLast = now + (gLast - run) / speed;
    o.nx = nx;
    o.nz = nz;
    o.vn = vn;
    o.grown = grown;
    o.ageNow = ageNow;
    o.src = src;
    o.qx = qx;
    o.qz = qz;
    o.hx = n.hDirX;
    o.hz = n.hDirZ;
    o.ecc = ecc;
    o.slide = slide;
    o.slideRun = slideRun;
    o.slideX = slideX;
    o.slideZ = slideZ;
}

bool FireGrid::RayRun(float x0, float z0, float x1, float z1, float grown, float nearM, float& run, float& runNear)
{
    _stats.rays++;
    run = runNear = 0.0f;
    const float dx = x1 - x0, dz = z1 - z0;
    const float len = std::sqrt(dx * dx + dz * dz);
    if (len < 1e-3f)
        return true;
    // Walked from the cell (x0, z0) back to the anchor (x1, z1); the fire
    // travels the other way, (ux, uz).
    const float bx = dx / len, bz = dz / len, ux = -bx, uz = -bz;
    const float cell = _p.cellM;
    std::int32_t i = IndexOf(x0), j = IndexOf(z0);
    const std::int32_t iEnd = IndexOf(x1), jEnd = IndexOf(z1);
    const int stepI = bx > 0.0f ? 1 : -1, stepJ = bz > 0.0f ? 1 : -1;
    auto first = [cell](float p, float d, std::int32_t idx)
    {
        if (std::fabs(d) < 1e-6f)
            return 1e30f;
        const float edge = static_cast<float>(d > 0.0f ? idx + 1 : idx) * cell;
        return (edge - p) / d;
    };
    float tx = first(x0, bx, i), tz = first(z0, bz, j);
    const float stepX = std::fabs(bx) > 1e-6f ? cell / std::fabs(bx) : 1e30f;
    const float stepZ = std::fabs(bz) > 1e-6f ? cell / std::fabs(bz) : 1e30f;
    float s0 = 0.0f;
    for (int guard = 0; guard < 64; ++guard)
    {
        const bool last = i == iEnd && j == jEnd;
        const float s1 = last ? len : std::min(std::min(tx, tz), len);
        Cell* c = CellAt(i, j, false);
        if (!c || c->state == CellState::Unknown || c->model == 0 || !_surface[c->model].burns)
            return false;
        if (c->roadHalfM > 0.0f)
        {
            // The ray's piece in this cell must stay off its road's bare surface, on one side of it.
            const float nrx = c->roadDZ, nrz = -c->roadDX;
            const float d0 = (x0 + bx * s0 - c->roadX) * nrx + (z0 + bz * s0 - c->roadZ) * nrz;
            const float d1 = (x0 + bx * s1 - c->roadX) * nrx + (z0 + bz * s1 - c->roadZ) * nrz;
            if (d0 * d1 < 0.0f || std::fabs(d0) < c->roadHalfM || std::fabs(d1) < c->roadHalfM)
                return false;
        }
        UpdateHead(*c, _windX, _windZ);
        const float e = EccAt(*c, grown);
        const float slow =
            c->hRate > 1e-6f ? (1.0f - e * (ux * c->hDirX + uz * c->hDirZ)) / (c->hRate * (1.0f - e)) : 1e9f;
        run += (s1 - s0) * slow;
        if (s0 < nearM)
            runNear += (std::min(s1, nearM) - s0) * slow;
        if (last || s1 >= len)
            return true;
        s0 = s1;
        if (tx < tz)
        {
            i += stepI;
            tx += stepX;
        }
        else
        {
            j += stepJ;
            tz += stepZ;
        }
    }
    return false;
}

const Cell* FireGrid::RoadBetween(const Cell& a, const Cell& b, float ax, float az, float bx, float bz) const
{
    for (const Cell* r : {&a, &b})
    {
        if (r->roadHalfM <= 0.0f)
            continue;
        const float nrx = r->roadDZ, nrz = -r->roadDX;
        const float sa = (ax - r->roadX) * nrx + (az - r->roadZ) * nrz;
        const float sb = (bx - r->roadX) * nrx + (bz - r->roadZ) * nrz;
        if (sa * sb < 0.0f || std::fabs(sa) < r->roadHalfM || std::fabs(sb) < r->roadHalfM)
            return r;
    }
    return nullptr;
}

void FireGrid::GapReach(const Cell& g, const Cell& road, float wind10X, float wind10Z, float& gapEff,
                        float& reach) const
{
    const float u10 = std::sqrt(wind10X * wind10X + wind10Z * wind10Z);
    const float nrx = road.roadDZ, nrz = -road.roadDX;
    const float cosWN = u10 > 1e-3f ? (wind10X * nrx + wind10Z * nrz) / u10 : 1.0f;
    gapEff = GapAlongWindM(2.0f * road.roadHalfM, cosWN);
    const float waf = g.sheltered ? 0.15f : Model(g.model).waf;
    reach = FlameReachM(g.flameM, FlameTiltRad(waf * u10, g.flameM));
}

void FireGrid::RoadEmbers(Cell& g, std::int32_t gi, std::int32_t gj, const Cell& road, float wind10X, float wind10Z)
{
    // Once per cell, as its front burns out against the gap.
    if (g.gapDrawn || g.state != CellState::Burning)
        return;
    g.gapDrawn = true;
    _stats.roadDraws++;
    const float u10 = std::sqrt(wind10X * wind10X + wind10Z * wind10Z);
    if (!_p.spotting || u10 < kRoadEmberMinWindMps)
        return;
    float gapEff = 0.0f, reach = 0.0f;
    GapReach(g, road, wind10X, wind10Z, gapEff, reach);
    const float gx = CentreX(gi), gz = CentreZ(gj);
    const float nrx = road.roadDZ, nrz = -road.roadDX;
    const float sG = (gx - road.roadX) * nrx + (gz - road.roadZ) * nrz;
    const float fx = sG >= 0.0f ? -nrx : nrx, fz = sG >= 0.0f ? -nrz : nrz; // toward the far side
    // The front's intensity toward the road (a head square on, a flank along it).
    const float now = static_cast<float>(_stats.simTime);
    const float grown = std::max(BuildUp(g.fireAge0 + std::max(0.0f, now - g.entryTime), _p.buildUpPerMin), 0.02f);
    const float v = FrontSpeed(std::max(g.hRate, 1e-3f), g.hDirX, g.hDirZ, EccAt(g, grown), fx, fz) * grown;
    const float pig = std::pow(std::clamp((0.30f - _p.moisture.m1h) / 0.27f, 0.0f, 1.0f), 1.5f);
    const float rate = RoadEmberRate(IntensityAt(_surface[g.model], v), u10, gapEff, reach, pig);
    if (Rand() >= 1.0f - std::exp(-kRoadFrontM * rate))
        return;
    // A spot fire 1 to 5 m past the far edge, up to 3 m along the road, 5 to 25 s on.
    const float footX = gx - sG * nrx, footZ = gz - sG * nrz;
    const float across = road.roadHalfM + 1.0f + 4.0f * Rand(), along = 3.0f * (2.0f * Rand() - 1.0f);
    _pending.push_back({footX + fx * across + road.roadDX * along, footZ + fz * across + road.roadDZ * along,
                        now + 5.0f + 20.0f * Rand()});
    _stats.roadEmbers++;
}

void FireGrid::Pending()
{
    const float now = static_cast<float>(_stats.simTime);
    std::size_t w = 0;
    for (std::size_t r = 0; r < _pending.size(); ++r)
    {
        const PendingSpot p = _pending[r];
        if (p.t > now)
        {
            _pending[w++] = p;
            continue;
        }
        // A spot fire starts young (its build-up ahead of it), as a point.
        const std::int32_t i = IndexOf(p.x), j = IndexOf(p.z);
        Cell* c = CellAt(i, j, true);
        if (c && c->model && CatchPoint(i, j, *c, p.x, p.z, 0.0f))
            _stats.roadSpots++;
    }
    _pending.resize(w);
}

void FireGrid::Torches(float wind10X, float wind10Z)
{
    const double now = _stats.simTime;
    const float u10 = std::sqrt(wind10X * wind10X + wind10Z * wind10Z);
    // The fine fuel's ignition chance for a landing ember (after Schroeder 1969).
    const float pig = std::pow(std::clamp((0.30f - _p.moisture.m1h) / 0.27f, 0.0f, 1.0f), 1.5f);
    std::size_t w = 0;
    for (std::size_t r = 0; r < _torches.size(); ++r)
    {
        const Key key = _torches[r];
        Cell* c = CellAt(key.i, key.j, false);
        if (!c || !c->torching)
            continue;
        const float age = TorchAgeAt(*c, now);
        const float torchSec = c->olive ? kOliveTorchSec : kTorchSec;
        if (age >= kTorchDelaySec + torchSec)
            continue; // its torch is over
        _torches[w++] = key;
        // Embers only while the crown torches, kSpotEmbersPerTree over its torch.
        if (!_p.spotting || u10 <= kSpotMinWindMps || TorchShape(*c, age) <= 0.0f)
            continue;
        if (Rand() >= kSpotEmbersPerTree / torchSec * _p.stepSec)
            continue;
        _stats.embers++;
        const float cap = AlbiniTorchSpotMaxM(c->treeH, u10);
        const float u1 = Rand(), u2 = Rand();
        const float dist = SpotDistanceM(cap, 1.0f - u1, u2);
        const float side = (Rand() - 0.5f) * 0.4f;
        const float wx = wind10X / u10, wz = wind10Z / u10;
        const float tx = CentreX(key.i) + c->treeOX + (wx - wz * side) * dist;
        const float tz = CentreZ(key.j) + c->treeOZ + (wz + wx * side) * dist;
        if (Rand() >= pig)
            continue;
        const std::int32_t ti = IndexOf(tx), tj = IndexOf(tz);
        Cell* tc = CellAt(ti, tj, true);
        // A spot fire starts young (its build-up ahead of it), round, from where it landed.
        if (tc && tc->model && CatchPoint(ti, tj, *tc, tx, tz, 0.0f))
        {
            _stats.spots++;
            _stats.spotMaxM = std::max(_stats.spotMaxM, dist);
        }
    }
    _torches.resize(w);
}

void FireGrid::FillFront(const Cell& c, std::int32_t i, std::int32_t j, BurningView& v) const
{
    v.x = CentreX(i);
    v.z = CentreZ(j);
    v.y = c.heightM;
    const double now = DisplayTime();
    // The planar front across the cell: its head phase, or an ignition line
    // cell's backing phase once the head's window is over.
    v.backing = c.hasBack && now >= static_cast<double>(c.igniteTime) + c.flameSec;
    if (v.backing)
    {
        v.age = static_cast<float>(now - static_cast<double>(c.backIgnite));
        v.life = std::max(c.backFlameSec - _p.stepSec, 0.5f);
        v.crossSec = std::min(c.backCross, v.life);
        v.frontX = c.backX;
        v.frontZ = c.backZ;
        v.rateMps = c.backRate;
    }
    else
    {
        v.age = static_cast<float>(now - static_cast<double>(c.igniteTime));
        v.life = std::max(c.flameSec - _p.stepSec, 0.5f);
        v.crossSec = std::min(c.crossSec, v.life);
        v.frontX = c.frontX;
        v.frontZ = c.frontZ;
        v.rateMps = c.rateMps;
    }
    v.residenceSec = _surface[c.model].residenceSec;
    // The front across the cell: near a seed (a point, an area, the ignition
    // line) the seed's own field, through the cell's ellipse, matched to the
    // centre's arrival (a young fire is round, a line two-sided); elsewhere
    // the planar front through the centre's arrival square to its normal
    // (its neighbours' planes meet it, so the drawn front runs on).
    // (the dance fix) A point's drawn age never goes back: the ellipse is the
    // cell's head as it caught (FreezeDrawnHead: its head now followed the
    // wandering wind and swung the ellipse round its anchor, so the ground
    // unburnt and the flames jumped back where it withdrew), and the
    // build-up's share is the one when the front passed the centre behind
    // it (gGrown; the share now shrank the ages there as it grew) and the
    // one now ahead of it (gGrownAhead: it only brings the arrival sooner).
    v.planar = true;
    v.ageC = static_cast<float>(now - static_cast<double>(c.centreTime));
    v.ancRadius = c.anchor.radiusM;
    v.ancX = c.anchor.cx - v.x;
    v.ancZ = c.anchor.cz - v.z;
    v.ancUX = c.anchor.ux;
    v.ancUZ = c.anchor.uz;
    v.ancHalf = c.anchor.halfM;
    const float grownNow =
        std::max(BuildUp(c.anchor.age + std::max(0.0f, static_cast<float>(now - static_cast<double>(c.anchor.t))),
                         _p.buildUpPerMin),
                 0.02f);
    const float grown =
        std::max(BuildUp(c.anchor.age + std::max(0.0f, c.centreTime - c.anchor.t), _p.buildUpPerMin), 0.02f);
    v.gRate = std::max(c.drawRate, 1e-3f);
    v.gDirX = c.drawDirX;
    v.gDirZ = c.drawDirZ;
    v.gEcc = EccentricityOf(1.0f + (c.drawLB - 1.0f) * std::clamp(grownNow, 0.0f, 1.0f));
    v.gGrown = grown;
    v.gGrownAhead = std::max(grownNow, grown);
    Anchor a = c.anchor;
    a.cx = v.ancX;
    a.cz = v.ancZ;
    float qx = 0.0f, qz = 0.0f;
    const float gc = AnchorGauge(a, -v.ancX, -v.ancZ, v.gRate, v.gDirX, v.gDirZ, v.gEcc, &qx, &qz);
    v.gRun = gc + v.ageC * (v.ageC > 0.0f ? v.gGrownAhead : grown);
    const float qd = std::sqrt((v.ancX + qx) * (v.ancX + qx) + (v.ancZ + qz) * (v.ancZ + qz));
    v.exact = c.anchor.seed && qd <= kSeedExactM + c.anchor.radiusM;
    // The arrival field round it: its own centre's and its neighbours'
    // (caught or offered: their drawn arrival, DrawnArrivalOf; bare ground:
    // its own, never drawn there; else one cell on at its own speed, a cell
    // not looked at yet too, not as bare ground).
    v.grid = true;
    const float vC = std::max(c.rateMps, 0.004f);
    for (int dj = -1; dj <= 1; ++dj)
        for (int di = -1; di <= 1; ++di)
        {
            const Cell* k = (di || dj) ? CellAtConst(i + di, j + dj) : &c;
            float age = v.ageC;
            double t = 0.0;
            if (k == &c || (k && k->state != CellState::Unknown && k->model == 0))
                age = v.ageC;
            else if (k && DrawnArrivalOf(*k, now, t))
                age = static_cast<float>(now - t);
            else
                age = v.ageC - _p.cellM * std::sqrt(static_cast<float>(di * di + dj * dj)) / vC;
            v.nbAge[(dj + 1) * 3 + di + 1] = age;
        }
    if (c.roadHalfM > 0.0f)
    {
        v.roadX = c.roadX - v.x;
        v.roadZ = c.roadZ - v.z;
        v.roadDX = c.roadDX;
        v.roadDZ = c.roadDZ;
        v.roadHalf = c.roadHalfM;
        v.roadSide = (-v.roadX) * v.roadDZ - (-v.roadZ) * v.roadDX >= 0.0f ? 1.0f : -1.0f;
    }
    v.roadSides = c.roadSides;
    v.bareSides = c.bareSides;
    v.builtSides = c.builtSides;
}

void FireGrid::FillView(const Cell& c, std::int32_t i, std::int32_t j, BurningView& v) const
{
    FillFront(c, i, j, v);
    v.strength = StrengthOf(c);
    const float flame = v.backing ? c.backFlameM : c.flameM;
    v.flameM = flame * (0.3f + 0.7f * v.strength);
    v.flameMaxM = flame;
    v.intensityKwM = v.backing ? c.backIntensityKwM : c.intensityKwM;
    v.shrubH = c.shrubH;
    v.model = c.model;
    v.torching = c.torching;
    v.sheltered = c.sheltered;
    v.treeH = c.treeH;
    v.treeOX = c.treeOX;
    v.treeOZ = c.treeOZ;
    if (c.torching)
    {
        v.torchAge = TorchAgeOf(c);
        v.torchM = TorchFlameM(c, i, j, &v.torchMaxM);
    }
}

void FireGrid::CollectBurning(std::vector<BurningView>& out) const
{
    out.clear();
    out.reserve(_flaming.size());
    for (const Key& k : _flaming)
    {
        const Cell* c = CellAtConst(k.i, k.j);
        if (!c || c->state != CellState::Burning)
            continue;
        BurningView v;
        FillView(*c, k.i, k.j, v);
        out.push_back(v);
    }
}

void FireGrid::CollectBurntInterior(std::vector<std::pair<float, float>>& out) const
{
    out.clear();
    for (const auto& kv : _tiles)
    {
        const std::int32_t ti = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first >> 32));
        const std::int32_t tj = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first & 0xffffffffu));
        for (int b = 0; b < kTile; ++b)
            for (int a = 0; a < kTile; ++a)
            {
                const Cell& c = kv.second->cells[b * kTile + a];
                if (c.state != CellState::Burnt || c.roadHalfM > 0.0f)
                    continue;
                const std::int32_t i = ti * kTile + a, j = tj * kTile + b;
                bool edge = false;
                for (int dj = -1; dj <= 1 && !edge; ++dj)
                    for (int di = -1; di <= 1 && !edge; ++di)
                    {
                        const Cell* o = CellAtConst(i + di, j + dj);
                        edge = !o || (o->state != CellState::Burnt && o->state != CellState::Burning);
                    }
                if (!edge)
                    out.emplace_back(CentreX(i), CentreZ(j));
            }
    }
}

void FireGrid::CollectBurntEdges(std::vector<BurningView>& out) const
{
    out.clear();
    for (const auto& kv : _tiles)
    {
        const std::int32_t ti = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first >> 32));
        const std::int32_t tj = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first & 0xffffffffu));
        for (int b = 0; b < kTile; ++b)
            for (int a = 0; a < kTile; ++a)
            {
                const Cell& c = kv.second->cells[b * kTile + a];
                if (c.state != CellState::Burnt)
                    continue;
                const std::int32_t i = ti * kTile + a, j = tj * kTile + b;
                bool edge = c.roadHalfM > 0.0f;
                for (int dj = -1; dj <= 1 && !edge; ++dj)
                    for (int di = -1; di <= 1 && !edge; ++di)
                    {
                        const Cell* o = CellAtConst(i + di, j + dj);
                        edge = !o || (o->state != CellState::Burnt && o->state != CellState::Burning);
                    }
                if (!edge)
                    continue;
                BurningView v;
                FillFront(c, i, j, v);
                v.model = c.model;
                out.push_back(v);
            }
    }
}

float FireGrid::DrawnAgeAt(float x, float z) const
{
    const std::int32_t i = IndexOf(x), j = IndexOf(z);
    const Cell* c = CellAtConst(i, j);
    BurningView v;
    if (c && (c->state == CellState::Burning || c->state == CellState::Burnt))
    {
        FillFront(*c, i, j, v);
        return FrontAgeAt(v, x - v.x, z - v.z, _p.cellM);
    }
    // Bare ground never burns (a road cell's verges do, from beside it).
    if (c && c->state != CellState::Unknown && c->model == 0 && c->roadHalfM <= 0.0f)
        return -1e9f;
    // Not caught yet: a caught neighbour's front coming into it (a road
    // cell's verge burns from beside it, up to the bare edge).
    float best = -1e9f;
    for (int dj = -1; dj <= 1; ++dj)
        for (int di = -1; di <= 1; ++di)
        {
            if (di == 0 && dj == 0)
                continue;
            const Cell* o = CellAtConst(i + di, j + dj);
            if (!o || (o->state != CellState::Burning && o->state != CellState::Burnt))
                continue;
            FillFront(*o, i + di, j + dj, v);
            best = std::max(best, FrontAgeAt(v, x - v.x, z - v.z, _p.cellM));
        }
    return best;
}

bool FireGrid::ViewAt(float x, float z, BurningView& out, CellState* state) const
{
    const std::int32_t i = IndexOf(x), j = IndexOf(z);
    const Cell* c = CellAtConst(i, j);
    if (!c || c->state == CellState::Unknown)
        return false;
    out = BurningView();
    if (c->state == CellState::Burning)
        FillView(*c, i, j, out);
    else
    {
        out.x = CentreX(i);
        out.z = CentreZ(j);
        out.model = c->model;
        out.treeH = c->treeH;
        out.shrubH = c->shrubH;
    }
    if (state)
        *state = c->state;
    return true;
}

CellState FireGrid::StateAt(float x, float z) const
{
    const Cell* c = CellAtConst(IndexOf(x), IndexOf(z));
    return c ? c->state : CellState::Unknown;
}

float FireGrid::CentreAgeAt(float x, float z) const
{
    const Cell* c = CellAtConst(IndexOf(x), IndexOf(z));
    if (!c || (c->state != CellState::Burning && c->state != CellState::Burnt))
        return -1e9f;
    return static_cast<float>(DisplayTime() - static_cast<double>(c->centreTime));
}

float FireGrid::StrengthAt(float x, float z, float* flameM) const
{
    const Cell* c = CellAtConst(IndexOf(x), IndexOf(z));
    const float s = c ? StrengthOf(*c) : 0.0f;
    if (flameM)
    {
        *flameM = (c && s > 0.0f) ? c->flameM * (0.3f + 0.7f * s) : 0.0f;
        // A torching crown's flame (it reaches the men under it).
        if (c && s > 0.0f && c->torching)
            *flameM = std::max(*flameM, TorchFlameM(*c, IndexOf(x), IndexOf(z)) + kTorchFootShare * c->treeH);
    }
    return s;
}

void FireGrid::CollectBurnt(std::vector<std::pair<float, float>>& out) const
{
    out.clear();
    for (const auto& kv : _tiles)
    {
        const std::int32_t ti = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first >> 32));
        const std::int32_t tj = static_cast<std::int32_t>(static_cast<std::uint32_t>(kv.first & 0xffffffffu));
        for (int b = 0; b < kTile; ++b)
            for (int a = 0; a < kTile; ++a)
                if (kv.second->cells[b * kTile + a].state == CellState::Burnt)
                    out.emplace_back(CentreX(ti * kTile + a), CentreZ(tj * kTile + b));
    }
}

bool FireGrid::Bounds(float& minX, float& minZ, float& maxX, float& maxZ) const
{
    if (!_haveBounds)
        return false;
    minX = _minI * _p.cellM;
    minZ = _minJ * _p.cellM;
    maxX = (_maxI + 1) * _p.cellM;
    maxZ = (_maxJ + 1) * _p.cellM;
    return true;
}

} // namespace Poseidon::firespread
