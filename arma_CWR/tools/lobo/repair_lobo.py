#!/usr/bin/env python3
"""Report/apply reproducible issue-69 LoBo repairs without running the game."""
import argparse
import datetime
import hashlib
import importlib
import json
from pathlib import Path
import shutil
import stat
import sys
import tempfile
import uuid
from repair.pbo import Archive, normalize

REPAIR_MODULES=('ammo','scripts','models','followups','loops','mines','clusters','numbers','effects',
                'identifiers','content','weapons','textures','tailgun','references','wounds')
REPAIR_VERSION='2026-10-11.4'

def rule_fingerprint():
    base=Path(__file__).resolve().parent
    files=[Path(__file__).resolve()]+sorted((base/'repair').glob('*.py'))
    return digest(b''.join(p.relative_to(base).as_posix().encode()+b'\0'+p.read_bytes() for p in files))

def digest(data):return hashlib.sha256(data).hexdigest()

def modules():return [importlib.import_module('repair.'+name) for name in REPAIR_MODULES]

def plan(mod,tool,compatibility='ud',asset_source=None):
    rules=modules();wanted=set().union(*(set(m.ADDONS) for m in rules))
    model_keys=set().union(*(set(getattr(m,'MODEL_MEMBERS',())) for m in rules))
    added_keys=set().union(*(set(getattr(m,'ADDED_MEMBERS',())) for m in rules))
    # Members a repoint names as its target: they must exist in the archive
    # entry tables (their bytes are not read), so a reinstall that lacks one
    # fails closed instead of repointing to nothing.
    required_keys=set().union(*(set(getattr(m,'REQUIRED_MEMBERS',())) for m in rules))
    # Archive members a module generates its added members from (wounds: the
    # LoBo_Pilot textures). Their original bytes go to its add_members as
    # `sources`; they must be plain members no rule rewrites or adds.
    source_keys=set().union(*(set(getattr(m,'SOURCE_MEMBERS',())) for m in rules))
    paths={p.stem.lower():p for p in (mod/'addons').glob('*.pbo')}
    missing=wanted-paths.keys()
    if missing:raise ValueError(f'Required addons missing: {sorted(missing)}')
    if {k.split('/',1)[0] for k in added_keys}-wanted:raise ValueError('Added members outside the repaired addons')
    for name,keys in (('Required',required_keys),('Source',source_keys)):
        if any('/' not in k or normalize(k)!=k for k in keys):raise ValueError(f'{name} members must be normalized addon/member keys')
        if {k.split('/',1)[0] for k in keys}-wanted:raise ValueError(f'{name} members outside the repaired addons')
    if source_keys&(model_keys|added_keys) or any(k.endswith(('/config.cpp','.sqs','.sqf')) for k in source_keys):
        raise ValueError('Source members must be untouched archive members')
    docs={};binary={};hashes={};original={};existing_added={};entry_keys=set();sources={}
    # Read original archive bytes once per addon; no full-mod binary extraction.
    for stem in sorted(wanted):
        path=paths[stem];raw=path.read_bytes();hashes[stem]=digest(raw);archive=Archive(raw)
        entry_keys|={stem+'/'+key for key in archive.by_key}
        for key,entry in archive.by_key.items():
            full=stem+'/'+key
            if key=='config.cpp' or key.endswith(('.sqs','.sqf')):
                data=archive.read(key,path,tool);docs[full]=data.decode('latin1');original[full]=data
            elif full in model_keys:
                data=archive.read(key,path,tool);binary[full]=data;original[full]=data
            elif full in added_keys:
                existing_added[full]=archive.read(key,path,tool)
            elif full in source_keys:
                sources[full]=archive.read(key,path,tool)
    missing_models=model_keys-binary.keys()
    if missing_models:raise ValueError(f'Required models missing: {sorted(missing_models)}')
    missing_required=required_keys-entry_keys
    if missing_required:raise ValueError(f'Required members missing: {sorted(missing_required)}')
    missing_sources=source_keys-sources.keys()
    if missing_sources:raise ValueError(f'Source members missing: {sorted(missing_sources)}')
    # New members come only from a module's declared ADDED_MEMBERS and a source
    # the module verifies itself (pinned hashes); a skip line is reported, not
    # fatal. They are planned first because a text rule may depend on whether
    # its target member will exist (weapons: the MSG90 picture path).
    skipped=[];added={};present=set(existing_added)
    for module in rules:
        if not hasattr(module,'add_members'):continue
        own=set(getattr(module,'ADDED_MEMBERS',()))
        mine={k:v for k,v in existing_added.items() if k in own}
        # A module that declares SOURCE_MEMBERS also receives their bytes.
        extra={'sources':{k:sources[k] for k in module.SOURCE_MEMBERS}} if hasattr(module,'SOURCE_MEMBERS') else {}
        additions,skips=module.add_members(dict(mine),asset_source,**extra)
        if set(additions)-own:raise ValueError(f'{module.__name__} tried to add an undeclared member')
        if set(additions)&(mine.keys()|original.keys()|added.keys()):raise ValueError(f'{module.__name__} tried to add an existing member')
        again,skips_again=module.add_members(mine|{k:v for k,(_,v) in additions.items()},asset_source,**extra)
        if again or (skips_again if additions else skips_again!=skips):raise ValueError(f'Non-idempotent member additions: {module.__name__}')
        for key,(stored,data) in additions.items():
            if not isinstance(data,bytes) or normalize(stored)!=key.split('/',1)[1]:raise ValueError(f'Invalid added member {key}')
        added.update(additions);present|=set(additions);skipped+=skips
    def options(module):
        if module.__name__=='repair.scripts':return {'compatibility':compatibility}
        if module.__name__=='repair.weapons':return {'present':frozenset(present)}
        return {}
    for module in rules:
        edits=module.repair(dict(docs),**options(module))
        if set(edits)-docs.keys():raise ValueError(f'{module.__name__} tried to add an unknown text member')
        docs.update(edits)
        if hasattr(module,'repair_models'):
            edits=module.repair_models(dict(binary))
            if set(edits)-binary.keys():raise ValueError('Unknown model replacement')
            binary.update(edits)
    # Require a stable fixed point across all rules, including overlapping addons.
    for module in rules:
        edits=module.repair(dict(docs),**options(module))
        if any(docs.get(k)!=v for k,v in edits.items()):raise ValueError(f'Non-idempotent rules: {module.__name__}')
        if hasattr(module,'repair_models'):
            edits=module.repair_models(dict(binary))
            if any(binary.get(k)!=v for k,v in edits.items()):raise ValueError('Non-idempotent model rules')
    changed={k:v.encode('latin1') for k,v in docs.items() if v.encode('latin1')!=original[k]}
    changed.update({k:v for k,v in binary.items() if v!=original[k]})
    records=[{'member':k,'before_sha256':digest(original[k]),'after_sha256':digest(v),
              'before_bytes':len(original[k]),'after_bytes':len(v)} for k,v in sorted(changed.items())]
    for key,(stored,data) in added.items():
        changed[key]=data
        records.append({'member':key,'added':True,'stored_name':stored,'before_sha256':None,'before_bytes':0,
                        'after_sha256':digest(data),'after_bytes':len(data)})
    return paths,hashes,changed,records,docs,skipped

