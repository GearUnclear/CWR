$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath 'fixture-noarchive\relcwd'
"PS location: " + (Get-Location).Path
"process cwd: " + [Environment]::CurrentDirectory
if ([Environment]::CurrentDirectory -ieq (Get-Location).Path) { throw 'precondition: process cwd equals the PS location, the test is meaningless' }
& 'D:\Arma_CWA\.claude\worktrees\lobo-family-package\arma_CWR\tools\lobo\package-lobo-family.ps1' -LoBoDir '..\fake-lobo' -OutDir 'rel-out' -CacheDir '..\fake-cache' -SkipFetch -PlanOnly -NoArchive -Tools '..\stubs\PoseidonTools.cmd' -RepairTool '..\stubs\repair_lobo.py' -Version REL 6>&1 | ForEach-Object { [string]$_ }
"SCRIPT EXIT=$LASTEXITCODE"