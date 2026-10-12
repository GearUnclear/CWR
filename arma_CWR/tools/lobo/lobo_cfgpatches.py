#!/usr/bin/env python3
"""List the CfgPatches class names that pbo addons declare.

Used by package-lobo-family.ps1 to check that an optional modfolder
(@LoBo_Extras) declares no CfgPatches name that @LoBo, @LoBo_Deps or
@LoBo_UD already declare. Read-only: pbos are opened for reading only.
A binarized config.bin is converted to text with
"PoseidonTools config debin" through a temporary file in the system temp
folder, which is removed before exit. Nothing else is written.

The engine reads an addon's config the same way (AddonSystem::ParseAddonConfig):
config.cpp at the pbo root when present, otherwise config.bin. Only classes
with a body directly inside a top-level "class CfgPatches" count; a forward
declaration ("class X;") declares no patch.

Output (stdout, JSON):
  {"pbos": [{"path": ..., "config": "config.cpp" | "config.bin" | null,
             "cfgpatches": [...], "error": null | "..."}]}
Exit code 0 when every pbo was read, 3 when at least one could not be
(its entry carries the error), 2 on bad usage.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

VERS = 0x56657273
MAX_NAME = 1024


class PboError(Exception):
    pass


def _read_asciiz(fh):
    out = bytearray()
    while True:
        b = fh.read(1)
        if not b:
            raise PboError('truncated pbo header')
        if b == b'\0':
            return bytes(out)
        out += b
        if len(out) > MAX_NAME:
            raise PboError('pbo header name too long')


def read_header(path):
    """Returns (entries, header_end); entries are (name, method, original, size, offset)."""
    entries = []
    with open(path, 'rb') as fh:
        first = True
        while True:
            name = _read_asciiz(fh)
            raw = fh.read(20)
            if len(raw) != 20:
                raise PboError('truncated pbo header')
            method, original, _reserved, _stamp, size = struct.unpack('<5I', raw)
            if not name:
                if first and method == VERS:
                    # Product/prefix properties: key/value pairs ended by an empty key.
                    while True:
                        key = _read_asciiz(fh)
                        if not key:
                            break
                        _read_asciiz(fh)
                    first = False
                    continue
                break
            first = False
            entries.append([name, method, original, size, 0])
        offset = fh.tell()
    total = os.path.getsize(path)
    for e in entries:
        e[4] = offset
        offset += e[3]
        if offset > total:
            raise PboError('pbo payload exceeds the file size')
    return entries


def member_key(name):
    return name.decode('latin1').replace('\\', '/').lower()


def read_member(path, entry, tools):
    name, method, original, size, offset = entry
    if method == 0 and original in (0, size):
        with open(path, 'rb') as fh:
            fh.seek(offset)
            data = fh.read(size)
        if len(data) != size:
            raise PboError(f'short read of {name.decode("latin1")}')
        return data
    if tools is None:
        raise PboError(f'{name.decode("latin1")} is compressed and no --tools was given')
    result = subprocess.run([str(tools), 'pbo', 'show', str(path), name.decode('latin1')], capture_output=True)
    if result.returncode != 0:
        raise PboError(f'pbo show failed for {name.decode("latin1")}: {result.stderr.decode("latin1", "replace").strip()}')
    if original and len(result.stdout) != original:
        raise PboError(f'decoded size mismatch for {name.decode("latin1")}')
    return result.stdout


def strip_comments_and_directives(text):
    """Removes // and /* */ comments (outside strings) and preprocessor lines."""
    out = []
    i = 0
    n = len(text)
    at_line_start = True
    while i < n:
        c = text[i]
        if at_line_start and c in ' \t':
            out.append(c)
            i += 1
            continue
        if at_line_start and c == '#':
            # Skip the directive and its backslash continuations.
            while i < n:
                j = text.find('\n', i)
                if j < 0:
                    i = n
                    break
                line = text[i:j].rstrip('\r')
                i = j + 1
                if not line.endswith('\\'):
                    break
            out.append('\n')
            at_line_start = True
            continue
        at_line_start = False
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == '"':
                    if j + 1 < n and text[j + 1] == '"':
                        j += 2
                        continue
                    break
                if text[j] == '\n':
                    break
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
            continue
        if text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
            continue
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            i = n if j < 0 else j + 2
            out.append(' ')
            continue
        if c == '\n':
            at_line_start = True
        out.append(c)
        i += 1
    return ''.join(out)


