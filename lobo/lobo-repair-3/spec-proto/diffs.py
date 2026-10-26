"""Unified diffs (LF-normalized) of every changed text member in dump/, to diffs.txt."""
import difflib
from pathlib import Path

root = Path('dump')
out = []
for before in sorted((root / 'before').rglob('*')):
    if not before.is_file() or before.suffix.lower() not in ('.sqs', '.sqf', '.cpp'):
        continue
    rel = before.relative_to(root / 'before').as_posix()
    a = before.read_bytes().decode('latin1')
    b = (root / 'after' / rel).read_bytes().decode('latin1')
    nl = 'CRLF' if '\r\n' in a else 'LF'
    out.append(f'### {rel} ({nl}, {len(a)} -> {len(b)} bytes)')
    out += difflib.unified_diff(a.replace('\r\n', '\n').split('\n'), b.replace('\r\n', '\n').split('\n'),
                                'a/' + rel, 'b/' + rel, lineterm='', n=1)
Path('diffs.txt').write_text('\n'.join(out) + '\n', encoding='utf8')
print(len(out))
