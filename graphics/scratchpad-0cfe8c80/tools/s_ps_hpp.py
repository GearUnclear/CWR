R = ['''    // The air pass (B8): one of airHaze, valleyMist or airBeams and its frame
    // gate are on. It asks for the pass by itself and runs where the world
    // depth is there and no screen overlay was drawn (the haze would grey an
    // optics mask's black surround).
    bool air = false;
    // The air pass draws sun beams this frame (airBeams and its gate on):
    // they replace the legacy shafts.
    bool airBeams = false;
    // The volumetric fog (FeatureSet::volFog and its frame gate, look-fog):
    // the Air stage applies the fog volume to every pixel by its world depth
    // in place of B8's air pass (whose gates are off while it runs) and
    // replaces the legacy shafts (its sun light in the air is the beams). It
    // needs the world depth and no screen overlay; held off, the frame keeps
    // the forward fog's classic edge (VolumetricFog.hpp).
    bool fog = false;
''']
