#!/bin/bash
# agent 1: re-measure candidate clips with the kit; usage: remeasure.sh out.txt clip.mkv ...
out=$1; shift
python D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit/fogmotion.py "$@" --from 15 > $out 2>&1
