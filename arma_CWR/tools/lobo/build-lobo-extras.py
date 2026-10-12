#!/usr/bin/env python3
"""Build the optional @LoBo_Extras modfolder from the recovered 2026-10-11 LoBo downloads.

Every source archive, every extracted payload and every patched output is
hash-pinned below. Payloads are read with 7-Zip (or, for the one .ace archive,
with the acefile package run by a separate Python in an isolated venv); nothing
from the downloads is ever executed. The fold-in config patches are applied to
the pinned bytes only after each patch target has been re-verified, the pbos are
repacked without touching any other member, and the result is written as
addons/, Missions/, docs/<pack>/ and README.txt. With --against/--stock-game the
extras are class-checked against core; --check runs only that check on an
already-built folder (package-lobo-family.ps1 does, after staging). See EXTRAS.md.
"""
import argparse
from dataclasses import dataclass, field
import difflib
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from repair.pbo import Archive, normalize  # noqa: E402  (read-only reuse of the repair tool's pbo reader/writer)

BUILD_VERSION = '2026-10-11.1'
ACEFILE_VERSION = '0.6.14'
DEFAULT_SEVEN_ZIP = Path(r'C:\Program Files\7-Zip\7z.exe')
DEFAULT_TOOLS = Path(__file__).resolve().parents[2] / 'dist' / 'x64-win-rwdi' / 'PoseidonTools.exe'
MODFOLDER_NAME = '@LoBo_Extras'
# Fixed mtime for every output file (2026-10-11T00:00:00Z) so archiving the folder is reproducible.
OUTPUT_MTIME = 1791676800


class BuildError(Exception):
    """A pin, a patch precondition or a layout guard failed; nothing is published."""


# ---------------------------------------------------------------------------
# Pins
# ---------------------------------------------------------------------------

# Recovered downloads this build reads: name -> (sha256, size).
SOURCES = {
    'ofphk-175.exe': ('f8921148ad24eaa2cae50dd5495df91a14b5af5af6b2e9219b5a69444ead7a5a', 12233198),
    'HYK_USsol_152_JAM.rar': ('7355cdb5231b4c19712aaf99b6fb31f4fb344dac477d650b3a78805e3102468b', 20406037),
    'Vit_Iraq_Shilka.ace': ('fcc61d2f706b532c5d12720c4f2bc9ec315d4583eae4d34971483cda8bd17dd8', 448476),
    'baslittlebirds1_1for185.zip': ('3dab77b395250ab58d84b503a178018e76e909a4523fff771289b264755373e5', 5597722),
    'BWMOD_BasicPack_v1.56.rar': ('431b217adbed63e2fcc9e5ea0e7059a95e6385d245949306182727864020a33f', 11591481),
    'FAMAS.zip': ('336da0a3f6c89f177049b7cc652e0e9bd6551b6fef72e3acd3c2acd9e71a2a76', 372248),
}


@dataclass(frozen=True)
class Payload:
    pack: str
    archive: str
    member: str      # path inside the archive, as 7-Zip / acefile list it
    sha256: str
    size: int
    dest: str        # path inside @LoBo_Extras
    patch: str = ''  # key into PATCHES; empty means copied byte for byte


PAYLOADS = (
    Payload('hk', 'ofphk-175.exe', 'TheHKpack.pbo',
            '79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5', 31104109,
            'addons/TheHKpack.pbo', 'thehkpack'),
    Payload('hk', 'ofphk-175.exe', 'readme.htm',
            '1cd4c6bfc7a4ff54549f0e26294741c0ae77cf9ef17f3ffaa420ea731e2809e9', 47566,
            'docs/TheHKpack/readme.htm'),
    Payload('hyk', 'HYK_USsol_152_JAM.rar', 'HYK_USsol.pbo',
            'f345222f652142966622bebbeeb5b8f76d6871b26bd4df82896db34705397a22', 42263750,
            'addons/HYK_USsol.pbo', 'hyk'),
    Payload('hyk', 'HYK_USsol_152_JAM.rar', 'Missions\\HYK_Infantry.Abel.pbo',
            '208ed5e08ac1c8c36182322dae4d0705eb2ea5342455e73990303327683939a4', 3815552,
            'Missions/HYK_Infantry.Abel.pbo'),
    Payload('hyk', 'HYK_USsol_152_JAM.rar', 'Missions\\HYK_Revenge.Abel.pbo',
            '6c893f84b316cd5228984fd243eea9bc29c3b580c6081f8488437700befc2903', 389372,
            'Missions/HYK_Revenge.Abel.pbo'),
    Payload('hyk', 'HYK_USsol_152_JAM.rar', 'Readme.txt',
            'c0b9d0daca284fad7beeb4eeda1eb62f3056a8e57d804d91b278ee577723c311', 3533,
            'docs/HYK_USsol/Readme.txt'),
    Payload('hyk', 'HYK_USsol_152_JAM.rar', 'ClassNameList.txt',
            '8f2fa7c12c40084e38e524d51ca2d64645a49387273fe34be4654febb3430f40', 8606,
            'docs/HYK_USsol/ClassNameList.txt'),
    Payload('vit', 'Vit_Iraq_Shilka.ace', 'Vit_Iraq_Shilka.pbo',
            '59b425d761e87c77e4d29b9d3232a41eb565ea83920c21b6df130e2b1be41675', 867460,
            'addons/Vit_Iraq_Shilka.pbo', 'vit'),
    Payload('vit', 'Vit_Iraq_Shilka.ace', 'Vit Addons.txt',
            '650ee27374564c131b79f7cf9e9893a27fc345dde9f1c807b4aee66927e8b8d5', 276,
            'docs/Vit_Iraq_Shilka/Vit Addons.txt'),
    Payload('bas', 'baslittlebirds1_1for185.zip', 'bas_lbs.pbo',
            '72a860afb6447ec87ac3e32873e1535d099bc31a9776ca82db727bfe43dd013f', 4907557,
            'addons/bas_lbs.pbo'),
    Payload('bas', 'baslittlebirds1_1for185.zip', 'bas_lbs185.pbo',
            '1c588aa9b7689088ab79d1f05470b376e1fed56a78e63d75598df4e6f29389dc', 6123,
            'addons/bas_lbs185.pbo'),
    Payload('bas', 'baslittlebirds1_1for185.zip',
            'Documents and Settings\\jvanlint\\Desktop\\BAS Littlebird Readme.txt',
            'ddbab6bf0fa917fa2cc1b9eab267ec499b2d88eec2ab9a9716ab616d1c21acef', 18385,
            'docs/BAS_Littlebirds/BAS Littlebird Readme.txt'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'BWMOD_Config.pbo',
            'd56b327173dfec0036879e805a5426595d915458e7b6a28ebe19214fd3812c75', 47324,
            'addons/BWMOD_Config.pbo'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'BWMOD_i.pbo',
            '6f4abbcae4ec3bcaf2830d3fa0bc0f681148cce7b142964b073d2a4315ffae3e', 28728556,
            'addons/BWMOD_i.pbo'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'BWMOD_w.pbo',
            '2ba4998fa3ce7310dcf305b6e754f756907902897191a5bd4372b0d41f3ceec5', 11112338,
            'addons/BWMOD_w.pbo'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'BWMOD_Misc.pbo',
            '7aa06b882a4adda0119b62ef7e303d005fd0636887389d84524cb34db4b68f28', 555297,
            'addons/BWMOD_Misc.pbo'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'BWMOD_Scripts.pbo',
            '33badac88f9cc59b05abd2e9fc3cdca87c0a2a070ef1804186bc4dae27a07c7d', 29390,
            'addons/BWMOD_Scripts.pbo'),
    Payload('bwmod', 'BWMOD_BasicPack_v1.56.rar', 'readme_basicpack.txt',
            'ba2ad42475ac6053e7210760d735fc59650628c6e970dc29fd780d087d455c62', 12323,
            'docs/BWMOD/readme_basicpack.txt'),
    Payload('famas', 'FAMAS.zip', 'FAMAS.pbo',
            '23cbf8d259a34552622c4871abf8306d497741dc55b17833e7e256453747425c', 833683,
            'addons/FAMAS.pbo', 'famas'),
    Payload('famas', 'FAMAS.zip', 'readme.txt',
            'e22314aa5804157e4070a6a34fb105d8d2705c9115adcb959a6f8ae7ef0ff54a', 2085,
            'docs/FAMAS/readme.txt'),
)

# SHA-256 of each patched pbo. A mismatch means the patch rules or the repacker
# changed: review the change, then update the pin together with the rule.
OUTPUT_SHA256 = {
    'addons/TheHKpack.pbo': '04c08f51222493f8a0a8bb47ae96861e3ac6d4f0114a7ff08b7b3845ffc211e3',
    'addons/HYK_USsol.pbo': 'a4c623201dd4b781c775b9e27a43d6e6c52a6bb210bde8fb51b9aa46348b9752',
    'addons/Vit_Iraq_Shilka.pbo': '63cd3a10f3daf9419f76b3d832cf4089dd4a988636d32515292fed82e294aff8',
    'addons/FAMAS.pbo': 'd8628ddec0e3e2284d63bb031b23a4322d9528d0c2eb1089c38c7cfd90c2d789',
}

# The fold-in plan's never-ship list. Nothing from these archives, and no pbo with
# these names, may ever land in @LoBo_Extras.
NEVER_SHIP_ARCHIVES = frozenset({
    'OFPHKpack.zip', 'TheHKpack2.zip', 'HKmp.zip', 'HKar.zip', 'HKbo.zip', 'HKupdate.zip',
    'LoBoWeapons1.1.rar', 'LoBoTzahal1.3_2.rar', 'OPGWC Patch.rar', 'LoBoC130pack.rar',
    'idf_weapons.zip', 'AK47IraqDemo.rar', 'Galil public Beta.rar', 'LoBoDemo1.wmv',
    'LoBoDemoVideo4.wmv', 'HAYAL_READ_THIS.txt', 'OFPR_Addon_BAS_JAM2.zip',
    'BN_tracers1_25.zip', 'CoC_diver_v1.0.zip', 'CSJairlinesUpdate.rar',
})
NEVER_SHIP_MEMBERS = frozenset({
    ('baslittlebirds1_1for185.zip', 'bas_soar.pbo'),
    ('baslittlebirds1_1for185.zip', 'bas_soar185.pbo'),
})
NEVER_SHIP_PBOS = frozenset({
    'thehkpack2', 'hkmp', 'hkar', 'hkbo', 'loboweapons', 'lobotzahal', 'swi_ui', 'loboc130',
    'idf_weapons', 'ak47i', 'galil', 'jam_magazines', 'jam_sounds', 'bn_tracer', 'coc_diver',
    'csjairlines', 'bas_soar', 'bas_soar185', 'pvo.noe',
})
OUTPUT_SUFFIXES = frozenset({'.pbo', '.txt', '.htm', '.json'})

