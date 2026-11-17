// Dedushka animation timing, written by gen_ded_anim.py. Do not edit: rerun the generator.
// Positions are [right, forward, up] in metres from the unit's position, unit facing forward.
// KJ_animClips: [move, seconds, looped, metres per cycle, [[event, seconds, [right, forward, up]], ...]]
KJ_animClips = [
  ["DedIdle", 12, true, 0, [["breath_in", 1.5, [-0.35, 5.25, 27.56]], ["breath_in", 7.5, [0.47, 5.25, 27.56]]]],
  ["DedWalk", 7, true, 16.4108, [["foot_r_down", 1.26, [6.15, 7.6, 0]], ["foot_l_up", 2.24, [-6.65, -5.07, 0.79]], ["foot_l_down", 4.76, [-6.65, 5.55, 0]], ["foot_r_up", 5.74, [6.15, -3.02, 0.79]]]],
  ["DedTurnL", 6, true, 0, [["foot_r_down", 1.08, [4.82, 4.48, 0]], ["foot_l_down", 4.08, [-6.28, -2.21, 0]]]],
  ["DedTurnR", 6, true, 0, [["foot_l_down", 1.08, [-6.05, 2.77, 0]], ["foot_r_down", 4.08, [6.58, -0.13, 0]]]],
  ["DedRoar", 9, false, 0, [["head_down", 2.4, [-1.2, 11.75, 25.62]], ["roar_start", 3.4, [-1.2, 14.28, 32.33]], ["roar_full", 4.5, [-1.79, 13.66, 40.54]], ["roar_end", 7.1, [-1.59, 14.23, 38.1]]]],
  ["DedBeamRise", 6.5, false, 0, [["ribs_start", 4.3, [0, 4.41, 29.15]], ["ribs_open", 6.5, [0, 4.41, 29.15]]]],
  ["DedBeamHold", 2.4, true, 0, [["furnace", 0, [0, 4.41, 29.15]]]],
  ["DedBeamLower", 7.5, false, 0, [["ribs_closed", 2.2, [0, 4.74, 28.52]], ["lowest", 4.5, [-2.43, 14.35, 35.13]]]],
  ["DedStomp", 8, false, 0, [["foot_r_up", 2, [6.15, 2.35, 0]], ["foot_top", 4.5, [6.15, 3.87, 7.26]], ["impact", 5.5, [6.15, 9.26, 0]], ["foot_r_home", 7.8, [6.15, 2.35, 0]]]],
  ["DedSwat", 9, false, 0, [["arm_raised", 2.7, [28.09, -6.65, 45.65]], ["sweep_00", 2.9, [28.1, -6.65, 45.65]], ["sweep_01", 3.275, [28.61, -4.85, 45.89]], ["sweep_02", 3.65, [29.18, 0.7, 46.35]], ["sweep_03", 4.025, [27.59, 9.4, 46.41]], ["sweep_04", 4.4, [22.05, 18.6, 45.69]], ["sweep_05", 4.775, [13.4, 24.81, 44.45]], ["sweep_06", 5.15, [4.78, 26.89, 43.35]], ["sweep_07", 5.525, [-1.11, 26.4, 42.73]], ["sweep_08", 5.9, [-3.27, 25.84, 42.51]], ["arm_down", 8.8, [13.52, 9.23, 9.81]]]],
  ["DedFlinch", 2.6, false, 0, [["deepest", 0.4, [-2.64, 11.54, 40.1]]]],
  ["DedDormant", 16, true, 0, []],
  ["DedDying", 5, false, 0, []]
];
KJ_animWalkStep = 16.4108;
KJ_animWalkSecs = 7;
KJ_animWalkSpeed = 2.3444;
KJ_animWalkSlowest = 0.6;
KJ_animWalkBlend = 1;
// Footfalls of the walk: [phase 0..1, "L" or "R", [right, forward, up] where the sole lands].
KJ_animWalkFalls = [[0.18, "R", [6.15, 7.6, 0]], [0.68, "L", [-6.65, 5.55, 0]]];
KJ_animTurnSecs = 6;
KJ_animTurnRate = 0.2;
KJ_animTurnFallsL = [[0.18, "R", [4.82, 4.48, 0]], [0.68, "L", [-6.28, -2.21, 0]]];
KJ_animTurnFallsR = [[0.18, "L", [-6.05, 2.77, 0]], [0.68, "R", [6.58, -0.13, 0]]];
// The stomp: seconds from the start of DedStomp to the impact, and where the sole comes down.
KJ_animStompTime = 5.5;
KJ_animStompPos = [6.15, 9.26, 0];
KJ_animStompFoot = "R";
KJ_animStompSecs = 8;
KJ_animSwatSecs = 9;
KJ_animRoarSecs = 9;
KJ_animFlinchSecs = 2.6;
// The swat: [seconds, hand position] samples across the sweep.
KJ_animSwatPath = [[2.9, [28.1, -6.65, 45.65]], [3.275, [28.61, -4.85, 45.89]], [3.65, [29.18, 0.7, 46.35]], [4.025, [27.59, 9.4, 46.41]], [4.4, [22.05, 18.6, 45.69]], [4.775, [13.4, 24.81, 44.45]], [5.15, [4.78, 26.89, 43.35]], [5.525, [-1.11, 26.4, 42.73]], [5.9, [-3.27, 25.84, 42.51]]];
// The roar: seconds from the start of DedRoar to the mouth opening and to the end of the cry.
KJ_animRoarStart = 3.4;
KJ_animRoarEnd = 7.1;
KJ_animRoarMouth = [-1.79, 13.66, 40.54];
// The beam: seconds DedBeamRise takes, and the furnace point while he holds.
KJ_animBeamRiseSecs = 6.5;
KJ_animBeamHoldSecs = 2.4;
KJ_animBeamLowerSecs = 7.5;
KJ_animBeamFurnace = [0, 4.41, 29.15];
KJ_animFurnace = [0, 5.34, 27.29];
KJ_animHeightTall = 43.82;
KJ_animHeight = 39.88;
KJ_animLoaded = true;
