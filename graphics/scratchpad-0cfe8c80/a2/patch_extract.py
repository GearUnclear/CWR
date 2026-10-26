import pathlib

p = pathlib.Path(r"D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\tests\rendering\lighting_lab\glsl_extract.py")
s = p.read_text(encoding="utf-8")

old_doc = '''`extract_glsl(text)` returns {variable name: GLSL source}. `load_source(spec)`
reads the C++ file either from disk or from a git revision, so the lab can
render the shaders of any commit:

    load_source(None)                       # working tree EngineGL33_Shaders.cpp
    load_source("path/to/file.cpp")         # any file on disk
    load_source("HEAD~3")                   # git show HEAD~3:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp
    load_source("abc123:arma_CWR/engine/PoseidonGL33/EngineGL33_ShadowDepth.cpp")

CLI:  python glsl_extract.py [--source PATH_OR_REV] [--list] [--dump DIR]
"""'''
new_doc = '''`extract_glsl(text)` returns {variable name: GLSL source}. `load_source(spec)`
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
"""'''
assert old_doc in s
s = s.replace(old_doc, new_doc)

old_cli_anchor = '''def describe_source(spec: str | os.PathLike | None) -> str:'''
new_funcs = '''# Files whose shaders are assembled from pieces spread over the family's files.
_FAMILIES = ("EngineGL33_LightingShaders", "EngineGL33_Post")
_FRAGMENT_TABLE = re.compile(r"\\bkFragmentPieces\\s*\\[\\s*\\]\\s*=\\s*\\{(?P<body>.*?)\\};", re.S)
_POST_TABLE = re.compile(r"\\bkPostShaders\\s*\\[\\s*\\]\\s*=\\s*\\{(?P<body>.*?)\\n\\};", re.S)
_POST_ROW = re.compile(r'\\{\\s*"(?P<name>\\w+)"\\s*,\\s*\\{(?P<pieces>[^{}]*)\\}\\s*\\}')


def _family(name: str) -> str | None:
    return next((f for f in _FAMILIES if name.startswith(f)), None)


def load_family(spec: str | os.PathLike | None = None, default_relpath: str = DEFAULT_SHADER_RELPATH) -> list[str]:
    """The text of the C++ file at `spec` and of the other .cpp files of its
    family in the same directory (EngineGL33_LightingShaders*.cpp,
    EngineGL33_Post*.cpp), from disk or from the same git revision."""
    root = repo_root()
    if spec is None:
        spec = root / default_relpath
    p = Path(spec)
    if p.exists() and p.is_file():
        fam = _family(p.name)
        files = sorted(p.parent.glob(f"{fam}*.cpp")) if fam else [p]
        return [f.read_text(encoding="utf-8", errors="replace") for f in files]
    s = str(spec)
    rev, path = s.split(":", 1) if ":" in s and not os.path.isabs(s) else (s, default_relpath)
    fam = _family(Path(path).name)
    if not fam:
        return [git_show(f"{rev}:{path}", root)]
    parent = str(Path(path).parent).replace("\\\\", "/")
    names = subprocess.run(["git", "ls-tree", "--name-only", f"{rev}:{parent}"], cwd=root, capture_output=True,
                           text=True).stdout.split()
    files = sorted(n for n in names if n.startswith(fam) and n.endswith(".cpp"))
    return [git_show(f"{rev}:{parent}/{n}", root) for n in files]


def _listed_names(body: str) -> list[str]:
    body = re.sub(r"//[^\\n]*", "", body)
    return [n.split("::")[-1] for n in re.findall(r"[A-Za-z_][\\w:]*", body)]


def assemble_programs(raw: dict[str, str], text: str) -> dict[str, str]:
    """{engine override name: source} from the raw strings of a file family and
    its text: the kFragmentPieces prelude (psRemasterCommon), every kPostShaders
    row, and every raw string no table uses under its own override name."""
    out: dict[str, str] = {}
    used: set[str] = set()
    m = _FRAGMENT_TABLE.search(text)
    if m:
        names = _listed_names(m.group("body"))
        out["psRemasterCommon"] = "".join(raw[n] for n in names)
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
    programs = assemble_programs(raw, "\\n".join(texts))
    apply_override_names(programs, override_dir)
    return programs


def describe_source(spec: str | os.PathLike | None) -> str:'''
assert old_cli_anchor in s
s = s.replace(old_cli_anchor, new_funcs, 1)

old_main = '''    shaders = load_shaders(args.source, include_shadow_depth=args.shadow_depth)
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
    return 0'''
new_main = '''    # The programs the engine compiles, by override name: the remastered and
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
    return 0'''
assert old_main in s
s = s.replace(old_main, new_main)
p.write_text(s, encoding="utf-8", newline="\n")
print("ok")
