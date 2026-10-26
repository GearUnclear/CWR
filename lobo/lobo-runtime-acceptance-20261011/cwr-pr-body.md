The LoBo repair had offline verification but no reproducible runtime acceptance record. Add opt-in Trident missions, a bounded local runner, and evidence notes for the remaining audit acceptance work. These files are original test sources; logs, captures, saves and third-party game data stay local.

Verified in the installed UD profile: all 30 affected missiles plus two Mk20 releases, pylon clearing, both LCM number displays, native Jordanian M833 firing, 144 blast observations, nine direct MG impacts with zero nearby damage at a measured two metres, and eight-tank fresh-process save/load.

Acceptance remains partial. Long-range AI firing lacks a successful control in the clear-lane fixture. Hosted MP clocks advance, but remote peers do not observe the host's wreck destruction or tank damage; expected fire glow and cleanup remain unverified. Turret screenshots show no visible turn. The README records exact runs, local evidence paths, measured values and limitations, including the failed dedicated attempt with mismatched build tags.

Validation: successful scoped Trident runs and reviewed images, TOML parsing, PowerShell parser, git diff --check. Final UD repair check exits 0 with zero changed members; report-only doctor exits 0 with Nothing to do. All 124 PBO hashes, roster config and runtime binary hashes are unchanged. No repair rule or installed PBO was changed.

Related acceptance tracker: https://github.com/GearUnclear/lobo/issues/1. This PR does not close it.
