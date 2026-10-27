#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>

#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace Poseidon::remaster
{

namespace
{
constexpr float kPi = 3.14159265358979323846f;

float Clamp01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

float Smoothstep(float e0, float e1, float x)
{
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

float MaxOf(const Rgb& c)
{
    return std::max(c.r, std::max(c.g, c.b));
}

Mat3 Inverse(const Mat3& a)
{
    const float* m = a.m;
    const float c00 = m[4] * m[8] - m[5] * m[7];
    const float c01 = m[5] * m[6] - m[3] * m[8];
    const float c02 = m[3] * m[7] - m[4] * m[6];
    const float det = m[0] * c00 + m[1] * c01 + m[2] * c02;
    Mat3 r;
    if (std::fabs(det) < 1e-12f)
        return r;
    const float id = 1.0f / det;
    r.m[0] = c00 * id;
    r.m[1] = (m[2] * m[7] - m[1] * m[8]) * id;
    r.m[2] = (m[1] * m[5] - m[2] * m[4]) * id;
    r.m[3] = c01 * id;
    r.m[4] = (m[0] * m[8] - m[2] * m[6]) * id;
    r.m[5] = (m[2] * m[3] - m[0] * m[5]) * id;
    r.m[6] = c02 * id;
    r.m[7] = (m[1] * m[6] - m[0] * m[7]) * id;
    r.m[8] = (m[0] * m[4] - m[1] * m[3]) * id;
    return r;
}

// Linear sRGB (D65) to Bradford cone responses: Bradford times sRGB-to-XYZ.
Mat3 RgbToCone()
{
    const Mat3 rgbToXyz{
        {0.4124564f, 0.3575761f, 0.1804375f, 0.2126729f, 0.7151522f, 0.0721750f, 0.0193339f, 0.1191920f, 0.9503041f}};
    const Mat3 bradford{{0.8951f, 0.2664f, -0.1614f, -0.7502f, 1.7135f, 0.0367f, 0.0389f, -0.0685f, 1.0296f}};
    return Multiply(bradford, rgbToXyz);
}
} // namespace

// ---- Encoding -----------------------------------------------------------------

namespace
{
// The knee's code and the fold's span (1 - (knee / max)^p).
float KneeCode()
{
    return std::pow(kEncodeKnee, 1.0f / 2.2f);
}
float FoldSpan()
{
    return 1.0f - std::pow(kEncodeKnee / kDecodeMax, kEncodePower);
}
} // namespace

float EncodePeakCode(float m)
{
    if (!(m > kEncodeKnee))
        return std::pow(std::max(m, 0.0f), 1.0f / 2.2f);
    const float mc = std::min(m, kDecodeMax);
    const float cK = KneeCode();
    return cK + (1.0f - cK) * (1.0f - std::pow(kEncodeKnee / mc, kEncodePower)) / FoldSpan();
}

float DecodePeakCode(float code)
{
    const float cK = KneeCode();
    if (!(code > cK))
        return std::pow(std::max(code, 0.0f), 2.2f);
    const float u = std::min((code - cK) / (1.0f - cK), 1.0f);
    const float q = std::max(1.0f - u * FoldSpan(), 1e-6f); // (knee / m)^p
    return std::min(kEncodeKnee / std::pow(q, 1.0f / kEncodePower), kDecodeMax);
}

Rgb CameraEncode(const Rgb& linear)
{
    const Rgb c(std::max(linear.r, 0.0f), std::max(linear.g, 0.0f), std::max(linear.b, 0.0f));
    const float m = MaxOf(c);
    // Above the knee the peak's code is the fold's; the other channels keep
    // their linear ratio to it.
    const Rgb folded = m > kEncodeKnee ? c * (std::pow(EncodePeakCode(m), 2.2f) / m) : c;
    return {ToDisplay(folded.r), ToDisplay(folded.g), ToDisplay(folded.b)};
}

Rgb CameraDecode(const Rgb& display)
{
    const Rgb c = ToLinear(display);
    const float m = MaxOf(c);
    if (!(m > kEncodeKnee))
        return c;
    return c * (DecodePeakCode(ToDisplay(m)) / m);
}

// ---- Metering -----------------------------------------------------------------

namespace
{
// The meter's luminance histogram: kHistBins bins of 1/32 EV from
// kMeterFloorLog2 up; each bin keeps its weight and its weighted sum of
// values, so a percentile reads the mean value of the bin it falls in (exact
// when the samples in that bin are alike) and the mean between the cuts is
// the samples' own mean to within a bin's width. One pass, no sort: the
// render thread's cost stays flat.
constexpr int kHistBinsPerEv = 32;
constexpr int kHistBins = 32 * kHistBinsPerEv; // -16 .. +16 EV
struct Histogram
{
    double w[kHistBins] = {};
    double wv[kHistBins] = {};
    double total = 0.0;
    void Add(float v, double weight)
    {
        const int b = std::min(std::max(static_cast<int>((v - kMeterFloorLog2) * kHistBinsPerEv), 0), kHistBins - 1);
        w[b] += weight;
        wv[b] += weight * v;
        total += weight;
    }
    float Percentile(double p) const
    {
        const double target = p * total;
        double acc = 0.0;
        int last = 0;
        for (int b = 0; b < kHistBins; ++b)
        {
            if (w[b] <= 0.0)
                continue;
            last = b;
            acc += w[b];
            if (acc >= target)
                return static_cast<float>(wv[b] / w[b]);
        }
        return w[last] > 0.0 ? static_cast<float>(wv[last] / w[last]) : kMeterFloorLog2;
    }
};
} // namespace

MeterResult Meter(const float* logLum, const float* weight, const float* maxLog, int n)
{
    MeterResult r;
    static thread_local Histogram s, peaks;
    s = Histogram{};
    peaks = Histogram{};
    for (int i = 0; i < n; ++i)
    {
        const float w = weight[i];
        const float l = logLum[i];
        if (!(w > 0.0f) || !std::isfinite(l))
            continue;
        s.Add(std::max(l, kMeterFloorLog2), w);
        const float pk = maxLog && std::isfinite(maxLog[i]) ? maxLog[i] : l;
        peaks.Add(std::max(pk, kMeterFloorLog2), 1.0);
    }
    const double total = s.total;
    if (total <= 0.0)
        return r;
    // The mean over the weight between the two cuts (a bin straddling a cut
    // counts with the part of its weight inside, at its mean value).
    const double lo = kMeterLowCut * total;
    const double hi = (1.0 - kMeterHighCut) * total;
    double acc = 0.0, sum = 0.0, wsum = 0.0;
    for (int b = 0; b < kHistBins; ++b)
    {
        if (s.w[b] <= 0.0)
            continue;
        const double a0 = acc;
        acc += s.w[b];
        const double inside = std::min(acc, hi) - std::max(a0, lo);
        if (inside > 0.0)
        {
            sum += inside * (s.wv[b] / s.w[b]);
            wsum += inside;
        }
    }
    r.valid = wsum > 0.0;
    r.mean = r.valid ? static_cast<float>(sum / wsum) : 0.0f;
    r.low = s.Percentile(kMeterLowCut);
    r.high = s.Percentile(1.0 - kMeterHighCut);
    r.highlight = peaks.Percentile(kMeterHighlight);
    r.weight = static_cast<float>(total);
    return r;
}

float SunRingPeak(const float* maxLog, const float* weight, float sunU, float sunV, float aspect)
{
    float best = kMeterFloorLog2;
    if (!maxLog || !std::isfinite(sunU) || !std::isfinite(sunV))
        return best;
    const float a = std::max(aspect, 0.1f);
    for (int y = 0; y < kMeterHeight; ++y)
    {
        for (int x = 0; x < kMeterWidth; ++x)
        {
            const int i = y * kMeterWidth + x;
            if (weight && !(weight[i] > 0.0f))
                continue;
            // In frame heights.
            const float dx = ((x + 0.5f) / kMeterWidth - sunU) * a;
            const float dy = (y + 0.5f) / kMeterHeight - sunV;
            const float d = std::sqrt(dx * dx + dy * dy);
            if (d < kSunRingInner || d > kSunRingOuter || !std::isfinite(maxLog[i]))
                continue;
            best = std::max(best, maxLog[i]);
        }
    }
    return best;
}

float SkyTopPeak(const float* skyMaxLog, bool sunOn, float sunU, float sunV, float aspect)
{
    if (!skyMaxLog)
        return kMeterFloorLog2;
    static thread_local std::vector<float> v;
    v.clear();
    v.reserve(kMeterWidth * kMeterHeight);
    const float a = std::max(aspect, 0.1f);
    for (int y = 0; y < kMeterHeight; ++y)
    {
        for (int x = 0; x < kMeterWidth; ++x)
        {
            const float s = skyMaxLog[y * kMeterWidth + x];
            if (sunOn)
            {
                const float dx = ((x + 0.5f) / kMeterWidth - sunU) * a;
                const float dy = (y + 0.5f) / kMeterHeight - sunV;
                if (dx * dx + dy * dy < kSunRingInner * kSunRingInner)
                {
                    v.push_back(kMeterFloorLog2);
                    continue;
                }
            }
            v.push_back(std::isfinite(s) ? std::max(s, kMeterFloorLog2) : kMeterFloorLog2);
        }
    }
    const size_t k = std::min(v.size() - 1, static_cast<size_t>(kSkyTopPercentile * static_cast<float>(v.size())));
    std::nth_element(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k), v.end());
    return v[k];
}

float SkyRolloff(float m)
{
    if (!(m > kSkyRolloffKnee))
        return m;
    const float r = kSkyRolloffCap - kSkyRolloffKnee;
    return kSkyRolloffKnee + r * (1.0f - std::exp(-(m - kSkyRolloffKnee) / r));
}

float SkySunGlow(float cosToSun)
{
    const float c = std::min(std::max(cosToSun, -1.0f), 1.0f);
    return 1.0f / (1.0f + 2.0f * (1.0f - c) / (kSkySunGlowRad * kSkySunGlowRad));
}

float LowSunGlowCos(float sunElevation, float elevation, float cosAzimuth)
{
    const float e = std::min(sunElevation + std::max(elevation - sunElevation, 0.0f) * kLowSunGlowFlat, 1.5707963f);
    return std::sin(sunElevation) * std::sin(e) + std::cos(sunElevation) * std::cos(e) * cosAzimuth;
}

float SkyRolloffCapToward(float cosToSun)
{
    return kSkyRolloffCap + (kSkySunCap - kSkyRolloffCap) * SkySunGlow(cosToSun);
}

float SkyRolloffToward(float m, float cosToSun)
{
    if (!(m > kSkyRolloffKnee))
        return m;
    const float r = SkyRolloffCapToward(cosToSun) - kSkyRolloffKnee;
    return kSkyRolloffKnee + r * (1.0f - std::exp(-(m - kSkyRolloffKnee) / r));
}

float SkyMeterLight(float storedPeak, float cosToSun)
{
    if (!(storedPeak > kSkyRolloffKnee))
        return storedPeak;
    const float rs = SkyRolloffCapToward(cosToSun) - kSkyRolloffKnee;
    const float m = kSkyRolloffKnee - rs * std::log(std::max(1.0f - (storedPeak - kSkyRolloffKnee) / rs, 1e-6f));
    return SkyRolloff(m);
}

float SunRingWeight(float sunElevationDeg)
{
    return 1.0f - Smoothstep(kSunRingFullDeg, kSunRingOffDeg, sunElevationDeg);
}

// ---- Exposure -----------------------------------------------------------------

float TargetExposureLog2(const MeterResult& m, float nightEffect, const LightingTuning& t, float dusk, float duskKeep,
                         float timeEv, float veil)
{
    if (!m.valid)
        return 0.0f;
    const float night = Clamp01(nightEffect);
    // The dusk's step hands over to the night's compensation (camNight).
    float comp =
        t.camEv + t.camNight * night + kCameraDuskEv * Clamp01(dusk) * (1.0f - night) + timeEv * (1.0f - night);
    const float fromDay = m.mean - kDaylightMeanLog2;
    // At dusk a dark scene keeps more of its darkness (kDuskDarkKeep).
    const float darkKeep = kDarkKeep + (kDuskDarkKeep - kDarkKeep) * Clamp01(duskKeep) * (1.0f - night);
    comp += fromDay < 0.0f ? fromDay * darkKeep : fromDay * kBrightKeep;
    float e = std::log2(kMiddleGrey) + comp - m.mean;
    // The highlight guard: the bright end of the frame stays below the
    // curve's white (a sunset sky is not blown out to lift the dark ground).
    // On a grey day the bright end is the deck or the mist: a light grey (by
    // day: at night the bright end is a lamp).
    const float grey = Clamp01(veil) * (1.0f - night);
    const float limit = kHighlightLimit + (kGreyHighlightLimit - kHighlightLimit) * grey;
    const float eHigh = std::log2(limit) - m.highlight;
    if (e > eHigh)
        e += (eHigh - e) * kHighlightGuard;
    // The sky's bright end stays pale sky, not a white hole.
    if (m.skyTop > kMeterFloorLog2)
    {
        const float eSky = std::log2(kSkyTopLimit) - m.skyTop;
        if (e > eSky)
            e += (eSky - e) * kSkyTopGuard;
    }
    // The sky by a low sun in frame stays golden: the exposure comes down to
    // it (a sunrise is not a blown midday sky).
    if (m.sunRingWeight > 0.0f && m.sunRing > kMeterFloorLog2)
    {
        const float eRing = std::log2(kSunRingLimit) - m.sunRing;
        if (e > eRing)
            e += (eRing - e) * Clamp01(m.sunRingWeight);
    }
    const float maxE = kExposureMaxLog2 + (kExposureNightMaxLog2 - kExposureMaxLog2) * night;
    return std::min(std::max(e, kExposureMinLog2), maxE);
}

float CameraGolden(float sunElevationDeg)
{
    return Smoothstep(32.0f, 16.0f, sunElevationDeg) * Smoothstep(1.0f, 5.0f, sunElevationDeg);
}

Rgb GoldenWarmGain(float golden)
{
    const float g = Clamp01(golden);
    const Rgb w = kCameraGoldenWarm * (1.0f / Luminance(kCameraGoldenWarm));
    return {1.0f + (w.r - 1.0f) * g, 1.0f + (w.g - 1.0f) * g, 1.0f + (w.b - 1.0f) * g};
}

float GoldenWarmShare(const Rgb& c)
{
    const float peak = MaxOf(c);
    const float warmth = peak > 0.0f ? (c.r - c.b) / peak : 0.0f;
    return Smoothstep(kGoldenShareFrom, kGoldenShareTo, warmth);
}

float CameraTimeOfDayEv(float sunElevationDeg, float veil)
{
    return kCameraGoldenEv * CameraGolden(sunElevationDeg) +
           kCameraHighSunEv * Smoothstep(35.0f, 55.0f, sunElevationDeg) * (1.0f - Clamp01(veil));
}

float DaySkyDeepWeight(float sunElevationDeg, float sinViewElevation, float cosToSun, float veil)
{
    return Smoothstep(8.0f, 25.0f, sunElevationDeg) * Smoothstep(0.03f, 0.4f, sinViewElevation) *
           (1.0f - Clamp01(veil)) * (1.0f - Smoothstep(0.86f, 0.985f, cosToSun));
}

Rgb DaySkyDeepen(const Rgb& c, float weight)
{
    const float l = Luminance(c);
    const float lo = std::min(c.r, std::min(c.g, c.b));
    if (!(weight > 0.0f) || !(l > 1e-8f) || lo >= l)
        return c;
    const float s = std::max(
        1.0f, std::min(1.0f + kCameraDaySkyDeep * Clamp01(weight), (1.0f - kCameraDaySkyFloor) * l / (l - lo)));
    return {l + (c.r - l) * s, l + (c.g - l) * s, l + (c.b - l) * s};
}

float CameraDuskKeep(float sunElevationDeg)
{
    return Smoothstep(5.0f, 0.0f, sunElevationDeg) * Smoothstep(-6.0f, -3.0f, sunElevationDeg);
}

float CameraDusk(float sunElevationDeg)
{
    return 1.0f - Smoothstep(kCameraDuskFullDeg, kCameraDuskStartDeg, sunElevationDeg);
}

float CameraNight(float sunElevationDeg)
{
    return 1.0f - Smoothstep(kCameraNightFullDeg, kCameraNightStartDeg, sunElevationDeg);
}

float AdaptExposureLog2(float current, float target, float dt, float seconds)
{
    if (!(seconds > 0.0f) || !std::isfinite(current))
        return target;
    // Opening up to a darker scene (a larger exposure) is the slower way.
    const float tau = seconds * (target > current ? 1.5f : 1.0f);
    const float k = 1.0f - std::exp(-std::max(dt, 0.0f) / tau);
    return current + (target - current) * k;
}

// ---- Tone curve ---------------------------------------------------------------

ToneCurve MakeToneCurve(float contrast, float shoulder, float white)
{
    ToneCurve c;
    c.a = std::max(contrast, 0.2f);
    c.d = std::min(std::max(shoulder, 0.5f), 1.0f);
    const float midIn = kMiddleGrey;
    const float midOut = kCurveMidOut;
    const float hdrMax = std::max(kMiddleGrey * white, midIn * 1.5f);
    const float ad = c.a * c.d;
    const float mA = std::pow(midIn, c.a), mAd = std::pow(midIn, ad);
    const float hA = std::pow(hdrMax, c.a), hAd = std::pow(hdrMax, ad);
    const float den = (hAd - mAd) * midOut;
    c.b = (-mA + hA * midOut) / den;
    c.c = (hAd * mA - hA * mAd * midOut) / den;
    c.white = hdrMax;
    return c;
}

float ApplyToneCurve(const ToneCurve& c, float x)
{
    if (!(x > 0.0f))
        return 0.0f;
    const float y = std::pow(x, c.a) / (std::pow(x, c.a * c.d) * c.b + c.c);
    return Clamp01(y);
}

Rgb ApplyToneCurveHueKeeping(const ToneCurve& curve, const Rgb& in)
{
    const Rgb c(std::max(in.r, 0.0f), std::max(in.g, 0.0f), std::max(in.b, 0.0f));
    const float peak = MaxOf(c);
    if (!(peak > 0.0f))
        return {0.0f, 0.0f, 0.0f};
    const float y = ApplyToneCurve(curve, peak);
    // To white late: near the top of the curve, and with the light past the
    // curve's white (kWhiteExcessStops stops past it is white).
    const float excess = Clamp01(std::log2(std::max(peak / curve.white, 1.0f)) / kWhiteExcessStops);
    const Rgb ratio = c * (1.0f / peak);
    // Saturated light whitens less (kChromaKeep, as in the composite), and
    // each channel by its share of the peak (kHueRoll).
    const float chroma = 1.0f - std::min(ratio.r, std::min(ratio.g, ratio.b));
    const float toWhite =
        std::max(std::max(std::pow(y, kCrosstalk), excess) * (1.0f - kChromaKeep * chroma * chroma), 1e-6f);
    auto roll = [&](float share)
    { return share + (1.0f - share) * std::pow(toWhite, 1.0f + kHueRoll * (1.0f - share)); };
    return Rgb(roll(ratio.r), roll(ratio.g), roll(ratio.b)) * y;
}

// ---- Colour -------------------------------------------------------------------

Rgb Mat3::Apply(const Rgb& c) const
{
    return {m[0] * c.r + m[1] * c.g + m[2] * c.b, m[3] * c.r + m[4] * c.g + m[5] * c.b,
            m[6] * c.r + m[7] * c.g + m[8] * c.b};
}

Mat3 Multiply(const Mat3& a, const Mat3& b)
{
    Mat3 r;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r.m[i * 3 + j] = a.m[i * 3] * b.m[j] + a.m[i * 3 + 1] * b.m[3 + j] + a.m[i * 3 + 2] * b.m[6 + j];
    return r;
}

Mat3 WhiteBalance(const Rgb& illuminant, float strength)
{
    const float lum = Luminance(illuminant);
    const float s = Clamp01(strength);
    if (!(lum > 1e-9f) || s <= 0.0f)
        return Mat3{};
    const Rgb ill = illuminant * (1.0f / lum);
    const Mat3 toCone = RgbToCone();
    const Rgb coneIll = toCone.Apply(ill);
    const Rgb coneWhite = toCone.Apply(Rgb(1.0f, 1.0f, 1.0f));
    auto gain = [s](float w, float i) { return i > 1e-6f ? std::pow(w / i, s) : 1.0f; };
    Mat3 d;
    d.m[0] = gain(coneWhite.r, coneIll.r);
    d.m[4] = gain(coneWhite.g, coneIll.g);
    d.m[8] = gain(coneWhite.b, coneIll.b);
    Mat3 wb = Multiply(Inverse(toCone), Multiply(d, toCone));
    // Keep the luminance of a white surface under this light: the balance
    // moves its colour, the exposure owns its brightness.
    const float l = Luminance(wb.Apply(ill));
    if (l > 1e-6f)
        for (float& v : wb.m)
            v /= l;
    return wb;
}

Mat3 Saturation(float s)
{
    const float lr = 0.2126f, lg = 0.7152f, lb = 0.0722f;
    const float k = 1.0f - s;
    return Mat3{{lr * k + s, lg * k, lb * k, lr * k, lg * k + s, lb * k, lr * k, lg * k, lb * k + s}};
}

float WhiteBalanceShare(float sunElevationDeg)
{
    return kWhiteBalanceShareLowSun +
           (kWhiteBalanceShareHighSun - kWhiteBalanceShareLowSun) * Smoothstep(8.0f, 40.0f, sunElevationDeg);
}

Rgb WhiteBalanceTarget(const Rgb& reference, const Rgb& current, float share)
{
    const float lr = Luminance(reference), lc = Luminance(current);
    if (!(lr > 1e-9f))
        return current;
    if (!(lc > 1e-9f))
        return reference;
    const Rgb a = reference * (1.0f / lr), b = current * (1.0f / lc);
    const float s = Clamp01(share);
    auto mixLog = [s](float x, float y)
    { return std::pow(std::max(x, 1e-6f), 1.0f - s) * std::pow(std::max(y, 1e-6f), s); };
    return {mixLog(a.r, b.r), mixLog(a.g, b.g), mixLog(a.b, b.b)};
}

// ---- The physical sun and sky ---------------------------------------------------

PhysicalSky ComputePhysicalSky(const AtmosphereTables& t, float sunElevationRad)
{
    PhysicalSky p;
    if (!t.Valid())
        return p;
    // Rows above the horizon (the parametrisation is dense near it) and
    // columns from the sun's azimuth to the opposite one; the sky is mirror
    // symmetric about the sun's vertical plane.
    static const int kRows[] = {32, 33, 34, 35, 37, 39, 42, 45, 49, 53, 58, 63};
    static const int kCols[] = {0, 2, 5, 9, 14, 20, 27, 35, 44, 53, 63};
    constexpr int nr = static_cast<int>(sizeof(kRows) / sizeof(kRows[0]));
    constexpr int nc = static_cast<int>(sizeof(kCols) / sizeof(kCols[0]));
    float elev[nr], az[nc];
    for (int i = 0; i < nr; ++i)
        elev[i] = std::max(SkyViewRowElevation(kRows[i]), 0.0f);
    for (int j = 0; j < nc; ++j)
        az[j] = SkyViewColumnAzimuth(kCols[j]);
    const float sinS = std::sin(sunElevationRad), cosS = std::cos(sunElevationRad);
    Rgb up, toward, away;
    double horizonSum = 0.0, horizonW = 0.0;
    for (int i = 0; i < nr; ++i)
    {
        const float eLo = i == 0 ? 0.0f : 0.5f * (elev[i - 1] + elev[i]);
        const float eHi = i == nr - 1 ? 0.5f * kPi : 0.5f * (elev[i] + elev[i + 1]);
        const float sLo = std::sin(eLo), sHi = std::sin(eHi);
        // Integrals over the band: sin e cos e (up) and cos^2 e (walls).
        const float wUp = 0.5f * (sHi * sHi - sLo * sLo);
        auto cos2 = [](float e) { return 0.5f * e + 0.25f * std::sin(2.0f * e); };
        const float wWall = cos2(eHi) - cos2(eLo);
        for (int j = 0; j < nc; ++j)
        {
            const float pLo = j == 0 ? 0.0f : 0.5f * (az[j - 1] + az[j]);
            const float pHi = j == nc - 1 ? kPi : 0.5f * (az[j] + az[j + 1]);
            const float cosToSun = std::sin(elev[i]) * sinS + std::cos(elev[i]) * cosS * std::cos(az[j]);
            const Rgb l = SkyViewTexel(t, 0, kCols[j], kRows[i], sunElevationRad) +
                          SkyViewTexel(t, 1, kCols[j], kRows[i], sunElevationRad) * AtmosphereMiePhase(cosToSun);
            // Both halves of the sky (x2); the wall facing the sun sees
            // cos(phi) > 0, the one facing away cos(phi) < 0.
            const float dPhi = pHi - pLo;
            const float half = 0.5f * kPi;
            const float sinLoC = std::sin(std::min(pLo, half)), sinHiC = std::sin(std::min(pHi, half));
            const float posCos = std::max(sinHiC - sinLoC, 0.0f);
            const float negCos = std::max(std::sin(std::max(pLo, half)) - std::sin(std::max(pHi, half)), 0.0f);
            up = up + l * (2.0f * wUp * dPhi);
            toward = toward + l * (2.0f * wWall * posCos);
            away = away + l * (2.0f * wWall * negCos);
            if (kRows[i] == 37)
            {
                horizonSum += Luminance(l) * dPhi;
                horizonW += dPhi;
            }
        }
    }
    const float r0 = atmo::kGroundRadius + std::max(kSkyViewerHeight, 1.0f);
    p.sun = AtmosphereTransmittance(t, r0, sinS);
    p.skyUp = up;
    p.skySunward = toward;
    p.skyAntiSun = away;
    p.horizonRadiance = horizonW > 0.0 ? static_cast<float>(horizonSum / horizonW) : 0.0f;
    p.sunElevationDeg = sunElevationRad * 180.0f / kPi;
    p.valid = true;
    return p;
}

Rgb PhysicalIlluminant(const PhysicalSky& p)
{
    const float s = std::max(std::sin(p.sunElevationDeg * kPi / 180.0f), 0.0f);
    return p.sun * s + PhysicalSkyFill(p.skyUp, p.sun);
}

} // namespace Poseidon::remaster
