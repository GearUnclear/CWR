#!/usr/bin/env python3
"""Config repair/audit regressions using original synthetic legacy source."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools' / 'lobo'))
from repair.config import class_block, class_spans, scalar, set_scalar
from repair.audit import assignments, audit, config_references, script_candidates


class ConfigStructureTests(unittest.TestCase):
    def test_comments_strings_and_array_braces_cannot_create_classes(self):
        source = '''
// class FakeLine { bad={}; };
/* class FakeBlock { } }; */
class Root {
    description="a } class FakeString { ""quoted"" // text";
    values[]={{1,2},{"}","class ArrayFake {"}};
    class Actual: Parent { name="C:\\models\\thing"; };
};
'''
        self.assertEqual([n for n, _, _ in class_spans(source)], ['Root'])
        a, b = class_block(source, ('root',))
        self.assertEqual([n for n, _, _ in class_spans(source[a:b])], ['Actual'])
        self.assertEqual(scalar(source, ('ROOT', 'actual'), 'name'), '"C:\\models\\thing"')
        self.assertEqual(scalar(source, ('Root',), 'description'),
                         '"a } class FakeString { ""quoted"" // text"')

    def test_scoped_edits_preserve_nested_arrays_comments_and_strings(self):
        source = '''class Root {
    hit=1; // hit=900;
    message="hit=800; { }";
    matrix[]={{1,2},{3,4}};
    class Inner { hit=2; };
}; class Other { hit=3; };'''
        result = set_scalar(source, ('Root',), 'hit', '7')
        self.assertEqual(result, source.replace('hit=1;', 'hit=7;', 1))
        self.assertEqual(scalar(result, ('Root', 'Inner'), 'hit'), '2')
        self.assertEqual(scalar(result, ('Other',), 'hit'), '3')
        nested = set_scalar(result, ('Root', 'Inner'), 'hit', '8')
        self.assertEqual(scalar(nested, ('Root',), 'hit'), '7')
        self.assertEqual(scalar(nested, ('Root', 'Inner'), 'hit'), '8')

    def test_appended_property_is_direct_and_retains_crlf(self):
        source = 'class Root {\r\n class Inner { added=1; };\r\n};\r\n'
        result = set_scalar(source, ('Root',), 'added', '2')
        self.assertEqual(scalar(result, ('Root',), 'added'), '2')
        self.assertEqual(scalar(result, ('Root', 'Inner'), 'added'), '1')
        self.assertNotIn('\n', result.replace('\r\n', ''))
        self.assertEqual(set_scalar(result, ('Root',), 'added', '2'), result)

    def test_forward_declarations_do_not_shadow_complete_definitions(self):
        source = 'class Root; class Root { class Child; class Child { hit=1; }; };'
        self.assertEqual(scalar(source, ('Root', 'Child'), 'hit'), '1')

    def test_case_insensitive_duplicate_classes_are_ambiguous(self):
        source = 'class Root {class Child {hit=1;}; class child {hit=2;};};'
        with self.assertRaises(ValueError):
            class_block(source, ('Root', 'Child'))
        with self.assertRaises(ValueError):
            set_scalar(source, ('Root', 'Child'), 'hit', '3')

    def test_duplicate_direct_properties_fail_with_or_without_semicolons(self):
        for separator in (';', '\n', '\r\n'):
            source = 'class Root { hit=1' + separator + 'hit=2\n};'
            with self.subTest(separator=separator), self.assertRaises(ValueError):
                scalar(source, ('Root',), 'hit')
        self.assertEqual(scalar('class R {hit=1;class N {hit=2;};};', ('R',), 'hit'), '1')

    def test_legacy_newline_assignments_remain_independent(self):
        source = 'class Root {\n hit=1\n range=2\n radius=3\n};'
        for key, expected in [('hit', '1'), ('range', '2'), ('radius', '3')]:
            self.assertEqual(scalar(source, ('Root',), key), expected)
        result = set_scalar(source, ('Root',), 'range', '9')
        self.assertEqual(result, source.replace('range=2', 'range=9'))

    def test_multiline_expression_is_not_truncated_to_first_literal(self):
        for expression in ('1 /* continued\ncomment */ + 2', '(1 +\n2)', '1 +\n2'):
            source = 'class Root { hit=' + expression + '; other=9; };'
            self.assertEqual(scalar(source, ('Root',), 'hit'), expression)
            changed = set_scalar(source, ('Root',), 'hit', '4')
            self.assertEqual(changed, 'class Root { hit=4; other=9; };')

    def test_malformed_structure_or_lexical_state_is_rejected(self):
        for source in ('class Root {', 'class Root {}; };',
                       'class Root { name="unfinished; };',
                       'class Root {}; /* unfinished class Ghost {}'):
            with self.subTest(source=source), self.assertRaises(ValueError):
                class_spans(source)


class ReferenceAuditTests(unittest.TestCase):
    def test_newline_assignments_do_not_skip_alternating_references(self):
        body = ('ammo=LoBo_MissingAmmo\ncrew=LoBo_MissingCrew\n'
                'magazines[]={LoBo_MissingMagazine}\nweapons[]={LoBo_MissingWeapon}')
        self.assertEqual([key for key, _ in assignments(body)],
                         ['ammo', 'crew', 'magazines', 'weapons'])
        documents = {'sample/config.cpp': 'class CfgWeapons { class LoBo_Test {' + body + '};};'}
        problems = config_references(documents)
        self.assertEqual({p['reference'] for p in problems}, {
            'LoBo_MissingAmmo', 'LoBo_MissingCrew', 'LoBo_MissingMagazine', 'LoBo_MissingWeapon'})

    def test_multiline_nested_arrays_preserve_every_member(self):
        body = '''weapons[]={
            LoBo_A,
            {LoBo_B, LoBo_C},
            "LoBo_D"
        };
        magazines[]={LoBo_E};'''
        self.assertEqual(list(assignments(body)), [
            ('weapons', ['LoBo_A', 'LoBo_B', 'LoBo_C', 'LoBo_D']),
            ('magazines', ['LoBo_E']),
        ])

    def test_cross_file_case_insensitive_resolution_and_nested_fire_modes(self):
        documents = {
            'ammo/config.cpp': 'class CfgAmmo { class LoBo_Round {}; };',
            'weapons/config.cpp': '''class CfgWeapons {
                class LoBo_Mag { ammo="lObO_rOuNd"; };
                class LoBo_Rifle {
                    magazines[]={LoBo_Mag};
                    class Single { ammo=LOBO_ROUND; };
                };
            };''',
            'units/config.cpp': '''class CfgVehicles {
                class LoBo_Crew {};
                class LoBo_Vehicle {crew=LoBo_Crew;weapons[]={LoBo_Rifle};magazines[]={LoBo_Mag};};
            };''',
        }
        self.assertEqual(config_references(documents), [])
        self.assertTrue(audit(documents)['coverage_complete'])
        documents['weapons/config.cpp'] = documents['weapons/config.cpp'].replace(
            'ammo=LOBO_ROUND;', 'ammo=LoBo_MissingMode;')
        result = config_references(documents)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['reference'], 'LoBo_MissingMode')

    def test_comments_and_event_text_are_not_references(self):
        documents = {'sample/config.cpp': '''class CfgWeapons {
            class LoBo_Test {
                // ammo=LoBo_Comment;
                /* magazines[]={LoBo_OtherComment}; */
                event="ammo=LoBo_String; weapons[]={LoBo_Fake};";
                class Mode { ammo=LoBo_RealMissing; };
            };
        };'''}
        result = config_references(documents)
        self.assertEqual([p['reference'] for p in result], ['LoBo_RealMissing'])

    def test_scope_namespace_matters_and_external_base_names_are_not_flagged(self):
        documents = {'sample/config.cpp': '''class CfgVehicles { class LoBo_Round {}; };
            class CfgWeapons { class LoBo_Test {ammo=LoBo_Round;weapons[]={M16};};};'''}
        result = config_references(documents)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['property'], 'ammo')
        self.assertEqual(result[0]['reference'], 'LoBo_Round')

    def test_truncated_addon_reports_incomplete_coverage_without_aborting_others(self):
        # Same defect family as two installed configs that omit their final
        # CfgVehicles close. Never invent source braces during an audit.
        documents = {
            'broken/config.cpp': 'class CfgVehicles { class LoBo_Unit {};',
            'good/config.cpp': 'class CfgWeapons {class LoBo_Test {ammo=LoBo_Missing;};};',
        }
        result = audit(documents)
        self.assertFalse(result['coverage_complete'])
        self.assertIn('broken/config.cpp', result['unparsed_configs'])
        candidates = [p for p in result['candidates'] if p['kind'] == 'unsupported_parse']
        self.assertEqual([p['file'] for p in candidates], ['broken/config.cpp'])
        records = result['errors'] + result['candidates']
        self.assertTrue(any(p.get('reference') == 'LoBo_Missing' for p in records))
        self.assertIn('coverage', result['note'].lower())

    def test_script_candidates_are_labeled_as_heuristics(self):
        text = '; goto "commented"\n#launch\ngoto "launch"\ngoto "Exit"\ncall _AI\n'
        result = script_candidates('a/test.sqs', text)
        self.assertFalse(any(p.get('label') == 'commented' for p in result))
        self.assertFalse(any(p.get('label') == 'launch' for p in result))
        terminal = next(p for p in result if p.get('label') == 'Exit')
        self.assertTrue(terminal['terminal_spelling'])
        self.assertTrue(any(p['kind'] == 'uninitialized_local_call_candidate' for p in result))


if __name__ == '__main__':
    unittest.main()
