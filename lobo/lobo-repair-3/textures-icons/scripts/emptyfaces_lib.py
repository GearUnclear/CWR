"""For each model, list empty-texture faces in resolution (visual) LODs and
View-Pilot/Gunner/Cargo LODs: residue hint, area, selection membership,
neighbour textures sharing their points, and the same LOD's live textures that
share the residue leaf name.

Usage: emptyfaces.py out.json model.p3d [...]
"""
import json, math, struct, sys


def parse(data):
    sig, ver, count = struct.unpack_from('<4sII', data)
    assert sig == b'MLOD' and ver == 257
    pos = 12
    out = []
    for i in range(count):
        sig, header, version, points, normals, faces, flags = struct.unpack_from('<4s6i', data, pos)
        p0 = pos + header
        pts = [struct.unpack_from('<3f', data, p0 + k * 16) for k in range(points)]
        f0 = p0 + points * 16 + normals * 12
        fl = []
        for f in range(faces):
            off = f0 + f * 104
            raw = data[off:off + 32]
            name = raw.split(b'\0', 1)[0].decode('latin-1')
            n = struct.unpack_from('<i', data, off + 32)[0]
            vs = []
            for v in range(4):
                pi, ni, u, vv = struct.unpack_from('<iiff', data, off + 36 + v * 16)
                vs.append((pi, u, vv))
            fflags = struct.unpack_from('<i', data, off + 100)[0]
            fl.append(dict(off=off, name=name, raw=raw, n=n, vs=vs[:n], flags=fflags))
        pos = f0 + faces * 104 + 4
        sels = {}
        while True:
            raw, size = struct.unpack_from('<64si', data, pos)
            tname = raw.split(b'\0', 1)[0].decode('latin-1')
            body = data[pos + 68:pos + 68 + size]
            pos += 68 + size
            if tname == '#EndOfFile#':
                break
            if tname and tname[0] not in '#-.' and size == points + faces:
                sels[tname] = body[points:]
        res = struct.unpack_from('<f', data, pos)[0]
        pos += 4
        out.append(dict(index=i, res=res, pts=pts, faces=fl, sels=sels))
    return out


def area(pts, face):
    ps = [pts[v[0]] for v in face['vs']]
    def tri(a, b, c):
        ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
        vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
        cx, cy, cz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
        return 0.5 * math.sqrt(cx * cx + cy * cy + cz * cz)
    a = tri(ps[0], ps[1], ps[2])
    if len(ps) == 4:
        a += tri(ps[0], ps[2], ps[3])
    return a


def visual(res):
    return res < 1e4 or math.isclose(res, 1e3) or math.isclose(res, 1.1e3) or math.isclose(res, 1.2e3)


def main():
    report = {}
    for path in sys.argv[2:]:
        data = open(path, 'rb').read()
        lods = parse(data)
        model = []
        for lod in lods:
            if not visual(lod['res']):
                continue
            empties = [k for k, f in enumerate(lod['faces']) if not f['name']]
            if not empties:
                continue
            total_area = sum(area(lod['pts'], f) for f in lod['faces'])
            entries = []
            for k in empties:
                f = lod['faces'][k]
                residue = f['raw'][1:].split(b'\0', 1)[0].decode('latin-1')
                pset = {v[0] for v in f['vs']}
                neigh = {}
                for g in lod['faces']:
                    if g is f or not g['name']:
                        continue
                    if pset & {v[0] for v in g['vs']}:
                        neigh[g['name']] = neigh.get(g['name'], 0) + 1
                sel = sorted(s for s, w in lod['sels'].items() if w[k])
                uv = [(round(u, 3), round(v, 3)) for _, u, v in f['vs']]
                entries.append(dict(face=k, offset=f['off'], residue=residue, raw_first=f['raw'][:1].hex(),
                                    area=round(area(lod['pts'], f), 6), uv=uv, selections=sel,
                                    neighbours=neigh, flags=f['flags']))
            leafs = {}
            for f in lod['faces']:
                leaf = f['name'].lower().replace('\\', '/').rsplit('/', 1)[-1]
                leafs.setdefault(leaf, set()).add(f['name'])
            model.append(dict(lod=lod['index'], res=lod['res'], faces=len(lod['faces']),
                              lod_area=round(total_area, 4), empties=entries,
                              same_leaf_live={e['residue']: sorted(leafs.get(e['residue'].lower().replace('\\', '/').rsplit('/', 1)[-1], []))
                                              for e in entries}))
        report[path] = model
        print('==', path)
        for m in model:
            ar = sum(e['area'] for e in m['empties'])
            print(f"  LOD {m['lod']} res={m['res']:g} faces={m['faces']} empty={len(m['empties'])} empty_area={ar:.5f} m2 of {m['lod_area']:.3f} m2")
            for e in m['empties']:
                print(f"     face {e['face']} off={e['offset']} residue={e['residue']} area={e['area']:.5f} sel={e['selections'][:8]} neigh={e['neighbours']}")
            print('     same-leaf live textures:', m['same_leaf_live'])
    json.dump(report, open(sys.argv[1], 'w'), indent=1)


