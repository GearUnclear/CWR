// Dedushka animation timing, written by gen_ded_anim.py. Do not edit: rerun the generator.
// Positions are [right, forward, up] in metres from the unit's position, unit facing forward.
// KJ_animClips: [move, seconds, looped, metres per cycle, [[event, seconds, [right, forward, up]], ...]]
KJ_animClips = [
  ["DedIdle", 12, true, 0, [["breath_in", 1.5, [0.64, 2.6, 25.88]], ["breath_in", 7.5, [1.42, 2.62, 25.88]]]],
  ["DedWalk", 7, true, 15.3851, [["foot_r_down", 1.26, [6.5, 5.22, 0]], ["foot_l_up", 2.24, [-7.2, -6.1, 0.31]], ["foot_l_down", 4.76, [-7.2, 3.92, 0]], ["foot_r_up", 5.74, [6.5, -4.62, 0]]]],
  ["DedTurnL", 6, true, 0, [["foot_r_down", 1.08, [5.91, 2.71, 0]], ["foot_l_down", 4.08, [-6.3, -3.62, 0]]]],
  ["DedTurnR", 6, true, 0, [["foot_l_down", 1.08, [-7.05, 1.77, 0]], ["foot_r_down", 4.08, [6.14, -2.16, 0]]]],
  ["DedRoar", 9, false, 0, [["head_down", 2.4, [14.01, 8.75, 22.92]], ["roar_start", 3.4, [14.01, 11.08, 28.56]], ["roar_full", 4.5, [13.51, 10.36, 36.38]], ["roar_end", 7.1, [13.7, 11.22, 33.91]]]],
  ["DedBeamRise", 6.5, false, 0, [["ribs_start", 4.3, [0.96, 1.18, 27.52]], ["ribs_open", 6.5, [0.97, 1.17, 27.53]]]],
  ["DedBeamHold", 2.4, true, 0, [["furnace", 0, [0.97, 1.17, 27.53]]]],
  ["DedBeamLower", 7.5, false, 0, [["ribs_closed", 2.2, [0.97, 1.7, 26.99]], ["lowest", 4.5, [11.52, 8.58, 32.96]]]],
  ["DedStomp", 8, false, 0, [["foot_l_up", 2, [-7.2, -1, 0]], ["foot_top", 4.5, [-7.2, 0.43, 6.8]], ["impact", 5.5, [-7.2, 5.48, 0]], ["foot_l_home", 7.8, [-7.2, -1, 0]]]],
  ["DedSwat", 9, false, 0, [["arm_raised", 2.7, [-20.5, -4.44, 41.34]], ["sweep_00", 2.9, [-20.5, -4.44, 41.34]], ["sweep_01", 3.275, [-21.03, -2.86, 41.56]], ["sweep_02", 3.65, [-21.75, 1.99, 41.99]], ["sweep_03", 4.025, [-20.65, 9.75, 42.1]], ["sweep_04", 4.4, [-15.92, 18.19, 41.47]], ["sweep_05", 4.775, [-8.06, 24.15, 40.29]], ["sweep_06", 5.15, [0.1, 26.34, 39.16]], ["sweep_07", 5.525, [5.86, 26.03, 38.48]], ["sweep_08", 5.9, [8.02, 25.54, 38.23]], ["arm_down", 8.8, [-3.5, 8.99, 11.31]]]],
  ["DedFlinch", 2.6, false, 0, [["deepest", 0.4, [11.47, 6.15, 35.44]]]],
  ["DedDormant", 16, true, 0, []],
  ["DedDying", 5, false, 0, []]
];
KJ_animWalkStep = 15.3851;
KJ_animWalkSecs = 7;
KJ_animWalkSpeed = 2.1979;
KJ_animWalkSlowest = 0.6;
KJ_animWalkBlend = 1;
// Footfalls of the walk: [phase 0..1, "L" or "R", [right, forward, up] where the sole lands].
KJ_animWalkFalls = [[0.18, "R", [6.5, 5.22, 0]], [0.68, "L", [-7.2, 3.92, 0]]];
KJ_animTurnSecs = 6;
KJ_animTurnRate = 0.2;
KJ_animTurnFallsL = [[0.18, "R", [5.91, 2.71, 0]], [0.68, "L", [-6.3, -3.62, 0]]];
KJ_animTurnFallsR = [[0.18, "L", [-7.05, 1.77, 0]], [0.68, "R", [6.14, -2.16, 0]]];
// The stomp: seconds from the start of DedStomp to the impact, and where the sole comes down.
KJ_animStompTime = 5.5;
KJ_animStompPos = [-7.2, 5.48, 0];
KJ_animStompFoot = "L";
KJ_animStompSecs = 8;
KJ_animSwatSecs = 9;
KJ_animRoarSecs = 9;
KJ_animFlinchSecs = 2.6;
// The swat: [seconds, hand position] samples across the sweep.
KJ_animSwatPath = [[2.9, [-20.5, -4.44, 41.34]], [3.275, [-21.03, -2.86, 41.56]], [3.65, [-21.75, 1.99, 41.99]], [4.025, [-20.65, 9.75, 42.1]], [4.4, [-15.92, 18.19, 41.47]], [4.775, [-8.06, 24.15, 40.29]], [5.15, [0.1, 26.34, 39.16]], [5.525, [5.86, 26.03, 38.48]], [5.9, [8.02, 25.54, 38.23]]];
// The roar: seconds from the start of DedRoar to the mouth opening and to the end of the cry.
KJ_animRoarStart = 3.4;
KJ_animRoarEnd = 7.1;
KJ_animRoarMouth = [13.51, 10.36, 36.38];
// The beam: seconds DedBeamRise takes, and the furnace point while he holds.
KJ_animBeamRiseSecs = 6.5;
KJ_animBeamHoldSecs = 2.4;
KJ_animBeamLowerSecs = 7.5;
KJ_animBeamFurnace = [0.97, 1.17, 27.53];
KJ_animFurnace = [0.97, 2.73, 25.71];
KJ_animHeightTall = 37.63;
KJ_animHeight = 35.36;
KJ_animLoaded = true;
