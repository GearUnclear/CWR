"""Read-only prototype of the proposed 2026-10-11.4 rules against the installed @LoBo bytes.

Nothing is written to @LoBo. Prints anchors, counts, before/after hashes and
idempotence. The model repoint here is a local stand-in for the proposed
textures.repoint extension (per-LOD counts, pinned trailer).
"""
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402
from repair.rules import apply_rules, p  # noqa: E402
from repair.scripts import script_findings  # noqa: E402

TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
ADDONS = Path(r'D:/Arma_CWA/@LoBo/addons')
BS = chr(92)
OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else None


def sha(b):
    return hashlib.sha256(b).hexdigest()


def member(pbo, key):
    path = ADDONS / pbo
    return Archive(path.read_bytes()).read(key, path, TOOLS)


# ---------------------------------------------------------------- text rules
TAILGUN = 'loboil28/script/tailgun.sqs'
MIG = 'lobomig/config.cpp'
PILOT = 'lobo_pilot/config.cpp'
GUNNER = '_gunner = _tunits select (count(_tunits) -1)'


def wound(folder, name):
    old = '"' + p('LoBo_Pilot', *folder, name + '.pac') + '", "' + p('LoBo_Pilot', *folder, name + '_d.paa') + '"'
    new = '"' + p('LoBo_Pilot', *folder, name + '.paa') + '", "' + p('LoBo_Pilot', *folder, name + '_d.paa') + '"'
    return ('sub', old, new, 1)


RULES = {
    TAILGUN: (
        ('line', '? count (crew _plane) ==0: goto "crtGun"', '? isNull (driver _plane): goto "crtGun"', 1),
        ('sub', GUNNER, '? count _tunits == 0: goto "crtGun"\n' + GUNNER, 1),
    ),
    MIG: (
        ('line', '\t\tsoundLandCrash[]={"' + p('LoBoMig', 'soundss', 'jet_crash.wss') + '",0.006000,1};',
         '\t\tsoundLandCrash[]={"' + p('LoBoMig', 'sounds', 'jet_crash.wss') + '",0.006000,1};', 1),
    ),
    PILOT: (
        wound(('EAF', 'tex'), 'back'),
        wound(('EAF', 'tex'), 'legsbk'),
        wound(('IAF', 'tex2'), 'legsbk'),
        wound(('RJAF', 'tex'), 'back'),
        wound(('RJAF', 'tex'), 'legsbk'),
    ),
}
docs = {
    TAILGUN: member('LoBoIL28.pbo', 'script/tailgun.sqs').decode('latin1'),
    MIG: member('LoBoMig.pbo', 'config.cpp').decode('latin1'),
    PILOT: member('LoBo_Pilot.pbo', 'config.cpp').decode('latin1'),
}
report = {}
print('findings before', TAILGUN, script_findings(docs[TAILGUN].replace('\r\n', '\n')))
changed = apply_rules(docs, RULES)
again = apply_rules({**docs, **changed}, RULES)
print('text idempotent:', again == {})
for k, v in changed.items():
    a, b = docs[k].encode('latin1'), v.encode('latin1')
    print(k, 'before', len(a), sha(a), 'after', len(b), sha(b), 'crlf kept', b.count(b'\r\n') == b.count(b'\n'))
    report[k] = {'before_sha256': sha(a), 'before_bytes': len(a), 'after_sha256': sha(b), 'after_bytes': len(b)}
    if OUT:
        (OUT / (k.replace('/', '__') + '.after')).write_bytes(b)
print('findings after', TAILGUN, script_findings(changed[TAILGUN].replace('\r\n', '\n')))
# Partially repaired states are accepted.
half = dict(docs)
half[TAILGUN] = docs[TAILGUN].replace('? count (crew _plane) ==0: goto "crtGun"', '? isNull (driver _plane): goto "crtGun"')
assert apply_rules(half, RULES)[TAILGUN] == changed[TAILGUN]
half[PILOT] = changed[PILOT].replace(BS + 'EAF' + BS + 'tex' + BS + 'back.paa"', BS + 'EAF' + BS + 'tex' + BS + 'back.pac"')
assert apply_rules(half, RULES)[PILOT] == changed[PILOT]
print('partial states ok')

# ---------------------------------------------------------------- models
FACE, FIELD = 104, 32


