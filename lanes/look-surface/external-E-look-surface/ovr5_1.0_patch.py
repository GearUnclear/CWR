PATCHES = {
    "psDetailRemasterMain": [
        ("""                g_surfCavity = surfaceCavity(tex1, vUV1, r1x, r1y, true) * (base * litSurface3.x * fadeR);
            }""",
         """                g_surfCavity = surfaceCavity(tex1, vUV1, r1x, r1y, true) * (base * litSurface3.x * fadeR);
            }
            float fadeB = 1.0 - smoothstep(60.0, 200.0, length(vWorldRel));
            if (litSurface3.x > 0.0 && fadeB > 0.0) {
                float bm = dot(textureLod(tex0, vUV0, 3.5).rgb, kLuma);
                float bf = dot(textureLod(tex0, vUV0, 0.5).rgb, kLuma);
                g_surfCavity += (bf - bm) * (1.0 * litSurface3.x * fadeB);
            }"""),
    ],
}
