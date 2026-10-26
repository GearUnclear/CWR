R = ['''    {PostStage::Air, "air", "B8", true, true, Mark::AirEnd, Section::Air},
''' if False else '''    // B8: the air pass marches at half resolution, on the linear depth; the
    // look lane's volumetric fog (EngineGL33_PostFog.cpp) runs on the same
    // stage in its place.
    {PostStage::Air, "air", "B8", true, true, Mark::AirEnd, Section::Air},
''',
'''                       in.temporal || in.lumaSharpen || in.air || in.fog;
''',
'''    // The air pass needs the world depth, and over a screen overlay its haze
    // would grey the overlay's black surround. The volumetric fog has the same
    // needs and takes the stage in its place.
    plan.fog = in.fog && in.worldDepth && in.glow;
    const bool air = (in.air || in.fog) && in.worldDepth && in.glow;
    if (air)
        plan.runs |= Bit(PostStage::Air);
    // The shafts are depth tested glow toward a sun on screen; the air pass's
    // beams and the fog's own sun light in the air replace them.
    if (in.worldDepth && in.glow && in.sunShafts > 0.0f && in.shaftWeight > kShaftMinWeight &&
        !(air && (in.airBeams || plan.fog)))
''']
