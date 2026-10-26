R = ['''// string (LitConstantsBlock(); the post air and fog passes name it in kPostShaders); the
''',
'''    vec4 litHaze;         // 78: aerial haze (B8) or volumetric fog: x classic fog factor scale minus 1 (0 = the classic fog)
''',
'''// air and fog passes paste it too (cloudSunVisibility; the fog also the dome's
// cloud cover and the canopy shadows).
''',
'''    // With the aerial haze or the volumetric fog on (litHaze.x above 0) the
    // classic fog keeps only the view-distance edge: its start moves out
    // (AirMedium.hpp and VolumetricFog.hpp FogEdgeScale) and the post pass
    // takes over the distance before it.
''']
