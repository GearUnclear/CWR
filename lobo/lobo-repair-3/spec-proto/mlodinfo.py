"""Inspect MLOD LOD layout, trailer and texture-name counts for given pbo/member pairs (read-only)."""
import math
import struct
import sys
from collections import Counter
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402

MOD = Path(r'D:/Arma_CWA/@LoBo/addons')
paths = {p.stem.lower(): p for p in MOD.glob('*.pbo')}
for spec in sys.argv[1:]:
    stem, key = spec.split('/', 1)
    data = Archive(paths[stem].read_bytes()).read(key)
    sig, ver, count = struct.unpack_from('<4sII', data)
    print('==', spec, len(data), sig, ver, count)
    pos = 12
    for li in range(count):
        start = pos
        s, header, version, points, normals, faces, flags = struct.unpack_from('<4s6i', data, pos)
        ft = pos + header + points * 16 + normals * 12
        pos = ft + faces * 104
        assert data[pos:pos + 4] == b'TAGG', (li, data[pos:pos + 4])
        pos += 4
        while True:
            raw, size = struct.unpack_from('<64si', data, pos)
            name = raw.split(b'\0', 1)[0]
            pos += 68 + size
            if name == b'#EndOfFile#':
                break
        res = struct.unpack_from('<f', data, pos)[0]
        pos += 4
        names = Counter()
        for i in range(faces):
            f = data[ft + i * 104: ft + i * 104 + 32]
            n = f.split(b'\0', 1)[0] if b'\0' in f else b'[RAW32]' + f
            names[n] += 1
        interesting = {k.decode('latin1'): v for k, v in names.items()
                       if any(x in k.lower() for x in (b'bas_lbs', b'rba_', b'syr_crew', b'raw32', b'uh1ffarpod', b'uh1podend', b'/ba_', b'\\ba_'))}
        print(f'  LOD {li} res={res:g} faces={faces} {interesting}')
    print('  trailer', len(data) - pos, data[pos:pos + 40])
