#!/bin/bash
# agent 1 (fogwind): measure a finished real-time run: measure_rt.sh <tag>
# rtst (camera held, real time) with --static; rt60 (the pan, frames 0-150).
cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1
tag=$1
{
  python kit/fogmotion.py rec/${tag}_rtst.mkv --from 10 --static --noband --regions bay,valley,farhill,haze
  python kit/fogmotion.py rec/${tag}_rt60.mkv --from 0 --to 150 --noband --regions bay,valley,farhill,haze
} > out/rt_${tag}.txt 2>&1
grep -h "shader compile error" runs/rt-${tag} -r | head -3 >> out/rt_${tag}.txt
