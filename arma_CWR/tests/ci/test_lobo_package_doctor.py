"""package-lobo-family.ps1: the mod doctor pass on the staged @LoBo_Deps and
@LoBo_Extras, on generated pbos only (never the cache, the library or @LoBo).

The packager is dot-sourced by a small PowerShell driver (dot-sourcing defines
its functions and returns before the preflight). The parser and the target
guard need only Windows PowerShell; the end-to-end case also needs a built
PoseidonTools.exe (this checkout's dist, else the main checkout's, else the
LOBO_POSEIDON_TOOLS environment variable) and is skipped without one.
"""

import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

ARMA = Path(__file__).resolve().parents[2]
PACKAGER = ARMA / "tools" / "lobo" / "package-lobo-family.ps1"
TOOLS_REL = Path("dist") / "x64-win-rwdi" / "PoseidonTools.exe"


def powershell():
    if sys.platform != "win32":
        return None
    return shutil.which("powershell")


def main_checkout_arma():
    """arma_CWR of the main checkout when this file sits in a linked worktree."""
    for parent in ARMA.parents:
        dot_git = parent / ".git"
        if dot_git.is_dir():
            return parent / "arma_CWR"
        if dot_git.is_file():
            text = dot_git.read_text(encoding="utf-8", errors="replace")
            for line in text.splitlines():
                if line.startswith("gitdir:"):
                    gitdir = Path(line.split(":", 1)[1].strip())
                    if not gitdir.is_absolute():
                        gitdir = parent / gitdir
                    common = gitdir
                    commondir = gitdir / "commondir"
                    if commondir.is_file():
                        common = Path(os.path.normpath(gitdir / commondir.read_text().strip()))
                    return common.parent / "arma_CWR"
            return None
    return None


def poseidon_tools():
    env = os.environ.get("LOBO_POSEIDON_TOOLS")
    candidates = [Path(env)] if env else []
    candidates.append(ARMA / TOOLS_REL)
    other = main_checkout_arma()
    if other is not None:
        candidates.append(other / TOOLS_REL)
    return next((c for c in candidates if c.is_file()), None)


def make_pbo(entries):
    """Uncompressed pbo with a Vers header; entries: (name, bytes)."""
    header = [b"\0", struct.pack("<5I", 0x56657273, 0, 0, 0, 0), b"prefix\0synthetic\0", b"\0"]
    for name, data in entries:
        header += [name + b"\0", struct.pack("<5I", 0, len(data), 0, 1234, len(data))]
    return b"".join(header) + bytes(21) + b"".join(data for _, data in entries)


BAD_CONFIG = (b"class CfgPatches { class SynthBad { units[]={}; weapons[]={}; requiredAddons[]={}; }; };\r\n"
              b"class CfgAmmo { class Default; class SynthTracer: Default {\r\n"
              b"    tracerColor[]={0.8,0.5,0.0.1,1};\r\n"
              b"}; };\r\n")
CLEAN_CONFIG = (b"class CfgPatches { class SynthClean { units[]={}; weapons[]={}; requiredAddons[]={}; }; };\r\n"
                b"class CfgAmmo { class Default; class SynthOk: Default { tracerColor[]={0.8,0.5,0.1,1}; }; };\r\n")

