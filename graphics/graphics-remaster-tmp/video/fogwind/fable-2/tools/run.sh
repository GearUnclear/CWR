#!/bin/bash
# run.sh <jobs-file> [parallel]   each line: label|clip|extra args    (fable-2)
V=/d/Arma_CWA_worktrees/graphics-remaster/tmp/video
O=$V/fogwind/fable-2/out
T=$V/fogwind/fable-2/tools/fogflow.py
P=${2:-4}
mkdir -p "$O"
n=0
while IFS='|' read -r label clip extra; do
  [ -z "$label" ] && continue
  case "$label" in \#*) continue;; esac
  ( python "$T" "$V/$clip" --label "$label" --json "$O/$label.json" --out "$O" $extra > "$O/$label.txt" 2> "$O/$label.err"; echo "done $label rc=$?" ) &
  n=$((n+1))
  if [ $((n % P)) -eq 0 ]; then wait; fi
done < "$1"
wait
echo ALLDONE