def lods(data, trailer=None):
    _, _, count = struct.unpack_from('<4sII', data)
    pos = 12
    out = []
    for _ in range(count):
        _, header, _, points, normals, faces, _ = struct.unpack_from('<4s6i', data, pos)
        ft = pos + header + points * 16 + normals * 12
        pos = ft + faces * FACE + 4
        while True:
            raw, size = struct.unpack_from('<64si', data, pos)
            pos += 68 + size
            if raw.split(b'\0', 1)[0] == b'#EndOfFile#':
                break
        res = struct.unpack_from('<f', data, pos)[0]
        pos += 4
        out.append((res, ft, faces))
    tail = data[pos:]
    return out, tail


def repoint(data, resolutions, mapping, trailer=None):
    found, tail = lods(data)
    ok_tail = tail == trailer if trailer is not None else (len(tail) == 32 and tail[:1] != b'\0')
    if tail and not ok_tail:
        raise ValueError('trailer')
    fixed = bytearray(data)
    for res, start, faces in found:
        idx = [i for i, r in enumerate(resolutions) if math.isclose(res, r, rel_tol=1e-6)]
        counts = {o: 0 for o in mapping}
        rep = {o: 0 for o in mapping}
        for i in range(faces):
            at = start + i * FACE
            raw = bytes(fixed[at:at + FIELD])
            name = raw.split(b'\0', 1)[0]
            for old, (new, _) in mapping.items():
                if name.lower() == old.lower():
                    if not idx:
                        raise ValueError(f'outside listed LODs {res}')
                    fixed[at:at + FIELD] = new + bytes(FIELD - len(new))
                    counts[old] += 1
                elif idx and raw == new + bytes(FIELD - len(new)):
                    rep[old] += 1
        if idx:
            for old, (new, expected) in mapping.items():
                if counts[old] + rep[old] != expected[idx[0]]:
                    raise ValueError(f'LOD {res}: {old} expected {expected[idx[0]]} found {counts[old]}+{rep[old]}')
    assert len(fixed) == len(data)
    return bytes(fixed)


def b(*parts):
    return BS.join(parts).encode('ascii')


HILUX_OLD = b('LoBoTer', 'Toy', 't', 'black_matte.pac')
HILUX_NEW = b('LoBoTer', 'Toy', 't', 'black matte.pac')
PLANS = {
    ('LoBoTer.pbo', 'lobo_terhilux.p3d'): ((3.0, 5.65), {HILUX_OLD: (HILUX_NEW, (176, 151))},
        bytes.fromhex('000000000000000000000000e03ea1000d000000f8fb1e0100b5a000b4f3c600')),
    ('LoBoTer.pbo', 'lobo_terhiluxmg.p3d'): ((3.0, 5.65), {HILUX_OLD: (HILUX_NEW, (178, 151))},
        bytes.fromhex('000000000000000000000000e03ea1000e000000787e710100b5a000b4f3c600')),
    ('LoBo_Pilot.pbo', 'lobo_epilot.p3d'): ((1.0, 5.0), {
        b('LoBo_Pilot', 'EAF', 'tex', 'back.pac'): (b('LoBo_Pilot', 'EAF', 'tex', 'back.paa'), (21, 14)),
        b('LoBo_Pilot', 'EAF', 'tex', 'legsbk.pac'): (b('LoBo_Pilot', 'EAF', 'tex', 'legsbk.paa'), (34, 0)),
    }, None),
    ('LoBo_airammo.pbo', 'lobo_aim9.p3d'): ((1e13,), {
        b('rktf5', 'swtex.paa'): (b('LoBo_airammo', 'tex', 'sw.pac'), (14,)),
    }, None),
}
for (pbo, key), (resolutions, mapping, trailer) in PLANS.items():
    data = member(pbo, key)
    fixed = repoint(data, resolutions, mapping, trailer)
    again = repoint(fixed, resolutions, mapping, trailer)
    diff = sum(1 for x, y in zip(data, fixed) if x != y)
    print(pbo, key, 'before', len(data), sha(data), 'after', sha(fixed), 'bytes differing', diff,
          'idempotent', again == fixed)
    report[pbo.split('.')[0].lower() + '/' + key] = {'before_sha256': sha(data), 'after_sha256': sha(fixed),
                                                     'bytes': len(data), 'bytes_differing': diff}
if OUT:
    (OUT / 'proto-hashes.json').write_text(json.dumps(report, indent=2) + '\n')
