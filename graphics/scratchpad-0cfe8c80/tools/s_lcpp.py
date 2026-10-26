R = ['''    // The air pass (B8): this frame's medium, and the classic fog's edge
    // (litHaze.x) while the haze replaces the fog before it.
    _lightRigExposure = rig.exposure;
    UpdateAirMedium();
    // The volumetric fog (look-fog): the frame's medium; while the fog volume
    // is applied it sets the classic fog's edge in place of B8's haze (whose
    // gates are off then).
    UpdateFogMedium();
    // The night (B3): the physical lamps and low beams, while their gate is on
    // and the tiled list lighting the pass; zero otherwise, which leaves the
    // classic model for every light. w: the moon key's share.
    const rm::NightConstants night =
        rm::ComputeNightConstants(_lighting, _featureGates, _lightTilesActive, key.moonShare);
    SetSlot(LitConst::Night, night.night[0], night.night[1], night.night[2], night.night[3]);
    _nightLightsActive = night.night[0] > 0.5f;
''']
