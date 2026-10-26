#!/bin/sh
# Logs PoseidonGame PIDs with their data dir every 5 s (to map pmon rows to runs).
out="$1"
while true; do
  wmic process where "name='PoseidonGame.exe'" get ProcessId,CommandLine 2>/dev/null | tr -d '\r' | grep -o '\-C [^ ]*\|[0-9]\+ *$' | paste - - | sed "s/^/$(date +%H:%M:%S) /" >> "$out"
  sleep 5
done
