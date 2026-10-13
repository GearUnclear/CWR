#!/usr/bin/env python3
"""Check that a probe mission's addOns[] activates every owner addon it needs.

Static check for the "flashing main menu" failure
(agent_guidelines/flashing-main-menu.md): a class placed in mission.sqm, or a
weapon or magazine it carries, whose owner addon is not active makes
CheckAccess raise a warning, the boot is rejected and Trident sits on the main
menu. Classes a probe creates from its script after the boot are checked the
same way, because the same warning is raised then (at runtime the
ZoneRegistry's later additive activation of the faction addons can hide such
a gap, so a spawn report is conservative).

Read-only. Pbos are opened for reading; a compressed member goes through
"PoseidonTools pbo show" and a binarized base config through "PoseidonTools
config debin" (a temporary copy in the system temp folder, removed before
exit). Nothing else is written.

Model of the engine (engine/Poseidon/Asset/Addon/AddonSystem.cpp,
World/WorldImpl.cpp, Asset/Addon/AddonClosure.cpp):
- an addon config's owner is its first CfgPatches class;
- configs merge in requiredAddons order (preloaded ones first, otherwise pbo
  order), and a class belongs to the first config that creates it; a class
  the base config already has stays base content (always visible);
- active = CfgAddons/PreloadAddons lists + the mission's addOns[] and
  addOnsAuto[]; requiredAddons[] activates nothing;
- a unit needs its own class, every weapons[] and magazines[] entry, and
  the magazines[] each of its weapons accepts (OFP 1.x magazines are
  CfgWeapons classes; "this" names the weapon itself).

Merge order within one mod folder is approximated by pbo name; when several
configs define a class the report notes them. Probe .sqf files are read with
their #include lines expanded the way Trident does.

Usage (from arma_CWR/):
  python tools/lobo/runtime/check_mission_addons.py --mod D:/Arma_CWA/@LoBo \\
      --base-config "<data dir>/BIN/config.bin" \\
      --probe tools/lobo/runtime/repair4/lobo_pilot_wounds.test.sqf

--probe reads the sibling .test.toml for the mission and takes every quoted
string in the .sqf that names a CfgVehicles class as a script spawn.
--mission DIR plus --spawn CLASS does the same by hand. Exit 0 when every
needed owner is active, 1 when one is not or a class is unknown, 2 on bad
usage or unreadable input.
"""
import argparse
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import lobo_cfgpatches as pbo  # noqa: E402  (tools/lobo/lobo_cfgpatches.py)

ENGINE_ROOT = Path(__file__).resolve().parents[3]
SECTIONS = ('cfgvehicles', 'cfgweapons', 'cfgmagazines')
INCLUDE = re.compile(r'^[ \t]*#include[ \t]+["<]([^">]+)[">]', re.M)


def expand_includes(text, members, path, tools, depth=0):
    """Inline #include "x" from the same pbo (config-relative), 4 levels deep."""
    if depth > 4:
        return text

    def sub(match):
        key = match.group(1).replace('\\', '/').lower().lstrip('/')
        entry = members.get(key)
        if entry is None:
            # pbo prefix written into the path ("addon\\file.hpp")
            entry = members.get(key.split('/', 1)[1]) if '/' in key else None
        if entry is None:
            return ''
        inner = pbo.read_member(path, entry, tools).decode('latin1')
        return expand_includes(inner, members, path, tools, depth + 1)

    return INCLUDE.sub(sub, text)


