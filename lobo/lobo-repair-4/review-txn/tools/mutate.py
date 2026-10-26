import subprocess, sys, shutil, os
from pathlib import Path
root = Path('.')
muts = {
 'no-required-check': ('arma_CWR/tools/lobo/repair_lobo.py', "if missing_required:raise", "if False:raise"),
 'no-one-lod-check': ('arma_CWR/tools/lobo/repair/textures.py', "if found != 1:", "if False:"),
 'no-post': ('arma_CWR/tools/lobo/repair/tailgun.py', "return apply_rules(documents, RULES, post=POST)", "return apply_rules(documents, RULES)"),
 'no-zero-check': ('arma_CWR/tools/lobo/repair/textures.py', "if counts[old] + repaired[old] != expected:", "if expected and counts[old] + repaired[old] != expected:"),
 'trailer-loose': ('arma_CWR/tools/lobo/repair/models.py', "and not _oxygen_path_trailer(data[pos:]):", "and not (len(data[pos:]) == 32) and not _oxygen_path_trailer(data[pos:]):"),
 'outside-addons': ('arma_CWR/tools/lobo/repair_lobo.py', "if {k.split('/',1)[0] for k in required_keys}-wanted:raise", "if False:raise"),
 'unlisted-raise-off': ('arma_CWR/tools/lobo/repair/textures.py', 'raise ValueError(f"Unexpected {old!r} outside the listed LODs', 'continue\n                        raise ValueError(f"Unexpected {old!r} outside the listed LODs'),
}
env = dict(os.environ, PYTHONDONTWRITEBYTECODE='1')
for name, (f, a, b) in muts.items():
    p = root / f; orig = p.read_text(encoding='utf8')
    assert orig.count(a) == 1, (name, orig.count(a))
    p.write_text(orig.replace(a, b), encoding='utf8')
    r = subprocess.run([sys.executable, '-B', '-m', 'unittest', 'discover', '-s', 'arma_CWR/tests/ci', '-p', 'test_lobo_*.py'], capture_output=True, text=True, env=env)
    last = [l for l in r.stderr.splitlines() if l.startswith(('FAILED', 'OK', 'Ran'))]
    print(name, '->', last)
    p.write_text(orig, encoding='utf8')