# Overlaps the clash check reports but accepts, keyed by (pbo stem, class path), lower case.
ACCEPTED_OVERLAPS = {
    ('bas_lbs185', 'cfgmodels>>helicopter'):
        'stock-shaped base-less stub; core LoBoEghelo/LoBo_FAB declare Helicopter: Air (plan: low risk)',
}


# ---------------------------------------------------------------------------
# Small helpers
# ---------------------------------------------------------------------------

def sha256(data):
    return hashlib.sha256(data).hexdigest()


def file_sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def check_pin(label, data, digest, size):
    if len(data) != size or sha256(data) != digest:
        raise BuildError(f'{label}: expected {size} B sha256 {digest}, got {len(data)} B sha256 {sha256(data)}')


def selected_payloads(include_bwmod=True):
    return [p for p in PAYLOADS if include_bwmod or p.pack != 'bwmod']


def self_check():
    """Static guards on the tables above (also run by the unit tests)."""
    dests = set()
    for p in PAYLOADS:
        if p.archive not in SOURCES:
            raise BuildError(f'{p.dest}: archive {p.archive} is not pinned')
        if p.archive in NEVER_SHIP_ARCHIVES or (p.archive, p.member) in NEVER_SHIP_MEMBERS:
            raise BuildError(f'{p.dest}: {p.archive}/{p.member} is on the never-ship list')
        if p.dest.lower() in dests:
            raise BuildError(f'duplicate destination {p.dest}')
        if p.dest.lower().endswith('.pbo') and Path(p.dest).stem.lower() in NEVER_SHIP_PBOS:
            raise BuildError(f'{p.dest}: {Path(p.dest).stem} is on the never-ship list')
        dests.add(p.dest.lower())
        if p.patch and p.patch not in PATCHES:
            raise BuildError(f'{p.dest}: unknown patch {p.patch}')
        if p.patch and not re.fullmatch(r'[0-9a-f]{64}', OUTPUT_SHA256.get(p.dest, '')):
            raise BuildError(f'{p.dest}: patched output has no sha256 pin')
    for name in SOURCES:
        if name in NEVER_SHIP_ARCHIVES:
            raise BuildError(f'{name} is on the never-ship list')


# ---------------------------------------------------------------------------
# Extraction (7-Zip, or acefile in a separate venv Python). Nothing is executed.
# ---------------------------------------------------------------------------

def run(cmd, *, cwd=None, check=True):
    result = subprocess.run([str(c) for c in cmd], cwd=cwd, capture_output=True)
    if check and result.returncode:
        raise BuildError(f'{Path(cmd[0]).name} failed ({result.returncode}): '
                         f'{result.stderr.decode(errors="replace").strip()[:2000]}')
    return result


class SevenZip:
    def __init__(self, exe):
        self.exe = Path(exe)
        if not self.exe.is_file():
            raise BuildError(f'7-Zip not found: {self.exe}')

    def listing(self, archive):
        out = run([self.exe, 'l', '-slt', '-ba', '--', archive]).stdout.decode('utf-8', 'replace')
        members = {}
        for block in out.replace('\r\n', '\n').split('\n\n'):
            fields = dict(line.split(' = ', 1) for line in block.split('\n') if ' = ' in line)
            is_dir = fields.get('Folder') == '+' or fields.get('Attributes', '').startswith('D')
            if 'Path' in fields and not is_dir:
                members[fields['Path']] = int(fields.get('Size') or 0)
        return members

    def read(self, archive, member):
        if any(c in member for c in '*?@'):
            raise BuildError(f'7-Zip would treat {member!r} as a pattern')
        return run([self.exe, 'x', '-so', '-bso0', '-bsp0', '-y', '--', archive, member]).stdout


ACE_READER = r'''
import sys, acefile
if acefile.__version__ != sys.argv[1]:
    sys.exit("acefile %s found, %s required" % (acefile.__version__, sys.argv[1]))
with acefile.open(sys.argv[2]) as archive:
    if sys.argv[3] == "--list":
        for m in archive.getmembers():
            sys.stdout.write("%d\t%s\n" % (m.size, m.filename))
    else:
        sys.stdout.buffer.write(archive.read(archive.getmember(sys.argv[3])))
'''


class AceReader:
    """Runs acefile with a venv Python in isolated mode (-I -B) from an empty directory."""

    def __init__(self, python, scratch):
        self.python = Path(python) if python else None
        self.scratch = Path(scratch)

    def _run(self, archive, arg):
        if not self.python or not self.python.is_file():
            raise BuildError('Reading .ace needs --ace-python: a venv Python with acefile '
                             f'{ACEFILE_VERSION} installed (see EXTRAS.md)')
        self.scratch.mkdir(parents=True, exist_ok=True)
        return run([self.python, '-I', '-B', '-c', ACE_READER, ACEFILE_VERSION, archive, arg],
                   cwd=self.scratch).stdout

    def listing(self, archive):
        out = self._run(archive, '--list').decode('utf-8', 'replace')
        return {name: int(size) for size, name in (line.split('\t', 1) for line in out.splitlines() if line)}

    def read(self, archive, member):
        if member == '--list':
            raise BuildError('invalid ace member name')
        return self._run(archive, member)


def extract_payloads(recovered, payloads, seven_zip, ace):
    """Verify each pinned archive and return {dest: pinned payload bytes}."""
    by_archive = {}
    for p in payloads:
        by_archive.setdefault(p.archive, []).append(p)
    out = {}
    for name in sorted(by_archive):
        path = Path(recovered) / name
        if not path.is_file():
            raise BuildError(f'missing source archive {path}')
        digest, size = SOURCES[name]
        if path.stat().st_size != size or file_sha256(path) != digest:
            raise BuildError(f'{name}: does not match its pin (sha256 {digest}); refusing to read it')
        reader = ace if name.lower().endswith('.ace') else seven_zip
        listing = reader.listing(path)
        for p in by_archive[name]:
            if listing.get(p.member) != p.size:
                raise BuildError(f'{name}: member {p.member!r} missing or wrong size in the archive listing')
            data = reader.read(path, p.member)
            check_pin(f'{name}/{p.member}', data, p.sha256, p.size)
            out[p.dest] = data
        if digest != file_sha256(path):
            raise BuildError(f'{name} changed while it was being read')
    return out


# ---------------------------------------------------------------------------
# raP (binarized OFP config) codec. Mirrors ParamFile::SerializeBin: a shared
# string table (index, then the string the first time it is used), varint
# indices from version 3 and varint counts from version 4.
# ---------------------------------------------------------------------------

RAP_MAGIC = b'\0raP'
STRING, FLOAT, INT, ARRAY = 0, 1, 2, 3


@dataclass
class RapValue:
    name: str
    kind: int      # STRING, FLOAT (raw 4 bytes kept exactly) or INT
    value: object


@dataclass
class RapArray:
    name: str
    items: list    # (kind, value); ARRAY items hold a nested list


@dataclass
class RapClass:
    name: str
    base: str
    entries: list = field(default_factory=list)


@dataclass
class RapFile:
    version: int
    root: RapClass
    variables: list  # (name, int) pairs after the class tree


class _RapReader:
    def __init__(self, data):
        self.data = data
        self.pos = 0
        self.strings = []

    def take(self, n):
        if self.pos + n > len(self.data):
            raise ValueError('truncated raP config')
        chunk = self.data[self.pos:self.pos + n]
        self.pos += n
        return chunk

    def byte(self):
        return self.take(1)[0]

    def i32(self):
        return struct.unpack('<i', self.take(4))[0]

    def cstr(self):
        end = self.data.find(b'\0', self.pos)
        if end < 0:
            raise ValueError('unterminated raP string')
        value = self.data[self.pos:end].decode('latin1')
        self.pos = end + 1
        return value

    def varint(self):
        value = shift = 0
        while True:
            c = self.byte()
            value |= (c & 0x7f) << shift
            if not c & 0x80:
                return value
            shift += 7
            if shift > 28:
                raise ValueError('malformed raP varint')

    def index(self, version, encoded_from):
        return self.varint() if version >= encoded_from else self.i32()

    def string(self, version):
        i = self.index(version, 3)
        if i < len(self.strings):
            return self.strings[i]
        if i != len(self.strings):
            raise ValueError('raP string index out of sequence')
        value = self.cstr()
        self.strings.append(value)
        return value


class _RapWriter:
    def __init__(self, version):
        self.version = version
        self.out = bytearray()
        self.strings = {}

    def i32(self, value):
        self.out += struct.pack('<i', value)

    def varint(self, value):
        while True:
            c = value & 0x7f
            value >>= 7
            if value:
                self.out.append(c | 0x80)
            else:
                self.out.append(c)
                return

    def index(self, value, encoded_from):
        if self.version >= encoded_from:
            self.varint(value)
        else:
            self.i32(value)

    def cstr(self, value):
        raw = value.encode('latin1')
        if b'\0' in raw:
            raise ValueError('raP strings cannot contain NUL')
        self.out += raw + b'\0'

    def string(self, value):
        if value in self.strings:
            self.index(self.strings[value], 3)
        else:
            self.strings[value] = len(self.strings)
            self.index(self.strings[value], 3)
            self.cstr(value)


def _read_items(r, version):
    items = []
    for _ in range(r.index(version, 4)):
        kind = r.byte()
        if kind == STRING:
            items.append((kind, r.string(version)))
        elif kind == FLOAT:
            items.append((kind, r.take(4)))
        elif kind == INT:
            items.append((kind, r.i32()))
        elif kind == ARRAY:
            items.append((kind, _read_items(r, version)))
        else:
            raise ValueError(f'unknown raP array item type {kind}')
    return items


def _read_class(r, version):
    cls = RapClass(r.string(version), r.cstr())
    for _ in range(r.index(version, 4)):
        kind = r.byte()
        if kind == 0:
            cls.entries.append(_read_class(r, version))
        elif kind == 1:
            vtype = r.byte()
            name = r.string(version)
            if vtype == STRING:
                cls.entries.append(RapValue(name, vtype, r.string(version)))
            elif vtype == FLOAT:
                cls.entries.append(RapValue(name, vtype, r.take(4)))
            elif vtype == INT:
                cls.entries.append(RapValue(name, vtype, r.i32()))
            else:
                raise ValueError(f'unknown raP value type {vtype}')
        elif kind == 2:
            name = r.string(version)
            cls.entries.append(RapArray(name, _read_items(r, version)))
        else:
            raise ValueError(f'unknown raP entry type {kind}')
    return cls


