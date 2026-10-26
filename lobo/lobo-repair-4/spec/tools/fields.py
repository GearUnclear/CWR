"""Print raw 32-byte face texture fields matching given names, per LOD, plus the model trailer."""
import collections
import hashlib
import struct
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402

TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'


def lods(data):
    _, _, count = struct.unpack_from('<4sII', data)
    pos = 12
    out = []
    for _ in range(count):
        _, header, _, points, normals, faces, _ = struct.unpack_from('<4s6i', data, pos)
        ft = pos + header + points * 16 + normals * 12
        fields = [data[ft + j * 104:ft + j * 104 + 32] for j in range(faces)]
        pos = ft + faces * 104 + 4
        while True:
            raw, size = struct.unpack_from('<64si', data, pos)
            pos += 68 + size
            if raw.split(b'\0', 1)[0] == b'#EndOfFile#':
                break
        res = struct.unpack_from('<f', data, pos)[0]
        pos += 4
        out.append((res, fields))
    return out, data[pos:]


path = Path(sys.argv[1])
archive = Archive(path.read_bytes())
needles = [n.lower().encode() for n in sys.argv[3:]]
for key in sys.argv[2].split(','):
    data = archive.read(key, path, TOOLS)
    again = archive.read(key, path, TOOLS)
    found, tail = lods(data)
    print(f'== {path.stem}/{key} bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()} '
          f'decode-stable={data == again} trailer_hex={tail.hex()}')
    for res, fields in found:
        c = collections.Counter(f for f in fields if any(n in f.split(b"\0", 1)[0].lower() for n in needles))
        for raw, n in sorted(c.items()):
            print(f'   LOD {res!r:>12} {n:4d} x {raw.hex()} {raw.split(chr(0).encode(), 1)[0].decode("latin1")}')
