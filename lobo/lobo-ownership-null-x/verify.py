from pathlib import Path
from docx import Document
import shutil
root=Path('D:/Arma_CWA'); work=root/'tmp/lobo-ownership-null-x'
notice='LOBO ownership is under the new creator, Null_X.'
for p in (work/'docx').glob('*.docx'):
    before=[x.text for x in Document(p).paragraphs if x.text.strip()]
    after=[x.text for x in Document(work/'roundtrip'/p.name).paragraphs if x.text.strip() and x.text != notice]
    assert before == after, p.name
    assert notice in [x.text for x in Document(work/'roundtrip'/p.name).paragraphs]
    source=work/'legacy'/p.with_suffix('.doc').name
    targets=[root/'@LoBo/docs'/source.name]
    if 'Official' in source.name: targets.append(root/'@LoBo'/source.name)
    for target in targets:
        backup=work/'backups'/target.relative_to(root)
        backup.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists(): shutil.copy2(target,backup)
        readonly=not(target.stat().st_mode & 128)
        if readonly: target.chmod(target.stat().st_mode | 128)
        target.write_bytes(source.read_bytes())
        if readonly: target.chmod(target.stat().st_mode & ~128)
        assert target.read_bytes()==source.read_bytes()
        print('Verified and saved '+str(target.relative_to(root)))
for backup in (work/'backups').rglob('*'):
    if not backup.is_file() or backup.suffix=='.doc': continue
    target=root/backup.relative_to(work/'backups')
    before=backup.read_bytes(); after=target.read_bytes()
    assert after.count(notice.encode())==1, target
    if target.suffix=='.html':
        stripped=after.replace(b'\r\n<p><b>'+notice.encode()+b'</b></p>\r\n',b'',1)
    else:
        nl=b'\r\n' if b'\r\n' in before else b'\n'
        stripped=after.replace(notice.encode()+nl+nl,b'',1)
    assert stripped==before, target
print('Verified: 14 LOBO readmes and 2 assembly-note sources; original text retained.')