def parse_rap(data):
    r = _RapReader(bytes(data))
    if r.take(4) != RAP_MAGIC:
        raise ValueError('not a raP config')
    version = r.i32()
    if version < 2:
        raise ValueError(f'unsupported raP version {version}')
    root = _read_class(r, version)
    variables = [(r.cstr(), r.i32()) for _ in range(r.i32())]
    if r.pos != len(r.data):
        raise ValueError('trailing bytes after raP config')
    return RapFile(version, root, variables)


def _write_items(w, items):
    w.index(len(items), 4)
    for kind, value in items:
        w.out.append(kind)
        if kind == STRING:
            w.string(value)
        elif kind == FLOAT:
            w.out += value
        elif kind == INT:
            w.i32(value)
        else:
            _write_items(w, value)


def _write_class(w, cls):
    w.string(cls.name)
    w.cstr(cls.base)
    w.index(len(cls.entries), 4)
    for e in cls.entries:
        if isinstance(e, RapClass):
            w.out.append(0)
            _write_class(w, e)
        elif isinstance(e, RapArray):
            w.out.append(2)
            w.string(e.name)
            _write_items(w, e.items)
        else:
            w.out.append(1)
            w.out.append(e.kind)
            w.string(e.name)
            if e.kind == STRING:
                w.string(e.value)
            elif e.kind == FLOAT:
                if len(e.value) != 4:
                    raise ValueError('raP float must keep its 4 raw bytes')
                w.out += e.value
            else:
                w.i32(e.value)


def write_rap(rap):
    w = _RapWriter(rap.version)
    w.out += RAP_MAGIC
    w.i32(rap.version)
    _write_class(w, rap.root)
    w.i32(len(rap.variables))
    for name, value in rap.variables:
        w.cstr(name)
        w.i32(value)
    return bytes(w.out)


def rap_find(cls, name, kind=None):
    hits = [e for e in cls.entries if e.name.lower() == name.lower() and (kind is None or isinstance(e, kind))]
    if len(hits) != 1:
        raise BuildError(f'expected exactly one {name!r} in class {cls.name!r}, found {len(hits)}')
    return hits[0]


def rap_class(cls, path):
    for part in path:
        cls = rap_find(cls, part, RapClass)
    return cls


def rap_walk(cls, path=()):
    """Yield (path, class) for every class below cls, depth first."""
    for e in cls.entries:
        if isinstance(e, RapClass):
            yield path + (e.name,), e
            yield from rap_walk(e, path + (e.name,))


# ---------------------------------------------------------------------------
# Text config locator. Comments, strings and preprocessor lines cannot create
# fake classes or properties; values are located for exact-match replacement.
# ---------------------------------------------------------------------------

_TOKEN = re.compile(
    r'(?P<pp>^[ \t]*#(?:[^\r\n]*\\\r?\n)*[^\r\n]*)'
    r'|(?P<lc>//[^\r\n]*)'
    r'|(?P<bc>/\*.*?\*/)'
    r'|(?P<str>"(?:""|[^"\r\n])*")'
    r'|(?P<id>[A-Za-z_]\w*)'
    r'|(?P<sym>\S)',
    re.M | re.S)


@dataclass(frozen=True)
class Tok:
    kind: str
    text: str
    start: int
    end: int


def tokenize(text):
    out = []
    for m in _TOKEN.finditer(text):
        kind = m.lastgroup
        if kind in ('pp', 'lc', 'bc'):
            continue
        if kind == 'sym' and m.group() == '"':
            raise BuildError('unterminated string in config')
        if kind == 'sym' and text.startswith('/*', m.start()):
            raise BuildError('unterminated block comment in config')
        out.append(Tok(kind, m.group(), m.start(), m.end()))
    return out


@dataclass(frozen=True)
class ClassSpan:
    path: tuple
    name_tok: int     # token index of the class name
    base_tok: int     # token index of the base name, or -1
    open_tok: int     # token index of '{'
    close_tok: int    # token index of the matching '}'


def class_spans(toks):
    """Every class with a body, with its full path. Array initialisers are skipped."""
    spans, stack = [], []   # stack entries: (path or None for a non-class brace, open index, name, base)
    i = 0
    while i < len(toks):
        t = toks[i]
        if t.kind == 'id' and t.text.lower() == 'class' and i + 1 < len(toks) and toks[i + 1].kind == 'id':
            name_i, j, base_i = i + 1, i + 2, -1
            if j < len(toks) and toks[j].text == ':' and j + 1 < len(toks) and toks[j + 1].kind == 'id':
                base_i, j = j + 1, j + 2
            if j < len(toks) and toks[j].text == '{':
                parent = next((s[0] for s in reversed(stack) if s[0] is not None), ())
                stack.append((parent + (toks[name_i].text,), j, name_i, base_i))
                i = j + 1
                continue
            i = j
            continue
        if t.text == '{':
            stack.append((None, i, -1, -1))
        elif t.text == '}':
            if not stack:
                raise BuildError('unbalanced closing brace in config')
            path, open_i, name_i, base_i = stack.pop()
            if path is not None:
                spans.append(ClassSpan(path, name_i, base_i, open_i, i))
        i += 1
    if stack:
        raise BuildError('unbalanced braces in config')
    return spans


def find_class(spans, path):
    key = tuple(p.lower() for p in path)
    hits = [s for s in spans if tuple(p.lower() for p in s.path) == key]
    if len(hits) != 1:
        raise BuildError(f'expected exactly one class {">>".join(path)}, found {len(hits)}')
    return hits[0]


def property_span(text, toks, span, key):
    """(start, end) of the direct value of key= or key[]= inside a class; None if absent."""
    found, i, depth = [], span.open_tok + 1, 0
    while i < span.close_tok:
        t = toks[i]
        if t.text == '{':
            depth += 1
        elif t.text == '}':
            depth -= 1
        elif depth == 0 and t.kind == 'id' and t.text.lower() == key.lower():
            j = i + 1
            if j + 1 < len(toks) and toks[j].text == '[' and toks[j + 1].text == ']':
                j += 2
            if j < len(toks) and toks[j].text == '=':
                start = j + 1
                k, d = start, 0
                while k < span.close_tok and not (d == 0 and toks[k].text == ';'):
                    d += {'{': 1, '}': -1}.get(toks[k].text, 0)
                    k += 1
                if k == start or k >= span.close_tok:
                    raise BuildError(f'{">>".join(span.path)}>>{key}: value is not terminated by ";"')
                found.append((toks[start].start, toks[k - 1].end))
                i = k
                continue
        i += 1
    if len(found) > 1:
        raise BuildError(f'{">>".join(span.path)}>>{key}: defined {len(found)} times')
    return found[0] if found else None


def _squash(value):
    return re.sub(r'\s+', '', value)


class TextConfig:
    """Accumulates verified, non-overlapping replacements against one original text."""

    def __init__(self, data):
        self.text = data.decode('latin1')
        self.toks = tokenize(self.text)
        self.spans = class_spans(self.toks)
        self.edits = []
        self.notes = []

    def nl(self):
        return '\r\n' if '\r\n' in self.text else '\n'

    def cls(self, path):
        return find_class(self.spans, path)

    def has_class(self, path):
        key = tuple(p.lower() for p in path)
        return any(tuple(p.lower() for p in s.path) == key for s in self.spans)

    def replace_value(self, path, key, expected, new):
        span = property_span(self.text, self.toks, self.cls(path), key)
        if span is None:
            raise BuildError(f'{">>".join(path)}>>{key} is missing')
        current = self.text[span[0]:span[1]]
        if _squash(current) != _squash(expected):
            raise BuildError(f'{">>".join(path)}>>{key} is {current!r}, expected {expected!r}')
        self.edits.append((span[0], span[1], new))
        self.notes.append(f'{">>".join(path)}>>{key}: {current.strip()} -> {new}')

    def rename_class(self, path, new_name):
        span = self.cls(path)
        tok = self.toks[span.name_tok]
        self.edits.append((tok.start, tok.end, new_name))
        self.notes.append(f'{">>".join(path)} renamed to {new_name}')

    def append(self, block, note):
        self.edits.append((len(self.text), len(self.text), block))
        self.notes.append(note)

    def identifier_count(self, name):
        lowered = name.lower()
        return sum(1 for t in self.toks if (t.kind == 'id' and t.text.lower() == lowered)
                   or (t.kind == 'str' and t.text[1:-1].lower() == lowered))

    def result(self):
        out, last = self.text, None
        for start, end, new in sorted(self.edits, reverse=True):
            if last is not None and end > last:
                raise BuildError('overlapping config edits')
            out = out[:start] + new + out[end:]
            last = start
        data = out.encode('latin1')
        reparsed = TextConfig(data)   # the result must still tokenize and balance
        if len(reparsed.spans) < len(self.spans):
            raise BuildError('patched config lost classes')
        return data


# ---------------------------------------------------------------------------
# The fold-in patches. Each takes the original member bytes and returns
# (patched bytes, human-readable notes); each refuses unless the source is in
# exactly the state the plan describes.
# ---------------------------------------------------------------------------

HK_STRING_SCOPE_CLASSES = ('HK36Cmag', 'HKG36Kcmag', 'HKSL8mag', 'HKG37mag')
HK_SCOPE_FIXES = (('scopeWeapon', 'private', 0), ('scopeMagazine', 'public', 2))


def patch_thehkpack_config(data):
    """TheHKpack 1.75 config.bin: 4 string scopes to ints, M16GrenadeLauncher stub made base-less."""
    rap = parse_rap(data)
    if write_rap(rap) != bytes(data):
        raise BuildError('TheHKpack config.bin does not round-trip byte for byte; refusing to rewrite it')
    notes = []
    weapons = rap_class(rap.root, ('CfgWeapons',))
    for name in HK_STRING_SCOPE_CLASSES:
        cls = rap_class(weapons, (name,))
        for key, before, after in HK_SCOPE_FIXES:
            entry = rap_find(cls, key, RapValue)
            if entry.kind != STRING or entry.value.lower() != before:
                raise BuildError(f'CfgWeapons>>{name}>>{key} is {entry.value!r} (type {entry.kind}), '
                                 f'expected the string "{before}"')
            entry.kind, entry.value = INT, after
            notes.append(f'CfgWeapons>>{name}>>{key}: "{before}" -> {after}')
    stub = rap_class(weapons, ('M16GrenadeLauncher',))
    if stub.base.lower() != 'default' or stub.entries:
        raise BuildError(f'CfgWeapons>>M16GrenadeLauncher is "{stub.base}" with {len(stub.entries)} entries, '
                         'expected the empty stub ": Default"')
    stub.base = ''
    notes.append('CfgWeapons>>M16GrenadeLauncher: ": Default" stub made base-less (stock shape)')
    leftovers = [('>>'.join(p), e.name) for p, c in rap_walk(rap.root) for e in c.entries
                 if isinstance(e, RapValue) and e.kind == STRING and e.name.lower().startswith('scope')]
    if leftovers:
        raise BuildError(f'unexpected string scopes remain: {leftovers}')
    out = write_rap(rap)
    if write_rap(parse_rap(out)) != out:
        raise BuildError('patched TheHKpack config.bin does not re-read cleanly')
    return out, notes


