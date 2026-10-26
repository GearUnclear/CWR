"""SPEC PROTOTYPE of the two Archive extensions (not the implementation):
member additions, and same-length in-place replacement for StuffPBO archives.
"""
import hashlib
import struct

from repair.pbo import Archive, normalize


def replace(archive, replacements, additions=None):
    """replacements: {member key: bytes}; additions: {stem/member key: (stored name, bytes)},
    appended after the last member."""
    additions = additions or {}
    if archive.stuffpbo and (replacements or additions):
        if additions:
            raise ValueError("Cannot add members to a StuffPBO archive")
        out = bytearray(archive.data)
        for key, payload in replacements.items():
            e = archive.by_key[normalize(key)]
            if e.fields[0] != 0 or len(payload) != e.fields[4]:
                raise ValueError("StuffPBO replacement must be uncompressed and same-length")
            out[e.offset:e.offset + e.fields[4]] = payload
        result = bytes(out)
        after = Archive(result)
        assert after.trailer == archive.trailer and list(after.by_key) == list(archive.by_key)
        return result
    if not additions:
        return archive.replace(replacements)
    names = []
    for key, (stored, payload) in additions.items():
        if normalize(stored) != normalize(key.split('/', 1)[1]):
            raise ValueError(f"Stored name {stored} does not match {key}")
        if normalize(stored) in archive.by_key:
            raise ValueError(f"Member already exists: {stored}")
        names.append((stored.encode('latin1'), payload))
    if len({normalize(n.decode('latin1')) for n, _ in names}) != len(names):
        raise ValueError("Duplicate added member")
    base = Archive(archive.replace(replacements))
    headers = [base.prefix]
    payloads = []
    for e in base.entries:
        headers.extend((e.name + b'\0', struct.pack('<5I', *e.fields)))
        payloads.append(bytes(base.stored(e)))
    for name, payload in names:
        headers.extend((name + b'\0', struct.pack('<5I', 0, 0, 0, 0, len(payload))))
        payloads.append(payload)
    body = b''.join(headers + [base.terminator] + payloads)
    tail = b'\0' + hashlib.sha1(body).digest() if base.checksummed else base.trailer
    result = body + tail
    after = Archive(result)
    keys = list(after.by_key)
    if keys[:len(archive.by_key)] != list(archive.by_key) or keys[len(archive.by_key):] != [normalize(n.decode('latin1')) for n, _ in names]:
        raise ValueError("Member order changed")
    for e in archive.entries:
        if e.key not in {normalize(k) for k in replacements}:
            a = after.by_key[e.key]
            if e.fields != a.fields or archive.stored(e) != after.stored(a):
                raise ValueError(f"Untouched member changed: {e.key}")
    for name, payload in names:
        if bytes(after.stored(after.by_key[normalize(name.decode('latin1'))])) != payload:
            raise ValueError("Added member did not read back")
    return result
