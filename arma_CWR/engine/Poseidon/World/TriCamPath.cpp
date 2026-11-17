#include <Poseidon/World/TriCamPath.hpp>

#include <cmath>

namespace Poseidon::tricam
{

namespace
{

struct PathState
{
    std::vector<CamKey> keys;
    bool active = false;
    float time = 0.0f;
};

PathState& State()
{
    static PathState state;
    return state;
}

float Lerp(float a, float b, float u)
{
    return a + (b - a) * u;
}

} // namespace

bool ValidatePath(const std::vector<CamKey>& keys, std::string* why)
{
    if (keys.empty())
    {
        if (why)
            *why = "need at least one key";
        return false;
    }
    for (size_t i = 1; i < keys.size(); ++i)
    {
        if (!(keys[i].t > keys[i - 1].t))
        {
            if (why)
                *why = "key times must increase";
            return false;
        }
    }
    return true;
}

CamPose EvaluatePath(const std::vector<CamKey>& keys, float t)
{
    CamPose pose;
    if (keys.empty())
        return pose;
    size_t i = 0;
    float u = 0.0f;
    if (t <= keys.front().t || keys.size() == 1)
        i = 0;
    else if (t >= keys.back().t)
    {
        i = keys.size() - 2;
        u = 1.0f;
    }
    else
    {
        while (i + 1 < keys.size() && t >= keys[i + 1].t)
            ++i;
        u = (t - keys[i].t) / (keys[i + 1].t - keys[i].t);
    }
    const CamKey& a = keys[i];
    const CamKey& b = keys.size() > 1 ? keys[i + 1] : keys[i];
    if (b.ease)
        u = u * u * (3.0f - 2.0f * u);
    for (int k = 0; k < 3; ++k)
        pose.pos[k] = Lerp(a.pos[k], b.pos[k], u);
    const float d2r = 3.14159265358979f / 180.0f;
    const float h = Lerp(a.headingDeg, b.headingDeg, u) * d2r;
    const float p = Lerp(a.pitchDeg, b.pitchDeg, u) * d2r;
    pose.dir[0] = std::sin(h) * std::cos(p);
    pose.dir[1] = std::sin(p);
    pose.dir[2] = std::cos(h) * std::cos(p);
    return pose;
}

float PathDuration(const std::vector<CamKey>& keys)
{
    return keys.empty() ? 0.0f : keys.back().t - keys.front().t;
}

bool SetCamPath(const std::vector<CamKey>& keys, std::string* why)
{
    if (!ValidatePath(keys, why))
        return false;
    PathState& s = State();
    s.keys = keys;
    s.time = 0.0f;
    s.active = true;
    return true;
}

void StopCamPath()
{
    State().active = false;
}

bool CamPathActive()
{
    return State().active;
}

float CamPathTime()
{
    return State().time;
}

bool StepCamPath(float dt, CamPose& out)
{
    PathState& s = State();
    if (!s.active)
        return false;
    s.time += dt;
    out = EvaluatePath(s.keys, s.time);
    return true;
}

} // namespace Poseidon::tricam
