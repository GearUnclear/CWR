#!/usr/bin/env python3
"""Report/apply reproducible issue-69 LoBo repairs without running the game."""
import argparse
import datetime
import hashlib
import importlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import uuid
from repair.pbo import Archive

REPAIR_MODULES=('ammo','scripts','models','followups','clusters')
REPAIR_VERSION='2026-10-11.2'

def rule_fingerprint():
    base=Path(__file__).resolve().parent
    files=[Path(__file__).resolve()]+sorted((base/'repair').glob('*.py'))
    return digest(b''.join(p.relative_to(base).as_posix().encode()+b'\0'+p.read_bytes() for p in files))

def digest(data):return hashlib.sha256(data).hexdigest()

def modules():return [importlib.import_module('repair.'+name) for name in REPAIR_MODULES]

def plan(mod,tool,compatibility='ud'):
    rules=modules();wanted=set().union(*(set(m.ADDONS) for m in rules))
    model_keys=set().union(*(set(getattr(m,'MODEL_MEMBERS',())) for m in rules))
    paths={p.stem.lower():p for p in (mod/'addons').glob('*.pbo')}
    missing=wanted-paths.keys()
    if missing:raise ValueError(f'Required addons missing: {sorted(missing)}')
    docs={};binary={};hashes={};original={}
    # Read original archive bytes once per addon; no full-mod binary extraction.
    for stem in sorted(wanted):
        path=paths[stem];raw=path.read_bytes();hashes[stem]=digest(raw);archive=Archive(raw)
        for key,entry in archive.by_key.items():
            full=stem+'/'+key
            if key=='config.cpp' or key.endswith(('.sqs','.sqf')):
                data=archive.read(key,path,tool);docs[full]=data.decode('latin1');original[full]=data
            elif full in model_keys:
                data=archive.read(key,path,tool);binary[full]=data;original[full]=data
    missing_models=model_keys-binary.keys()
    if missing_models:raise ValueError(f'Required models missing: {sorted(missing_models)}')
    for module in rules:
        options={'compatibility':compatibility} if module.__name__=='repair.scripts' else {}
        edits=module.repair(dict(docs),**options)
        if set(edits)-docs.keys():raise ValueError(f'{module.__name__} tried to add an unknown text member')
        docs.update(edits)
        if hasattr(module,'repair_models'):
            edits=module.repair_models(dict(binary))
            if set(edits)-binary.keys():raise ValueError('Unknown model replacement')
            binary.update(edits)
    # Require a stable fixed point across all rules, including overlapping addons.
    for module in rules:
        options={'compatibility':compatibility} if module.__name__=='repair.scripts' else {}
        edits=module.repair(dict(docs),**options)
        if any(docs.get(k)!=v for k,v in edits.items()):raise ValueError(f'Non-idempotent rules: {module.__name__}')
        if hasattr(module,'repair_models'):
            edits=module.repair_models(dict(binary))
            if any(binary.get(k)!=v for k,v in edits.items()):raise ValueError('Non-idempotent model rules')
    changed={k:v.encode('latin1') for k,v in docs.items() if v.encode('latin1')!=original[k]}
    changed.update({k:v for k,v in binary.items() if v!=original[k]})
    records=[{'member':k,'before_sha256':digest(original[k]),'after_sha256':digest(v),
              'before_bytes':len(original[k]),'after_bytes':len(v)} for k,v in sorted(changed.items())]
    return paths,hashes,changed,records,docs

def apply(mod,paths,hashes,changed,records,compatibility='ud'):
    grouped={}
    for full,data in changed.items():
        stem,key=full.split('/',1);grouped.setdefault(stem,{})[key]=data
    if not grouped:return None
    # Stage and verify EVERY replacement before creating backups or replacing files.
    with tempfile.TemporaryDirectory(prefix='lobo-repair-',dir=mod) as stage_dir:
        stage=Path(stage_dir);new_hashes={}
        for stem,edits in grouped.items():
            raw=paths[stem].read_bytes()
            if digest(raw)!=hashes[stem]:raise ValueError(f'Archive changed since preflight: {paths[stem]}')
            result=Archive(raw).replace(edits)
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
        replaced=[]
        try:
            for stem in grouped:
                target=paths[stem]
                if digest(target.read_bytes())!=hashes[stem]:raise ValueError(f'Concurrent edit: {target}')
                (stage/target.name).replace(target);replaced.append(stem)
            manifest['state']='applied'
            manifest_path.write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
        except BaseException:
            conflicts=[];restore_errors=[]
            for stem in reversed(replaced):
                try:
                    if not paths[stem].exists() or digest(paths[stem].read_bytes())!=new_hashes[stem]:
                        conflicts.append(str(paths[stem]));continue
                    restore=stage/(paths[stem].name+'.restore');shutil.copy2(backup/paths[stem].name,restore);restore.replace(paths[stem])
                except OSError as exc:restore_errors.append({'file':str(paths[stem]),'error':str(exc)})
            manifest['state']='rollback-failed' if restore_errors else 'rollback-conflict' if conflicts else 'rolled-back'
            manifest['rollback_conflicts']=conflicts
            manifest['rollback_errors']=restore_errors
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
    args=parser.parse_args(argv);mod=args.mod.resolve()
    if not (mod/'addons').is_dir():parser.error('--mod must contain addons/')
    try:
        paths,hashes,changed,records,docs=plan(mod,args.tools.resolve(),args.compatibility)
        result={'repair_version':REPAIR_VERSION,'rule_sha256':rule_fingerprint(),'issue':'https://github.com/GearUnclear/lobo/issues/1','mode':'apply' if args.apply else 'check',
                'compatibility':args.compatibility,'changed_members':len(changed),'archives':sorted({k.split('/')[0] for k in changed}),'members':records}
        if args.apply:
            result['backup_manifest']=str(apply(mod,paths,hashes,changed,records,args.compatibility)) if changed else None
        if args.report:
            args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
        print(json.dumps({k:v for k,v in result.items() if k!='members'},indent=2))
        return 1 if args.check and changed else 0
    except (OSError,ValueError,KeyError,UnicodeError) as exc:
        print(f'LoBo repair aborted before completion: {exc}',file=sys.stderr);return 2

if __name__=='__main__':raise SystemExit(main())
