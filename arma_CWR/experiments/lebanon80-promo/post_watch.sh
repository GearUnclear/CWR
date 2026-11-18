#!/bin/bash
# Runs post_take.py on every finished promo run that has no post result yet; appends to work/post/watch.log.
# Leave it running beside a batch:  bash post_watch.sh &
# Probe runs (promo_*probe*, promo_jdiag*) are marked done without a check; so is any run listed in
# work/post/skip.txt (one run folder name per line).
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
W="${UD_PROMO_DIR:-$REPO/tmp/video/lebanon80-promo}/work"
mkdir -p "$W/post"
while true; do
  for R in "$W"/runs/promo_*; do
    [ -d "$R" ] || continue
    b=$(basename "$R")
    [ -f "$W/post/$b.done" ] && continue
    case $b in promo_*probe*|promo_jdiag*) touch "$W/post/$b.done"; continue;; esac
    if [ -f "$W/post/skip.txt" ] && grep -qxF "$b" "$W/post/skip.txt"; then touch "$W/post/$b.done"; continue; fi
    log=$(ls "$R"/*/game_stdout.log 2>/dev/null | head -1)
    [ -n "$log" ] || continue
    grep -aq "PROMO END\|aborting" "$log" || continue
    { echo "######## $b $(date +%H:%M:%S)"; grep -a "aborting" "$log" | tail -1; python "$HERE/post_take.py" "$R" 2>&1 | grep -v "^   log"; } >> "$W/post/watch.log"
    touch "$W/post/$b.done"
  done
  sleep 20
done
