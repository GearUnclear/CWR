// Dedushka animation timing, written by gen_ded_anim.py. Do not edit: rerun the generator.
// Positions are [right, forward, up] in metres from the unit's position, unit facing forward.
// KJ_animClips: [move, seconds, looped, metres per cycle, [[event, seconds, [right, forward, up]], ...]]
KJ_animClips = [
  ["DedIdle", 12, true, 0, [["breath_in", 1.5, [-0.24, 0.09, 31.44]], ["breath_in", 7.5, [0.27, 0.09, 31.44]]]],
  ["DedWalk", 7, true, 16.15, [["foot_r_down", 1.26, [5, 6.42, 0]], ["foot_l_up", 2.24, [-5, -4.08, 0.35]], ["foot_l_down", 4.76, [-5, 6.42, 0]], ["foot_r_up", 5.74, [5, -4.08, 0.35]]]],
  ["DedTurnL", 6, true, 0, [["foot_r_down", 1.08, [4.17, 3.03, 0]], ["foot_l_down", 4.08, [-5.1, -0.71, 0]]]],
  ["DedTurnR", 6, true, 0, [["foot_l_down", 1.08, [-4.17, 3.03, 0]], ["foot_r_down", 4.08, [5.1, -0.71, 0]]]],
  ["DedRoar", 9, false, 0, [["head_down", 2.4, [0, 4.91, 32.11]], ["roar_start", 3.4, [0, 3.9, 35.23]], ["roar_full", 4.5, [-0.21, 1.28, 37.38]], ["roar_end", 7.1, [-0.09, 2.21, 36.95]]]],
  ["DedBeamRise", 6.5, false, 0, [["ribs_start", 4.3, [-0.01, -0.45, 32.17]], ["ribs_open", 6.5, [0, -0.45, 32.17]]]],
  ["DedBeamHold", 2.4, true, 0, [["furnace", 0, [0, -0.45, 32.17]]]],
  ["DedBeamLower", 7.5, false, 0, [["ribs_closed", 2.2, [0, -0.25, 31.97]], ["lowest", 4.5, [0, 5.85, 36.17]]]],
  ["DedStomp", 8, false, 0, [["foot_r_up", 2, [5, 1.25, 0]], ["foot_top", 4.5, [5, 2.75, 7.14]], ["impact", 5.5, [5, 8.05, 0]], ["foot_r_home", 7.8, [5, 1.25, 0]]]],
  ["DedSwat", 9, false, 0, [["arm_raised", 2.7, [23.84, -9.34, 39.45]], ["sweep_00", 2.9, [23.84, -9.34, 39.45]], ["sweep_01", 3.275, [24.58, -8.16, 39.17]], ["sweep_02", 3.65, [26.1, -4.34, 38.24]], ["sweep_03", 4.025, [26.74, 2.09, 36.42]], ["sweep_04", 4.4, [24.73, 9.59, 33.75]], ["sweep_05", 4.775, [19.89, 15.6, 30.87]], ["sweep_06", 5.15, [14.16, 18.7, 28.6]], ["sweep_07", 5.525, [9.77, 19.43, 27.34]], ["sweep_08", 5.9, [8.07, 19.42, 26.91]], ["arm_down", 8.8, [11.5, 1.18, 13.46]]]],
  ["DedFlinch", 2.6, false, 0, [["deepest", 0.4, [0.12, 3.56, 38.18]]]],
  ["DedDormant", 16, true, 0, []],
  ["DedDying", 5, false, 0, []]
];
KJ_animWalkStep = 16.15;
KJ_animWalkSecs = 7;
KJ_animWalkSpeed = 2.3071;
KJ_animWalkSlowest = 0.6;
KJ_animWalkBlend = 1;
// Footfalls of the walk: [phase 0..1, "L" or "R", [right, forward, up] where the sole lands].
KJ_animWalkFalls = [[0.18, "R", [5, 6.42, 0]], [0.68, "L", [-5, 6.42, 0]]];
KJ_animTurnSecs = 6;
KJ_animTurnRate = 0.2;
KJ_animTurnFallsL = [[0.18, "R", [4.17, 3.03, 0]], [0.68, "L", [-5.1, -0.71, 0]]];
KJ_animTurnFallsR = [[0.18, "L", [-4.17, 3.03, 0]], [0.68, "R", [5.1, -0.71, 0]]];
// The stomp: seconds from the start of DedStomp to the impact, and where the sole comes down.
KJ_animStompTime = 5.5;
KJ_animStompPos = [5, 8.05, 0];
KJ_animStompFoot = "R";
KJ_animStompSecs = 8;
KJ_animSwatSecs = 9;
KJ_animRoarSecs = 9;
KJ_animFlinchSecs = 2.6;
// The swat: [seconds, hand position] samples across the sweep.
KJ_animSwatPath = [[2.9, [23.84, -9.34, 39.45]], [3.275, [24.58, -8.16, 39.17]], [3.65, [26.1, -4.34, 38.24]], [4.025, [26.74, 2.09, 36.42]], [4.4, [24.73, 9.59, 33.75]], [4.775, [19.89, 15.6, 30.87]], [5.15, [14.16, 18.7, 28.6]], [5.525, [9.77, 19.43, 27.34]], [5.9, [8.07, 19.42, 26.91]]];
// The roar: seconds from the start of DedRoar to the mouth opening and to the end of the cry.
KJ_animRoarStart = 3.4;
KJ_animRoarEnd = 7.1;
KJ_animRoarMouth = [-0.21, 1.28, 37.38];
// The beam: seconds DedBeamRise takes, and the furnace point while he holds.
KJ_animBeamRiseSecs = 6.5;
KJ_animBeamHoldSecs = 2.4;
KJ_animBeamLowerSecs = 7.5;
KJ_animBeamFurnace = [0, -0.45, 32.17];
KJ_animFurnace = [0, 0.19, 31.45];
KJ_animHeightTall = 39.87;
KJ_animHeight = 38.34;
KJ_animLoaded = true;
