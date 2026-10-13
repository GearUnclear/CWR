// Regression for the first two leans. The first slid the upper body sideways
// as a pure translation, so partially weighted parts sheared and nothing
// tilted. The second turned the whole hunched rifle stance 35 degrees about
// the belly, which folded it into a bow on the right and stood it upright on
// the left. A lean now sways the hips and rolls the spine and the head and
// shoulders about the bore: the head bone rolls 35 degrees, the eye moves
// sideways about as far either way with little drop, the feet stay put, every
// part keeps its lengths at its joints, and the bore keeps its direction.
triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
leanSub = { [((_this select 0) select 0) - ((_this select 1) select 0), ((_this select 0) select 1) - ((_this select 1) select 1), ((_this select 0) select 2) - ((_this select 1) select 2)] }
leanDot = { ((_this select 0) select 0) * ((_this select 1) select 0) + ((_this select 0) select 1) * ((_this select 1) select 1) + ((_this select 0) select 2) * ((_this select 1) select 2) }
leanLen = { sqrt ([_this, _this] call leanDot) }
leanFlat = { private ["_v", "_n", "_d"]; _v = _this select 0; _n = _this select 1; _d = [_v, _n] call leanDot; [(_v select 0) - _d * (_n select 0), (_v select 1) - _d * (_n select 1), (_v select 2) - _d * (_n select 2)] }
// Unsigned angle in degrees between two vectors seen along an axis.
leanRoll = { private ["_a", "_b", "_c"]; _a = [_this select 0, _this select 2] call leanFlat; _b = [_this select 1, _this select 2] call leanFlat; _c = ([_a, _b] call leanDot) / ((_a call leanLen) * (_b call leanLen)); if (_c > 1) then {_c = 1}; if (_c < -1) then {_c = -1}; acos _c }
// Memory points: 0 eye and head bone, 1-2 feet. LOD 0 selection centroids:
// 0 belly, 1 chest, 2 head, 3-4 shoulders, 5-6 hips, 7 neck.
leanMemory = ["pilot", "lchodidlo", "pchodidlo"]
leanBody = ["bricho", "hrudnik", "hlava", "prameno", "lrameno", "lzadek", "pzadek", "krk"]
leanSample = { [triGetLeanRig [player, leanMemory], triGetLeanRig [player, leanBody, 0]] }
// [sample, index] -> position or bone up vector of a memory point
leanMemPos = { ((((_this select 0) select 0) select 3) select (_this select 1)) select 1 }
leanMemUp = { ((((_this select 0) select 0) select 3) select (_this select 1)) select 2 }
// [sample, index] -> LOD 0 centroid
leanLodPos = { ((((_this select 0) select 1) select 3) select (_this select 1)) select 1 }
// [sample, index, index] -> distance between two LOD 0 centroids
leanDist = { ([[_this select 0, _this select 1] call leanLodPos, [_this select 0, _this select 2] call leanLodPos] call leanSub) call leanLen }
// [sample] -> distance from the neck centroid to the eye
leanNeckEye = { ([[_this, 7] call leanLodPos, [_this, 0] call leanMemPos] call leanSub) call leanLen }
// [sampleA, sampleB, index] -> how far a memory point moved
leanMoved = { ([[_this select 0, _this select 2] call leanMemPos, [_this select 1, _this select 2] call leanMemPos] call leanSub) call leanLen }
// [sampleA, sampleB] -> cosine between the two bores
leanBore = { [((_this select 0) select 0) select 4, ((_this select 1) select 0) select 4] call leanDot }
// [neutral, leaned, side sign] -> the checks every lean must pass
leanCheck = { private ["_n", "_l", "_s", "_axis", "_eye"]; _n = _this select 0; _l = _this select 1; _s = _this select 2; _axis = (_l select 0) select 2; _eye = [[_l, 0] call leanMemPos, [_n, 0] call leanMemPos] call leanSub; [_s * ((_l select 0) select 0) * 57.29578, [[_n, 0] call leanMemUp, [_l, 0] call leanMemUp, _axis] call leanRoll, _s * ((([_l, 0] call leanMemUp) select 0) - (([_n, 0] call leanMemUp) select 0)), _s * (_eye select 0), _eye select 1, [_n, _l, 1] call leanMoved, [_n, _l, 2] call leanMoved, ([_l, 0, 7] call leanDist) - ([_n, 0, 7] call leanDist), (_l call leanNeckEye) - (_n call leanNeckEye), ([_l, 0, 1] call leanDist) - ([_n, 0, 1] call leanDist), ([_l, 3, 4] call leanDist) - ([_n, 3, 4] call leanDist), ([_l, 5, 0] call leanDist) - ([_n, 5, 0] call leanDist), ([_l, 6, 0] call leanDist) - ([_n, 6, 0] call leanDist), [_n, _l] call leanBore] }
leanNeutral = [] call leanSample
// Every probed part exists on the stock soldier.
triAssertEq [count ([leanNeutral, 0] call leanMemPos), 3]
triAssertEq [count ([leanNeutral, 7] call leanLodPos), 3]
triAssertNear [(leanNeutral select 0) select 0, 0, 0.0001]

triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.999 }
leanRight = [] call leanSample
triKeyUp 8
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.999 }
leanLeft = [] call leanSample
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanR = [leanNeutral, leanRight, 1] call leanCheck
leanL = [leanNeutral, leanLeft, -1] call leanCheck
// The production roll is 35 degrees toward the leaning side.
triAssertNear [leanR select 0, 35, 0.5]
triAssertNear [leanL select 0, 35, 0.5]
// The head bone itself rolls 35 degrees about the bore, toward that side.
triAssertNear [leanR select 1, 35, 3]
triAssertNear [leanL select 1, 35, 3]
triAssertGt [leanR select 2, 0.1]
triAssertGt [leanL select 2, 0.1]
// The eye moves sideways at least 0.2 m each way, the two sides within 30%
// of each other, and neither drops it more than 0.15 m.
triAssertGt [leanR select 3, 0.2]
triAssertGt [leanL select 3, 0.2]
triAssertGt [(leanR select 3) / (leanL select 3), 0.7]
triAssertGt [(leanL select 3) / (leanR select 3), 0.7]
triAssertGt [leanR select 4, -0.15]
triAssertGt [leanL select 4, -0.15]
// The feet stay planted.
triAssertLt [leanR select 5, 0.01]
triAssertLt [leanR select 6, 0.01]
triAssertLt [leanL select 5, 0.01]
triAssertLt [leanL select 6, 0.01]
// No shear or stretch: belly to neck, neck to eye, belly to chest, the
// shoulder width and the hips to belly all keep their lengths (the first lean
// grew belly to head by about 30%).
triAssertNear [leanR select 7, 0, 0.015]
triAssertNear [leanR select 8, 0, 0.015]
triAssertNear [leanR select 9, 0, 0.015]
triAssertNear [leanR select 10, 0, 0.015]
triAssertNear [leanR select 11, 0, 0.015]
triAssertNear [leanR select 12, 0, 0.015]
triAssertNear [leanL select 7, 0, 0.015]
triAssertNear [leanL select 8, 0, 0.015]
triAssertNear [leanL select 9, 0, 0.015]
triAssertNear [leanL select 10, 0, 0.015]
triAssertNear [leanL select 11, 0, 0.015]
triAssertNear [leanL select 12, 0, 0.015]
// The sights and the shot keep their point of aim (within one degree).
triAssertGt [leanR select 13, 0.99985]
triAssertGt [leanL select 13, 0.99985]

// Crouched: the same checks.
triKeyDown 75
triSimFrames 3
triKeyUp 75
triSimUntil { (getMove player) == "crouch" }
leanSettle = time
triSimUntil { time > leanSettle + 2 }
leanNeutral = [] call leanSample
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.999 }
leanRight = [] call leanSample
triKeyUp 8
triKeyDown 20
triSimUntil { ((triLeanPose player) select 0) < -0.999 }
leanLeft = [] call leanSample
triKeyUp 20
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanR = [leanNeutral, leanRight, 1] call leanCheck
leanL = [leanNeutral, leanLeft, -1] call leanCheck
triAssertNear [leanR select 1, 35, 3]
triAssertNear [leanL select 1, 35, 3]
triAssertGt [leanR select 2, 0.1]
triAssertGt [leanL select 2, 0.1]
triAssertGt [leanR select 3, 0.2]
triAssertGt [leanL select 3, 0.2]
triAssertGt [(leanR select 3) / (leanL select 3), 0.7]
triAssertGt [(leanL select 3) / (leanR select 3), 0.7]
triAssertGt [leanR select 4, -0.15]
triAssertGt [leanL select 4, -0.15]
triAssertLt [leanR select 5, 0.01]
triAssertLt [leanR select 6, 0.01]
triAssertLt [leanL select 5, 0.01]
triAssertLt [leanL select 6, 0.01]
triAssertNear [leanR select 7, 0, 0.015]
triAssertNear [leanR select 8, 0, 0.015]
triAssertNear [leanR select 9, 0, 0.015]
triAssertNear [leanR select 10, 0, 0.015]
triAssertNear [leanR select 11, 0, 0.015]
triAssertNear [leanR select 12, 0, 0.015]
triAssertNear [leanL select 7, 0, 0.015]
triAssertNear [leanL select 8, 0, 0.015]
triAssertNear [leanL select 9, 0, 0.015]
triAssertNear [leanL select 10, 0, 0.015]
triAssertNear [leanL select 11, 0, 0.015]
triAssertNear [leanL select 12, 0, 0.015]
triAssertGt [leanR select 13, 0.99985]
triAssertGt [leanL select 13, 0.99985]
triEndTest
