import os, subprocess
os.chdir(r'D:\Arma_CWA')
p = 'arma_CWR/guerrilla-mode/mission/Showcase.Abel/human/coverage.json'
work = open(p, 'rb').read()
head = subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True, check=True).stdout
anchor = b'    "../engine/Poseidon/Game/Guerrilla/GarrisonCache.cpp": ['
entry = b'    "../engine/Poseidon/Game/Guerrilla/RoadRoute.cpp": [\n      "town_support",\n      "garrison"\n    ],\n'
assert head.count(anchor) == 1 and entry in work
staged = head.replace(anchor, entry + anchor)
tmp = p + '.stage.tmp'
open(tmp, 'wb').write(staged); os.replace(tmp, p)
subprocess.run(['git', 'add', p], check=True)
open(tmp, 'wb').write(work); os.replace(tmp, p)
print('staged coverage hunk')
