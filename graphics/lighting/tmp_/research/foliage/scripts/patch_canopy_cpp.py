"""Apply the canopy-map research prototype to EngineGL33_Lighting.cpp (gfx-foliage scratch worktree)."""
from pathlib import Path

P = Path(r"D:\Arma_CWA_worktrees\gfx-foliage\arma_CWR\engine\PoseidonGL33\EngineGL33_Lighting.cpp")


def patch(a, b):
    s = P.read_text(encoding="utf-8")
    crlf = "\r\n" in s
    if crlf:
        a = a.replace("\n", "\r\n")
        b = b.replace("\n", "\r\n")
    assert s.count(a) == 1, (a[:60], s.count(a))
    s = s.replace(a, b)
    P.write_bytes(s.encode("utf-8"))


patch("""#include <Poseidon/World/Terrain/Landscape.hpp>

#include <cmath>
#include <cstring>
""", """#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <vector>
""")
patch("""    delTex(_cloudNoiseTex);
    delTex(_dummyDepthArray);""", """    delTex(_cloudNoiseTex);
    delTex(_canopyTex);
    delTex(_dummyCanopyTex);
    delTex(_dummyDepthArray);""")
patch("""    _dummyHeightTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT, heightSky);
""", """    _dummyHeightTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT, heightSky);
    const float noCanopy[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    _dummyCanopyTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT, noCanopy);
""")
patch("""    GL33Bind::Tex2D(kCloudNoiseUnit, _cloudNoiseTex);
    GL33Bind::ActiveUnit(0);
}""", """    GL33Bind::Tex2D(kCloudNoiseUnit, _cloudNoiseTex);
    GL33Bind::Tex2D(kCanopyUnit, (map && _canopyTex) ? _canopyTex : _dummyCanopyTex);
    GL33Bind::ActiveUnit(0);
}""")
patch("""    GL33Bind::Invalidate();
    LOG_INFO(Graphics, "GL33: terrain horizon map {}x{} at {} m uploaded", n, n, m.cellSize);
}""", """    GL33Bind::Invalidate();
    LOG_INFO(Graphics, "GL33: terrain horizon map {}x{} at {} m uploaded", n, n, m.cellSize);
    BuildCanopyMap();
}

// RESEARCH PROTOTYPE (foliage track): a top-down canopy map of the island,
// rasterised from the landscape's tree, bush and forest-block objects when the
// horizon map is uploaded. Texel-centred cells over the whole island square
// (uv = world xz / island size); r = canopy cover 0..1, g = canopy top and
// b = crown base, both in metres above the terrain; mipmapped so the shaders
// can read the cover of wider neighbourhoods.
void EngineGL33::BuildCanopyMap()
{
    if (_canopyTex)
    {
        GL33Bind::OnTexDeleted(_canopyTex);
        glDeleteTextures(1, &_canopyTex);
        _canopyTex = 0;
    }
    Poseidon::Landscape* land = Poseidon::GLandscape;
    if (!land)
        return;
    const auto t0 = std::chrono::steady_clock::now();
    const int lr = land->GetLandRange();
    const float world = static_cast<float>(lr) * land->GetLandGrid();
    if (lr < 2 || !(world > 0.0f))
        return;
    const int n = std::min(2048, static_cast<int>(std::ceil(world / 6.0f)));
    const float cs = world / static_cast<float>(n);
    std::vector<float> cover(static_cast<size_t>(n) * n, 0.0f);
    std::vector<float> top(static_cast<size_t>(n) * n, 0.0f);
    std::vector<float> base(static_cast<size_t>(n) * n, 1e9f);
    int trees = 0;
    int forests = 0;
    auto splat = [&](int xi, int zi, float frac, float yTop, float yBase)
    {
        if (frac <= 0.0f || xi < 0 || zi < 0 || xi >= n || zi >= n)
            return;
        const size_t i = static_cast<size_t>(zi) * n + xi;
        const float g = land->SurfaceY((xi + 0.5f) * cs, (zi + 0.5f) * cs);
        cover[i] = 1.0f - (1.0f - cover[i]) * (1.0f - std::min(frac, 1.0f));
        top[i] = std::max(top[i], yTop - g);
        base[i] = std::min(base[i], std::max(yBase - g, 0.0f));
    };
    for (int z = 0; z < lr; ++z)
    {
        for (int x = 0; x < lr; ++x)
        {
            const Poseidon::ObjectList& ol = land->GetObjects(z, x);
            for (int k = 0; k < ol.Size(); ++k)
            {
                Poseidon::Object* o = ol[k];
                if (!o)
                    continue;
                Poseidon::LODShapeWithShadow* sh = o->GetShape();
                if (!sh)
                    continue;
                const Poseidon::MapType t = sh->GetMapType();
                const bool tree = t == Poseidon::MapTree || t == Poseidon::MapSmallTree || t == Poseidon::MapBush;
                const bool forest = t == Poseidon::MapForestBorder || t == Poseidon::MapForestTriangle ||
                                    t == Poseidon::MapForestSquare;
                if (!tree && !forest)
                    continue;
                const Poseidon::Vector3 mn = sh->Min();
                const Poseidon::Vector3 mx = sh->Max();
                float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f, z0 = 1e9f, z1 = -1e9f;
                for (int c = 0; c < 8; ++c)
                {
                    const Poseidon::Vector3 m((c & 1) ? mx.X() : mn.X(), (c & 2) ? mx.Y() : mn.Y(),
                                              (c & 4) ? mx.Z() : mn.Z());
                    const Poseidon::Vector3 w = o->PositionModelToWorld(m);
                    x0 = std::min(x0, w.X());
                    x1 = std::max(x1, w.X());
                    y0 = std::min(y0, w.Y());
                    y1 = std::max(y1, w.Y());
                    z0 = std::min(z0, w.Z());
                    z1 = std::max(z1, w.Z());
                }
                if (tree)
                {
                    const float cx = 0.5f * (x0 + x1);
                    const float cz = 0.5f * (z0 + z1);
                    const float r = 0.5f * std::min(x1 - x0, z1 - z0);
                    if (r < 0.4f)
                        continue;
                    ++trees;
                    const float yBase = t == Poseidon::MapBush ? y0 : y0 + 0.35f * (y1 - y0);
                    const int xa = static_cast<int>(std::floor((cx - r) / cs));
                    const int xb = static_cast<int>(std::floor((cx + r) / cs));
                    const int za = static_cast<int>(std::floor((cz - r) / cs));
                    const int zb = static_cast<int>(std::floor((cz + r) / cs));
                    for (int zi = za; zi <= zb; ++zi)
                        for (int xi = xa; xi <= xb; ++xi)
                        {
                            float frac = 0.0f;
                            for (int s = 0; s < 16; ++s)
                            {
                                const float sx = (xi + ((s & 3) + 0.5f) * 0.25f) * cs - cx;
                                const float sz = (zi + ((s >> 2) + 0.5f) * 0.25f) * cs - cz;
                                const float d = std::sqrt(sx * sx + sz * sz);
                                const float edge = std::min(std::max((d - 0.75f * r) / (0.25f * r), 0.0f), 1.0f);
                                frac += (1.0f - edge * edge * (3.0f - 2.0f * edge)) / 16.0f;
                            }
                            splat(xi, zi, frac, y1, yBase);
                        }
                }
                else
                {
                    ++forests;
                    const Poseidon::Matrix4 inv = o->CalcInvTransform();
                    const float ym = 0.5f * (y0 + y1);
                    const int xa = static_cast<int>(std::floor(x0 / cs));
                    const int xb = static_cast<int>(std::floor(x1 / cs));
                    const int za = static_cast<int>(std::floor(z0 / cs));
                    const int zb = static_cast<int>(std::floor(z1 / cs));
                    for (int zi = za; zi <= zb; ++zi)
                        for (int xi = xa; xi <= xb; ++xi)
                        {
                            float frac = 0.0f;
                            for (int s = 0; s < 16; ++s)
                            {
                                const Poseidon::Vector3 wp((xi + ((s & 3) + 0.5f) * 0.25f) * cs, ym,
                                                           (zi + ((s >> 2) + 0.5f) * 0.25f) * cs);
                                const Poseidon::Vector3 mp(Poseidon::VFastTransform, inv, wp);
                                if (mp.X() >= mn.X() && mp.X() <= mx.X() && mp.Z() >= mn.Z() && mp.Z() <= mx.Z())
                                    frac += 1.0f / 16.0f;
                            }
                            splat(xi, zi, frac, y1, y0 + 0.3f * (y1 - y0));
                        }
                }
            }
        }
    }
    std::vector<float> texels(static_cast<size_t>(n) * n * 4);
    for (size_t i = 0; i < static_cast<size_t>(n) * n; ++i)
    {
        const bool any = cover[i] > 0.0f;
        texels[i * 4 + 0] = cover[i];
        texels[i * 4 + 1] = any ? top[i] : 0.0f;
        texels[i * 4 + 2] = any ? std::min(base[i], top[i]) : 0.0f;
        texels[i * 4 + 3] = 1.0f;
    }
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glGenTextures(1, &_canopyTex);
    glBindTexture(GL_TEXTURE_2D, _canopyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, n, n, 0, GL_RGBA, GL_FLOAT, texels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    const long long ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    LOG_INFO(Graphics, "GL33: canopy map {}x{} at {} m: {} trees/bushes, {} forest blocks, {} ms (research)", n, n, cs,
             trees, forests, ms);
}""")
print("ok")
