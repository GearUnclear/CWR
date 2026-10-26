$out = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/r5_build'
Remove-Item "$out/unit.done" -ErrorAction SilentlyContinue
Set-Location 'D:\'
$exe = 'D:/Arma_CWA_worktrees/look-director/arma_CWR/build/look-director-rwdi/tests/unit/engine/Poseidon/PoseidonTests.exe'
& $exe '[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],[AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera],[TerrainHorizon],[StateOwnership],[atmosphere],[aisight],[Remaster],[NightLights]' *>&1 | Out-File -Encoding utf8 "$out/unit.txt"
$code = $LASTEXITCODE
[IO.File]::WriteAllText("$out/unit.done", "exit=$code")
