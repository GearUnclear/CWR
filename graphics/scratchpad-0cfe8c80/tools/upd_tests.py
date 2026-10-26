import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')
p = 'tests/unit/engine/Poseidon/Graphics/test_volumetric_fog.cpp'
s = open(p, encoding='utf-8', newline='').read()
start = s.index('TEST_CASE("Volumetric fog: clear noon is thin haze and blue air; grey sky, rain and fog thicken it"')
end = s.index('TEST_CASE("Volumetric fog: off, the forward fog is the classic one; on, it keeps only the view-distance edge"')
new = open(r'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\tools\tests_mid.cpp', encoding='utf-8').read()
s = s[:start] + new + s[end:]
old = '''    // The forward fog keeps only the edge through litHaze.x.'''
new2 = '''    // The near field: the low beam the shading uses (FrameLights.hpp).
    const std::string nearPass = gl::PostShaderSource(gl::PostShader::FogNear);
    REQUIRE_FALSE(nearPass.empty());
    CHECK(CountOf(nearPass, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamCutLow = -0.005;") == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamCutHigh = 0.025;") == 1);
    CHECK(CountOf(nearPass, "const float kAirBeamHotBand = -0.03;") == 1);
    CHECK(FogNearSteps(1.0f) == 20);
    CHECK(FogNearSteps(9.0f) == 32);
    // The eye's near thinning in the shaders is the C++ one.
    CHECK(CountOf(inject, "const float kMistNearKeep = 0.3;") == 1);
    CHECK(kMistNearKeep == 0.3f);
    // The forward fog keeps only the edge through litHaze.x.'''
assert s.count(old) == 1
s = s.replace(old, new2)
s = s.replace('#include <cmath>\n#include <string>\n#include <vector>', '#include <cmath>\n#include <memory>\n#include <string>\n#include <vector>')
s = s.replace('#include <cmath>\r\n#include <string>\r\n#include <vector>', '#include <cmath>\r\n#include <memory>\r\n#include <string>\r\n#include <vector>')
open(p, 'w', encoding='utf-8', newline='').write(s)
p = 'tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp'
s = open(p, encoding='utf-8', newline='').read()
old = '''    // air pass's march, blur, mean and apply, and the volumetric fog's
    // injection, integration and apply
    CHECK(post == 22);'''
new = '''    // air pass's march, blur, mean and apply, and the volumetric fog's
    // injection, integration, near field and apply
    CHECK(post == 23);'''
assert s.count(old) == 1
s = s.replace(old, new)
open(p, 'w', encoding='utf-8', newline='').write(s)
print('ok')