class Config:
    """The parts of one config this check needs."""

    def __init__(self, label, text):
        self.label = label
        self.patches = []          # CfgPatches class names, in order
        self.required = []         # union of requiredAddons[]
        self.preload = []          # CfgAddons >> PreloadAddons >> * >> list[]
        self.classes = {s: {} for s in SECTIONS}   # lower name -> dict
        self._parse(list(pbo.tokens(pbo.strip_comments_and_directives(text))))

    @property
    def owner(self):
        return self.patches[0] if self.patches else ''

    def _parse(self, toks):
        stack = []                 # (lower class name, original) per open body
        pending = None             # (name, base) after 'class NAME [: BASE]'
        i = 0
        n = len(toks)
        while i < n:
            kind, val = toks[i]
            if kind == 'id' and val == 'class' and i + 1 < n and toks[i + 1][0] == 'id':
                name = toks[i + 1][1]
                base = None
                i += 2
                if i + 1 < n and toks[i] == ('p', ':') and toks[i + 1][0] == 'id':
                    base = toks[i + 1][1]
                    i += 2
                pending = (name, base)
                continue
            if kind == 'p' and val == '{':
                if pending is not None:
                    self._open(stack, pending)
                    stack.append((pending[0].lower(), pending[0]))
                    pending = None
                    i += 1
                    continue
                # an array or value brace outside a class header: skip to its match
                depth = 0
                while i < n:
                    if toks[i] == ('p', '{'):
                        depth += 1
                    elif toks[i] == ('p', '}'):
                        depth -= 1
                        if depth == 0:
                            break
                    i += 1
                i += 1
                continue
            if kind == 'p' and val == '}':
                if stack:
                    stack.pop()
                pending = None
                i += 1
                continue
            if kind == 'p' and val == ';':
                pending = None
                i += 1
                continue
            if kind == 'id' and i + 3 < n and toks[i + 1] == ('p', '[') and toks[i + 2] == ('p', ']'):
                values, i = self._array(toks, i + 3)
                self._assign_array(stack, val.lower(), values)
                continue
            i += 1

    @staticmethod
    def _array(toks, i):
        """Parse '= { a, "b", ... }' starting at '='; returns (values, next index)."""
        n = len(toks)
        if i < n and toks[i] == ('p', '='):
            i += 1
        values = []
        if i >= n or toks[i] != ('p', '{'):
            return values, i
        depth = 0
        while i < n:
            kind, val = toks[i]
            if (kind, val) == ('p', '{'):
                depth += 1
            elif (kind, val) == ('p', '}'):
                depth -= 1
                if depth == 0:
                    return values, i + 1
            elif depth == 1 and kind == 'str':
                values.append(val[1:-1].replace('""', '"'))
            elif depth == 1 and kind == 'id':
                values.append(val)
            i += 1
        return values, i

    def _open(self, stack, pending):
        name, base = pending
        path = [s[0] for s in stack]
        if len(path) == 1 and path[0] in SECTIONS:
            entry = self.classes[path[0]].setdefault(name.lower(), {'name': name, 'base': None})
            if base:
                entry['base'] = base
        elif len(path) == 1 and path[0] == 'cfgpatches':
            if name.lower() not in [p.lower() for p in self.patches]:
                self.patches.append(name)

    def _assign_array(self, stack, key, values):
        path = [s[0] for s in stack]
        if len(path) == 2 and path[0] in SECTIONS and key in ('weapons', 'magazines'):
            self.classes[path[0]][path[1]][key] = values
        elif len(path) == 2 and path[0] == 'cfgpatches' and key == 'requiredaddons':
            self.required.extend(values)
        elif len(path) == 3 and path[:2] == ['cfgaddons', 'preloadaddons'] and key == 'list':
            self.preload.extend(values)


def load_addons(mod_dirs, tools):
    configs = []
    errors = []
    for mod in mod_dirs:
        addons = next((d for d in Path(mod).iterdir() if d.is_dir() and d.name.lower() == 'addons'), None)
        if addons is None:
            errors.append(f'{mod}: no addons folder')
            continue
        for path in sorted(addons.glob('*.pbo'), key=lambda p: p.name.lower()):
            try:
                entries = pbo.read_header(path)
                members = {pbo.member_key(e[0]): e for e in entries}
                if 'config.cpp' in members:
                    text = pbo.read_member(path, members['config.cpp'], tools).decode('latin1')
                    text = expand_includes(text, members, path, tools)
                elif 'config.bin' in members:
                    tmp = tempfile.mkdtemp(prefix='lobo-addon-check-')
                    try:
                        text = pbo.debin(tools, pbo.read_member(path, members['config.bin'], tools), tmp)
                    finally:
                        shutil.rmtree(tmp, ignore_errors=True)
                else:
                    continue
                configs.append(Config(f'{Path(mod).name}/{path.name}', text))
            except (OSError, pbo.PboError, UnicodeError) as exc:
                errors.append(f'{path}: {exc}')
    return configs, errors


def load_base(path, tools):
    if path is None:
        return None
    data = Path(path).read_bytes()
    if data[:4] == b'\0raP':
        tmp = tempfile.mkdtemp(prefix='lobo-addon-check-')
        try:
            text = pbo.debin(tools, data, tmp)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    else:
        text = data.decode('latin1')
    return Config(str(path), text)


