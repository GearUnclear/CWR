PATCHES = {
    "psRemasterCommon": [
        (
            "        g_surfAlbedo = sqrt(clamp(dot(a, kLuma), 0.0, 1.0));\n",
            "        g_surfAlbedo = sqrt(clamp(dot(a, kLuma), 0.0, 1.0));\n"
            "        if (litSurfK.x > 0.5) {\n"
            "            if (litSurfK.x < 1.5) return vec3(canopyVis);\n"
            "            if (litSurfK.x < 2.5) return toDisplay(tonemapNeutral(skyIrr * 1.6 * exposure));\n"
            "            return toDisplay(tonemapNeutral((specSun + specSky) * exposure * 4.0));\n"
            "        }\n",
        ),
    ],
}
