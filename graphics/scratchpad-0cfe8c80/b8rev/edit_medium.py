from pathlib import Path
R = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\engine\Poseidon\Graphics\Remaster")
def rep(path, old, new):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == 1, (path.name, old[:70], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")
h = R / "AirMedium.hpp"
rep(h, """//   - aerial haze (airHaze): thin air whose density falls off with height
//     (scale height kHazeScaleHeight), so low ground is hazier than ridges.
//     Beyond kHazeNearDistance a surface fades toward the sky behind it,
//     C' = C T + S, with a slightly stronger extinction of blue than of red
//     (kHazeChroma) and a forward-scattering lobe of sunlight that only lit
//     air adds (the ridge's shadow in the haze stays grey). The classic fog
//     then keeps only the view-distance edge: its start moves out to
//     kHazeEdgeFraction of its end in clear weather (FogEdgeScale);
//   - valley mist (valleyMist): a flat-topped layer kValleyMistTop metres
//     above the lowest ground within about 850 m (LocalLowGround), so it
//     pools on valley floors and water and leaves ridges clear. It forms with
//     an eastern sun from just before sunrise, is gone by 38 degrees, thins
//     under an overcast and also lies in weather fog;
//   - beams (airBeams): single scattering of the sun (and of headlights) in a
//     ground mist, marched at half resolution with the sun's visibility from
//     the shadow map, the terrain horizon and the cloud field, so crowns,
//     roofs and ridges cut light shafts out of it. The mist is thin in clear
//     weather (narrow beams through gaps, no wash over open ground) and thick
//     in setFog and rain (fans of god rays).
""", """//   - aerial haze (airHaze): thin air whose density falls off with height
//     (scale height kHazeScaleHeight), so low ground is hazier than ridges.
//     Beyond kHazeNearDistance a surface fades toward the sky behind it,
//     C' = C T + S, with a slightly stronger extinction of blue than of red
//     (kHazeChroma). The haze has no sun lobe of its own: the sky it fades
//     toward already holds the glow toward the sun. Air that a ridge or a
//     cloud shades (the march's sun visibility) keeps only a dimmer
//     zenith-coloured sky light, kHazeShadowAmbient of the lit air's, so the
//     ridge's shadow in the haze stays grey. The classic fog then keeps only
//     the view-distance edge: its start moves out to kHazeEdgeFraction of its
//     end in clear weather (FogEdgeScale);
//   - valley mist (valleyMist): a flat-topped layer kValleyMistTop metres
//     above the lowest ground within about 850 m (LocalLowGround), so it
//     pools on valley floors and water and leaves ridges clear. It forms with
//     an eastern sun from just before sunrise, is full up to
//     kMorningFadeSunDeg and gone by kMorningGoneSunDeg (MorningWeight),
//     thins under an overcast and also lies in weather fog;
//   - beams (airBeams): single scattering of the sun (and of headlights) in a
//     ground mist, marched at half resolution with the sun's visibility from
//     the shadow map, the terrain horizon and the cloud field, so crowns,
//     roofs and ridges cut light shafts out of it. The mist is thin in clear
//     weather and thick in setFog and rain (fans of god rays).
""")
rep(h, """constexpr float kHazeMie = 0.20f;             // sun single scattering of the air
constexpr float kHazeG = 0.72f;               // its Henyey-Greenstein asymmetry
constexpr float kHazeShadowAmbient = 0.55f;   // in-scatter of shadowed air relative to lit air
""", """constexpr float kHazeShadowAmbient = 0.55f;   // sky light of shaded air relative to lit air's (the apply)
""")
rep(h, """constexpr float kMorningFadeSunDeg = 5.0f; // and burns off between these (an hour or two after sunrise)
constexpr float kMorningGoneSunDeg = 15.0f;
""", """constexpr float kMorningFadeSunDeg = 6.0f; // and burns off between these (about an hour after sunrise)
constexpr float kMorningGoneSunDeg = 10.0f;
""")
g = R / "FeatureGates.hpp"
rep(g, "constexpr float kGateMorningMaxSunDeg = 15.0f;     // morning mist: gone by this sun height (AirMedium.hpp)",
       "constexpr float kGateMorningMaxSunDeg = 10.0f;     // morning mist: gone by this sun height (AirMedium.hpp)")
print("ok")
