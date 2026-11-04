#include <Poseidon/Graphics/Remaster/TemporalAA.hpp>

#include <cmath>

namespace Poseidon::remaster
{

float Halton(std::uint32_t index, std::uint32_t base)
{
    if (base < 2)
        return 0.0f;
    double f = 1.0;
    double r = 0.0;
    while (index > 0)
    {
        f /= static_cast<double>(base);
        r += f * static_cast<double>(index % base);
        index /= base;
    }
    return static_cast<float>(r);
}

TaaOffset TaaJitterPixels(std::uint32_t frame)
{
    const std::uint32_t k = frame % kTaaJitterCycle + 1u;
    return {Halton(k, 2) - 0.5f, Halton(k, 3) - 0.5f};
}

TaaOffset TaaJitterNdc(TaaOffset pixels, int viewportW, int viewportH)
{
    if (viewportW <= 0 || viewportH <= 0)
        return {};
    return {2.0f * pixels.x / static_cast<float>(viewportW), 2.0f * pixels.y / static_cast<float>(viewportH)};
}

TaaOffset TaaPassJitterNdc(bool worldPass, bool taaThisFrame, TaaOffset framePixels, int viewportW, int viewportH)
{
    if (!worldPass || !taaThisFrame)
        return {};
    return TaaJitterNdc(framePixels, viewportW, viewportH);
}

namespace
{

// 3x3 inverse of a row-major matrix; false when singular.
bool Invert3(const double m[9], double out[9])
{
    const double det =
        m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[8] - m[5] * m[6]) + m[2] * (m[3] * m[7] - m[4] * m[6]);
    if (!(std::fabs(det) > 1e-12))
        return false;
    const double id = 1.0 / det;
    out[0] = (m[4] * m[8] - m[5] * m[7]) * id;
    out[1] = -(m[1] * m[8] - m[2] * m[7]) * id;
    out[2] = (m[1] * m[5] - m[2] * m[4]) * id;
    out[3] = -(m[3] * m[8] - m[5] * m[6]) * id;
    out[4] = (m[0] * m[8] - m[2] * m[6]) * id;
    out[5] = -(m[0] * m[5] - m[2] * m[3]) * id;
    out[6] = (m[3] * m[7] - m[4] * m[6]) * id;
    out[7] = -(m[0] * m[7] - m[1] * m[6]) * id;
    out[8] = (m[0] * m[4] - m[1] * m[3]) * id;
    return true;
}

// The view direction in world space: the third column of R (p_view.z is
// p_rel dotted with it), normalised.
void Forward(const double r[9], double f[3])
{
    f[0] = r[2];
    f[1] = r[5];
    f[2] = r[8];
    const double len = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    for (int i = 0; i < 3; ++i)
        f[i] = len > 0.0 ? f[i] / len : 0.0;
}

} // namespace

TaaReprojection BuildTaaReprojection(const TaaCamera& prev, const TaaCamera& cur)
{
    TaaReprojection out;
    out.prevP11 = static_cast<float>(prev.p11);
    out.prevP22 = static_cast<float>(prev.p22);
    double inv[9];
    if (!Invert3(cur.rot, inv))
        return out;
    const double* rp = prev.rot;
    double m[9];
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            m[r * 3 + c] =
                inv[r * 3 + 0] * rp[0 * 3 + c] + inv[r * 3 + 1] * rp[1 * 3 + c] + inv[r * 3 + 2] * rp[2 * 3 + c];
    const double d[3] = {cur.pos[0] - prev.pos[0], cur.pos[1] - prev.pos[1], cur.pos[2] - prev.pos[2]};
    double t[3];
    for (int c = 0; c < 3; ++c)
        t[c] = d[0] * rp[0 * 3 + c] + d[1] * rp[1 * 3 + c] + d[2] * rp[2 * 3 + c];
    // Column-vector rows: p_prev[c] = sum_r p[r] * m[r][c] + t[c].
    for (int c = 0; c < 3; ++c)
    {
        out.rows[c][0] = static_cast<float>(m[0 * 3 + c]);
        out.rows[c][1] = static_cast<float>(m[1 * 3 + c]);
        out.rows[c][2] = static_cast<float>(m[2 * 3 + c]);
        out.rows[c][3] = static_cast<float>(t[c]);
    }
    out.move = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    double fc[3], fp[3];
    Forward(cur.rot, fc);
    Forward(prev.rot, fp);
    out.forwardCos = fc[0] * fp[0] + fc[1] * fp[1] + fc[2] * fp[2];
    out.valid = out.move <= kTaaCutDistance && out.forwardCos >= kTaaCutMinCos;
    return out;
}

bool TaaReprojectNdc(const TaaReprojection& r, const double p[3], double ndc[2])
{
    double q[3];
    for (int c = 0; c < 3; ++c)
        q[c] = r.rows[c][0] * p[0] + r.rows[c][1] * p[1] + r.rows[c][2] * p[2] + r.rows[c][3];
    if (!(q[2] > 0.0))
        return false;
    ndc[0] = q[0] * r.prevP11 / q[2];
    ndc[1] = q[1] * r.prevP22 / q[2];
    return true;
}

bool TaaHistoryUsable(const TaaHistory& history, std::uint32_t frame, int w, int h, bool reprojectionValid)
{
    return history.exists && history.frame + 1u == frame && history.w == w && history.h == h && reprojectionValid;
}

} // namespace Poseidon::remaster
