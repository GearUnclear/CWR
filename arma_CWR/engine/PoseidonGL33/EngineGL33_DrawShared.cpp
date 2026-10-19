#include <PoseidonGL33/EngineGL33.hpp>
#include <Poseidon/Graphics/Remaster/NightLights.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Rendering/RenderFlags.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Graphics/Rendering/Primitives/Poly.hpp>

#include <cstdint>

inline int FracAlpha(float a)
{
    int ia = toInt(a);
    saturate(ia, 0, 255);
    return ia;
}

void EngineGL33::DrawDecal(Vector3Par screen, float rhw, float sizeX, float sizeY, PackedColor color,
                           const MipInfo& mip, int specFlags)
{
    // A light's flare (Scene::DrawFlare: an IsLight decal drawn without the
    // depth test, the sun's and the lamps'; fire and explosion sprites are
    // depth tested and keep their light) under the photoreal camera
    // (photoLight): by day none (the sun's disc and the camera's glare carry
    // the sun; its rays were the starburst round a low sun and, added over the
    // bright sky, ran the frame's codes into the top of the encoding), at
    // night a lamp's halo (PhotoCamera.hpp kCameraFlareNight).
    const DWORD glow = (_camera.frameOn && _camera.frame == GetFrameCounter()) ? static_cast<DWORD>(_decalGlow) : 0u;
    _decalGlow = PackedColor(0u);
    constexpr int kFlareBits =
        static_cast<int>(Poseidon::render::Backend::IsLight) | static_cast<int>(Poseidon::render::Backend::NoZBuf);
    if ((specFlags & kFlareBits) == kFlareBits && _camera.frameOn && _camera.frame == GetFrameCounter())
    {
        const float night =
            _camera.nightEffect < 0.0f ? 0.0f : (_camera.nightEffect > 1.0f ? 1.0f : _camera.nightEffect);
        const float share = Poseidon::remaster::kCameraFlareNight * night;
        if (share <= 0.0f)
            return;
        color.SetA8(toInt(static_cast<float>(color.A8()) * share));
    }
    float vx = screen.X();
    float vy = screen.Y();
    float z = screen.Z();

    float oow = rhw;

    // perform simple clipping
    float xBeg = vx - sizeX;
    float xEnd = vx + sizeX;
    float yBeg = vy - sizeY;
    float yEnd = vy + sizeY;
    float uBeg = 0;
    float vBeg = 0;
    float uEnd = 1;
    float vEnd = 1;

    if (xBeg < 0)
    {
        // -xBeg is out, side length is 2*sizeX
        uBeg = -xBeg / (2 * sizeX);
        xBeg = 0;
    }
    if (xEnd > _w)
    {
        // xEnd-_w is out, side length is 2*sizeX
        uEnd = 1 - (xEnd - _w) / (2 * sizeX);
        xEnd = _w;
    }
    if (yBeg < 0)
    {
        // -yBeg is out, side length is 2*sizeY
        vBeg = -yBeg / (2 * sizeY);
        yBeg = 0;
    }
    if (yEnd > _h)
    {
        // yEnd-_h is out, side length is 2*sizeY
        vEnd = 1 - (yEnd - _h) / (2 * sizeY);
        yEnd = _h;
    }

    if (xBeg >= xEnd || yBeg >= yEnd)
    {
        return;
    }

    TLVertex v[4];

    // set vertex 0
    v[0].pos[0] = xBeg;
    v[0].pos[1] = yBeg;
    v[0].pos[2] = z;
    v[0].t0.u = uBeg;
    v[0].t0.v = vBeg;
    // set vertex 1
    v[1].pos[0] = xEnd;
    v[1].pos[1] = yBeg;
    v[1].pos[2] = z;
    v[1].t0.u = uEnd;
    v[1].t0.v = vBeg;
    // set vertex 2
    v[2].pos[0] = xEnd;
    v[2].pos[1] = yEnd;
    v[2].pos[2] = z;
    v[2].t0.u = uEnd;
    v[2].t0.v = vEnd;
    // set vertex 3
    v[3].pos[0] = xBeg;
    v[3].pos[1] = yEnd;
    v[3].pos[2] = z;
    v[3].t0.u = uBeg;
    v[3].t0.v = vEnd;

    // add vertices to vertex buffer

    if (specFlags & IsAlphaFog)
    {
        v[0].color = color;
        v[0].specular = PackedColor(0xff000000);
    }
    else
    {
        // use fog with z
        v[0].specular = PackedColor(0xff000000 - (color & 0xff000000));
        v[0].color = PackedColor(color | 0xff000000);
    }
    // The light past the colour (SetDecalGlow) rides in the specular colour,
    // which the shaders add to the decal's (the pre-lit path, r0 + vSpecColor)
    // and blend by its coverage; only the photoreal camera stores it above
    // white.
    if (glow != 0u)
        v[0].specular = PackedColor((static_cast<DWORD>(v[0].specular) & 0xff000000u) | (glow & 0x00ffffffu));

    int i;
    for (i = 0; i < 4; i++)
    {
        v[i].rhw = oow;
        v[i].pos[2] = z;
        v[i].color = v[0].color;
        v[i].specular = v[0].specular;
    }

    SwitchRenderMode(RMTris);
    BeginScreenPass();

    AddVertices(v, 4);

    QueuePrepareTriangle(mip, specFlags);
    static const VertexIndex indices[4] = {0, 1, 2, 3};
    QueueFan(indices, 4);
}

