#!/bin/sh
# runvar.sh tests variantdir...: sequential runs with each override dir (lane-relative)
lane=/d/Arma_CWA_worktrees/graphics-remaster/tmp/look/fog
win='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fog'
tests=$1; shift
for v in "$@"; do
  if [ "$v" = none ]; then
    powershell -NoProfile -File $lane/run.ps1 -Tests "$tests" -Tag "_$TAG" > $lane/run_$TAG.log 2>&1
  else
    powershell -NoProfile -File $lane/run.ps1 -Tests "$tests" -Tag "_$v" -Override "$win/$v" > $lane/run_$v.log 2>&1
  fi
done
echo ALLDONE
