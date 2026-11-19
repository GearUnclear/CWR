#include <Poseidon/World/Simulation/FireSparks.hpp>

namespace Poseidon::firespread
{

void SparkPool::Emit(const SparkBirth& b, int n)
{
    if (n <= 0)
        return;
    const SparkKindShape k = SparkKindShapeOf(b.kind);
    for (int i = 0; i < n; ++i)
    {
        if (static_cast<int>(_sparks.size()) >= kMaxSparks)
        {
            _dropped += n - i;
            return;
        }
        Spark s{};
        const bool longOne = k.longShare > 0.0f && Next01() < k.longShare;
        s.x = b.x + (Next01() - 0.5f) * b.spreadXZ;
        s.z = b.z + (Next01() - 0.5f) * b.spreadXZ;
        s.y = b.y + Next01() * b.spreadY;
        // the gas round it, give or take its eddies
        s.vx = b.vx + (Next01() - 0.5f) * (kSparkTurbMin + kSparkTurb * std::fabs(b.vx));
        s.vz = b.vz + (Next01() - 0.5f) * (kSparkTurbMin + kSparkTurb * std::fabs(b.vz));
        s.vy = b.vy * (0.7f + 0.6f * Next01());
        s.life = longOne ? k.longMin + (k.longMax - k.longMin) * Next01()
                         : (k.lifeMin + (k.lifeMax - k.lifeMin) * Next01()) * b.lifeScale;
        s.life = std::max(s.life, 0.1f);
        s.age = 0.0f;
        s.tipY = b.tipY;
        s.rise = b.riseMps * (0.8f + 0.4f * Next01());
        s.decay = SparkRiseDecaySec((b.riseMps / kSparkRiseK) * (b.riseMps / kSparkRiseK));
        s.sink = longOne ? k.longSink * (0.7f + 0.6f * Next01()) : k.sinkMin + (k.sinkMax - k.sinkMin) * Next01();
        s.size = (k.sizeMin + (k.sizeMax - k.sizeMin) * Next01()) * (longOne ? k.longSize : 1.0f);
        s.k0 = k.kMin + (k.kMax - k.kMin) * Next01() + (longOne ? 50.0f : 0.0f);
        s.kEnd = k.kEnd;
        s.weight = std::max(b.weight, 0.0f);
        s.gy = b.groundY;
        s.twinkleHz = 3.0f + 5.0f * Next01();
        s.twinklePh = 6.2831853f * Next01();
        s.kind = static_cast<std::uint8_t>(b.kind);
        s.out = s.y > s.tipY;
        _sparks.push_back(s);
        ++_made;
    }
}

void SparkPool::EmitExpected(const SparkBirth& b, float expected)
{
    if (!(expected > 0.0f))
        return;
    const float whole = std::floor(expected);
    Emit(b, static_cast<int>(whole) + (Next01() < expected - whole ? 1 : 0));
}

void SparkPool::Step(float dt, float wind10X, float wind10Z, const GroundFn& ground)
{
    ++_frame;
    if (!(dt > 0.0f))
        return;
    _time += static_cast<double>(dt);
    const float aw = std::min(1.0f, dt / kSparkWindTakeSec);
    const float nw = std::sqrt(2.0f * aw);
    for (std::size_t i = 0; i < _sparks.size();)
    {
        Spark& s = _sparks[i];
        s.age += dt;
        if (s.age >= s.life)
        {
            s = _sparks.back();
            _sparks.pop_back();
            continue;
        }
        // Sideways: the wind at its height, and the eddies (an
        // Ornstein-Uhlenbeck walk of the wind's taking time).
        const float share = SparkWindShare(s.y - s.gy);
        const float ux = wind10X * share, uz = wind10Z * share;
        const float sigH = kSparkTurb * std::sqrt(ux * ux + uz * uz) + kSparkTurbMin;
        s.vx += (ux - s.vx) * aw + sigH * nw * Gauss();
        s.vz += (uz - s.vz) * aw + sigH * nw * Gauss();
        // Up: the gas's rise inside the flame, then over its tip the rise
        // dying away to the sink.
        if (!s.out && s.y > s.tipY)
            s.out = true;
        const float target = s.out ? -s.sink : s.rise;
        const float av = std::min(1.0f, dt / (s.out ? s.decay : kSparkRiseTakeSec));
        const float sigV = s.out ? kSparkTurbMin : kSparkTurbFlame * s.rise + kSparkTurbMin;
        s.vy += (target - s.vy) * av + sigV * std::sqrt(2.0f * av) * Gauss();
        s.x += s.vx * dt;
        s.y += s.vy * dt;
        s.z += s.vz * dt;
        // The ground under one sinking near it (every fourth frame, staggered):
        // one that lands goes out.
        if (s.vy < 0.0f && s.y - s.gy < 3.0f && ((i + _frame) & 3U) == 0 && ground)
            s.gy = ground(s.x, s.z);
        if (s.y <= s.gy)
        {
            ++_landed;
            s = _sparks.back();
            _sparks.pop_back();
            continue;
        }
        ++i;
    }
}

float SparkPool::LightOverLife(float a, float life)
{
    if (a <= 0.0f || a >= life)
        return 0.0f;
    const float in = std::min(1.0f, a / kSparkFadeInSec);
    const float outK = std::clamp((life - a) / std::max(kSparkFadeOutShare * life, 1e-3f), 0.0f, 1.0f);
    return in * outK * outK * (3.0f - 2.0f * outK);
}

float SparkPool::KelvinOverLife(float a, float life, float kStart, float kEnd)
{
    const float x = std::clamp(a / std::max(life, 1e-3f), 0.0f, 1.0f);
    return kStart + (kEnd - kStart) * std::pow(x, kSparkCoolPow);
}

const std::vector<SparkPoint>& SparkPool::BuildPoints()
{
    _points.resize(_sparks.size());
    const float t = static_cast<float>(std::fmod(_time, 3600.0));
    for (std::size_t i = 0; i < _sparks.size(); ++i)
    {
        const Spark& s = _sparks[i];
        SparkPoint& p = _points[i];
        const float fade = LightOverLife(s.age, s.life);
        // a tumbling fragment twinkles
        const float twinkle = 1.0f + 0.3f * std::sin(6.2831853f * s.twinkleHz * t + s.twinklePh);
        p.x = s.x;
        p.y = s.y;
        p.z = s.z;
        p.vx = s.vx;
        p.vy = s.vy;
        p.vz = s.vz;
        p.size = s.size * (0.6f + 0.4f * fade); // it burns down as it dies
        p.kelvin = KelvinOverLife(s.age, s.life, s.k0, s.kEnd);
        p.light = s.weight * fade * twinkle;
        p.area = std::max(1.0f, s.weight);
    }
    return _points;
}

SparkCounts SparkPool::Counts() const
{
    SparkCounts c;
    c.alive = static_cast<int>(_sparks.size());
    for (const Spark& s : _sparks)
        c.byKind[std::min<int>(s.kind, kSparkKinds - 1)]++;
    c.made = _made;
    c.dropped = _dropped;
    c.landed = _landed;
    return c;
}

int SparkPool::CountNear(SparkKind kind, float x, float z, float r) const
{
    int n = 0;
    const auto k = static_cast<std::uint8_t>(kind);
    for (const Spark& s : _sparks)
        if (s.kind == k && (s.x - x) * (s.x - x) + (s.z - z) * (s.z - z) <= r * r)
            ++n;
    return n;
}

} // namespace Poseidon::firespread
