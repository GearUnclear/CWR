cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/5
until grep -q "^done" runs/m3moving-c8h2/status.txt 2>/dev/null; do sleep 5; done
sleep 3
K=D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit/run_probe_fog.ps1
R='D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\5\runs'
S='D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\5\shaders'
powershell -NoProfile -ExecutionPolicy Bypass -File $K -Probe fog1_static -Tag static-c8h2 -OutRoot $R -Junction RemasterUD-fog-5 -ShaderOverrideDir "$S\c8h" > runs/static-c8h2.out 2>&1
echo done
