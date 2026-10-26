#!/bin/bash
# usage: loop.sh <test> <from> <to> [per-trial timeout s]
cd /d/Arma_CWA/arma_CWR
t=$1
lim=${4:-1800}
for i in $(seq $2 $3); do
  out=tmp/of3-air/$t-$i
  rm -rf "$out"
  RUST_LOG=tri=debug timeout $lim ./target/debug/tri.exe test -j1 --retries 0 --game-dir dist/agent67-rwdi --data-dir D:/Arma_CWA/tmp/remaster-shadow-of3 --output-dir $out tmp/of3-tests/$t.test.sqf > $out.tri.txt 2>&1
  rc=$?
  powershell -NoProfile -File tools/stop-owned-games.ps1 -DataDir 'D:\Arma_CWA\tmp\remaster-shadow-of3' > /dev/null 2>&1
  echo "trial $i exit $rc" >> tmp/of3-air/$t-results.txt
done
