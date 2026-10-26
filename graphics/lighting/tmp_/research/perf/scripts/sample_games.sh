#!/bin/sh
# Logs the number of running PoseidonGame processes every 3 s (GPU contention context).
out="$1"
while true; do
  n=$(tasklist //FI "IMAGENAME eq PoseidonGame.exe" 2>/dev/null | grep -c PoseidonGame)
  echo "$(date +%H:%M:%S) $n" >> "$out"
  sleep 3
done