def merge_order(configs, preloaded):
    """AddonSystem::ParseAllAddonConfigs: topological by requiredAddons, preloaded first."""
    by_patch = {}
    for cfg in configs:
        for name in cfg.patches:
            by_patch.setdefault(name.lower(), cfg)
    deps = []
    for cfg in configs:
        needs = {id(by_patch[r.lower()]) for r in cfg.required if r.lower() in by_patch}
        needs.discard(id(cfg))
        pre = bool(cfg.patches) and cfg.patches[-1].lower() in preloaded
        deps.append([cfg, needs, pre])
    order = []
    while deps:
        progressed = False
        for want_pre in (True, False):
            i = 0
            while i < len(deps):
                cfg, needs, pre = deps[i]
                if pre == want_pre and not needs:
                    order.append(cfg)
                    for other in deps:
                        other[1].discard(id(cfg))
                    deps.pop(i)
                    progressed = True
                    continue
                i += 1
        if not progressed:      # circular: the engine stops merging here too
            break
    return order


class World:
    def __init__(self, base, configs):
        self.preload = {p.lower() for p in (base.preload if base else [])}
        for cfg in configs:
            self.preload.update(p.lower() for p in cfg.preload)
        self.order = merge_order(configs, self.preload)
        self.base = base

    def owner(self, section, name):
        """(owner or '' for base content or None if unknown, every defining config label)."""
        key = name.lower()
        definers = [c.label + ':' + c.owner for c in self.order if key in c.classes[section]]
        if self.base is not None and key in self.base.classes[section]:
            return '', definers
        for cfg in self.order:
            if key in cfg.classes[section]:
                return cfg.owner, definers
        return None, definers

    def array(self, section, name, key, seen=None):
        """Effective (inherited) array: last merged definition wins, then the base class."""
        seen = seen or set()
        lname = name.lower()
        if lname in seen:
            return []
        seen.add(lname)
        layers = ([self.base] if self.base is not None else []) + self.order
        value = None
        base_class = None
        for cfg in layers:
            entry = cfg.classes[section].get(lname)
            if entry is None:
                continue
            if key in entry:
                value = entry[key]
            if entry.get('base'):
                base_class = entry['base']
        if value is not None:
            return value
        if base_class:
            return self.array(section, base_class, key, seen)
        return []


def magazine_owner(world, mag):
    """OFP 1.x keeps magazines in CfgWeapons; a CfgMagazines class wins when present."""
    owner, definers = world.owner('cfgmagazines', mag)
    if owner is None:
        owner, definers = world.owner('cfgweapons', mag)
    return owner, definers


def needs_of(world, cls):
    """Owner requirements of one vehicle class: [(kind, class, owner, definers)], de-duplicated."""
    out = []
    seen = set()

    def add(kind, name, found):
        if (kind, name.lower()) not in seen:
            seen.add((kind, name.lower()))
            out.append((kind, name) + found)

    owner, definers = world.owner('cfgvehicles', cls)
    add('class', cls, (owner, definers))
    if owner is None:
        return out
    for mag in world.array('cfgvehicles', cls, 'magazines'):
        add('magazine', mag, magazine_owner(world, mag))
    for weapon in world.array('cfgvehicles', cls, 'weapons'):
        add('weapon', weapon, world.owner('cfgweapons', weapon))
        for mag in world.array('cfgweapons', weapon, 'magazines'):
            if mag.lower() == 'this':       # OFP: the weapon class is its own magazine
                mag = weapon
            add('weapon magazine', f'{weapon}/{mag}', magazine_owner(world, mag))
    return out


def read_mission(directory):
    text = (Path(directory) / 'mission.sqm').read_text(encoding='latin1')
    body = pbo.strip_comments_and_directives(text)
    mission = re.search(r'class\s+Mission\b(.*?)class\s+Intro\b', body, re.S | re.I)
    mission = mission.group(1) if mission else body
    addons = []
    for key in ('addOns', 'addOnsAuto'):
        m = re.search(key + r'\[\]\s*=\s*\{([^}]*)\}', mission, re.I)
        if m:
            addons.extend(v.strip().strip('"') for v in m.group(1).split(',') if v.strip())
    placed = re.findall(r'\bvehicle\s*=\s*"([^"]+)"', mission, re.I)
    return addons, sorted(set(placed), key=placed.index)


def sqf_with_includes(path, depth=0):
    """Trident's expand_includes: a line '#include "rel/path"' is replaced by that file."""
    out = []
    for line in Path(path).read_text(encoding='utf8').splitlines():
        m = re.match(r'\s*#include\s+"([^"]+)"\s*$', line)
        if m and depth < 8:
            out.append(sqf_with_includes(Path(path).parent / m.group(1), depth + 1))
        else:
            out.append(line)
    return '\n'.join(out)


