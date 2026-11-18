#!/bin/bash
# Runs several promo takes one after another (one game at a time through run-promo.ps1) and encodes each
# take's dumps while the next take runs.  bash run-batch.sh land occ1 ...
# At 2560 x 1440 a dump is about 11 MB a frame (a 10 s clip about 3.5 GB, a take up to 35 GB), so before
# each take the batch waits for the earlier encodes until the production folder's drive has 50 GB free
# (run-promo.ps1 itself stops below 15 GB). The production folder is $UD_PROMO_DIR, default
# <worktree>/tmp/video/lebanon80-promo (udpaths.py); run-promo.ps1 reads the same variable.
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
PROMO="${UD_PROMO_DIR:-$REPO/tmp/video/lebanon80-promo}"
ROOTW="$(cygpath -w "$PROMO" | cut -c1-3)"
free_gb() { powershell -NoProfile -Command "[int]((New-Object IO.DriveInfo '$ROOTW').AvailableFreeSpace / 1GB)" | tr -d '\r'; }
for n in "$@"; do
  while [ "$(free_gb)" -lt 50 ] && [ -n "$(jobs -rp)" ]; do sleep 10; done
  out=$(powershell -NoProfile -File "$(cygpath -w "$HERE/run-promo.ps1")" -Name "promo_$n" -TimeoutSec 3600 | tee /dev/stderr | grep '^OUT=' | sed 's/^OUT=//' | tr -d '\r')
  echo "TAKE $n done -> $out"
  [ -n "$out" ] && python "$HERE/encode_promo.py" "$out" > "$out/encode.log" 2>&1 &
done
wait
echo "BATCH END"
