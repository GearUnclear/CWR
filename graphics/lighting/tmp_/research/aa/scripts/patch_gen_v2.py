"""One-off: switch gen_tests.py to the v2 variant set (scratch build)."""
from pathlib import Path

p = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\aa\scripts\gen_tests.py")
s = p.read_text(encoding="utf-8")
if 'SET = "v2"' in s:
    print("already v2")
    raise SystemExit(0)
old_start = s.index("# (label suffix, mode, j, k)")
old_end = s.index("PAN_FRAMES = 4")
new = '''# (label suffix, mode, j, k). v2 (scratch build): k = composite variant + 10 * present variant.
SET = "v2"
if SET == "v1":
    STILLS_H = [
        ("H-classic", "classic", 0, 0), ("H-rem", "remastered", 0, 0), ("H-cov1", "remastered", 1, 0),
        ("H-cov2", "remastered", 2, 0), ("H-cov4", "remastered", 4, 0), ("H-fxaa", "remastered", 0, 1),
        ("H-cas05", "remastered", 0, 2), ("H-cas10", "remastered", 0, 4), ("H-best", "remastered", 2, 3),
    ]
    STILLS_D = [
        ("D-classic", "classic", 0, 0), ("D-rem", "remastered", 0, 0), ("D-cov1", "remastered", 1, 0),
        ("D-cov3", "remastered", 3, 0), ("D-fxaa", "remastered", 0, 1), ("D-cas05", "remastered", 0, 2),
        ("D-best", "remastered", 1, 3),
    ]
    PANS_H = [("panH-rem", "remastered", 0, 0), ("panH-best", "remastered", 2, 3)]
    PANS_D = [("panD-rem", "remastered", 0, 0), ("panD-best", "remastered", 1, 3), ("panD-fxaa", "remastered", 0, 1)]
else:
    STILLS_H = [
        ("H-classic", "classic", 0, 0),
        ("H-rem", "remastered", 0, 0),
        ("H-cov2", "remastered", 2, 0),
        ("H-cov3", "remastered", 3, 0),
        ("H-lz", "remastered", 0, 10),
        ("H-lzrcas05", "remastered", 0, 20),
        ("H-lzrcas10", "remastered", 0, 40),
        ("H-blrcas05", "remastered", 0, 30),
        ("H-compcas10", "remastered", 0, 4),
        ("H-best2", "remastered", 2, 20),
        ("H-best3", "remastered", 3, 20),
    ]
    STILLS_D = [
        ("D-classic", "classic", 0, 0),
        ("D-rem", "remastered", 0, 0),
        ("D-cov1", "remastered", 1, 0),
        ("D-cov3", "remastered", 3, 0),
        ("D-fxaa", "remastered", 0, 1),
        ("D-rcas05", "remastered", 0, 20),
        ("D-best", "remastered", 3, 21),
    ]
    PANS_H = [("panH-rem", "remastered", 0, 0), ("panH-best3", "remastered", 3, 20)]
    PANS_D = [("panD-rem", "remastered", 0, 0), ("panD-best", "remastered", 3, 21), ("panD-cov3", "remastered", 3, 0)]
'''
s = s[:old_start] + new + s[old_end:]
p.write_text(s, encoding="utf-8", newline="\n")
print("patched to v2")
