import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in s else '\n'
    for a, b in pairs:
        a2, b2 = a.replace('\n', nl), b.replace('\n', nl)
        assert s.count(a2) == 1, (p, a[:70], s.count(a2))
        s = s.replace(a2, b2)
    open(p, 'w', encoding='utf-8', newline='').write(s)


edit('tests/unit/engine/Poseidon/Graphics/test_feature_switches.cpp', [
    ('''    const LightingTuning forced = Apply("remastered,gates=0,ssr=0");
    const FeatureGates g = ComputeFeatureGates(Noon(), forced);
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        INFO(GetFeatureInfo(f).key);
        CHECK(g[f].on == (f != Feature::Ssr));
    }''',
     '''    // (The volumetric fog off: it takes B8's air pass's place even forced.)
    const LightingTuning forced = Apply("remastered,gates=0,ssr=0,volFog=0");
    const FeatureGates g = ComputeFeatureGates(Noon(), forced);
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        INFO(GetFeatureInfo(f).key);
        CHECK(g[f].on == (f != Feature::Ssr && f != Feature::VolFog));
    }'''),
    ('''    FeatureGates g = ComputeFeatureGates(in, Apply("remastered,stars=0"));
    g.frame = 42;
    const std::string s = FeatureStateString(Apply("remastered,stars=0"), g);''',
     '''    FeatureGates g = ComputeFeatureGates(in, Apply("remastered,stars=0,volFog=0"));
    g.frame = 42;
    const std::string s = FeatureStateString(Apply("remastered,stars=0,volFog=0"), g);'''),
])
print('ok')
