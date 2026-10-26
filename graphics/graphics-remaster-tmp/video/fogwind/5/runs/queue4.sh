cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/5
until grep -q "^done" runs/m3moving-c8b/status.txt 2>/dev/null; do sleep 5; done
sleep 3
K=D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit/run_probe_fog.ps1
R='D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\5\runs'
S='D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\5\shaders'
powershell -NoProfile -ExecutionPolicy Bypass -File $K -Probe fog5_static40 -Tag s40-prodb -OutRoot $R -Junction RemasterUD-fog-5 -ShaderOverrideDir "$S\prodb" > runs/s40-prodb.out 2>&1
echo done