void EngineGL33::DrawPolygon(const VertexIndex* ii, int n)
{
    QueueFan(ii, n);
}

void EngineGL33::DrawSection(const FaceArray& face, Offset beg, Offset end)
{
    if (_resetNeeded)
    {
        return;
    }
    for (Offset i = beg; i < end; face.Next(i))
    {
        const Poly& f = face[i];
        QueueFan(f.GetVertexList(), f.N());
    }
}

void EngineGL33::DrawPoints(const TLVertex* vs, int nVertex)
{
    if (_resetNeeded)
    {
        return;
    }
    // now we are drawing quads directly
    for (int i = 0; i < nVertex; i++)
    {
        const TLVertex& v = vs[i];
        PackedColor color = v.color;
        if (color.A8() < 8)
        {
            continue; // do not draw stars that are not visible
        }

        // calculate alpha in TL, TR, BL, BR corners
        // v is used as TL corner
        // we have to simulate TR, BL, BR corners
        int xI = toIntFloor(v.pos[0]);
        int yI = toIntFloor(v.pos[1]);
        float xFrac = v.pos[0] - xI;
        float yFrac = v.pos[1] - yI;
        float ixFrac = 1 - xFrac;
        float iyFrac = 1 - yFrac;

        // quad clipping may be required
        if (xI < 0 || xI + 2 > _w || yI < 0 || yI + 2 > _h)
        {
            // we need all four corners to be on screen
            // otherwise we need to apply clipping
            // we discard whole point instead
            continue;
        }

        float a = color.A8();

        float aTL = FracAlpha(ixFrac * iyFrac * a);
        float aTR = FracAlpha(xFrac * iyFrac * a);
        float aBR = FracAlpha(xFrac * yFrac * a);
        float aBL = FracAlpha(ixFrac * yFrac * a);
        TLVertex vs[4];
        vs[0] = v; // TL
        vs[0].pos[0] = xI + 0.5f;
        vs[0].pos[1] = yI + 0.5f;
        vs[0].color = PackedColorRGB(color, aTL);
        // Screen-space points (stars / laser dots) must NOT be fogged: the
        // screen vertex shader reads vFogTC from specular.a, so alpha 0 mixes
        // the point fully to the (night-dark) fog colour and hides it.
        // 0xff000000 = full vFogTC (no fog) + zero specular tint, matching the
        // 2D/HUD path (EngineGL33_2D Draw2D).
        vs[0].specular = PackedColor(0xff000000);

        vs[1] = vs[0]; // TR
        vs[1].pos[0] = xI + 2.5f;
        vs[1].color = PackedColorRGB(color, aTR);
        //
        vs[2] = vs[1]; // BR
        vs[2].pos[1] = yI + 2.5f;
        vs[2].color = PackedColorRGB(color, aBR);

        vs[3] = vs[2]; // BL
        vs[3].pos[0] = vs[0].pos[0];
        vs[3].color = PackedColorRGB(color, aBL);

        static const VertexIndex indices[6] = {0, 1, 2, 3};

        AddVertices(vs, 4);
        QueueFan(indices, 4);
    }
}

