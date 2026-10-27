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

The remastered and post shaders are assembled from pieces kept in several
files (EngineGL33_LightingShaders*.cpp, EngineGL33_Post*.cpp): the engine joins
the raw strings its kFragmentPieces and kPostShaders tables name. `load_programs
(spec)` reads the file at `spec` with its siblings of the same family, parses
those tables and returns {engine override name: source}, the programs the
engine compiles (psRemasterCommon, psNormalRemasterMain, vsTransformRemaster,
vsPost, psPostAo, ...). A file without the tables (an older revision) gives
the same names from its whole raw strings.

CLI:  python glsl_extract.py [--source PATH_OR_REV] [--list] [--dump DIR]
      (--list and --dump show the assembled programs, under the override names)
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
    # The sources are UTF-8 (comments carry dashes); decode them as such, not
    # in the console's code page.
    res = subprocess.run(["git", "show", rev_path], cwd=root, capture_output=True, encoding="utf-8",
                         errors="replace")
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


# Files whose shaders are assembled from pieces spread over the family's files.
_FAMILIES = ("EngineGL33_LightingShaders", "EngineGL33_Post")
# Families whose tables also name pieces of another family: the post air pass
# pastes the remastered prelude's LitConstants block, terrain and clouds piece
# and sky piece (kPostShaders rows psPostAirMarch, psPostAirApply), the
# reflections' trace its LitConstants block and sky piece (psPostSsrTrace), the
# volumetric fog the same three (psFogInject, psFogApply).
_FAMILY_ALSO = {"EngineGL33_Post": ("EngineGL33_LightingShaders",)}
_FRAGMENT_TABLE = re.compile(r"\bkFragmentPieces\s*\[\s*\]\s*=\s*\{(?P<body>.*?)\};", re.S)
_SCREEN_VS_TABLE = re.compile(r"\bkScreenVertexPieces\s*\[\s*\]\s*=\s*\{(?P<body>.*?)\};", re.S)
_POST_TABLE = re.compile(r"\bkPostShaders\s*\[\s*\]\s*=\s*\{(?P<body>.*?)\n\};", re.S)
_POST_ROW = re.compile(r'\{\s*"(?P<name>\w+)"\s*,\s*\{(?P<pieces>[^{}]*)\}\s*\}')


def _family(name: str) -> str | None:
    return next((f for f in _FAMILIES if name.startswith(f)), None)


def load_family(spec: str | os.PathLike | None = None, default_relpath: str = DEFAULT_SHADER_RELPATH) -> list[str]:
    """The text of the C++ file at `spec` and of the other .cpp files of its
    family in the same directory (EngineGL33_LightingShaders*.cpp,
    EngineGL33_Post*.cpp), plus the files of the families it also names
    (_FAMILY_ALSO: a post shader can paste a piece of the fragment prelude),
    from disk or from the same git revision."""
    root = repo_root()
    if spec is None:
        spec = root / default_relpath
    p = Path(spec)
    if p.exists() and p.is_file():
        fam = _family(p.name)
        files = sorted(p.parent.glob(f"{fam}*.cpp")) if fam else [p]
        for other in _FAMILY_ALSO.get(fam, ()):
            files += sorted(p.parent.glob(f"{other}*.cpp"))
        return [f.read_text(encoding="utf-8", errors="replace") for f in files]
    s = str(spec)
    rev, path = s.split(":", 1) if ":" in s and not os.path.isabs(s) else (s, default_relpath)
    fam = _family(Path(path).name)
    if not fam:
        return [git_show(f"{rev}:{path}", root)]
    parent = str(Path(path).parent).replace("\\", "/")
    names = subprocess.run(["git", "ls-tree", "--name-only", f"{rev}:{parent}"], cwd=root, capture_output=True,
                           text=True).stdout.split()
    files = sorted(n for n in names if n.startswith(fam) and n.endswith(".cpp"))
    for other in _FAMILY_ALSO.get(fam, ()):
        files += sorted(n for n in names if n.startswith(other) and n.endswith(".cpp"))
    return [git_show(f"{rev}:{parent}/{n}", root) for n in files]


def _listed_names(body: str) -> list[str]:
    body = re.sub(r"//[^\n]*", "", body)
    return [n.split("::")[-1] for n in re.findall(r"[A-Za-z_][\w:]*", body)]


def assemble_programs(raw: dict[str, str], text: str) -> dict[str, str]:
    """{engine override name: source} from the raw strings of a file family and
    its text: the kFragmentPieces prelude (psRemasterCommon), the
    kScreenVertexPieces screen vertex shader (vsScreenRemaster), every
    kPostShaders row, and every raw string no table uses under its own override
    name."""
    out: dict[str, str] = {}
    used: set[str] = set()
    m = _FRAGMENT_TABLE.search(text)
    if m:
        names = _listed_names(m.group("body"))
        out["psRemasterCommon"] = "".join(raw[n] for n in names)
        used.update(names)
    m = _SCREEN_VS_TABLE.search(text)
    if m:
        names = _listed_names(m.group("body"))
        out["vsScreenRemaster"] = "".join(raw[n] for n in names)
        used.update(names)
    m = _POST_TABLE.search(text)
    if m:
        for row in _POST_ROW.finditer(m.group("body")):
            names = _listed_names(row.group("pieces"))
            out[row.group("name")] = "".join(raw[n] for n in names)
            used.update(names)
    for var, src in raw.items():
        if var not in used:
            out.setdefault(override_name(var), src)
    return out


def apply_override_names(programs: dict[str, str], override_dir: str | os.PathLike | None) -> list[str]:
    """Replace programs by <dir>/<name>.glsl files, keyed by override name."""
    if not override_dir:
        return []
    d = Path(override_dir)
    used = []
    for name in list(programs):
        f = d / f"{name}.glsl"
        if f.is_file():
            programs[name] = f.read_text(encoding="utf-8")
            used.append(name)
    return used


def load_programs(spec: str | os.PathLike | None = None, override_dir: str | os.PathLike | None = None,
                  default_relpath: str = DEFAULT_SHADER_RELPATH) -> dict[str, str]:
    """The programs the engine compiles from the file at `spec` (and its
    family), by override name, with the override files applied."""
    texts = load_family(spec, default_relpath)
    raw: dict[str, str] = {}
    for t in texts:
        raw.update(extract_glsl(t))
    programs = assemble_programs(raw, "\n".join(texts))
    apply_override_names(programs, override_dir)
    return programs


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

    # The programs the engine compiles, by override name: the remastered and
    # post sets assembled from their pieces (load_programs).
    shaders = load_programs(args.source)
    if args.shadow_depth:
        shaders.update({override_name(k): v for k, v in load_shaders(args.source, include_shadow_depth=True).items()
                        if k.startswith("shadowDepth.")})
    if args.list or not args.dump:
        print(f"# {describe_source(args.source)}")
        for name, src in shaders.items():
            first = src.strip().splitlines()[0] if src.strip() else ""
            print(f"{name:28s} {len(src):6d} bytes  {first}")
    if args.dump:
        # file names follow the engine's override convention (vsTransform.glsl,
        # psNormal.glsl, psRemasterCommon.glsl, ...), so a dumped + edited
        # directory works both as the lab's --shader-override-dir and as the
        # game's CWR_SHADER_OVERRIDE_DIR
        d = Path(args.dump)
        d.mkdir(parents=True, exist_ok=True)
        for name, src in shaders.items():
            (d / f"{name}.glsl").write_text(src, encoding="utf-8")
        print(f"wrote {len(shaders)} shaders to {d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