def patch_hyk_config(data):
    """HYK_USsol config.cpp: CfgTextureToMaterial>>GoggleGlass renamed (Deps CBT_Crew owns that name)."""
    cfg = TextConfig(data)
    cfg.cls(('CfgTextureToMaterial', 'GoggleGlass'))
    if cfg.has_class(('CfgTextureToMaterial', 'HYK_GoggleGlass')):
        raise BuildError('HYK_GoggleGlass already exists')
    if cfg.identifier_count('GoggleGlass') != 1:
        raise BuildError('GoggleGlass is referenced elsewhere in the config; a rename would break it')
    cfg.rename_class(('CfgTextureToMaterial', 'GoggleGlass'), 'HYK_GoggleGlass')
    return cfg.result(), cfg.notes


VIT_CREW = 'SoldierECrew'   # stock East crewman: exists on every engine, needs no core addon


def patch_vit_config(data):
    """Vit_Iraq_Shilka config.cpp: crew remapped to a real East crew, CfgPatches units[] fixed."""
    cfg = TextConfig(data)
    cfg.cls(('CfgVehicles', 'MAF_ZSU234'))
    if cfg.identifier_count('Vit_Iraq_Crew') != 1:
        raise BuildError('Vit_Iraq_Crew is referenced more than once')
    cfg.replace_value(('CfgVehicles', 'MAF_ZSU234'), 'crew', '"Vit_Iraq_Crew"', f'"{VIT_CREW}"')
    cfg.replace_value(('CfgPatches', 'Vit_Iraq_Shilka'), 'units', '{"Vit_Iraq_Shilka"}', '{"MAF_ZSU234"}')
    return cfg.result(), cfg.notes


FAMAS_UNITS = (
    ('VIPFAMAS_Officer', '{VIPFAMASd_DesOfficer}', 'VIPFAMASOfficer'),
    ('VIPFAMAS_Soldier', '{VIPFAMASd_DesSoldier}', 'VIPFAMASSoldier'),
    ('VIPFAMAS', '{VIPFAMAS}', 'VIPFAMASm203Soldier'),
)
FAMAS_MODELS = (('famas', '\\FAMAS\\FAMAS'), ('famasgranat', '\\FAMAS\\FAMASgranat'))


def patch_famas_config(data):
    """FAMAS config.cpp: CfgModels famas/famasgranat (zasleh muzzle flash) and real CfgPatches units[]."""
    cfg = TextConfig(data)
    for patch, before, unit in FAMAS_UNITS:
        cfg.cls(('CfgVehicles', unit))
        cfg.replace_value(('CfgPatches', patch), 'units', before, f'{{"{unit}"}}')
    if any(s.path[0].lower() == 'cfgmodels' for s in cfg.spans):
        raise BuildError('FAMAS already declares CfgModels; review the plan before patching')
    lowered = cfg.text.lower()
    for _, model in FAMAS_MODELS:
        if f'"{model.lower()}"' not in lowered:
            raise BuildError(f'FAMAS config no longer uses model {model}')
    nl = cfg.nl()
    block = (f'{nl}// Uslu dur! build-lobo-extras.py: give the FAMAS models the stock Weapon{nl}'
             f'// model sections, so the "zasleh" muzzle flash is only shown when firing.{nl}'
             f'class CfgModels{nl}{{{nl}'
             f'\tclass Default {{}};{nl}'
             f'\tclass Weapon: Default {{}};{nl}'
             + ''.join(f'\tclass {name}: Weapon {{}};{nl}' for name, _ in FAMAS_MODELS)
             + f'}};{nl}')
    if not cfg.text.endswith(('\n', '\r')):
        block = nl + block
    cfg.append(block, 'CfgModels: added famas: Weapon and famasgranat: Weapon')
    return cfg.result(), cfg.notes


# Read-back checks: what the engine's own parser (PoseidonTools "config bin") makes
# of each patched text config. Run whenever PoseidonTools is available.

def _array_strings(cls, name):
    entry = rap_find(cls, name, RapArray)
    if any(kind != STRING for kind, _ in entry.items):
        raise BuildError(f'{cls.name}>>{name}[] is not a list of strings')
    return [value for _, value in entry.items]


def expect_hyk(rap):
    table = rap_class(rap.root, ('CfgTextureToMaterial',))
    names = [e.name.lower() for e in table.entries if isinstance(e, RapClass)]
    if 'hyk_goggleglass' not in names or 'goggleglass' in names:
        raise BuildError(f'CfgTextureToMaterial reads back as {names}')


def expect_vit(rap):
    crew = rap_find(rap_class(rap.root, ('CfgVehicles', 'MAF_ZSU234')), 'crew', RapValue)
    if (crew.kind, crew.value) != (STRING, VIT_CREW):
        raise BuildError(f'MAF_ZSU234 crew reads back as {crew.value!r}')
    if _array_strings(rap_class(rap.root, ('CfgPatches', 'Vit_Iraq_Shilka')), 'units') != ['MAF_ZSU234']:
        raise BuildError('Vit_Iraq_Shilka units[] did not read back')


def expect_famas(rap):
    for patch, _, unit in FAMAS_UNITS:
        if _array_strings(rap_class(rap.root, ('CfgPatches', patch)), 'units') != [unit]:
            raise BuildError(f'{patch} units[] did not read back')
    for name, _ in FAMAS_MODELS:
        if rap_class(rap.root, ('CfgModels', name)).base.lower() != 'weapon':
            raise BuildError(f'CfgModels>>{name} does not derive from Weapon')


@dataclass(frozen=True)
class PatchRule:
    member: str
    func: object
    expect: object = None          # checks the engine parser's reading of a patched text config
    required_members: tuple = ()


PATCHES = {
    'thehkpack': PatchRule('config.bin', patch_thehkpack_config),
    'hyk': PatchRule('config.cpp', patch_hyk_config, expect_hyk),
    'vit': PatchRule('config.cpp', patch_vit_config, expect_vit, ('vit_iraq_shilka.p3d',)),
    'famas': PatchRule('config.cpp', patch_famas_config, expect_famas, ('famas.p3d', 'famasgranat.p3d')),
}


# ---------------------------------------------------------------------------
# PBO repacking
# ---------------------------------------------------------------------------

def repack_pbo(raw, replacements):
    """Replace existing members only; every other byte of every other member is kept.

    Pbos with a SHA-1 trailer get it recomputed (repair.pbo). Old StuffPBO
    trailers use an undocumented checksum and would be stale after an edit; the
    engine ignores anything after the data block, so they are dropped.
    """
    archive = Archive(raw)
    if not archive.stuffpbo:
        return archive.replace(replacements)
    wanted = {k.replace('\\', '/').lower(): v for k, v in replacements.items()}
    unknown = set(wanted) - archive.by_key.keys()
    if unknown:
        raise BuildError(f'cannot add members: {sorted(unknown)}')
    headers, payloads = [archive.prefix], []
    for e in archive.entries:
        if e.key in wanted:
            payload = wanted[e.key]
            fields = (0, 0, e.fields[2], e.fields[3], len(payload))
        else:
            payload, fields = archive.stored(e), e.fields
        headers += [e.name + b'\0', struct.pack('<5I', *fields)]
        payloads.append(payload)
    result = b''.join(headers + [archive.terminator] + payloads)
    after = Archive(result)
    if [e.name for e in after.entries] != [e.name for e in archive.entries]:
        raise BuildError('member order changed while repacking')
    for e in archive.entries:
        if e.key not in wanted:
            a = after.by_key[e.key]
            if a.fields != e.fields or bytes(after.stored(a)) != bytes(archive.stored(e)):
                raise BuildError(f'untouched member changed: {e.key}')
    return result


def apply_patch(payload, raw):
    rule = PATCHES[payload.patch]
    archive = Archive(raw)
    entry = archive.by_key.get(rule.member)
    if entry is None:
        raise BuildError(f'{payload.dest}: has no {rule.member}')
    if entry.fields[0] != 0:
        raise BuildError(f'{payload.dest}: {rule.member} is stored compressed; this build expects it stored')
    for member in rule.required_members:
        if member not in archive.by_key:
            raise BuildError(f'{payload.dest}: expected member {member} is missing')
    original = bytes(archive.stored(entry))
    patched, notes = rule.func(original)
    if patched == original:
        raise BuildError(f'{payload.dest}: patch changed nothing')
    out = repack_pbo(raw, {rule.member: patched})
    if bytes(Archive(out).stored(Archive(out).by_key[rule.member])) != patched:
        raise BuildError(f'{payload.dest}: repacked member differs from the patch result')
    return out, original, patched, notes


# ---------------------------------------------------------------------------
# PoseidonTools cross-checks (optional; the build runs them whenever the exe exists)
# ---------------------------------------------------------------------------

def line_changes(before, after):
    """(removed, added) stripped lines between two line lists."""
    removed, added = [], []
    for tag, a0, a1, b0, b1 in difflib.SequenceMatcher(None, before, after, autojunk=False).get_opcodes():
        if tag != 'equal':
            removed += [line.strip() for line in before[a0:a1]]
            added += [line.strip() for line in after[b0:b1]]
    return removed, added


def validate_with_tools(tools, payload, original, patched, scratch):
    """Run the engine's own config reader over the patched member."""
    scratch = Path(scratch).resolve()
    scratch.mkdir(parents=True, exist_ok=True)
    if PATCHES[payload.patch].member == 'config.bin':
        texts = []
        for label, data in (('before', original), ('after', patched)):
            src, dst = scratch / f'{label}.bin', scratch / f'{label}.cpp'
            src.write_bytes(data)
            run([tools, 'config', 'debin', src, '-o', dst])
            texts.append(dst.read_text('latin1').splitlines())
        removed, added = line_changes(*texts)
        want_removed = sorted(['scopeWeapon="private";', 'scopeMagazine="public";'] * 4
                              + ['class M16GrenadeLauncher: Default'])
        want_added = sorted(['scopeWeapon=0;', 'scopeMagazine=2;'] * 4 + ['class M16GrenadeLauncher'])
        if sorted(removed) != want_removed or sorted(added) != want_added:
            raise BuildError(f'{payload.dest}: debinned diff is not exactly the planned patch: '
                             f'-{sorted(removed)} +{sorted(added)}')
        return 'debinned diff matches the planned 9 lines'
    # "config bin" exits 0 even for unreadable input, so the output itself is checked:
    # every class the text holds must be there, and the patched values must read back.
    rap = engine_parse_text(tools, patched, scratch)
    engine_paths = {tuple(p.lower() for p in path) for path, _ in rap_walk(rap.root)}
    text_paths = {tuple(p.lower() for p in s.path) for s in TextConfig(patched).spans}
    if not text_paths <= engine_paths:
        raise BuildError(f'{payload.dest}: the engine parser lost classes {sorted(text_paths - engine_paths)[:5]}')
    PATCHES[payload.patch].expect(rap)
    return f'engine parser reads all {len(text_paths)} classes and the patched values'