void EngineGL33::DrawPoints(int beg, int end)
{
    // now we are drawing quads directly

    if (_resetNeeded)
    {
        return;
    }

    // The remastered stars (B3, the stars switch and its night gate; the
    // frame's gate as last computed, so the sky drawn before the world's first
    // pass may read the previous frame's): a soft splat with a faint colour
    // per star (NightLights.hpp MakeStarSplat) instead of the two-pixel
    // bilinear dot.
    const bool splats = _lightingActive && !_nightVision && !_portraitTargetSize && _featureGates.computed &&
                        _featureGates[Poseidon::remaster::Feature::Stars].on;

    for (int i = beg; i < end; i++)
    {
        if (_mesh->Clip(i) & ClipAll)
        {
            continue;
        }

        const TLVertex& v = _mesh->GetVertex(i);
        PackedColor color = v.color;
        // The volumetric fog's wet air over the eye hides the stars
        // (EngineGL33_PostFog.cpp, FogStarKeep).
        if (_fog.on && _fog.starKeep < 0.999f)
            color = PackedColorRGB(color, static_cast<int>(color.A8() * _fog.starKeep));
        if (color.A8() < 8)
        {
            continue; // do not draw stars that are not visible
        }

        if (splats)
        {
            const Poseidon::remaster::StarSplat s =
                Poseidon::remaster::MakeStarSplat(v.pos[0], v.pos[1], color.A8(), static_cast<std::uint32_t>(i));
            // The whole splat on screen, or none of it (as the dot below).
            if (s.x[1] < 0.0f || s.y[1] < 0.0f || s.x[3] > _w || s.y[3] > _h)
            {
                continue;
            }
            Color base(color.R8() / 255.0f * s.tint[0], color.G8() / 255.0f * s.tint[1],
                       color.B8() / 255.0f * s.tint[2]);
            base.SaturateMinMax();
            TLVertex fan[5];
            for (int c = 0; c < 5; c++)
            {
                fan[c] = v;
                fan[c].pos[0] = s.x[c];
                fan[c].pos[1] = s.y[c];
                fan[c].color = PackedColorRGB(PackedColor(base), FracAlpha(s.alpha[c]));
                fan[c].specular = PackedColor(0xff000000); // no fog, as the dot
            }
            static const VertexIndex fanIndices[6] = {0, 1, 2, 3, 4, 1};
            AddVertices(fan, 5);
            QueueFan(fanIndices, 6);
            continue;
        }

        // calculate alpha in TL, TR, BL, BR corners
        // v is used as TL corner
        // we have to simulate TR, BL, BR corners
        int xI = toIntFloor(v.pos[0]);
        int yI = toIntFloor(v.pos[1]);
        float xFrac = v.pos[0] - xI;
        float yFrac = v.pos[1] - yI;
        float ixFrac = 1 - xFrac;
        float iyFrac = 1 - yFrac;

        float a = color.A8();

        TLVertex vs[4];
        vs[0] = v; // TL
        vs[0].pos[0] = xI + 0.5f;
        vs[0].pos[1] = yI + 0.5f;
        vs[0].color = PackedColorRGB(color, FracAlpha(ixFrac * iyFrac * a));
        vs[0].specular = PackedColor(0xff000000); // no fog (vFogTC = specular.a); see DrawPoints overload above

        vs[1] = vs[0]; // TR
        vs[1].pos[0] = xI + 2.5f;
        vs[1].color = PackedColorRGB(color, FracAlpha(xFrac * iyFrac * a));
        //
        vs[2] = vs[1]; // BR
        vs[2].pos[1] = yI + 2.5f;
        vs[2].color = PackedColorRGB(color, FracAlpha(xFrac * yFrac * a));

        vs[3] = vs[2]; // BL
        vs[3].pos[0] = vs[0].pos[0];
        vs[3].color = PackedColorRGB(color, FracAlpha(ixFrac * yFrac * a));

        static const VertexIndex indices[6] = {0, 1, 2, 3};

        AddVertices(vs, 4);
        QueueFan(indices, 4);
    }
}

bool EngineGL33::CanGrass() const
{
    return false;
}