def tokens(text):
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c.isspace():
            i += 1
        elif c == '"':
            j = i + 1
            while j < n:
                if text[j] == '"':
                    if j + 1 < n and text[j + 1] == '"':
                        j += 2
                        continue
                    break
                j += 1
            yield ('str', text[i:j + 1])
            i = j + 1
        elif c.isalnum() or c == '_':
            j = i
            while j < n and (text[j].isalnum() or text[j] == '_'):
                j += 1
            yield ('id', text[i:j])
            i = j
        else:
            yield ('p', c)
            i += 1


def cfgpatches_names(text):
    """Class names with a body directly inside a top-level class CfgPatches."""
    toks = list(tokens(strip_comments_and_directives(text)))
    names = []
    depth = 0
    in_patches_depth = None   # depth of the CfgPatches body while inside it
    pending_class = None      # (name, depth) after 'class NAME [: BASE]'
    i = 0
    while i < len(toks):
        kind, val = toks[i]
        if kind == 'id' and val == 'class' and i + 1 < len(toks) and toks[i + 1][0] == 'id':
            pending_class = (toks[i + 1][1], depth)
            i += 2
            if i < len(toks) and toks[i] == ('p', ':') and i + 1 < len(toks) and toks[i + 1][0] == 'id':
                i += 2
            continue
        if kind == 'p' and val == '{':
            if pending_class is not None:
                cname, cdepth = pending_class
                if in_patches_depth is None and cdepth == 0 and cname.lower() == 'cfgpatches':
                    in_patches_depth = depth + 1
                elif in_patches_depth is not None and cdepth == in_patches_depth:
                    names.append(cname)
            pending_class = None
            depth += 1
        elif kind == 'p' and val == '}':
            depth -= 1
            if in_patches_depth is not None and depth < in_patches_depth:
                in_patches_depth = None
            pending_class = None
        elif kind == 'p' and val == ';':
            pending_class = None
        i += 1
    seen = set()
    unique = []
    for name in names:
        if name.lower() not in seen:
            seen.add(name.lower())
            unique.append(name)
    return unique


def debin(tools, data, tmpdir):
    src = Path(tmpdir) / 'config.bin'
    src.write_bytes(data)
    try:
        result = subprocess.run([str(tools), 'config', 'debin', str(src)], capture_output=True)
    finally:
        src.unlink(missing_ok=True)
    if result.returncode != 0:
        raise PboError(f'config debin failed: {result.stderr.decode("latin1", "replace").strip()}')
    return result.stdout.decode('latin1')


def inspect(path, tools, tmpdir):
    entries = read_header(path)
    root = {member_key(e[0]): e for e in entries if '/' not in member_key(e[0])}
    if 'config.cpp' in root:
        text = read_member(path, root['config.cpp'], tools).decode('latin1')
        return 'config.cpp', cfgpatches_names(text)
    if 'config.bin' in root:
        if tools is None:
            raise PboError('config.bin needs --tools for debin')
        data = read_member(path, root['config.bin'], tools)
        return 'config.bin', cfgpatches_names(debin(tools, data, tmpdir))
    return None, []


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--tools', type=Path, help='PoseidonTools executable (compressed members, config debin)')
    parser.add_argument('pbo', nargs='*', help='pbo files; each is echoed back verbatim as "path"')
    args = parser.parse_args(argv)
    tmpdir = tempfile.mkdtemp(prefix='lobo-cfgpatches-')
    results = []
    failed = False
    try:
        for arg in args.pbo:
            item = {'path': arg, 'config': None, 'cfgpatches': [], 'error': None}
            try:
                item['config'], item['cfgpatches'] = inspect(Path(arg), args.tools, tmpdir)
            except (OSError, PboError, UnicodeError, struct.error) as exc:
                item['error'] = str(exc)
                failed = True
            results.append(item)
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)
    sys.stdout.write(json.dumps({'pbos': results}, indent=1) + '\n')
    return 3 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