# Driver: dot-source the packager, set the script-scope paths a build would
# set, then run the requested case and print one JSON document.
DRIVER = r"""
param([string]$Packager, [string]$Root, [string]$ToolsExe, [string]$Case, [string]$TextFile = '')
$ErrorActionPreference = 'Stop'
# Dot-sourcing binds the packager's own param() defaults in this scope, so
# none of this driver's parameter names may match one of the packager's.
. $Packager
$script:StageBase = Join-Path $Root 'stage'
$script:WorkBase = Join-Path $Root '_work'
$script:WorkRoot = Join-Path $script:WorkBase 'T'
$script:StageRoot = Join-Path $script:StageBase 'LoBo-Reloaded-T'
$script:ReportDir = Join-Path $Root 'reports'
$script:ToolsFull = $ToolsExe
$script:LoBoDirFull = Join-Path $Root 'src\@LoBo'
$script:ExtraDirFull = ''
$script:ExtrasDirFull = Join-Path $Root 'src\@LoBo_Extras'
$script:CacheDirFull = Join-Path $Root 'cache'
$script:LibraryDirFull = Join-Path $Root 'library'
switch ($Case) {
    'parse' {
        $recs = @(ConvertFrom-DoctorFixOutput ([IO.File]::ReadAllText($TextFile)))
        $out = @($recs | ForEach-Object { [pscustomobject]@{ Pbo = $_.Pbo; Sites = $_.Sites; Findings = @($_.Findings) } })
        ConvertTo-Json -InputObject $out -Depth 5 -Compress
    }
    'guard' {
        $inside = Join-Path $script:StageRoot '@LoBo_Deps'
        [void][IO.Directory]::CreateDirectory($inside)
        $refused = @()
        foreach ($t in @($script:CacheDirFull, $script:StageRoot, (Join-Path $script:LibraryDirFull 'x'), $script:ExtrasDirFull, $Root)) {
            try { Assert-DoctorFixTarget $t; $refused += $false } catch { $refused += $true }
        }
        $okInside = $true
        try { Assert-DoctorFixTarget $inside } catch { $okInside = $false }
        # A staged folder that holds a build input (never the case in a real
        # build, the preflight forbids it) is refused too.
        $script:CacheDirFull = Join-Path $inside 'Addons'
        $aboveInput = $false
        try { Assert-DoctorFixTarget $inside } catch { $aboveInput = $true }
        ConvertTo-Json -InputObject ([pscustomobject]@{ Refused = $refused; InsideAccepted = $okInside; AboveInputRefused = $aboveInput }) -Compress
    }
    'fix' {
        $deps = @(Invoke-StagedDoctorFix -ModDir (Join-Path $script:StageRoot '@LoBo_Deps') -Label '@LoBo_Deps')
        $extras = @(Invoke-StagedDoctorFix -ModDir (Join-Path $script:StageRoot '@LoBo_Extras') -Label '@LoBo_Extras')
        $gateIn = New-Object System.Collections.Generic.List[object]
        $gateIn.Add(@('@LoBo_Deps', (Join-Path $script:StageRoot '@LoBo_Deps')))
        $gateIn.Add(@('@LoBo_Extras', (Join-Path $script:StageRoot '@LoBo_Extras')))
        $gate = @(Assert-StagedDoctorClean $gateIn.ToArray())
        $lines = Get-DoctorFixLines -DoctorFixes ($deps + $extras) -HasExtras $true
        Write-TextFileCrlf (Join-Path $Root 'changes.txt') $lines
        $out = [pscustomobject]@{
            Deps = @($deps | ForEach-Object { [pscustomobject]@{ Folder = $_.Folder; Pbo = $_.Pbo; Relative = $_.Relative; Sites = $_.Sites; Findings = @($_.Findings); OrigHash = $_.OrigHash; NewHash = $_.NewHash } })
            Extras = $extras.Count
            Gate = @($gate | ForEach-Object { [pscustomobject]@{ Folder = $_.Folder; Pbos = $_.Pbos } })
        }
        ConvertTo-Json -InputObject $out -Depth 6 -Compress
    }
}
"""


