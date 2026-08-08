// One-shot probe: which of the @udshowcase preload entries actually ended up
// in World::_activeAddons?  The assert is designed to FAIL - its message is the
// payload (the harness prints "expected X, got Y", and Y is the answer).
//
// Two harness constraints shape this file: every statement must fit on ONE line
// (the runner feeds the script to the evaluator line by line, so a wrapped array
// literal aborts with "Invalid number in expression"), and triAddonActive returns
// a Bool that must be folded to 1/0 before it reaches format/triAssertEq or the
// harness connection dies mid-read.
triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
probeNames = ["sinai", "LoBoTerror", "LoBoSyria", "LoBoWreck", "LoBo_Objects", "MAP_OilAddon", "MAP_Editorupgrade", "Lobo_Fighting_pos", "LoBolebObject", "LoBoF15", "GWbuild1", "LoBo_Cars_Egy"]
probeOut = ""
probeI = 0
while "probeI < (count probeNames)" do { probeN = probeNames select probeI; probeV = 0; if (triAddonActive probeN) then { probeV = 1 }; probeOut = probeOut + format ["%1=%2 ", probeN, probeV]; probeI = probeI + 1 }
triAssertEq [probeOut, "PROBE"]
triEndTest
