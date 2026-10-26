PATCHES = {
    "psDetailRemasterMain": [
        ("""                g_surfCavity = surfaceCavity(tex1, vUV1, r1x, r1y, true) * (base * litSurface3.x * fadeR);
            }""",
         """                g_surfCavity = surfaceCavity(tex1, vUV1, r1x, r1y, true) * (base * litSurface3.x * fadeR);
            }
            float fadeB = 1.0 - smoothstep(60.0, 200.0, length(vWorldRel));
            if (litSurface3.x > 0.0 && fadeB > 0.0) {
                vec2 b0x = dFdx(vUV0);
                vec2 b0y = dFdy(vUV0);
                float bm = dot(textureGrad(tex0, vUV0, b0x * 8.0, b0y * 8.0).rgb, kLuma);
                g_surfCavity += (dot(t0.rgb, kLuma) - bm) * (1.5 * litSurface3.x * fadeB);
            }"""),
    ],
}
