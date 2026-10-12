# Arma 3 aircraft keyboard controls

Bring Arma 3's familiar aircraft keyboard layout to Uslu dur! through the existing context profiles. Helicopters use W/S for pitch, A/D for bank, Q/E for pedals and Left Shift/Z for lift. Planes use the same pitch, bank and rudder keys, with Left Shift/Z for acceleration. Keep permanent mouse freelook and the existing simulation, assistance, joystick response and aircraft data.

**Status:** implementation plan, 2026-10-12; tracked in [issue #82](https://github.com/GearUnclear/CWR/issues/82). Research baseline: [`4b5c940`](https://github.com/GearUnclear/CWR/commit/4b5c94060f30c3df7492e072f95f5fe8082fbd33). The implementation targets the full official CWA Remaster. Runtime acceptance checks below are future work; the research did not launch the game.

## Scope and intended behavior

This completes the aircraft keyboard portion of the existing **Arma 3 controls** preset. Preserve its current RMB tap/hold, target/watch and infantry-lean behavior (issues [#62](https://github.com/GearUnclear/CWR/issues/62) and [#68](https://github.com/GearUnclear/CWR/issues/68)). Other vehicle and infantry layouts are outside this issue.

The transfer is about keys and discoverable controls. Helicopter lift keys still adjust the existing terrain-relative height target; plane acceleration keys still adjust the existing requested-speed target. Preserve attitude assistance, autohover behavior, automatic thrust/braking and all input gains. The preset must describe these behaviors accurately rather than promise Arma 3 dynamics.

Mouse movement continues to look around every aircraft pilot seat. Do not exempt aircraft from the seat freelook lock or reinstate the deleted mouse-flight branches. No RotorLib, lift/drag changes, direct-collective mode, manual-throttle replacement, new speedbrake policy, VTOL system, trim system or Classic compatibility work belongs here.

## Reference layout and deliberate differences

Bohemia's [Arma 3 vehicle field manual](https://community.bohemia.net/wiki/Arma_3%3A_Field_Manual_-_Vehicle_Controls?useskin=darkvector) describes the plane layout: W/S pitch, A/D bank, Q/E rudder, Left Shift/Z throttle. Its [action and preset reference](https://community.bohemia.net/wiki/inputAction/actions/bindings) documents the helicopter equivalents, X autohover and G landing gear. Use the **Arma 3 / Apex** column, not Arma 2, Alternative, Reforger or Industry Standard.

| Existing UD action | Helicopter default and UI label | Plane default and UI label |
| --- | --- | --- |
| `UAMoveForward` | W: Nose down | Left Shift: Increase speed |
| `UAMoveBack` | S: Nose up | Z: Decrease speed |
| `UATurnLeft` / `UATurnRight` | A / D: Bank left / right | A / D: Bank left / right |
| `UAMoveLeft` / `UAMoveRight` | Q / E: Left / right pedal | Q / E: Rudder left / right |
| `UAMoveUp` | Left Shift: Raise lift | S: Nose up |
| `UAMoveDown` | Z: Lower lift | W: Nose down |
| `UAMoveFastForward`, `UAMoveSlowForward`, `UATurbo` | No keyboard defaults | No keyboard defaults |
| Proposed `UAAircraftAutoHover` | X: Toggle autohover | Unbound and hidden |
| Proposed `UAAircraftGear` | Unbound and hidden unless an existing supported gear operation is verified | G: Toggle landing gear, where supported |
| `UACompass` | K: Compass | K: Compass |

The table specifies primary defaults. Replace the affected aircraft keyboard/mouse rows as a bundle when applying the preset; do not leave CWA arrow/Page Up/Page Down alternates attached to their old plane meanings. Any alternates added later must follow the displayed aircraft action. Preserve controller entries.

UD differences are deliberate: mouse freelook stays permanent; plane acceleration is a speed target; helicopter lift is a height target; plane X speedbrake is not supplied; unsupported helicopter landing gear remains unavailable. Flaps remain available through the existing action menu. Bohemia's reference pages give inconsistent flap-chord details, and this engine permits a modified key to also activate its unmodified binding. Do not promise a literal flap shortcut or silently introduce Ctrl+L headlights activation. Bindable flap shortcuts can follow after chord conflicts are resolved, using the existing flap operations.

## Verified implementation entry points

All paths below are under `arma_CWR/`. Links pin the inspected source rather than a moving branch.

| Area | Source and implication |
| --- | --- |
| Aircraft consumers | [`HelicopterAI.cpp`](https://github.com/GearUnclear/CWR/blob/4b5c94060f30c3df7492e072f95f5fe8082fbd33/arma_CWR/engine/Poseidon/World/Entities/Vehicles/Air/HelicopterAI.cpp#L416) and [`AirplaneAI.cpp`](https://github.com/GearUnclear/CWR/blob/4b5c94060f30c3df7492e072f95f5fe8082fbd33/arma_CWR/engine/Poseidon/World/Entities/Vehicles/Air/AirplaneAI.cpp#L569) already read the actions in the table. Remapping them does not require replacing flight controls. |
| Context defaults and preset | [`UI/Settings/ContextControlsConfig.cpp`](https://github.com/GearUnclear/CWR/blob/4b5c94060f30c3df7492e072f95f5fe8082fbd33/arma_CWR/engine/Poseidon/UI/Settings/ContextControlsConfig.cpp#L297) seeds defaults by context. `ApplyArma3Preset()` currently updates only aiming/HUD actions and infantry lean. |
| Hidden speed modifier | [`Input/InputSubsystem.cpp`](https://github.com/GearUnclear/CWR/blob/4b5c94060f30c3df7492e072f95f5fe8082fbd33/arma_CWR/engine/Poseidon/Input/InputSubsystem.cpp#L722) promotes forward into fast-forward whenever `UATurbo` is held. Global defaults put Q on slow-forward, E on fast-forward and Shift on Turbo. |
| Shared Pilot UI | [`UI/Options/BindingsPage.cpp`](https://github.com/GearUnclear/CWR/blob/4b5c94060f30c3df7492e072f95f5fe8082fbd33/arma_CWR/engine/Poseidon/UI/Options/BindingsPage.cpp#L43) reads the helicopter as the primary Pilot context and broadcasts edits to both aircraft. `InputSubsystem.cpp` and `GamepadPage.cpp` also own category/context tables. |
| Category actions and labels | `Input/ControlsCategory.{hpp,cpp}`, `UI/Options/BindingsPage.{hpp,cpp}`, `KbmPage.cpp` and `OptionsPage.cpp` supply categories, visible rows, labels, descriptions and capture-dialog text. `ActionLabelOverride()` already provides an extension point. |
| Reset paths | `BindingsPage::ResetCurrentCategoryToDefaults()` and `InputSubsystem::ResetCategoryDefaults()` rebuild global descriptor defaults, rather than the proposed aircraft layout. Reset-all loops categories, including Common. |
| Saved profiles | `ContextControlsConfig::Load/Save` uses named per-context rows, modifier and scale arrays; the current version is 5. Loading an older version seeds defaults before overlaying stored rows. `Save()` skips fully empty rows. |
| Existing aircraft operations | `HelicopterAuto::PerformAction(ATAutoHover)` and `AirplaneAuto::PerformAction(ATLandGear/ATFlapsUp/ATFlapsDown)` already implement menu commands. Their `GetActions()` methods enforce availability. These are `ActionType`s, not currently bindable `UserAction`s. |
| Remaster localization | `setup-guerrilla.ps1` installs manifest-owned files and generates a missing-key `STRINGTABLE_UD` shard. Official Options resources are kept. Use UD-owned localization keys/files and update `guerrilla-mode/runtime/manifest.json`; do not replace Steam resources or extend the Classic shim. |

## How to implement

### 1. Share the aircraft layout definition

Add a context-aware aircraft keyboard-default provider, for example `ApplyAircraftKeyboardDefaults(InputContext, InputProfile&)`. This is a proposed helper, not an existing API. Restrict it to `HeliPilot` and `PlanePilot`; reuse existing `UserAction` identities and serialized names for movement.

Construct each affected row using `GetBindingEntries()` and `SetBindingEntries()`. Replace only keyboard/mouse entries; preserve stick/button/POV/axis entries, their modifiers, scales, order and duplicate entries. Do not round-trip through `Bind()` when preservation is required: it deduplicates.

Use the same provider for new-profile defaults, explicit preset application and aircraft keyboard reset. Do not change global `UserActionDesc` movement keys to accomplish this: the same actions control soldiers, cars and boats. Preserve the existing unrelated preset behavior and controller configuration.

Clear the aircraft keyboard entries for `UAMoveFastForward`, `UAMoveSlowForward` and `UATurbo` in the same operation. Otherwise E can request yaw and fast-forward together, and Shift+W can change helicopter pitch authority; plane Shift can also promote its acceleration input and double the target adjustment rate. Retaining the global descriptors is fine; active aircraft profiles must not contain these overlaps after applying the layout.

### 2. Separate helicopter and plane editing

Replace the shared Pilot category with **Helicopter** and **Plane** categories. Keep the existing `InputContext` identities and saved context prefixes. Category IDs are UI projection data, not a reason to rename saved actions.

Update category lists, counts, localized names, row arrays and every category-to-context mapping, including the copies in `BindingsPage.cpp`, `InputSubsystem.cpp` and `GamepadPage.cpp`. Prefer sharing the context mapping so edits, conflict detection and reset cannot disagree. Editing, clearing or resetting one aircraft category must affect only that aircraft context.

Supply the table's contextual labels and useful descriptions. Explain the height/speed targets in row descriptions and the player help; leave internal movement names out of player-facing text. Use the same contextual label in the binding table, capture dialog and conflict prompt, not only `Provider::RowLabel()`.

Common edits/reset currently broadcast across contexts. A Common compass reset must not restore G in a plane profile that uses it for gear. Define context-aware defaults for affected common actions and make conflict checks include all actions active in the selected context, even if they appear in another UI category. Add reset-all ordering coverage; invoking Common last must not undo aircraft defaults.

### 3. Keep saved-profile adoption explicit

New installations and deliberate reset use the new defaults. Existing saved profiles keep their aircraft movement layout until the player applies the preset. Do not infer consent from a version upgrade or rewrite custom rows individually. An explicit preset may replace customized keyboard rows in its documented aircraft bundle; controller entries remain intact.

Update the existing preset confirmation to state that it changes aircraft keyboard movement and supported shortcuts, alongside its existing aim/watch/lean changes. Keep its existing apply/cancel interaction. Applying twice must be idempotent; applying and saving must survive reload.

If the two new shortcut actions require a version-6 migration, make it narrowly initialize those actions **unbound for existing profiles** and preserve every old aircraft movement row. The current older-file seeding must not silently adopt the new layout or populate intentionally cleared rows. Preserve explicit empties, duplicate entries, modifiers and non-default scales. Write empty arrays in the new format where absence would otherwise be confused with a missing action. Test real version-5 fixtures, older files and future-version handling before finalizing the migration. A pure movement-binding milestone may avoid a version bump if its existing-format round-trip tests pass.

### 4. Bind existing aircraft commands

Add the proposed autohover and landing-gear `UserAction`s with aligned descriptor entries and appropriate category rows. New action defaults are context-owned: do not globally bind X or G. Plane gear moves the plane's keyboard compass from G to K within the explicit preset/default bundle. Autohover replaces the heli's old X rudder binding through the movement remap.

Poll command edges through the production input API (`GetActionToDo`, with its current-context semantics verified), so a held key toggles once. Resolve the controlled local driver and enforce the same availability as `GetActions()`: correct seat, manual-control eligibility, supported retracting gear and damage restrictions. Dispatch the existing `PerformAction()` operation through the normal action path. Do not mutate `_hoveringAutopilot`, `_pilotGear` or flap fields directly from generic input code.

Handle simultaneous menu and shortcut input once. Map/dialog/chat focus, lost focus, seat changes, load and control transfer must not replay a held key as a stale command. Keep the action menu available. Unsupported helicopter gear gets no fake shortcut or hidden new simulation. Plane speedbrake remains outside this issue because `_pilotBrake` is owned by the speed controller; adding an independent braking policy would change behavior.

### 5. Ship Remaster text and documentation

Add UD-owned strings for the two categories, contextual labels/descriptions, shortcut labels and expanded preset explanation through the tracked Remaster installation path. A dedicated controls shard under `guerrilla-mode/runtime/common/bin/` with an install manifest entry is a possible implementation; verify discovery and ordering before choosing it. Use new UD identifiers where official text would otherwise win or be overwritten. Update source hashes in the runtime manifest.

Update `guerrilla-mode/FRESH-INSTALL.md` and the player-facing controls help with the how-to below. Installer checks use a disposable Remaster shadow, not the Steam folder. No Classic lane or Demo gameplay target is required.

## How to use the planned controls

These steps describe the result after implementation, not functionality already delivered by this planning commit.

1. Open **Options -> Controls -> Arma 3 controls**, review the affected controls and apply the preset. Existing saved aircraft bindings are kept until this step.
2. Under **Keyboard & Mouse**, select **Helicopter** or **Plane** to inspect/rebind that aircraft only. Saving or resetting one category must leave the other intact.
3. In a helicopter, W lowers the nose, S raises it, A/D banks and Q/E uses the pedals. Left Shift raises lift and Z lowers it through the existing height controller. X toggles autohover.
4. In a plane, W lowers the nose, S raises it, A/D banks and Q/E uses the rudder. Left Shift increases requested speed; Z decreases it. G operates supported landing gear. Use the action menu for flaps and automatic landing.
5. Mouse movement always looks around. Aircraft flight assistance is unchanged. Use K for the compass in the aircraft preset; existing aiming/watch controls continue to work.

## Delivery and acceptance

- [ ] **A: layout and UI.** Shared context-default provider; separate aircraft categories; contextual labels in table/capture/conflict views; no keyboard Turbo/fast-forward collisions.
- [ ] **B: preset and persistence.** New/reset defaults agree with explicit preset; existing movement profiles remain unchanged on load; device-preserving replacement and clear-row round trips pass; reset-all cannot restore conflicting Common defaults.
- [ ] **C: existing-command shortcuts.** X autohover and supported plane G gear are edge-triggered, correctly guarded and use existing operations. Flaps remain accessible through the menu.
- [ ] **D: Remaster delivery.** Tracked localization and manifest hashes, installation checks and player how-to are complete.
- [ ] **E: acceptance evidence.** Focused tests plus later authorized full-Remaster input/UI and flight checks pass. Record limitations honestly; do not close the issue after documentation or unit tests alone.

Extend existing suites rather than assert only that a constant table equals itself:

| Suite or scenario | Required behavior |
| --- | --- |
| `tests/unit/engine/Poseidon/UI/Settings/test_contextControlsConfig.cpp` | Fresh defaults, explicit preset twice, v5/custom/cleared profiles unchanged on load, new-format save/reload, preserved controller/modifier/scale/duplicate/empty-slot entries. Existing RMB/lean preset cases remain valid. |
| `tests/unit/engine/Poseidon/Input/test_controls_category.cpp` and `tests/unit/engine/Poseidon/UI/test_kbm_page.cpp` | Distinct aircraft labels and rows; rebind/clear/reset isolates the selected context; conflicts consider active Common actions; category reset preserves controller bindings. |
| `tests/unit/engine/Poseidon/Input/test_input_integration.cpp` | Feed actual key events. Shift+W heli requests normal nose-down plus lift, not Turbo/fast-forward; E produces only right pedal/rudder; plane W/S pitches while Shift/Z adjusts its speed target. Release/opposite-key behavior retains existing control law. |
| Aircraft command tests | One toggle per press; held/repeated/focus-lost input does not retrigger; wrong-seat/unsupported/damaged gear is rejected; menu and key dispatch agree. Add a narrow test seam if necessary, not a second aircraft simulation. |
| Remaster installer/UI checks | New strings resolve; preset confirmation is accurate; source hashes validate; Bohemia package files stay intact; helicopter/plane categories and save/reload are usable at supported resolutions. |
| Later full-Remaster acceptance | Keyboard takeoff, turns, hover/autohover and landing; plane pitch/acceleration/gear; mouse motion changes camera without flight commands; original controller mode remains usable. Run only when game launch is authorized. |

Future implementation can build `PoseidonTests` and run the focused `[ContextControlsConfig]`, `[ControlsCategory]` and relevant input/UI cases after checking their registered tags. Extend `tests/integration/ui/options/controls/` or add a Remaster-pinned companion; do not assume existing fixtures already provide aircraft runtime probes. Test names and harness commands added by the implementation must be real.

The research/planning commit needs documentation and roadmap validation, not an engine build. No runtime pass is claimed. The substantive open work is implementation and the later Remaster acceptance matrix, not retuning the flight model.