@unittest.skipIf(powershell() is None, "needs Windows PowerShell")
class PackagerDoctorTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="lobo-pkg-doctor-"))
        self.driver = self.tmp / "driver.ps1"
        self.driver.write_text(DRIVER, encoding="ascii")

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def run_case(self, case, tools="", text_file=""):
        args = [powershell(), "-NoProfile", "-NonInteractive", "-File", str(self.driver),
                "-Packager", str(PACKAGER), "-Root", str(self.tmp), "-ToolsExe", tools or "none", "-Case", case]
        if text_file:
            args += ["-TextFile", text_file]
        result = subprocess.run(args, capture_output=True, text=True, timeout=300)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        payload = [line for line in result.stdout.splitlines() if line.startswith(("{", "["))]
        self.assertTrue(payload, result.stdout + result.stderr)
        return json.loads(payload[-1])

    def test_parser_keeps_patchable_findings_and_sites_only(self):
        text = "\n".join([
            "CBT_Crew.pbo:",
            "    NOT_PATCHABLE  config.cpp  entry is compressed; an in-place patch would move the header table  [not patchable]",
            "IRAR_EE9.pbo:",
            "    MALFORMED_FLOAT  config.cpp:55  '0.0.1' -> '0.000'",
            "        patch @78792: '0.0.1' -> '0.000'",
            "    backed up original -> X:\\m\\_ud-orig\\IRAR_EE9.pbo",
            "    patched 1 site(s) in place",
            "Wreck.PBO:",
            "    MISSING_DEFINE_HEADER  config.cpp  scope keywords without #define",
            "        UNDEFINED_SCOPE_KEYWORD  config.cpp:12  scope = public",
            "            patch @100: 'public' -> '2     '",
            "    patched 1 site(s) in place",
            "Found 2 patch site(s) in 2 pbo(s), plus 1 not patchable. Rerun with --fix to apply.",
        ]) + "\n"
        path = self.tmp / "doctor.txt"
        path.write_text(text, encoding="ascii")
        records = self.run_case("parse", text_file=str(path))
        self.assertEqual([r["Pbo"] for r in records], ["IRAR_EE9.pbo", "Wreck.PBO"])
        self.assertEqual([r["Sites"] for r in records], [1, 1])
        self.assertEqual(records[0]["Findings"], ["MALFORMED_FLOAT  config.cpp:55  '0.0.1' -> '0.000'"])
        self.assertEqual(records[1]["Findings"][1], "  UNDEFINED_SCOPE_KEYWORD  config.cpp:12  scope = public")

    def test_fix_refuses_every_target_outside_the_staged_package(self):
        result = self.run_case("guard")
        self.assertEqual(result["Refused"], [True, True, True, True, True])
        self.assertTrue(result["InsideAccepted"])
        self.assertTrue(result["AboveInputRefused"])

    def test_staged_fix_patches_moves_backups_out_and_passes_the_gate(self):
        tools = poseidon_tools()
        if tools is None:
            self.skipTest("no PoseidonTools.exe (build arma_CWR or set LOBO_POSEIDON_TOOLS)")
        stage = self.tmp / "stage" / "LoBo-Reloaded-T"
        bad = make_pbo([(b"config.cpp", BAD_CONFIG)])
        clean = make_pbo([(b"config.cpp", CLEAN_CONFIG)])
        (stage / "@LoBo_Deps" / "Addons").mkdir(parents=True)
        (stage / "@LoBo_Deps" / "Addons" / "SynthBad.pbo").write_bytes(bad)
        (stage / "@LoBo_Deps" / "Addons" / "SynthClean.pbo").write_bytes(clean)
        (stage / "@LoBo_Extras" / "addons").mkdir(parents=True)
        (stage / "@LoBo_Extras" / "addons" / "SynthExtra.pbo").write_bytes(clean)
        # The "cache" copy of the defective pbo must keep its bytes.
        (self.tmp / "cache").mkdir()
        (self.tmp / "cache" / "SynthBad.pbo").write_bytes(bad)

        result = self.run_case("fix", tools=str(tools))

        self.assertEqual(len(result["Deps"]), 1, result)
        rec = result["Deps"][0]
        self.assertEqual((rec["Folder"], rec["Pbo"], rec["Relative"], rec["Sites"]),
                         ("@LoBo_Deps", "SynthBad.pbo", "Addons\\SynthBad.pbo", 1))
        self.assertTrue(rec["Findings"][0].startswith("MALFORMED_FLOAT  config.cpp:3"), rec)
        self.assertNotEqual(rec["OrigHash"], rec["NewHash"])
        self.assertEqual(result["Extras"], 0)
        self.assertEqual([(g["Folder"], g["Pbos"]) for g in result["Gate"]], [("@LoBo_Deps", 2), ("@LoBo_Extras", 1)])

        patched = (stage / "@LoBo_Deps" / "Addons" / "SynthBad.pbo").read_bytes()
        self.assertEqual(len(patched), len(bad))
        self.assertNotIn(b"0.0.1", patched)
        self.assertEqual((self.tmp / "cache" / "SynthBad.pbo").read_bytes(), bad)
        # Backups left the package for <WorkRoot>\doctor-orig, holding the pre-fix bytes.
        self.assertFalse((stage / "@LoBo_Deps" / "_ud-orig").exists())
        self.assertFalse((stage / "@LoBo_Extras" / "_ud-orig").exists())
        moved = self.tmp / "_work" / "T" / "doctor-orig" / "@LoBo_Deps" / "SynthBad.pbo"
        self.assertEqual(moved.read_bytes(), bad)
        changes = (self.tmp / "changes.txt").read_text(encoding="ascii")
        self.assertIn("@LoBo_Deps\\Addons\\SynthBad.pbo, 1 site(s)", changes)
        self.assertIn("MALFORMED_FLOAT", changes)
        self.assertTrue((self.tmp / "reports" / "doctor-fix-LoBo_Deps.txt").is_file())
        self.assertTrue((self.tmp / "reports" / "doctor-gate-LoBo_Deps.txt").is_file())


if __name__ == "__main__":
    unittest.main()
