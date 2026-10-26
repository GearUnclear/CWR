"""Read-only asset checks for issue 8: digit textures exist and look like PAA,
models carry num1..num3 selections, and which nums textures models reference."""
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
SCRIPTS = {
    "LoBoF15/scr/number.sqs": None,
    "LoBoF16/scr/number.sqs": None,
    "LoBoF16/scr/number2.sqs": None,
    "LoBoIL28/script/number.sqs": None,
    "LoBo_F4/scripts/number.sqs": None,
    "LoBo_F4/scripts/number2.sqs": None,
}
members = {}
for d in root.iterdir():
    if d.is_dir():
        for f in d.rglob("*"):
            if f.is_file():
                members[(d.name.lower() + "/" + f.relative_to(d).as_posix()).lower()] = f

for s in SCRIPTS:
    text = (root / s).read_bytes().decode("latin1")
    paths = re.findall(r'"\\([^"]+?\.paa)"', text)
    missing, bad = [], []
    for p in sorted(set(paths)):
        key = p.replace("\\", "/").lower()
        f = members.get(key)
        if f is None:
            missing.append(p)
            continue
        head = f.read_bytes()[:2]
        # PAA type tags: 0xFF01 DXT1 .. 0x4444 ARGB4444, 0x1555, 0x8080 AI88; else palette paa
        if head not in (b"\x01\xff", b"\x02\xff", b"\x03\xff", b"\x04\xff", b"\x05\xff", b"\x44\x44", b"\x55\x15", b"\x80\x80"):
            bad.append((p, head.hex()))
    print(f"{s}: {len(set(paths))} distinct textures, missing={missing}, unusual_header={bad}")

for p3d in sorted(k for k in members if k.endswith(".p3d") and k.split("/")[0] in ("lobof15", "lobof16", "loboil28", "lobo_f4")):
    data = members[p3d].read_bytes()
    magic = data[:4]
    sels = {n: (b"\0" + n.encode() + b"\0") in data.lower() for n in ("num1", "num2", "num3", "burn", "roundl")}
    nums = sorted(set(m.decode("latin1").lower() for m in re.findall(rb"[A-Za-z0-9_\\]*\\nums\\[A-Za-z0-9_]+\.pa[ac]", data, re.I)))
    print(f"{p3d}: {magic!r} sels={sels} nums_tex={nums}")