def replace_file(source,target):
    """Path.replace that keeps the target's permission bits, including a read-only one.

    Several original LoBo archives carry the Windows read-only attribute, which
    makes os.replace onto them fail. Lift it only for the rename and put the
    target's original mode back on the replacing file (or on the target when the
    rename fails). The rename is the commit point: once it succeeded this never
    raises, so the caller always records the replaced (or restored) archive. A
    failure to put the attribute back is returned as a warning string instead.
    """
    mode=target.stat().st_mode
    readonly=not mode&stat.S_IWRITE
    if readonly:target.chmod(mode|stat.S_IWRITE)
    try:source.replace(target)
    except BaseException:
        if readonly:
            try:target.chmod(mode)
            except OSError:pass
        raise
    if readonly:
        try:target.chmod(mode)
        except OSError as exc:return f'{target}: replaced, but the read-only attribute was not restored: {exc}'
    return None

def apply(mod,paths,hashes,changed,records,compatibility='ud'):
    # Added members (record 'added') are appended to their archive; restoring a
    # batch's original archives removes them again.
    added={r['member']:r['stored_name'] for r in records if r.get('added')}
    grouped={}
    for full,data in changed.items():
        stem,key=full.split('/',1);edits=grouped.setdefault(stem,({},{}))
        if full in added:edits[1][added[full]]=data
        else:edits[0][key]=data
    if not grouped:return None
    # Stage and verify EVERY replacement before creating backups or replacing files.
    with tempfile.TemporaryDirectory(prefix='lobo-repair-',dir=mod) as stage_dir:
        stage=Path(stage_dir);new_hashes={}
        for stem,(edits,additions) in grouped.items():
            raw=paths[stem].read_bytes()
            if digest(raw)!=hashes[stem]:raise ValueError(f'Archive changed since preflight: {paths[stem]}')
            result=Archive(raw).replace(edits,additions)
            (stage/paths[stem].name).write_bytes(result);new_hashes[stem]=digest(result)
        stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ')+'-'+uuid.uuid4().hex[:8]
        backup=mod/'.ud-backups'/'issue-69'/stamp;backup.mkdir(parents=True)
        manifest={'schema':1,'issue':'https://github.com/GearUnclear/lobo/issues/1','state':'prepared',
                  'repair_version':REPAIR_VERSION,'rule_sha256':rule_fingerprint(),'compatibility':compatibility,
                  'created_utc':stamp,'archives':[],'members':records}
        for stem in grouped:
            if digest(paths[stem].read_bytes())!=hashes[stem]:raise ValueError(f'Concurrent edit: {paths[stem]}')
            target=backup/paths[stem].name;shutil.copy2(paths[stem],target)
            if digest(target.read_bytes())!=hashes[stem]:raise ValueError('Backup verification failed')
            manifest['archives'].append({'file':paths[stem].name,'before_sha256':hashes[stem],
                                         'after_sha256':new_hashes[stem]})
        manifest_path=backup/'manifest.json'
        manifest_path.write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
        replaced=[];attribute_warnings=[]
        def replaced_with(source,target):
            # The archive counts as replaced as soon as the rename succeeded.
            warning=replace_file(source,target)
            if warning:
                attribute_warnings.append(warning);print(f'WARN: {warning}',file=sys.stderr)
        try:
            for stem in grouped:
                target=paths[stem]
                if digest(target.read_bytes())!=hashes[stem]:raise ValueError(f'Concurrent edit: {target}')
                replaced_with(stage/target.name,target);replaced.append(stem)
            manifest['state']='applied'
            if attribute_warnings:manifest['attribute_warnings']=attribute_warnings
            manifest_path.write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
        except BaseException:
            conflicts=[];restore_errors=[]
            for stem in reversed(replaced):
                try:
                    if not paths[stem].exists() or digest(paths[stem].read_bytes())!=new_hashes[stem]:
                        conflicts.append(str(paths[stem]));continue
                    restore=stage/(paths[stem].name+'.restore');shutil.copy2(backup/paths[stem].name,restore);replaced_with(restore,paths[stem])
                except OSError as exc:restore_errors.append({'file':str(paths[stem]),'error':str(exc)})
            manifest['state']='rollback-failed' if restore_errors else 'rollback-conflict' if conflicts else 'rolled-back'
            manifest['rollback_conflicts']=conflicts
            manifest['rollback_errors']=restore_errors
            if attribute_warnings:manifest['attribute_warnings']=attribute_warnings
            if conflicts:print(f'Concurrent edits preserved during rollback; originals remain at {backup}: {conflicts}',file=sys.stderr)
            if restore_errors:print(f'Rollback incomplete; verified originals remain at {backup}: {restore_errors}',file=sys.stderr)
            try:manifest_path.write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
            except OSError as exc:print(f'Cannot update rollback manifest {manifest_path}: {exc}',file=sys.stderr)
            raise
        return manifest_path

