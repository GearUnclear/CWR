Lobo's M833 and Micro Uzi references were broken, MG ammunition stopped contributing to AI weapon evaluation beyond 109 m, and copied scripts contained invalid state, labels and leaked helpers. Tank configs also targeted missing model selections while authoring unsupported directional components.

Adds reproducible repairs for the transferred audit, now tracked at GearUnclear/lobo#1 (formerly CWR #69): ammunition and blast policies, missile/landing-craft/wreck scripts, missing diver models, six tank model bindings, and display text. Preserves existing mission-facing class/component identifiers and the six-component damage layout. The archive runner validates every transformation, stages all output, keeps verified originals, preserves untouched bytes, rejects concurrent changes, and handles rollback conflicts.

The default UD fire query covers nearby infantry. An explicit Classic profile uses a documented group-limited fallback because Classic lacks `nearestObjects`. Includes offline diagnostics, policy documentation and Windows/Linux synthetic CI. No third-party addon payloads are committed.

Validation on the local UD installation:

- 81 synthetic tests pass, including corruption, idempotence, concurrency and rollback failures.
- Applied 76 members in 25 archives; second pass makes zero changes.
- 11 changed configs export through PoseidonTools; all 7,993 explicit LoBo references resolve in the 114-export definition index.
- Mounted standalone Man/Land probe improves from 1,747/1,749 to 1,749/1,749.
- Independent verification confirms all archive/backups/member hashes, 6,129 untouched members byte-identical, and all six installed models readable.

No game was launched. Runtime combat, multiplayer and save acceptance remain pending, so this PR is draft. The lightweight parser explicitly reports five malformed legacy configs as incomplete coverage; the separate export index omits two previously unexportable AGS configs. Neither the export index nor lexical SQS checks claim runtime or merged-inheritance validation.
