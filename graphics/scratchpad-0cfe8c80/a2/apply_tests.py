import pathlib

here = pathlib.Path(__file__).parent
p = pathlib.Path(r"D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\tests\unit\engine\Poseidon\Graphics\test_remaster_lighting.cpp")
s = p.read_text(encoding="utf-8")
i = s.index("// --- GLSL contracts --------------------------------------------------------")
s = s[:i] + (here / "tests_section.cpp").read_text(encoding="utf-8")
old_inc = "#include <PoseidonGL33/EngineGL33_Lighting.hpp>\n\n#include <atomic>\n#include <cmath>\n"
assert old_inc in s
s = s.replace(old_inc, "#include <PoseidonGL33/EngineGL33_Lighting.hpp>\n\n#include <algorithm>\n#include <atomic>\n#include <cmath>\n")
old_inc2 = "#include <filesystem>\n#include <fstream>\n#include <limits>\n"
assert old_inc2 in s
s = s.replace(old_inc2, "#include <filesystem>\n#include <fstream>\n#include <iterator>\n#include <limits>\n")
old_inc3 = "#include <string>\n#include <vector>\n"
assert old_inc3 in s
s = s.replace(old_inc3, "#include <string>\n#include <utility>\n#include <vector>\n", 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("ok")
