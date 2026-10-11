"""Fault-injection tests for offline LoBo installation; synthetic temporary PBOs only."""
import contextlib
import io
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools' / 'lobo'))
import repair_lobo as runner
from repair.pbo import Archive


def archive(config):
    entries = [(b'config.cpp', config), (b'unchanged.txt', b'unrelated original data')]
    return (b''.join(name + b'\0' + struct.pack('<5I', 0, 0, 7, 42, len(data)) for name, data in entries)
            + bytes(21) + b''.join(data for _, data in entries))


class TransactionTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='lobo-transaction-test-')
        self.addCleanup(temporary.cleanup)
        self.mod = Path(temporary.name)
        addons = self.mod / 'addons'
        addons.mkdir()
        self.paths = {name: addons / (name + '.pbo') for name in ('first', 'second', 'third')}
        self.originals = {name: archive(name.encode() + b' original') for name in self.paths}
        for name, path in self.paths.items():
            path.write_bytes(self.originals[name])
        self.hashes = {name: runner.digest(data) for name, data in self.originals.items()}
        self.changes = {name + '/config.cpp': name.encode() + b' repaired' for name in self.paths}
        self.records = [dict(member=key, after_sha256=runner.digest(value)) for key, value in self.changes.items()]

    def apply(self, changed=None):
        return runner.apply(self.mod, self.paths, self.hashes,
                            self.changes if changed is None else changed, self.records)

    def installed(self):
        return {name: path.read_bytes() for name, path in self.paths.items()}

    def manifest(self):
        files = list(self.mod.glob('.ud-backups/issue-69/*/manifest.json'))
        self.assertEqual(len(files), 1)
        return files[0], json.loads(files[0].read_text())

    def test_success_has_verified_backups_and_hashes_for_every_archive(self):
        manifest_path = self.apply()
        path, manifest = self.manifest()
        self.assertEqual(manifest_path, path)
        self.assertEqual(manifest['state'], 'applied')
        self.assertEqual(len(manifest['archives']), 3)
        for item in manifest['archives']:
            name = Path(item['file']).stem
            backup = (path.parent / item['file']).read_bytes()
            installed = self.paths[name].read_bytes()
            self.assertEqual(backup, self.originals[name])
            self.assertEqual(item['before_sha256'], runner.digest(backup))
            self.assertEqual(item['after_sha256'], runner.digest(installed))
            self.assertEqual(Archive(installed).read('config.cpp'), self.changes[name + '/config.cpp'])
            self.assertEqual(Archive(installed).read('unchanged.txt'), b'unrelated original data')
        self.assertFalse(list(self.mod.glob('lobo-repair-*')))

    def test_empty_change_set_creates_no_backup_or_staging(self):
        self.assertIsNone(self.apply({}))
        self.assertEqual(self.installed(), self.originals)
        self.assertEqual({p.name for p in self.mod.iterdir()}, {'addons'})

    def test_corrupt_backup_prevents_all_replacements(self):
        real_copy = shutil.copy2
        def corrupt(source, target, *args, **kwargs):
            result = real_copy(source, target, *args, **kwargs)
            if Path(target).name == 'second.pbo':
                Path(target).write_bytes(b'corrupt backup')
            return result
        with patch.object(runner.shutil, 'copy2', corrupt), self.assertRaisesRegex(ValueError, 'Backup verification'):
            self.apply()
        self.assertEqual(self.installed(), self.originals)
        self.assertFalse(list(self.mod.glob('lobo-repair-*')))

    def test_concurrent_change_after_planning_is_preserved(self):
        external = archive(b'concurrent original edit')
        self.paths['second'].write_bytes(external)
        with self.assertRaisesRegex(ValueError, 'changed since preflight'):
            self.apply()
        self.assertEqual(self.installed(), self.originals | {'second': external})
        self.assertFalse((self.mod / '.ud-backups').exists())

    def test_concurrent_change_after_backups_rolls_back_prior_replacement(self):
        real_replace = Path.replace
        external = archive(b'concurrent edit between archive installs')
        def inject(source, target):
            result = real_replace(source, target)
            if source.name == 'first.pbo':
                self.paths['second'].write_bytes(external)
            return result
        with patch.object(Path, 'replace', inject), self.assertRaisesRegex(ValueError, 'Concurrent edit'):
            self.apply()
        self.assertEqual(self.installed(), self.originals | {'second': external})
        _, manifest = self.manifest()
        self.assertEqual(manifest['state'], 'rolled-back')

    def test_last_replacement_failure_rolls_back_every_prior_archive(self):
        real_replace = Path.replace
        def fail(source, target):
            if source.name == 'third.pbo':
                raise OSError('injected staged replacement failure')
            return real_replace(source, target)
        with patch.object(Path, 'replace', fail), self.assertRaisesRegex(OSError, 'injected staged'):
            self.apply()
        self.assertEqual(self.installed(), self.originals)
        _, manifest = self.manifest()
        self.assertEqual(manifest['state'], 'rolled-back')

    def test_rollback_preserves_later_edit_and_restores_other_archives(self):
        real_replace = Path.replace
        external = archive(b'concurrent edit of already repaired first archive')
        def fail(source, target):
            if source.name == 'third.pbo':
                self.paths['first'].write_bytes(external)
                raise OSError('injected staged replacement failure')
            return real_replace(source, target)
        with contextlib.redirect_stderr(io.StringIO()), patch.object(Path, 'replace', fail):
            with self.assertRaisesRegex(OSError, 'injected staged'):
                self.apply()
        self.assertEqual(self.installed(), self.originals | {'first': external})
        path, manifest = self.manifest()
        self.assertEqual(manifest['state'], 'rollback-conflict')
        self.assertEqual(manifest['rollback_conflicts'], [str(self.paths['first'])])
        self.assertEqual((path.parent / 'first.pbo').read_bytes(), self.originals['first'])

    def test_one_restore_failure_does_not_abandon_remaining_archives(self):
        real_replace = Path.replace
        def fail(source, target):
            if source.name == 'third.pbo':
                raise OSError('injected staged replacement failure')
            if source.name == 'second.pbo.restore':
                raise OSError('injected rollback replacement failure')
            return real_replace(source, target)
        with contextlib.redirect_stderr(io.StringIO()), patch.object(Path, 'replace', fail):
            with self.assertRaises(OSError):
                self.apply()
        installed = self.installed()
        self.assertEqual(installed['first'], self.originals['first'])
        self.assertEqual(installed['third'], self.originals['third'])
        self.assertEqual(Archive(installed['second']).read('config.cpp'), b'second repaired')
        path, manifest = self.manifest()
        self.assertEqual(manifest['state'], 'rollback-failed')
        self.assertTrue(manifest['rollback_errors'])
        self.assertEqual((path.parent / 'second.pbo').read_bytes(), self.originals['second'])


if __name__ == '__main__':
    unittest.main()
