import pathlib

d = pathlib.Path(r"D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\engine\PoseidonGL33")
p = d / "EngineGL33_PostFX.cpp"
s = p.read_text(encoding="utf-8")

ns_open = "namespace\n{\n\nstatic const char s_vsPostGLSL[]"
a = s.index(ns_open)
strings_start = a + len("namespace\n{\n\n")
consts_start = s.index("// Post constants shared with the lab")
strings = s[strings_start:consts_start]

table_start = s.index("// Every post shader, assembled from the raw strings above")
table_end = s.index("GLuint CompilePostShader(")
table = s[table_start:table_end]

acc_start = s.index("namespace Poseidon::gl33lighting\n{\n\nconst char* PostShaderName")
acc_end = s.index("} // namespace Poseidon::gl33lighting\n", acc_start) + len("} // namespace Poseidon::gl33lighting\n")
acc = s[acc_start:acc_end]

# new PostFX.cpp: drop the strings, the table and the accessors
s2 = s[:strings_start] + s[consts_start:table_start] + s[table_end:acc_start] + s[acc_end:]
s2 = s2.replace("} // namespace\n\n\nvoid EngineGL33::DestroyPostTargets()", "} // namespace\n\nvoid EngineGL33::DestroyPostTargets()")
old_comment = """// Each post shader is assembled from raw strings by the kPostShaders table
// (PostShaderSource in EngineGL33_Lighting.hpp); s_glslPostCommon holds the
// depth reconstruction the passes share.
"""
new_comment = """// The shaders live in EngineGL33_PostShaders.cpp, which has no GL code, so the
// unit tests compile the sources the engine builds (PostShaderSource in
// EngineGL33_Lighting.hpp).
"""
assert old_comment in s2
s2 = s2.replace(old_comment, new_comment)
s2 = s2.replace("#include <algorithm>\n#include <array>\n#include <cmath>\n#include <iterator>\n#include <string>\n",
                "#include <algorithm>\n#include <cmath>\n#include <string>\n")
p.write_text(s2, encoding="utf-8", newline="\n")

table = table.replace("// Every post shader, assembled from the raw strings above: these pieces in this\n// order, in PostShader order.",
                      "// Every post shader, assembled from the raw strings above: these pieces in this\n// order, in PostShader order.")
header = """// Remastered lighting: the post-pass shaders. EngineGL33_PostFX.cpp runs the
// passes; this file holds their GLSL and no GL code, so the unit tests compile
// the very sources the engine builds. Each shader is assembled from the raw
// strings below by the kPostShaders table and read through PostShaderName and
// PostShaderSource (EngineGL33_Lighting.hpp). s_glslPostCommon holds the depth
// reconstruction the passes share; a pass that adds a shader adds its row to
// the table.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <array>
#include <iterator>
#include <string>

namespace
{

"""
new = header + strings + table.rstrip("\n") + "\n\n} // namespace\n\n" + acc
(d / "EngineGL33_PostShaders.cpp").write_text(new, encoding="utf-8", newline="\n")
print("ok", len(s), len(s2), len(new))
