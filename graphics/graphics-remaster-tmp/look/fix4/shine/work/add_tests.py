p = r'D:/Arma_CWA_worktrees/look-fix-shine/arma_CWR/tests/unit/engine/Poseidon/Graphics/test_wet_weather.cpp'
s = open(p, encoding='utf-8').read()
old = '''    CHECK(surface.find("pud = wetPuddle(posAbs, footT, g_wetDraw == 0 ? 1.0 : 0.0, veg, litWeather.z);") !=
          std::string::npos);'''
new = '''    CHECK(surface.find("pud = wetPuddle(posAbs, footT, g_wetDraw == 0 ? 1.0 : 0.0, vegPud, litWeather.z);") !=
          std::string::npos);'''
if old in s:
    s = s.replace(old, new)
assert new in s
if 'WetGroundGlossMirror' not in s:
    s = s.rstrip('\n') + '\n' + open(r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix4/shine/work/new_tests.cpp.txt',
                                     encoding='utf-8').read()
if '#include <cstdint>' not in s:
    s = s.replace('#include <cmath>\n', '#include <cmath>\n#include <cstdint>\n', 1)
open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