def engine_parse_text(tools, text, scratch):
    scratch = Path(scratch).resolve()
    scratch.mkdir(parents=True, exist_ok=True)
    src, out = scratch / 'config.cpp', scratch / 'config.bin'
    src.write_bytes(text)
    if out.exists():
        out.unlink()
    run([tools, 'config', 'bin', src, '-o', out], cwd=scratch)
    return parse_rap(out.read_bytes())


# ---------------------------------------------------------------------------
# Class-clash check: an extras config usually merges AFTER core, so it must be
# class-clean against core, deps and UD, not only pbo-name clean (plan risk 1).
# ---------------------------------------------------------------------------

PA_DEFAULT, PA_READ_AND_CREATE, PA_READ_ONLY = -1, 1, 2   # ParamAccessMode values


@dataclass
class ClassInfo:
    display: str          # path as written, e.g. 'CfgVehicles>>MAF_ZSU234'
    base: str             # lower-case base name, '' when base-less
    values: dict          # lower-case key -> normalised value of each direct value/array entry
    access: int = PA_DEFAULT   # effective access (computed for the stock config only)


@dataclass
class ConfigIndex:
    label: str            # e.g. '@LoBo/LoBoIs.pbo'
    stem: str             # lower-case pbo stem ('' for a game config)
    patches: set          # lower-case CfgPatches class names
    classes: dict         # lower-case path tuple -> ClassInfo
    notes: list = field(default_factory=list)
    sha256: str = ''      # of the whole pbo ('' for a game config)


# Notes that mean an index is incomplete. Tolerated for core, deps, UD and stock;
# an extras config must be read in full, or its clashes would go unchecked.
INCOMPLETE_NOTES = ('not indexed', 'indexed with the tolerant text reader', 'engine parser missed')


def _norm_item(kind, value):
    if kind == ARRAY:
        return ('a', tuple(_norm_item(k, v) for k, v in value))
    if kind == FLOAT:
        return ('n', round(struct.unpack('<f', value)[0], 5))
    if kind == INT:
        return ('n', round(float(value), 5))
    try:
        return ('n', round(float(value), 5))
    except ValueError:
        return ('s', value.lower())


def _norm_entry(entry):
    if isinstance(entry, RapArray):
        return _norm_item(ARRAY, entry.items)
    return _norm_item(entry.kind, entry.value)


def _stock_access(rap):
    """Effective access per class path, following ParamClass::CheckInheritedAccess."""
    nodes = {(): rap.root}
    for path, cls in rap_walk(rap.root):
        nodes[tuple(p.lower() for p in path)] = cls

    def own(path):
        hits = [e.value for e in nodes[path].entries
                if isinstance(e, RapValue) and e.name.lower() == 'access' and e.kind == INT]
        return hits[0] if hits else PA_DEFAULT

    def base_of(path):
        name = nodes[path].base.lower()
        scope = path[:-1]
        while name:
            if scope + (name,) in nodes and scope + (name,) != path:
                return scope + (name,)
            if not scope:
                return None
            scope = scope[:-1]
        return None

    cache = {}

    def access(path, depth=0):
        if path in cache:
            return cache[path]
        if depth > 64:
            return PA_DEFAULT
        result = own(path)
        if result <= PA_DEFAULT:
            base = base_of(path)
            if base is not None and access(base, depth + 1) > PA_DEFAULT:
                result = access(base, depth + 1)
            else:
                parent = path[:-1]
                while parent:
                    if own(parent) > PA_DEFAULT:
                        result = own(parent)
                        break
                    pbase = base_of(parent)
                    if pbase is not None and access(pbase, depth + 1) > PA_DEFAULT:
                        result = access(pbase, depth + 1)
                        break
                    parent = parent[:-1]
        cache[path] = result
        return result

    return {path: access(path) for path in nodes if path}


def index_rap(label, stem, rap, with_access=False):
    classes, patches = {}, set()
    for path, cls in rap_walk(rap.root):
        key = tuple(p.lower() for p in path)
        values = {e.name.lower(): _norm_entry(e) for e in cls.entries if not isinstance(e, RapClass)}
        classes[key] = ClassInfo('>>'.join(path), cls.base.lower(), values)
        if len(key) == 2 and key[0] == 'cfgpatches':
            patches.add(key[1])
    if with_access:
        for key, access in _stock_access(rap).items():
            classes[key].access = access
    return ConfigIndex(label, stem, patches, classes)


def text_values(toks, span, child_by_class_tok):
    """Direct key=value / key[]={...} entries of a class body, as squashed source text."""
    values, i = {}, span.open_tok + 1
    while i < span.close_tok:
        t = toks[i]
        if i in child_by_class_tok:
            i = child_by_class_tok[i] + 1
            continue
        if t.kind == 'id' and t.text.lower() == 'class':
            j = i + 2
            if j + 1 < len(toks) and toks[j].text == ':':
                j += 2
            i = j + 1 if j < len(toks) and toks[j].text == ';' else j
            continue
        if t.text in (';', '}'):
            i += 1
            continue
        j, depth = i, 0
        while j < span.close_tok and not (depth == 0 and toks[j].text == ';'):
            depth += {'{': 1, '}': -1}.get(toks[j].text, 0)
            j += 1
        values[t.text.lower()] = ('t', ''.join(x.text for x in toks[i:j]).lower())
        i = j + 1
    return values


def index_text(label, stem, data):
    """Tolerant fallback when the engine parser cannot read a text config (no macro expansion)."""
    try:
        cfg = TextConfig(data)
    except BuildError as exc:
        return ConfigIndex(label, stem, set(), {}, [f'not indexed: {exc}'])
    child_by_class_tok = {s.name_tok - 1: s.close_tok for s in cfg.spans}
    classes, patches = {}, set()
    for s in cfg.spans:
        key = tuple(p.lower() for p in s.path)
        base = cfg.toks[s.base_tok].text.lower() if s.base_tok >= 0 else ''
        classes[key] = ClassInfo('>>'.join(s.path), base, text_values(cfg.toks, s, child_by_class_tok))
        if len(key) == 2 and key[0] == 'cfgpatches':
            patches.add(key[1])
    return ConfigIndex(label, stem, patches, classes, ['indexed with the tolerant text reader'])


def pbo_prefix(archive):
    raw = archive.prefix
    if not raw:
        return ''
    parts = raw[21:].split(b'\0')
    props = dict(zip(parts[0::2], parts[1::2]))
    return props.get(b'prefix', b'').decode('latin1').strip('\\')


@dataclass
class RootConfig:
    """A pbo's root config as the engine reads it."""
    kind: str               # 'bin', 'cpp' or '' (no root config)
    rap: object = None      # RapFile; None when "config bin" produced nothing readable
    text: bytes = None      # the config.cpp bytes
    tool_output: str = ''   # what "config bin" printed for a text config
    sha256: str = ''        # of the whole pbo


def read_root_config(path, tools, scratch):
    path = Path(path)
    raw = path.read_bytes()
    archive = Archive(raw)
    digest = sha256(raw)
    if 'config.bin' in archive.by_key:
        return RootConfig('bin', parse_rap(archive.read('config.bin', path, tools)), sha256=digest)
    if 'config.cpp' not in archive.by_key:
        return RootConfig('', sha256=digest)
    scratch = Path(scratch).resolve()
    scratch.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='cfg-', dir=scratch))
    try:
        # Lay the text members out under the pbo prefix so <prefix\file.hpp> includes
        # resolve. Member names are already normalised by Archive; an unsafe prefix
        # (absolute, drive letter, "..") falls back to the pbo name.
        prefix = pbo_prefix(archive)
        try:
            prefix = normalize(prefix) if prefix else path.stem
        except ValueError:
            prefix = path.stem
        base = root / prefix
        for key in archive.by_key:
            if key.endswith(('.cpp', '.hpp', '.h', '.hh', '.inc')):
                dest = base / key
                dest.parent.mkdir(parents=True, exist_ok=True)
                dest.write_bytes(archive.read(key, path, tools))
        text = (base / 'config.cpp').read_bytes()
        out = root / 'out.bin'
        result = run([tools, 'config', 'bin', base / 'config.cpp', '-o', out], cwd=root, check=False)
        output = (result.stdout + result.stderr).decode('utf-8', 'replace')
        for spelling in {str(root), str(root).replace('\\', '/')}:
            output = output.replace(spelling, '<scratch>')
        try:
            rap = parse_rap(out.read_bytes())
        except (OSError, ValueError):
            rap = None
        return RootConfig('cpp', rap, text, output, digest)
    finally:
        shutil.rmtree(root, ignore_errors=True)


def index_config(label, stem, cfg):
    if cfg.kind == 'bin':
        index = index_rap(label, stem, cfg.rap)
    elif not cfg.kind:
        index = ConfigIndex(label, stem, set(), {}, ['no root config'])
    else:
        fallback = index_text(label, stem, cfg.text)
        if cfg.rap is None:
            index = fallback
        else:
            index = index_rap(label, stem, cfg.rap)
            # "config bin" can silently drop what it cannot parse (a forward
            # declaration "class X;" loses the rest of its class); keep whatever only
            # the tolerant reader saw, and say so.
            missing = {k: v for k, v in fallback.classes.items() if k not in index.classes}
            if missing:
                index.classes.update(missing)
                index.patches |= fallback.patches
                index.notes.append(f'engine parser missed {len(missing)} class(es); added from the text reader')
    index.sha256 = cfg.sha256
    return index


def index_pbo(path, label, tools, scratch):
    return index_config(label, Path(path).stem.lower(), read_root_config(path, tools, scratch))


def addon_dir(folder):
    folder = Path(folder)
    for child in folder.iterdir() if folder.is_dir() else ():
        if child.is_dir() and child.name.lower() == 'addons':
            return child
    return None


