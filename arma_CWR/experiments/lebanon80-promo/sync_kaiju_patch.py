"""Patches the kaiju shoot mission built by sync-kaiju-mission.sh (the copies only, never a source file).

    python sync_kaiju_patch.py <src mission.sqm> <dst mission.sqm> <dst kj\\roar\\shake.sqs>

- mission.sqm: the three kaiju addons are added to addOns[].
- kj\\roar\\shake.sqs: one guard line at the top of the body. With PROMO_NOSHAKE set no tremor starts, so the
  engine never offsets the camera (Kaiju::ApplyCameraShake in World::Simulate). Every footfall, roar and beam
  tremor goes through this script (kj\\roar\\step.sqs and KJ_fnShake both exec it). The judder in the first
  shoot's kaiju and helicopter clips was this tremor (see clips\\shoot-notes.md).
"""
import sys

src, dst, shake = sys.argv[1:4]
s = open(src, encoding='utf-8').read()
s = s.replace('\t\t"Lobo_Fighting_pos"\n\t};', '\t\t"Lobo_Fighting_pos",\n\t\t"uddedushka",\n\t\t"kjbeam",\n\t\t"kj_roar"\n\t};', 1)
assert '"kj_roar"' in s, 'mission.sqm: addOns[] anchor not found'
open(dst, 'w', encoding='utf-8', newline='\n').write(s)

k = open(shake, encoding='utf-8', newline='').read()
nl = '\r\n' if '\r\n' in k else '\n'
anchor = '_s = _this select 0' + nl
assert anchor in k, 'shake.sqs changed: anchor not found'
k = k.replace(anchor, '? not (isNil "PROMO_NOSHAKE") : exit' + nl + anchor, 1)
open(shake, 'w', encoding='utf-8', newline='').write(k)
print('patched', dst, shake)
