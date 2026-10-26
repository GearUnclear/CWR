# B8 landing unit gate on the merged tree: plan section 8 item 3 plus the landed lanes' tags and [AirMedium].
Set-Location D:\
$exe = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe'
$tags = '[remaster],[Shaders],[tri],[triSetLighting],[GL33],[gpu],[ShadowSampler],[DebugGroup],[ShutdownOrder],[UBORestoreGuard],[StateOwnership],[StateCacheAudit],[BufferUsageAudit],[IboBindAudit],[Conventions],[Settings],[GraphicsConfig],[GraphicsApply],[scene],[Shape],[TemporalAA],[occlusion],[UboMirror],[PerfCounters],[FeatureSwitches],[FeatureGates],[PostStages],[triSetFixedStep],[triPinCloudDrift],[atmosphere],[TerrainDetail],[LumaSharpen],[SunShadows],[FrameLights],[SeaWaves],[build-descriptor],[SkyDensity],[clouds],[Canopy],[AirMedium]'
& $exe $tags
"exit: $LASTEXITCODE"
