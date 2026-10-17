// C shim over the engine's pure remastered-lighting code, loaded by native.py
// through ctypes so the lab runs the same light rig, horizon map and cloud
// noise as the engine.

#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>

#include <cstring>

using namespace Poseidon::remaster;

extern "C"
{
    // in[22]: towardSun xyz, towardMoon xyz, moonPhase, sunDiffuse rgb, ambient rgb,
    //         fog rgb, skyThrough, nightEffect, cloudShadowsActive, (pad) x3
    // tuning[16]: exposure, sun, sky, bounce, warmth, terrainShadows, terrainOcclusion,
    //             cloudShadows, specular, localLights, moonlight, ao, bloom, shadowSoftness, saturation, sunShafts
    // out[24]: sun rgb, zenith rgb, horizon rgb, ground rgb, moon rgb, elevation, cover, transmit, exposure, (pad),
    //          horizonSun rgb, (pad)
    int lab_compute_rig(const float* in, const float* tuning, float* out)
    {
        LightRigInputs i;
        std::memcpy(i.towardSun, in + 0, 3 * sizeof(float));
        std::memcpy(i.towardMoon, in + 3, 3 * sizeof(float));
        i.moonPhaseLight = in[6];
        i.classicSunDiffuse = Rgb(in[7], in[8], in[9]);
        i.classicAmbient = Rgb(in[10], in[11], in[12]);
        i.fogColor = Rgb(in[13], in[14], in[15]);
        i.skyThrough = in[16];
        i.nightEffect = in[17];
        i.cloudShadowsActive = in[18] > 0.5f;

        LightingTuning t;
        t.exposure = tuning[0];
        t.sun = tuning[1];
        t.sky = tuning[2];
        t.bounce = tuning[3];
        t.warmth = tuning[4];
        t.terrainShadows = tuning[5];
        t.terrainOcclusion = tuning[6];
        t.cloudShadows = tuning[7];
        t.specular = tuning[8];
        t.localLights = tuning[9];
        t.moonlight = tuning[10];
        t.ambientOcclusion = tuning[11];
        t.bloom = tuning[12];
        t.shadowSoftness = tuning[13];
        t.saturation = tuning[14];
        t.sunShafts = tuning[15];

        const LightRig r = ComputeLightRig(i, t);
        const Rgb cols[5] = {r.sun, r.skyZenith, r.skyHorizon, r.skyGround, r.moon};
        for (int k = 0; k < 5; ++k)
        {
            out[k * 3 + 0] = cols[k].r;
            out[k * 3 + 1] = cols[k].g;
            out[k * 3 + 2] = cols[k].b;
        }
        out[15] = r.sunElevationDeg;
        out[16] = r.cloudCover;
        out[17] = r.cloudTransmit;
        out[18] = r.exposure;
        out[20] = r.skyHorizonSun.r;
        out[21] = r.skyHorizonSun.g;
        out[22] = r.skyHorizonSun.b;
        return 1;
    }

    // Default tuning, so the lab starts from the shipped look.
    void lab_default_tuning(float* tuning)
    {
        const LightingTuning t;
        const float v[16] = {t.exposure,
                             t.sun,
                             t.sky,
                             t.bounce,
                             t.warmth,
                             t.terrainShadows,
                             t.terrainOcclusion,
                             t.cloudShadows,
                             t.specular,
                             t.localLights,
                             t.moonlight,
                             t.ambientOcclusion,
                             t.bloom,
                             t.shadowSoftness,
                             t.saturation,
                             t.sunShafts};
        std::memcpy(tuning, v, sizeof(v));
    }

    // Builds the horizon map. Returns the texel count per side (0 on failure);
    // horizonOut needs 4 * size * size * 4 uint16, skyOut and heightOut size * size,
    // tintOut size * size * 2 (written only when groundRgb, size * size * 3, is given).
    int lab_build_horizon(const float* heights, int size, float cellSize, std::uint16_t* horizonOut,
                          std::uint8_t* skyOut, float* heightOut, const float* groundRgb, float* tintOut)
    {
        TerrainHorizonMap m;
        TerrainHorizonOptions o;
        if (!BuildTerrainHorizonMap(heights, size, cellSize, o, m, groundRgb))
            return 0;
        std::memcpy(horizonOut, m.horizon.data(), m.horizon.size() * sizeof(std::uint16_t));
        std::memcpy(skyOut, m.skyVisibility.data(), m.skyVisibility.size());
        std::memcpy(heightOut, m.height.data(), m.height.size() * sizeof(float));
        if (groundRgb && tintOut)
            std::memcpy(tintOut, m.groundTint.data(), m.groundTint.size() * sizeof(float));
        return m.size;
    }

    void lab_crown_ellipsoid(const float* boxMin, const float* boxMax, float* center, float* invHalf)
    {
        CrownEllipsoid(boxMin, boxMax, center, invHalf);
    }

    int lab_cloud_noise(int size, unsigned seed, std::uint8_t* out)
    {
        const auto v = BuildCloudNoise(size, seed);
        if (v.empty())
            return 0;
        std::memcpy(out, v.data(), v.size());
        return 1;
    }
}