def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mod',type=Path,required=True)
    parser.add_argument('--tools',type=Path,default=Path(__file__).resolve().parents[2]/'dist/x64-win-rwdi/PoseidonTools.exe')
    mode=parser.add_mutually_exclusive_group();mode.add_argument('--apply',action='store_true');mode.add_argument('--check',action='store_true')
    parser.add_argument('--report',type=Path)
    parser.add_argument('--compatibility',choices=('ud','classic'),default='ud',
                        help='UD uses full nearby-unit queries; Classic uses a limited group-only wreck-fire query')
    parser.add_argument('--asset-source',type=Path,
                        help='Hash-pinned external archive for members LoBo references but never shipped (TheHKpack.pbo 1.75)')
    args=parser.parse_args(argv);mod=args.mod.resolve()
    if not (mod/'addons').is_dir():parser.error('--mod must contain addons/')
    try:
        source=args.asset_source.resolve() if args.asset_source else None
        paths,hashes,changed,records,docs,skipped=plan(mod,args.tools.resolve(),args.compatibility,source)
        for line in skipped:print(f'WARN: {line}',file=sys.stderr)
        result={'repair_version':REPAIR_VERSION,'rule_sha256':rule_fingerprint(),'issue':'https://github.com/GearUnclear/lobo/issues/1','mode':'apply' if args.apply else 'check',
                'compatibility':args.compatibility,'changed_members':len(changed),'added_members':sum(1 for r in records if r.get('added')),
                'archives':sorted({k.split('/')[0] for k in changed}),'skipped':skipped,'members':records}
        if source:result['asset_source_sha256']=digest(source.read_bytes())
        if args.apply:
            result['backup_manifest']=str(apply(mod,paths,hashes,changed,records,args.compatibility)) if changed else None
        if args.report:
            args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
        print(json.dumps({k:v for k,v in result.items() if k!='members'},indent=2))
        return 1 if args.check and changed else 0
    except (OSError,ValueError,KeyError,UnicodeError) as exc:
        print(f'LoBo repair aborted before completion: {exc}',file=sys.stderr);return 2

if __name__=='__main__':raise SystemExit(main())