def probe_inputs(sqf, world):
    sqf = Path(sqf)
    toml = sqf.with_name(sqf.name[:-len('.sqf')] + '.toml') if sqf.name.endswith('.sqf') else None
    mission = None
    if toml and toml.exists():
        m = re.search(r'^\s*mission\s*=\s*"([^"]+)"', toml.read_text(encoding='utf8'), re.M)
        if m:
            mission = ENGINE_ROOT / m.group(1)
    code = '\n'.join(line.split('//', 1)[0] for line in sqf_with_includes(sqf).splitlines())
    spawns = []
    code = re.sub(r'disableAI\s+"[^"]*"', '', code, flags=re.I)   # "TARGET" is also a class name
    for s in re.findall(r'"([A-Za-z_][A-Za-z0-9_]*)"', code):
        if world.owner('cfgvehicles', s)[0] is not None and s not in spawns:
            spawns.append(s)
    return mission, spawns


def check(world, mission_dir, spawns):
    addons, placed = read_mission(mission_dir)
    active = {a.lower() for a in addons} | world.preload
    report = {'mission': str(mission_dir), 'addOns': addons, 'placed': [], 'spawned': [], 'missing': [],
              'unknown': []}
    for group, classes in (('placed', placed), ('spawned', spawns)):
        for cls in classes:
            rows = []
            for kind, name, owner, definers in needs_of(world, cls):
                ok = owner is not None and (owner == '' or owner.lower() in active)
                rows.append({'kind': kind, 'name': name, 'owner': owner, 'active': ok,
                             'definers': definers if len(definers) > 1 else None})
                if owner is None:
                    report['unknown'].append(f'{group} {cls}: {kind} {name}')
                elif not ok:
                    report['missing'].append(f'{group} {cls}: {kind} {name} -> {owner}')
            report[group].append({'class': cls, 'needs': rows})
    return report


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--mod', action='append', required=True, help='mod folder (repeatable), e.g. D:/Arma_CWA/@LoBo')
    ap.add_argument('--base-config', help="the data dir's BIN/config.bin (or a text config) for base classes")
    ap.add_argument('--tools', type=Path, default=ENGINE_ROOT / 'dist/x64-win-rwdi/PoseidonTools.exe')
    ap.add_argument('--probe', action='append', default=[], help='.test.sqf with a sibling .test.toml (repeatable)')
    ap.add_argument('--mission', help='mission folder for --spawn')
    ap.add_argument('--spawn', action='append', default=[], help='class created from script (with --mission)')
    ap.add_argument('--json', action='store_true', help='print the full report as JSON')
    args = ap.parse_args(argv)
    if not args.probe and not args.mission:
        ap.error('give --probe or --mission')
    if not args.tools.exists():
        alt = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
        if alt.exists():
            args.tools = alt
        else:
            ap.error(f'PoseidonTools not found: {args.tools}; pass --tools')
    configs, errors = load_addons(args.mod, args.tools)
    if errors:
        for e in errors:
            print('UNREADABLE', e, file=sys.stderr)
        return 2
    base = load_base(args.base_config, args.tools)
    world = World(base, configs)
    jobs = []
    for sqf in args.probe:
        mission, spawns = probe_inputs(sqf, world)
        if mission is None:
            print(f'{sqf}: no mission in its .test.toml', file=sys.stderr)
            return 2
        jobs.append((sqf, mission, spawns))
    if args.mission:
        jobs.append(('(manual)', Path(args.mission), args.spawn))
    failed = False
    reports = []
    for label, mission, spawns in jobs:
        if not (Path(mission) / 'mission.sqm').exists():
            print(f'{label}: {mission}/mission.sqm does not exist', file=sys.stderr)
            return 2
        rep = check(world, mission, spawns)
        rep['probe'] = str(label)
        reports.append(rep)
        bad = rep['missing'] or rep['unknown']
        failed = failed or bool(bad)
        if not args.json:
            print(f'{"FAIL" if bad else "OK  "} {label}  mission={Path(mission).name}  '
                  f'placed={len(rep["placed"])} spawned={len(rep["spawned"])}')
            owners = sorted({r['owner'] for g in ('placed', 'spawned') for c in rep[g] for r in c['needs']
                             if r['owner']}, key=str.lower)
            print('     owners needed:', ', '.join(owners) or '(base content only)')
            for line in rep['missing']:
                print('     MISSING', line)
            for line in rep['unknown']:
                print('     UNKNOWN', line)
            for g in ('placed', 'spawned'):
                for c in rep[g]:
                    for r in c['needs']:
                        if r['definers']:
                            print(f'     note: {r["kind"]} {r["name"]} defined by {len(r["definers"])} configs, '
                                  f'owner resolved to {r["owner"]!r}: {r["definers"]}')
    if args.json:
        print(json.dumps({'base_config': args.base_config, 'mods': args.mod, 'merged_configs': len(world.order),
                          'preload': sorted(world.preload), 'reports': reports}, indent=1))
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
