#!/bin/bash
# usage: analyse.sh <tag> [more tags]; writes metrics/<tag>.txt (agent 1's fogmotion) per run
cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/3
for t in "$@"; do
  python ../1/kit/fogmotion.py runs/$t/$t.mkv --from 15 --json metrics/$t.json > metrics/$t.txt 2>&1
  python tools/gpucost.py runs/$t >> metrics/$t.txt 2>&1
done