def addon_pbos(folder):
    found = addon_dir(folder)
    if found is None:
        raise BuildError(f'{folder} has no addons folder')
    return [p for p in sorted(found.iterdir(), key=lambda p: p.name.lower())
            if p.suffix.lower() == '.pbo' and p.is_file()]


def index_folder(folder, tools, scratch, label=None):
    label = label or Path(folder).name
    out = []
    for p in addon_pbos(folder):
        try:
            out.append(index_pbo(p, f'{label}/{p.name}', tools, scratch))
        except (BuildError, ValueError, KeyError) as exc:
            # Keep the pbo name in the check and report the unreadable config.
            out.append(ConfigIndex(f'{label}/{p.name}', p.stem.lower(), set(), {}, [f'not indexed: {exc}'],
                                   file_sha256(p)))
    return out


def folder_identity(folder, indexes):
    """Name, pbo count and a digest over the (pbo name, sha256) pairs of one folder."""
    lines = ''.join(f'{i.label.rsplit("/", 1)[-1].lower()}\t{i.sha256}\n'
                    for i in sorted(indexes, key=lambda i: i.label.lower()))
    return {'name': Path(folder).name, 'pbos': len(indexes), 'sha256': sha256(lines.encode('utf8'))}


# The mod doctor's config defect classes, as the engine's own parser reports them
# for a text config ("config bin" resolves them and prints a warning).
CONFIG_DEFECT_RE = re.compile(r'(?:undefined scope keyword|malformed float literal)[^\r\n]*', re.I)
NUMBER_RE = re.compile(r'\s*[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?\s*')
MALFORMED_FLOAT_RE = re.compile(r'\s*[-+]?\d*\.\d+\.[\d.]*\s*')


def config_defects(cfg):
    """UNDEFINED_SCOPE_KEYWORD / MALFORMED_FLOAT findings in one root config.

    A text config is judged by the warnings of the engine's own parser. In a
    binarized config the old binarizer kept such values as strings, which the
    official exes read as 0: a non-numeric scope string, or a string that is a
    malformed float.
    """
    if cfg.kind == 'cpp':
        found = [m.group(0).strip() for m in CONFIG_DEFECT_RE.finditer(cfg.tool_output)]
        if cfg.rap is None:
            found.append('the engine parser produced no config')
        return found
    found = []
    if cfg.kind == 'bin':
        def strings(items):
            for kind, value in items:
                if kind == ARRAY:
                    yield from strings(value)
                elif kind == STRING:
                    yield value
        for path, cls in rap_walk(cfg.rap.root):
            where = '>>'.join(path)
            for e in cls.entries:
                if isinstance(e, RapValue) and e.kind == STRING:
                    if e.name.lower().startswith('scope') and not NUMBER_RE.fullmatch(e.value):
                        found.append(f'string scope {where}>>{e.name}="{e.value}" (reads as 0)')
                    elif MALFORMED_FLOAT_RE.fullmatch(e.value):
                        found.append(f'malformed float {where}>>{e.name}="{e.value}"')
                elif isinstance(e, RapArray):
                    found += [f'malformed float {where}>>{e.name}[]: "{v}"' for v in strings(e.items)
                              if MALFORMED_FLOAT_RE.fullmatch(v)]
    return found


def read_extras_configs(folder, tools, scratch):
    """[(pbo path, RootConfig)] for every addons/*.pbo of an extras folder; any
    pbo whose config cannot be read stops the build."""
    out = []
    for p in addon_pbos(folder):
        try:
            out.append((p, read_root_config(p, tools, scratch)))
        except (BuildError, ValueError, KeyError, OSError) as exc:
            raise BuildError(f'{p.name}: cannot read its config: {exc}') from exc
    return out


def index_extras(configs, label=MODFOLDER_NAME):
    """(indexes, problems): an extras config the engine parser did not read in
    full is a problem, since its clashes could not be checked."""
    indexes, problems = [], []
    for p, cfg in configs:
        idx = index_config(f'{label}/{p.name}', p.stem.lower(), cfg)
        problems += [f'{idx.label}: {n}' for n in idx.notes if n.startswith(INCOMPLETE_NOTES)]
        indexes.append(idx)
    return indexes, problems


def clash_check(extras_idx, against, stock_game, tools, scratch):
    """Index the other folders and run check_clashes. Returns (errors, clash dict)."""
    scratch = Path(scratch)
    scratch.mkdir(parents=True, exist_ok=True)
    others, identities = [], []
    for folder in against:
        indexes = index_folder(folder, tools, scratch)
        others += indexes
        identities.append(folder_identity(folder, indexes))
    stock = []
    if stock_game:
        # Classes the stock AddOns pbos create carry no access lock, so they are
        # checked like core; the stock CONFIG.BIN gets the access-aware checks.
        cfg = find_ci(stock_game, 'BIN', 'CONFIG.BIN')
        if cfg is None:
            raise BuildError(f'{stock_game} has no BIN/CONFIG.BIN')
        stock.append(index_rap('stock BIN/CONFIG.BIN', '', parse_rap(cfg.read_bytes()), with_access=True))
        others += index_folder(stock_game, tools, scratch, 'stock AddOns')
    errors, warnings = check_clashes(extras_idx, others, stock)
    warnings += [f'index note: {i.label}: {note}' for i in extras_idx + others
                 for note in i.notes if note != 'no root config']
    clash = {'warnings': warnings, 'checked': True,
             'against': [Path(a).name for a in against],
             'against_folders': identities,
             'stock_game': 'BIN/CONFIG.BIN + AddOns' if stock_game else '',
             'indexed_configs': len(others) + len(stock)}
    return errors, clash


def find_ci(folder, *parts):
    """Case-insensitive child lookup."""
    cur = Path(folder)
    for part in parts:
        hits = [c for c in cur.iterdir() if c.name.lower() == part.lower()] if cur.is_dir() else []
        if not hits:
            return None
        cur = hits[0]
    return cur


def _compare_values(mine, theirs):
    """(conflicting keys, uncomparable keys) between two value dicts."""
    conflicts, unknown = [], []
    for key in sorted(mine.keys() & theirs.keys()):
        a, b = mine[key], theirs[key]
        if (a[0] == 't') != (b[0] == 't'):
            unknown.append(key)
        elif a != b:
            conflicts.append(key)
    return conflicts, unknown


def _is_array(value):
    return value[0] == 'a' or (value[0] == 't' and '[]=' in value[1])


def check_clashes(extras, others, stock):
    """Return (errors, warnings) for the extras indexes.

    others: core, deps, UD and stock AddOns pbos; stock: the game's CONFIG.BIN.
    A class that both define with a differing value is an error, because whichever
    config merges last overwrites the other's value. It is only a warning when the
    extras value is one that core, deps or stock already use for that key (for
    example the editor side labels of CfgGroups>>West, on which core pbos already
    disagree), since the extras then introduce nothing new. Entries that only add
    keys merge additively. A differing base is an error, because whichever config
    merges last sets the base (that includes classes the stock AddOns pbos create,
    such as HKG3 in O.pbo, which carry no access lock); the only exceptions are a
    stock CONFIG.BIN class whose access lock refuses the rebase (a warning) and
    an overlap listed in ACCEPTED_OVERLAPS (a warning). Keys the extras add to an
    unlocked stock CONFIG.BIN class are reported as warnings.
    """
    errors, grouped = [], {}
    known = {}   # (class path, key) -> values core, deps, UD and stock AddOns use
    for o in others:
        for key, info in o.classes.items():
            for name, value in info.values.items():
                known.setdefault((key, name), set()).add(value)

    def note(kind, e, shown, detail, other):
        grouped.setdefault((kind, e.label, shown, detail), []).append(other.label)

    for e in extras:
        peers = [o for o in extras if o is not e] + others
        for o in others + stock:
            if o.stem and o.stem == e.stem:
                errors.append(f'pbo name clash: {e.label} vs {o.label}')
        for o in peers + stock:
            for name in sorted(e.patches & o.patches):
                errors.append(f'CfgPatches clash: {name} in {e.label} and {o.label}')
        for key, mine in sorted(e.classes.items()):
            if key[0] == 'cfgpatches':
                continue
            accepted = ACCEPTED_OVERLAPS.get((e.stem, '>>'.join(key)))
            st = next((o.classes[key] for o in stock if key in o.classes), None)
            stock_locked = st is not None and st.access >= PA_READ_ONLY
            for o in peers:
                theirs = o.classes.get(key)
                if theirs is None:
                    continue
                conflicts, unknown = _compare_values(mine.values, theirs.values)
                novel = [k for k in conflicts if mine.values[k] not in known.get((key, k), ())
                         and not (st and st.values.get(k) == mine.values[k])]
                if novel:
                    if accepted:
                        note('accepted', e, mine.display, f'keys {novel} differ; {accepted}', o)
                    else:
                        errors.append(f'class clash: {mine.display} keys {novel} differ in '
                                      f'{e.label} and {o.label}')
                elif conflicts:
                    note('existing-value', e, mine.display, f'keys {conflicts} differ, but the value '
                         'here is one that core or stock already use', o)
                elif unknown:
                    note('uncomparable', e, mine.display, f'keys {unknown} (text-read config)', o)
                elif mine.values and theirs.values:
                    note('additive', e, mine.display, 'both add entries, no shared key differs', o)
                if mine.base != theirs.base and not stock_locked:
                    # (a locked stock class keeps its base whatever either side declares)
                    shape = f'": {mine.base or "<none>"}" here, ": {theirs.base or "<none>"}" there'
                    if st is not None and st.base == mine.base:
                        shape += ' (the stock shape)'
                    if accepted:
                        note('accepted-base', e, mine.display, f'{shape}; {accepted}', o)
                    else:
                        errors.append(f'class rebase: {mine.display} {shape} ({e.label} vs {o.label}); '
                                      'whichever config merges last sets the base')
            for o in stock:
                theirs = o.classes.get(key)
                if theirs is None:
                    continue
                if mine.base != theirs.base:
                    msg = f'": {mine.base or "<none>"}" here, ": {theirs.base or "<none>"}" in stock'
                    if theirs.access >= PA_READ_ONLY:
                        note('stock-base-locked', e, mine.display,
                             f'{msg}; stock access {theirs.access} refuses the rebase', o)
                    else:
                        errors.append(f'stock base mismatch: {mine.display} {msg} ({e.label}); '
                                      'the unlocked stock class would be rebased')
                conflicts, _ = _compare_values(mine.values, theirs.values)
                if conflicts and theirs.access >= PA_READ_AND_CREATE:
                    note('stock-values-locked', e, mine.display,
                         f'keys {conflicts} differ; stock access {theirs.access} keeps the stock values', o)
                elif conflicts:
                    novel = [k for k in conflicts if mine.values[k] not in known.get((key, k), ())]
                    if novel:
                        errors.append(f'stock value clash: {mine.display} keys {novel} ({e.label}) '
                                      'would overwrite unlocked stock values')
                    else:
                        note('existing-value', e, mine.display, f'keys {conflicts} differ from stock, '
                             'but core already uses the value here', o)
                if theirs.access < PA_READ_ONLY:
                    # ParamClass::Update: an add-only (access 1) class still takes new
                    # arrays but refuses every scalar; below that everything merges.
                    added = [k for k in sorted(mine.values.keys() - theirs.values.keys())
                             if theirs.access < PA_READ_AND_CREATE or _is_array(mine.values[k])]
                    if added:
                        note('stock-keys-added', e, mine.display,
                             f'adds keys {added} to the stock class (stock access {theirs.access})', o)
    warnings = []
    for (kind, label, shown, detail), seen in sorted(grouped.items()):
        listed = ', '.join(seen[:3]) + (f' and {len(seen) - 3} more' if len(seen) > 3 else '')
        warnings.append(f'{kind}: {shown} in {label}: {detail} (vs {listed})')
    return errors, warnings


