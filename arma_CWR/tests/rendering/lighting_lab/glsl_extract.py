"""Extract GLSL sources embedded as C++ raw strings in the engine sources.

The GL33 backend embeds its shaders as raw string literals, e.g.

    static const char s_vsTransformGLSL[] = R"(#version 330 core ... )";
    static const char* s_psGammaGLSL = R"(#version 330 core ... )";
    const char* kVS = R"(#version 330 core ... )";          // EngineGL33_ShadowDepth.cpp
    static const char s_x[] = R"GLSL(#version 330 core ... )GLSL";

`extract_glsl(text)` returns {variable name: GLSL source}. `load_source(spec)`
reads the C++ file either from disk or from a git revision, so the lab can
render the shaders of any commit:

    load_source(None)                       # working tree EngineGL33_Shaders.cpp
    load_source("path/to/file.cpp")         # any file on disk
    load_source("HEAD~3")                   # git show HEAD~3:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp
    load_source("abc123:arma_CWR/engine/PoseidonGL33/EngineGL33_ShadowDepth.cpp")

CLI:  python glsl_extract.py [--source PATH_OR_REV] [--list] [--dump DIR]
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

LAB_DIR = Path(__file__).resolve().parent
ENGINE_DIR = LAB_DIR.parents[2]  # .../arma_CWR
DEFAULT_SHADER_RELPATH = "arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp"
SHADOW_DEPTH_RELPATH = "arma_CWR/engine/PoseidonGL33/EngineGL33_ShadowDepth.cpp"

# char-array or char-pointer declaration followed by a raw string literal.
#   [static] [const] char [*] [const] NAME [ [] ] = R"DELIM( BODY )DELIM";
# The C++ raw-string delimiter is at most 16 chars and may not contain
# parentheses, backslash or whitespace.
_RAW_DECL = re.compile(
    r"""(?:\bstatic\s+)?(?:\bconst\s+)?\bchar\s*(?:\*\s*)?(?:\bconst\s+)?
        (?P<name>[A-Za-z_]\w*)\s*(?:\[\s*\d*\s*\])?\s*=\s*
        R"(?P<delim>[^()\\\s"]{0,16})\(""",
    re.VERBOSE,
)


def _strip_line_comments_mask(text: str) -> list[tuple[int, int]]:
    """Spans of // and /* */ comments outside raw strings (best effort).

    Used only to ignore declarations that appear inside comments; the GLSL
    bodies themselves are returned verbatim.
    """
    spans = []
    for m in re.finditer(r"//[^\n]*|/\*.*?\*/", text, re.S):
        spans.append((m.start(), m.end()))
    return spans


def extract_glsl(text: str) -> dict[str, str]:
    """Return {variable name: raw-string body} for every char raw-string decl."""
    out: dict[str, str] = {}
    pos = 0
    comment_spans = _strip_line_comments_mask(text)

    def in_comment(i: int) -> bool:
        for a, b in comment_spans:
            if a <= i < b:
                return True
            if a > i:
                break
        return False

    while True:
        m = _RAW_DECL.search(text, pos)
        if not m:
            break
        body_start = m.end()
        terminator = ")" + m.group("delim") + '"'
        end = text.find(terminator, body_start)
        if end < 0:
            raise ValueError(f"unterminated raw string for {m.group('name')}")
        if in_comment(m.start()):
            pos = m.end()
            continue
        # Comments that start inside a raw string are not comments; drop the
        # spans that overlap this body so later matches are judged correctly.
        comment_spans = [s for s in comment_spans if not (body_start <= s[0] < end)]
        out[m.group("name")] = text[body_start:end]
        pos = end + len(terminator)
    return out


def repo_root() -> Path:
    try:
        top = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=LAB_DIR,
            capture_output=True,
            text=True,
            check=True,
        ).stdout.strip()
        if top:
            return Path(top)
    except (OSError, subprocess.CalledProcessError):
        pass
    return ENGINE_DIR.parent


def git_show(rev_path: str, root: Path | None = None) -> str:
    root = root or repo_root()
    res = subprocess.run(["git", "show", rev_path], cwd=root, capture_output=True, text=True)
    if res.returncode != 0:
        raise FileNotFoundError(f"git show {rev_path!r} failed: {res.stderr.strip()}")
    return res.stdout


def load_source(spec: str | os.PathLike | None = None, default_relpath: str = DEFAULT_SHADER_RELPATH) -> str:
    """Read C++ source text from a path, a git revision, or the working tree.

    spec None        -> <repo>/<default_relpath> from the working tree
    existing path    -> that file
    'REV:path'       -> git show REV:path (path relative to the repo root)
    'REV'            -> git show REV:<default_relpath>
    """
    root = repo_root()
    if spec is None:
        return (root / default_relpath).read_text(encoding="utf-8", errors="replace")
    p = Path(spec)
    if p.exists() and p.is_file():
        return p.read_text(encoding="utf-8", errors="replace")
    s = str(spec)
    if ":" in s and not os.path.isabs(s):
        return git_show(s, root)
    return git_show(f"{s}:{default_relpath}", root)


def override_name(var: str) -> str:
    """C++ variable -> engine override file stem: s_vsTransformGLSL -> vsTransform
    (CompileGLShader looks for <CWR_SHADER_OVERRIDE_DIR>/<name>.glsl)."""
    n = var[2:] if var.startswith("s_") else var
    return n[:-4] if n.endswith("GLSL") else n


def apply_override_dir(shaders: dict[str, str], override_dir: str | os.PathLike | None) -> list[str]:
    """Replace shaders by <dir>/<name>.glsl files, like TryLoadShaderOverride."""
    if not override_dir:
        return []
    d = Path(override_dir)
    used = []
    for var in list(shaders):
        f = d / f"{override_name(var)}.glsl"
        if f.is_file():
            shaders[var] = f.read_text(encoding="utf-8")
            used.append(var)
    return used


def load_shaders(spec: str | os.PathLike | None = None, include_shadow_depth: bool = False,
                 override_dir: str | os.PathLike | None = None) -> dict[str, str]:
    """Extract all shaders of EngineGL33_Shaders.cpp (at `spec`).

    With include_shadow_depth, the kVS/kFS/kAlphaVS/kAlphaFS of
    EngineGL33_ShadowDepth.cpp at the same revision are merged in (prefixed
    'shadowDepth.'). override_dir: per-shader <name>.glsl replacements, the
    engine's --shader-override-dir / CWR_SHADER_OVERRIDE_DIR convention.
    """
    shaders = extract_glsl(load_source(spec))
    apply_override_dir(shaders, override_dir)
    if include_shadow_depth:
        s = None if spec is None else str(spec)
        if s is None:
            sd_spec = None
        elif Path(s).exists():
            sd_spec = str(Path(s).with_name("EngineGL33_ShadowDepth.cpp"))
        else:
            rev = s.split(":", 1)[0]
            sd_spec = f"{rev}:{SHADOW_DEPTH_RELPATH}"
        try:
            sd = extract_glsl(load_source(sd_spec, SHADOW_DEPTH_RELPATH))
            shaders.update({f"shadowDepth.{k}": v for k, v in sd.items()})
        except FileNotFoundError:
            pass
    return shaders


def describe_source(spec: str | os.PathLike | None) -> str:
    if spec is None:
        return f"worktree:{DEFAULT_SHADER_RELPATH}"
    if Path(spec).exists():
        return str(Path(spec).resolve())
    return f"git:{spec}"


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--source", default=None, help="C++ file path, git rev, or rev:path (default: worktree)")
    ap.add_argument("--shadow-depth", action="store_true", help="also extract EngineGL33_ShadowDepth.cpp")
    ap.add_argument("--list", action="store_true", help="list variable names and sizes")
    ap.add_argument("--dump", default=None, help="write each shader to DIR/<name>.glsl")
    args = ap.parse_args(argv)

    shaders = load_shaders(args.source, include_shadow_depth=args.shadow_depth)
    if args.list or not args.dump:
        print(f"# {describe_source(args.source)}")
        for name, src in shaders.items():
            first = src.strip().splitlines()[0] if src.strip() else ""
            print(f"{name:28s} {len(src):6d} bytes  {first}")
    if args.dump:
        # file names follow the engine's override convention (vsTransform.glsl,
        # psNormal.glsl, ...), so a dumped + edited directory works both as the
        # lab's --shader-override-dir and as the game's CWR_SHADER_OVERRIDE_DIR
        d = Path(args.dump)
        d.mkdir(parents=True, exist_ok=True)
        for name, src in shaders.items():
            (d / f"{override_name(name)}.glsl").write_text(src, encoding="utf-8")
        print(f"wrote {len(shaders)} shaders to {d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
