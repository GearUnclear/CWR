"""Independent no-game reference validation of the actual runner preview."""
from pathlib import Path
import hashlib
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
AUDIT = ROOT / 'tmp/lobo-audit-20261011'
OUT = Path(__file__).resolve().parent
TOOL = ROOT / 'arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
sys.path.insert(0, str(ROOT / 'arma_CWR/tools/lobo'))
import repair_lobo as runner

SECTIONS = {'cfgammo', 'cfgweapons', 'cfgvehicles'}
TARGET = {'ammo': 'cfgammo', 'weapons': 'cfgweapons', 'magazines': 'cfgweapons', 'crew': 'cfgvehicles'}


def canonical(document):
    return {name.casefold(): value for name, value in document.items()}


def index_and_references(documents):
    definitions = {section: set() for section in SECTIONS}
    references = []
    def visit(addon, obj, location):
        if not isinstance(obj, dict):
            return
        for name, value in obj.items():
            if isinstance(value, dict):
                visit(addon, value, location + '/' + name)
            if name.casefold() not in TARGET:
                continue
            for reference in value if isinstance(value, list) else [value]:
                if isinstance(reference, str) and reference.casefold().startswith('lobo'):
                    references.append({'addon': addon, 'location': location + '/' + name,
                                       'reference': reference, 'target_section': TARGET[name.casefold()]})
    for addon, document in documents.items():
        for section in SECTIONS:
            body = document.get(section, {})
            if not isinstance(body, dict):
                raise ValueError(f'{addon}/{section} is not an exported class collection')
            definitions[section].update(name.casefold() for name in body)
            visit(addon, body, section)
    missing = [row for row in references
               if row['reference'].casefold() not in definitions[row['target_section']]]
    return definitions, references, missing


def lookup(document, section, name):
    return canonical(document[section])[name.casefold()]


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def main():
    baseline = {}
    failed_baselines = []
    legacy_encoded_exports = []
    for config in sorted(AUDIT.glob('*/config.cpp')):
        file = config.with_name('audit.json')
        if not file.is_file():
            failed_baselines.append({'addon': config.parent.name, 'reason': 'No successful original audit.json export'})
            continue
        try:
            raw = file.read_bytes()
            try:
                text = raw.decode('utf-8-sig')
            except UnicodeDecodeError:
                text = raw.decode('latin1')
                legacy_encoded_exports.append(config.parent.name)
            baseline[config.parent.name.casefold()] = canonical(json.loads(text))
        except (ValueError, UnicodeError) as error:
            failed_baselines.append({'addon': config.parent.name, 'reason': str(error)})
    print('Baseline standalone JSON exports:', len(baseline), 'unavailable:', len(failed_baselines), flush=True)
    paths, hashes, changed, records, preview = runner.plan(ROOT / '@LoBo', TOOL)
    print('Actual runner preview:', len(changed), 'changed members', flush=True)
    updated = dict(baseline)
    parses = []
    for member, data in sorted(changed.items()):
        if not member.endswith('/config.cpp'):
            continue
        addon = member.split('/')[0]
        folder = OUT / 'preview' / addon
        folder.mkdir(parents=True, exist_ok=True)
        cpp, exported = folder / 'config.cpp', folder / 'audit.json'
        cpp.write_bytes(data)
        result = subprocess.run([str(TOOL), 'config', 'tojson', str(cpp), '-o', str(exported)], capture_output=True)
        output = result.stdout.decode('utf-8', errors='replace') + result.stderr.decode('utf-8', errors='replace')
        (folder / 'parse.log').write_text(output, encoding='utf-8')
        parse = {'member': member, 'exit_code': result.returncode,
                 'preview_sha256': hashlib.sha256(data).hexdigest(),
                 'output': str(folder / 'parse.log'), 'json': str(exported)}
        parses.append(parse)
        if result.returncode or not exported.is_file():
            raise ValueError(f'Engine config export failed: {member}; see {folder / "parse.log"}')
        updated[addon] = canonical(json.loads(exported.read_text(encoding='utf-8-sig')))
        print('Engine parsed:', member, flush=True)
    before_defs, before_refs, missing_before = index_and_references(baseline)
    after_defs, after_refs, missing_after = index_and_references(updated)
    m833 = canonical(lookup(updated['loboammo'], 'cfgammo', 'LoBo_105Sabot2'))
    m833_magazine = canonical(lookup(updated['loboammo'], 'cfgweapons', 'LoBo_105Sabot2'))
    micro = canonical(lookup(updated['loboweapons'], 'cfgweapons', 'LoBoMUZIbase'))
    micro_magazines = {name.casefold() for name in micro['magazines']}
    gbu = canonical(lookup(updated['lobo_airammo'], 'cfgammo', 'LoBo_GBU12'))
    mk82 = canonical(lookup(updated['lobo_airammo'], 'cfgammo', 'LoBo_MK82'))
    checks = {
        'm833_projectile_exists_in_cfgammo': 'lobo_105sabot2' in after_defs['cfgammo'],
        'm833_magazine_keeps_its_projectile_id': m833_magazine['ammo'].casefold() == 'lobo_105sabot2',
        'm833_distinct_baseline_hit_400': m833['hit'] == 400,
        'm833_no_splash': m833['indirecthit'] == 0 and m833['indirecthitrange'] == 0,
        'micro_uzi_lists_existing_hd_magazine': 'lobomuzihdmag' in micro_magazines and 'lobomuzihdmag' in after_defs['cfgweapons'],
        'micro_uzi_missing_dd_spelling_absent': 'lobomuziddmag' not in micro_magazines,
        'gbu12_mk82_warhead_equal': all(gbu[key] == mk82[key] for key in ('hit', 'indirecthit', 'indirecthitrange')),
        'all_changed_configs_engine_exported': all(row['exit_code'] == 0 for row in parses),
        'no_unresolved_lobo_references_in_exported_definition_index': not missing_after,
    }
    # Verify the current installed input snapshots remained untouched by preview
    # and independent export; this validator never calls runner.apply.
    checks['preview_source_archives_unchanged'] = all(runner.digest(paths[stem].read_bytes()) == expected
                                                    for stem, expected in hashes.items())
    report = {
        'scope': 'Standalone PoseidonTools JSON exports; source definition index and explicit references only. '
                 'Not mounted load-order/inheritance resolution and not runtime/game validation.',
        'repair_rule_sha256': runner.rule_fingerprint(),
        'input_addon_exports': len(baseline), 'preview_addon_exports': len(updated),
        'unavailable_baseline_exports': failed_baselines,
        'legacy_single_byte_json_exports': legacy_encoded_exports,
        'known_definition_counts_before': {k: len(v) for k, v in before_defs.items()},
        'known_definition_counts_after': {k: len(v) for k, v in after_defs.items()},
        'checked_lobo_reference_occurrences_before': len(before_refs),
        'checked_lobo_reference_occurrences_after': len(after_refs),
        'unresolved_before': missing_before, 'unresolved_after': missing_after,
        'checks': checks, 'changed_config_exports': parses,
        'all_checks_passed': all(checks.values()),
        'game_launched': False, 'installed_files_written': False,
    }
    write_json(OUT / 'validation.json', report)
    print(json.dumps({key: report[key] for key in ('input_addon_exports', 'unavailable_baseline_exports',
                                                 'checked_lobo_reference_occurrences_after', 'unresolved_before',
                                                 'unresolved_after', 'checks', 'all_checks_passed')}, indent=2), flush=True)
    return 0 if report['all_checks_passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
