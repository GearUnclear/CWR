# Issue #62: full environment verification

Run against the **Classic 1.99** data package with the tracked Classic runtime
shims installed, using a newly built `PoseidonGame`. The Abel fixture, LAW and
M60 are Classic assets. See [engine setup](../../../../CLAUDE.md) for the build
and data prerequisites.

From `arma_CWR`, build the game and Trident:

```powershell
cmake --build build/win-x64-clang-rwdi --target PoseidonGame
cargo build --manifest-path engine/Trident/Cargo.toml
```

Close other game instances before running this command. It runs the scenario
with the dummy renderer and guarantees cleanup even after a test failure:

```powershell
try {
    & .\engine\Trident\target\debug\tri.exe test `
        --game-dir dist/x64-win-rwdi `
        --data-dir 'D:\Arma_CWA\ARMA Cold War Assault [Classic]' `
        --render dummy -j1 --retries 2 `
        tests/integration/ingame/controls/rmb_zoom_ads.test.sqf
    $testExit = $LASTEXITCODE
} finally {
    Get-Process PoseidonGame -ErrorAction SilentlyContinue | Stop-Process -Force
}
if ($testExit -ne 0) { throw "RMB integration test failed: $testExit" }
```

The automated scenario checks zoom while RMB remains pressed and a tap is
still eligible, release-only sight toggles, real holds longer than the default
250 ms window, unchanged sight FOV during a hold and its release, V toggles on
keydown, LAW hold zoom, and M60 driver/gunner behavior. It uses `triMouseBtn`
through SDL and observes `triCamView` / `triCamFov`. Tap cases widen the window
to 2000 ms to accommodate harness latency; hold cases use 250 ms and explicitly
pump the press before waiting 350 ms. Unit tests cover the exact 250 ms boundary.

Also run these checks in the rendered game with the default 250 ms tap window:

1. With a rifle in first and third person, RMB starts zooming on press. Release
   a short click to enter sights, and click again to return. The brief zoom
   before a tap enters sights is expected. Holding and releasing RMB zooms and
   recovers without toggling sights. Repeat with the LAW equipped.
2. In rifle and vehicle sights, hold/release RMB: neither view nor magnification
   changes. V still toggles sights on press. Numpad `+` / `-` retain their
   existing behavior, including magnification in optics that support it.
3. Repeat the tap/hold checks from vehicle driver and gunner seats. Open the
   map and right-drag or right-click: the map remains open.
4. Check fresh-profile defaults: Optics shows `tap RMB` and V, temporary zoom
   uses RMB, Lock Target uses T, Watch uses O plus POV-right, and Reveal Target
   still uses RMB. With a compatible target, T locks it and RMB still reveals
   it without triggering the lock action.
5. Open a profile with exactly the previous stock Optics / Lock Target / Watch
   bindings and confirm the migration. Save and reopen it to confirm the new
   bindings persist. Repeat with custom bindings for each action and confirm
   they are preserved; changing one action must not prevent migration of the
   other unchanged stock actions.
6. Connect a gamepad and confirm LT still operates optics and the existing pad
   controls remain intact. This change does not add a pad zoom binding.
7. In the Input dev panel, change the tap window and confirm the gesture cutoff
   follows it; restore 250 ms afterward. Persistence of this debug setting and
   capture of new tap bindings in the rebinding UI are separate follow-ups.

Record the build commit, package used, automated result/artifact location, and
manual outcomes on the PR. The full environment label remains useful until
these runtime checks have been executed; source checks alone do not verify
camera feel, asset-specific optics, or physical controller behavior.