# ---------------------------------------------------------------------------
# README
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class PackInfo:
    title: str
    files: str
    credits: tuple
    adds: tuple


PACK_INFO = {
    'hk': PackInfo(
        'OFP: HK pack 1.75 ("beauty patched", Resistance version)', 'addons\\TheHKpack.pbo',
        ('The LoBo Team and Vipersheart, 2002. Full credits in docs\\TheHKpack\\readme.htm.',),
        ('About 30 Heckler & Koch weapons and their magazines: G3 family, G36',
         'family, G37, G8, HK21E, MG3, G11, OICW, SL8, PSG1, MSG90, G22, MP5 family,',
         'MP7 PDW, UMP, P8 and USSOCOM.',
         '84 soldiers: KSK, Fernspaeher recon, GSG9 and SEK (Resistance side), SEP/NAT',
         'terrorists and mafia (East), HK soldiers and MSG90 snipers (West).',
         'Known leftover from the original pack: 7 wound textures are missing, which',
         'only adds log lines when these soldiers are hit. The pack readme asks that',
         'the authors be contacted before the pack is distributed.')),
    'hyk': PackInfo(
        'HYK Modern U.S. Army Infantry Pack 1.52 (JAM version)',
        'addons\\HYK_USsol.pbo, Missions\\HYK_Infantry.Abel.pbo, Missions\\HYK_Revenge.Abel.pbo',
        ('Hyakushiki (models and textures), RyoGuGu and the Battle Over Hokkaido',
         'team (base body), BAS (JAM weapons). Credits in docs\\HYK_USsol.'),
        ('About 240 West US Army infantry classes in Woodland, Desert and Arctic',
         'looks, both PASGT/ALICE (1985, 1991, Ranger 1993) and IBA/MOLLE (2003),',
         'six "Ultra" weapons, and the editor group "HYK U.S. Soldiers".',
         'Two Malden demo missions: "Ambush" (HYK_Infantry.Abel, uses BAS_MAH60 from',
         '@LoBo_Deps) and "Revenge!" (HYK_Revenge.Abel).',
         'Needs JAM_Magazines, which @LoBo ships.')),
    'vit': PackInfo(
        'Shilka Iraq', 'addons\\Vit_Iraq_Shilka.pbo',
        ('Vit. Credit note in docs\\Vit_Iraq_Shilka.',),
        ('One East anti-aircraft vehicle, "Shilka Iraq" (class MAF_ZSU234), an Iraqi',
         'reskin of the ZSU-23-4 with the stock cannon. MAF_ZSU234 is also the class',
         'name of the old MAF ZSU-23-4 addon, so do not load both.')),
    'bas': PackInfo(
        'BAS A/MH-6 Littlebird 1.1 (for OFP 1.85)', 'addons\\bas_lbs.pbo, addons\\bas_lbs185.pbo',
        ('Ballistic Addon Studios, 2002. Credits in docs\\BAS_Littlebirds.',),
        ('Five West helicopters under "BAS Special Ops - Vehicles": MH-6, MH-6 Armed,',
         'AH-6J Guns, AH-6J MP and MD500D TOW. The pilots come from the newer',
         'bas_soar185 in @LoBo_Deps; the 2002 bas_soar pbos from the same download',
         'are left out on purpose.')),
    'bwmod': PackInfo(
        'BWMod Basic Pack 1.56 beta', 'addons\\BWMOD_Config.pbo, BWMOD_i.pbo, BWMOD_w.pbo, '
        'BWMOD_Misc.pbo, BWMOD_Scripts.pbo',
        ('The BWMod team, 2005. Credits in docs\\BWMOD.',),
        ('Bundeswehr infantry in Flecktarn and desert dress (officers, riflemen,',
         'medics, MG3, Panzerfaust 3, Fliegerfaust 2, engineers, G22 snipers, KSK,',
         'crews and pilots) and their weapons. BWMod soldiers show the hint',
         '"BWMOD_Server Gamelogic missing!" unless a BWMOD_Server logic is placed.',
         'The pack also tries to replace the group callsigns; the stock config should',
         'refuse that (read from the engine source, not yet tested in game).',
         'Two textures were never part of the pack: gerflag.paa and the Tiger',
         'addon\'s fenster.paa.')),
    'famas': PackInfo(
        'The FAMAS AddOn 1.00', 'addons\\FAMAS.pbo',
        ('Vipersheart, a LoBo contributor, 2002. Credits in docs\\FAMAS.',),
        ('FAMAS F1 and FAMAS with M203 (weapons VIPFAMAS and VIPFAMASm203) and three',
         'West soldiers under West > Men: Officer, Soldier and Grenadier (FAMAS).')),
}


PATCH_WHY = {
    'thehkpack': 'Why: the binarizer stored the 4 magazine scopes as the strings "private" and '
                 '"public", which read as scope 0, so these magazines were hidden in the gear '
                 'screen. The ": Default" stub would also rebase the stock M16GrenadeLauncher.',
    'hyk': 'Why: @LoBo_Deps CBT_Crew.pbo owns CfgTextureToMaterial>>GoggleGlass; whichever pack '
           'loaded last replaced the other\'s goggle lens list.',
    'vit': 'Why: the crew class Vit_Iraq_Crew was never released, so the vehicle had no crew; '
           'units[] named a class that does not exist.',
    'famas': 'Why: without a CfgModels entry the models fall back to Default, which has no '
             '"zasleh" section, so the muzzle flash showed all the time; units[] named classes '
             'that do not exist.',
}


def wrap(text, width=76, indent='  '):
    lines, line = [], ''
    for word in text.split():
        if line and len(indent + line + ' ' + word) > width:
            lines.append(indent + line)
            line = word
        else:
            line = f'{line} {word}' if line else word
    return lines + ([indent + line] if line else [])


def render_readme(packs, patch_notes, clash):
    nl = '\r\n'
    if clash.get('checked'):
        checked = ['The build checked every pack for pbo-name, CfgPatches and class clashes',
                   f'against {", ".join(clash["against"]) or "no modfolder"}'
                   + (' and the stock game config' if clash.get('stock_game') else '') + '.',
                   'Reported overlaps are listed in docs\\BUILD-MANIFEST.json.']
    else:
        checked = ['This copy was built without the class-clash check (see EXTRAS.md).']
    out = [
        'LoBo Reloaded: @LoBo_Extras (optional add-on packs)',
        '===================================================',
        '',
        'Optional third-party packs that were recovered alongside the Lost Brothers',
        'mod. None of them is needed by @LoBo or its missions. Each pack keeps its',
        'original readme under docs\\. Built by arma_CWR/tools/lobo/build-lobo-extras.py',
        f'(build {BUILD_VERSION}) from hash-pinned downloads; see docs\\BUILD-MANIFEST.json.',
        '',
        'LOAD ORDER',
        '----------',
        'List @LoBo_Extras FIRST, in front of @LoBo:',
        '',
        '  Official Remastered (PoseidonGame.exe):',
        '      PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps"',
        '  Uslu dur! (@LoBo_UD stays last):',
        '      PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps;@LoBo_UD"',
        '  Legacy 1.99 (ColdWarAssault.exe):',
        '      ColdWarAssault.exe -mod=@LoBo_Extras;@LoBo;@LoBo_Deps',
        '',
        'Because @LoBo_Extras is listed first, @LoBo and @LoBo_Deps win any pbo-name',
        'clash, so a stray same-name pbo here could never replace a repaired core',
        'file. Config classes merge in their own order (an extras config usually',
        'merges after core), so the packs here must not change any class that core',
        'already defines.',
        *checked,
        '',
        'Never put a Bin folder (or a Bin\\config.cpp) into @LoBo_Extras. To turn the',
        'extras off, drop @LoBo_Extras from the launch line or delete the folder.',
        '',
        'WHAT EACH PACK ADDS',
        '-------------------',
    ]
    for pack in packs:
        info = PACK_INFO[pack]
        out += ['', info.title, f'  Files:   {info.files}', f'  Credits: {info.credits[0]}']
        out += [f'           {line}' for line in info.credits[1:]]
        out += [f'  {line}' for line in info.adds]
    out += ['', 'PATCHES APPLIED', '---------------',
            'Small config fixes, applied by the build script to the original bytes;',
            'every other file inside each pbo is unchanged.']
    for dest, patch, notes in patch_notes:
        out += ['', dest.replace('/', '\\') + ':'] + [f'  - {n}' for n in notes]
        out += wrap(PATCH_WHY[patch])
    out += ['', 'NOT INCLUDED ON PURPOSE', '-----------------------',
            'Older or same-name builds of pbos that @LoBo and @LoBo_Deps already ship',
            '(LoBoWeapons 1.1, the LoBoC130 beta, JAM2, coc_diver 1.0, BN_tracer 1.25,',
            'the 2002 bas_soar pair, the OPGWC SWI_UI), the TheHKpack 1.0 build, the',
            'small HK packs that collide with TheHKpack or the stock G3, LoBoTzahal (it',
            'needs weapons only LoBoWeapons 1.1 has), and every executable.', '']
    text = nl.join(out)
    if '\u2014' in text or '\u2013' in text:
        raise BuildError('README must not contain em or en dashes')
    return text.encode('ascii')


# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------

