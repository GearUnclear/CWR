// kj\brain\have.sqf: which other features' files are in this mission, in the
// order [roar, beam, crush, anim timing table]. Empty means "look for the
// files" (start.sqs then asks loadFile for each; a missing file is reported
// by the engine as a warning, which is harmless but noisy). run-brain.ps1
// writes the real list into the staged copy. A mission may also set
// KJ_brainHas itself before the brain starts.
KJ_brainHas = [];
