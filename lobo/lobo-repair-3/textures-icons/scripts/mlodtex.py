"""Read-only MLOD 257 / SP3X texture census.

Usage: mlodtex.py <pattern-regex> model.p3d [...]
Prints per model, per LOD (index, resolution, face count): every texture
matching the pattern with face counts, plus the full distinct texture list.
Also records the byte offset of every face texture field so a fixed-width
repoint can be specified exactly.
"""
import json, re, struct, sys, math


def lods(data):
    sig, ver, count = struct.unpack_from('<4sII', data)
    assert sig == b'MLOD' and ver == 257, (sig, ver)
    pos = 12
    out = []
    for i in range(count):
        sig, header, version, points, normals, faces, flags = struct.unpack_from('<4s6i', data, pos)
        assert sig == b'SP3X' and version == 153 and header == 28, (sig, header, version)
        face_start = pos + header + points * 16 + normals * 12
        textures = []
        for f in range(faces):
            off = face_start + f * 104
            raw = data[off:off + 32]
            name = raw.split(b'\0', 1)[0].decode('latin-1')
            textures.append((off, name, raw))
        pos = face_start + faces * 104
        assert data[pos:pos + 4] == b'TAGG'
        pos += 4
        while True:
            raw, size = struct.unpack_from('<64si', data, pos)
            name = raw.split(b'\0', 1)[0].decode('latin-1')
            pos += 68 + size
            if name == '#EndOfFile#':
                break
        res = struct.unpack_from('<f', data, pos)[0]
        pos += 4
        out.append(dict(index=i, resolution=res, points=points, faces=faces, textures=textures))
    return out, pos, len(data)


def resname(r):
    table = {1e3: 'View-Gunner', 1.1e3: 'View-Pilot', 1.2e3: 'View-Cargo', 1e13: 'Geometry',
             1e15: 'Memory', 2e15: 'LandContact', 3e15: 'Roadway', 4e15: 'Paths',
             5e15: 'Hitpoints', 6e15: 'ViewGeometry', 7e15: 'FireGeometry',
             8e15: 'ViewCargoGeometry', 9e15: 'ViewCargoFireGeometry', 1e16: 'ViewCommander',
             1.1e16: 'ViewCommanderGeometry', 1.2e16: 'ViewCommanderFireGeometry',
             1.3e16: 'ViewPilotGeometry', 1.4e16: 'ViewPilotFireGeometry',
             1.5e16: 'ViewGunnerGeometry', 1.6e16: 'ViewGunnerFireGeometry'}
    for k, v in table.items():
        if math.isclose(r, k, rel_tol=1e-5):
            return v
    if r >= 1e4 and r < 2e4:
        return f'ShadowVolume {r - 1e4:g}'
    return f'{r:g}'


def main():
    pat = re.compile(sys.argv[1], re.I)
    report = {}
    for path in sys.argv[2:]:
        data = open(path, 'rb').read()
        ls, end, size = lods(data)
        model = {'trailer_bytes': size - end, 'lods': []}
        for lod in ls:
            counts = {}
            hits = {}
            for off, name, raw in lod['textures']:
                counts[name] = counts.get(name, 0) + 1
                if pat.search(name):
                    hits.setdefault(name, []).append(off)
            model['lods'].append({
                'index': lod['index'], 'resolution': lod['resolution'],
                'kind': resname(lod['resolution']), 'faces': lod['faces'],
                'distinct_textures': len(counts),
                'hits': {k: {'faces': len(v), 'first_offset': v[0]} for k, v in hits.items()},
            })
        report[path] = model
        print('==', path, 'LODs', len(ls), 'trailer', size - end)
        for l in model['lods']:
            if l['hits']:
                for k, v in l['hits'].items():
                    print(f"   LOD {l['index']:2d} {l['kind']:<14} faces={l['faces']:5d}  {k}  x{v['faces']}")
    if len(sys.argv) > 2:
        json.dump(report, open('mlodtex-last.json', 'w'), indent=1)


if __name__ == '__main__':
    main()