def assert_layout(root):
    root = Path(root)
    stray = [c.name for c in root.iterdir() if c.name.lower() not in ('addons', 'missions', 'docs', 'readme.txt')]
    if stray:
        raise BuildError(f'unexpected top-level entries in {MODFOLDER_NAME}: {stray}')
    for path in root.rglob('*'):
        rel = path.relative_to(root)
        if path.is_dir():
            if rel.parts[0].lower() == 'bin':
                raise BuildError('@LoBo_Extras must never contain a Bin folder')
            continue
        if path.suffix.lower() not in OUTPUT_SUFFIXES:
            raise BuildError(f'unexpected file type in output: {rel}')
        if path.suffix.lower() == '.pbo' and path.stem.lower() in NEVER_SHIP_PBOS:
            raise BuildError(f'never-ship pbo in output: {rel}')
    if not (root / 'addons').is_dir() or not (root / 'README.txt').is_file():
        raise BuildError('output is missing addons/ or README.txt')


def write_tree(stage, files):
    for rel, data in sorted(files.items()):
        dest = stage / rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(data)
    for path in sorted(stage.rglob('*'), reverse=True):
        os.utime(path, (OUTPUT_MTIME, OUTPUT_MTIME))


def build(recovered, out, *, include_bwmod=True, seven_zip=DEFAULT_SEVEN_ZIP, ace_python=None,
          tools=None, against=(), stock_game=None, work=None, replace=False, extractor=None, log=print):
    self_check()
    out = Path(out)
    if out.name != MODFOLDER_NAME:
        raise BuildError(f'output folder must be named {MODFOLDER_NAME}')
    if out.exists() and any(out.iterdir()):
        marker = out / 'docs' / 'BUILD-MANIFEST.json'
        if not replace or not marker.is_file():
            raise BuildError(f'{out} exists; pass --replace to rebuild a previous build-lobo-extras output')
    work = Path(work or tempfile.mkdtemp(prefix='lobo-extras-')).resolve()
    work.mkdir(parents=True, exist_ok=True)
    tools = Path(tools) if tools else None
    if tools and not tools.is_file():
        raise BuildError(f'PoseidonTools not found: {tools}')

    payloads = selected_payloads(include_bwmod)
    if extractor is None:
        data = extract_payloads(recovered, payloads, SevenZip(seven_zip), AceReader(ace_python, work / 'ace'))
    else:
        data = extractor(payloads)
        for p in payloads:
            check_pin(p.dest, data[p.dest], p.sha256, p.size)
    log(f'extracted {len(data)} pinned payloads')

    files, patch_notes, records = {}, [], []
    for p in payloads:
        blob = data[p.dest]
        record = {'dest': p.dest, 'pack': p.pack, 'source': f'{p.archive}/{p.member}', 'source_sha256': p.sha256}
        if p.patch:
            blob, before, after, notes = apply_patch(p, blob)
            record.update({'patched_member': PATCHES[p.patch].member, 'member_before_sha256': sha256(before),
                           'member_after_sha256': sha256(after), 'patches': notes})
            if tools:
                record['tools_check'] = validate_with_tools(tools, p, before, after, work / 'validate' / p.pack)
            if OUTPUT_SHA256[p.dest] != sha256(blob):
                raise BuildError(f'{p.dest}: patched pbo sha256 {sha256(blob)} does not match its pin '
                                 f'{OUTPUT_SHA256[p.dest]}; review the rule change, then update the pin')
            patch_notes.append((p.dest, p.patch, notes))
            log(f'patched {p.dest}: {len(notes)} change(s)')
        record['sha256'] = sha256(blob)
        record['bytes'] = len(blob)
        files[p.dest] = blob
        records.append(record)

    manifest = {
        'tool': 'build-lobo-extras', 'build_version': BUILD_VERSION, 'bwmod': include_bwmod,
        'sources': {name: SOURCES[name][0] for name in sorted({p.archive for p in payloads})},
        'files': records,
    }

    if (against or stock_game) and not tools:
        raise BuildError('--against/--stock-game need PoseidonTools (--tools)')
    stage = out.parent / (out.name + '.partial')
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)
    try:
        write_tree(stage, files)
        # Every shipped addon config, read the way the engine reads it: none may
        # carry the mod doctor's config defects, patched or not (the doctor itself
        # cannot read the binarized HK config or the compressed bas_lbs185 one).
        manifest['config_scan'] = {'checked': False}
        configs = []
        if tools:
            configs = read_extras_configs(stage, tools, work / 'scan')
            defects = [f'{p.name}: {d}' for p, cfg in configs for d in config_defects(cfg)]
            if defects:
                raise BuildError('config defects in the extras:\n  ' + '\n  '.join(defects))
            manifest['config_scan'] = {
                'checked': True, 'configs': sum(1 for _, cfg in configs if cfg.kind),
                'without_root_config': [p.name for p, cfg in configs if not cfg.kind]}
            log(f'config scan: {manifest["config_scan"]["configs"]} configs, no undefined scope keyword '
                'or malformed float')
        clash = {'warnings': [], 'checked': False}
        if against or stock_game:
            extras_idx, problems = index_extras(configs)
            if problems:
                raise BuildError('the engine parser did not read every extras config, so its classes '
                                 'cannot be checked:\n  ' + '\n  '.join(problems))
            errors, clash = clash_check(extras_idx, against, stock_game, tools, work / 'index')
            for w in clash['warnings']:
                log(f'clash warning: {w}')
            if errors:
                raise BuildError('class-clash check failed:\n  ' + '\n  '.join(errors))
        manifest['clash_check'] = clash
        packs = [k for k in PACK_INFO if any(p.pack == k for p in payloads)]
        write_tree(stage, {
            'README.txt': render_readme(packs, patch_notes, clash),
            'docs/BUILD-MANIFEST.json': json.dumps(manifest, indent=2, sort_keys=True).encode('utf8') + b'\n',
        })
        assert_layout(stage)
        os.utime(stage, (OUTPUT_MTIME, OUTPUT_MTIME))
        if out.exists():
            shutil.rmtree(out)
        stage.replace(out)
        os.utime(out, (OUTPUT_MTIME, OUTPUT_MTIME))   # the folder itself too, so archives of it match
    except BaseException:
        shutil.rmtree(stage, ignore_errors=True)
        raise
    return manifest


def check_folder(folder, *, tools, against=(), stock_game=None, work=None):
    """Class-check an already-built extras folder (for package-lobo-family.ps1).

    Its addon configs are read strictly; returns a report dict whose 'errors'
    lists every clash and every config the engine parser did not read in full.
    Config defects are reported under 'config_defects' (advice, not errors).
    """
    if not tools or not Path(tools).is_file():
        raise BuildError('--check needs PoseidonTools (--tools)')
    if not against:
        raise BuildError('--check needs at least one --against folder')
    folder = Path(folder)
    own_work = work is None
    work = Path(work or tempfile.mkdtemp(prefix='lobo-extras-check-')).resolve()
    work.mkdir(parents=True, exist_ok=True)
    try:
        configs = read_extras_configs(folder, tools, work / 'scan')
        extras_idx, problems = index_extras(configs, folder.name)
        errors, clash = clash_check(extras_idx, against, stock_game, tools, work / 'index')
    finally:
        if own_work:
            shutil.rmtree(work, ignore_errors=True)
    return {'tool': 'build-lobo-extras --check', 'build_version': BUILD_VERSION, 'folder': folder.name,
            'pbos': len(configs), 'errors': problems + errors,
            'config_defects': [f'{p.name}: {d}' for p, cfg in configs for d in config_defects(cfg)],
            **clash}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--recovered', type=Path,
                        help='folder holding the recovered downloads (incoming/recovered-2026-10-11/files)')
    parser.add_argument('--out', type=Path, help=f'the {MODFOLDER_NAME} folder to create')
    parser.add_argument('--check', type=Path, metavar='FOLDER',
                        help='build nothing: class-check this already-built extras folder against --against '
                             '(and --stock-game), print a JSON report, exit 1 when it finds errors')
    parser.add_argument('--no-bwmod', action='store_true', help='leave the BWMod Basic Pack out')
    parser.add_argument('--seven-zip', type=Path, default=DEFAULT_SEVEN_ZIP)
    parser.add_argument('--ace-python', type=Path, help=f'venv Python with acefile {ACEFILE_VERSION}')
    parser.add_argument('--tools', type=Path, default=DEFAULT_TOOLS if DEFAULT_TOOLS.is_file() else None,
                        help='PoseidonTools.exe for the config cross-checks (default: the rwdi dist build)')
    parser.add_argument('--against', type=Path, action='append', default=[],
                        help='modfolder to clash-check against (repeat: @LoBo, @LoBo_Deps, @LoBo_UD)')
    parser.add_argument('--stock-game', type=Path, help='game folder whose BIN/CONFIG.BIN and AddOns are stock')
    parser.add_argument('--work', type=Path, help='scratch folder (default: a new temp folder)')
    parser.add_argument('--replace', action='store_true', help='replace a previous output of this script')
    parser.add_argument('--doctor', action='store_true', help='run "PoseidonTools mod doctor" (report only) after the build')
    parser.add_argument('--report', type=Path, help='write the build manifest JSON here as well')
    args = parser.parse_args(argv)
    if args.check:
        if args.recovered or args.out or args.no_bwmod or args.replace or args.doctor:
            parser.error('--check takes only --against, --stock-game, --tools, --work and --report')
        try:
            report = check_folder(args.check, tools=args.tools, against=args.against,
                                  stock_game=args.stock_game, work=args.work)
        except (BuildError, OSError, ValueError, KeyError) as exc:
            print(f'@LoBo_Extras check aborted: {exc}', file=sys.stderr)
            return 2
        text = json.dumps(report, indent=2, sort_keys=True) + '\n'
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(text, encoding='utf8')
        print(text, end='')
        return 1 if report['errors'] else 0
    if not args.recovered or not args.out:
        parser.error('--recovered and --out are required (or use --check)')
    try:
        manifest = build(args.recovered, args.out, include_bwmod=not args.no_bwmod, seven_zip=args.seven_zip,
                         ace_python=args.ace_python, tools=args.tools, against=args.against,
                         stock_game=args.stock_game, work=args.work, replace=args.replace,
                         log=lambda m: print(m, file=sys.stderr))
        if args.doctor:
            if not args.tools:
                raise BuildError('--doctor needs PoseidonTools (--tools)')
            result = run([args.tools, 'mod', 'doctor', args.out], check=False)
            manifest['mod_doctor'] = {'exit_code': result.returncode,
                                      'output': result.stdout.decode('utf-8', 'replace').strip()}
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n', encoding='utf8')
        print(json.dumps({'out': str(args.out), 'files': [{k: r[k] for k in ('dest', 'sha256', 'bytes')}
                                                          for r in manifest['files']],
                          'clash_check': manifest['clash_check'],
                          **({'mod_doctor': manifest['mod_doctor']} if 'mod_doctor' in manifest else {})},
                         indent=2))
        return 0
    except (BuildError, OSError, ValueError, KeyError) as exc:
        print(f'@LoBo_Extras build aborted: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
