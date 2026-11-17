// INTEGRATION COPY of features\anim\moves_dedushka.hpp (do not edit the feature's file; edit this one).
// Changes against the anim feature's file, all marked [integration]:
//   - DedStride: the walk at full rate under playMove (the brain's request);
//   - footfall edges on the turns and the stomp, and none on any other move, so the
//     engine's kjSteps reports real footfalls only (walk, turns on the spot, the stomp).
// CfgMovesDedushka: Dedushka's move set (anim feature; standalone experiment,
// not wired into Guerrilla Mode). build-kj-anim.ps1 inlines this file, after
// the generated ded_anim_gen.hpp (the DED_SPD_* and DED_TURN_* numbers), into
// a copy of the pipeline's config.cpp in place of its CfgMovesDedushka. The
// packed pbo never relies on #include.
//
// Move names other features use (FEATURES-BRIEF contract):
//   DedIdle DedWalk DedTurnL DedTurnR DedRoar DedBeamRise DedBeamHold
//   DedBeamLower DedStomp DedSwat DedFlinch DedDormant
// How to drive them from a script is in the anim NOTES.md ("Driving the moves").
//
// Engine rules this file is built on (file:line in the cryptid moves notes and
// in the anim NOTES.md):
// - The skeleton is named after the moves class and filled from the RTM
//   headers; only the OWN entries of States become moves.
// - No forward declarations, never `class X : X`, edge costs at most 2.5.
// - Every movement action of an action class stays inside its own upDegree.
// - An unarmed man lives in upDegree 11; the AI asks him for 3 (under fire)
//   and 6 (COMBAT at low speed), so those families exist and are the same
//   upright clips. 10 and 8 are spawn aliases.
// - The body turns at the turnSpeed of the CURRENT move's action class, in
//   any move. The turn clips step round at exactly DED_TURN_SPOT, so their
//   planted foot stays put; walking and standing turn far slower, because
//   there the feet cannot follow.
// - A playMove holds until the move has played once (a loop: until it
//   wraps). After that the AI picks from the action class of the move it is
//   in. DedBeamHold and DedDormant have action classes whose every action is
//   the move itself, so he stays in them until a script plays him out.

#define DED_RTM_IDLE     "\uddedushka\anim\ded_idle.rtm"
#define DED_RTM_WALK     "\uddedushka\anim\ded_walk.rtm"
#define DED_RTM_TURNL    "\uddedushka\anim\ded_turnl.rtm"
#define DED_RTM_TURNR    "\uddedushka\anim\ded_turnr.rtm"
#define DED_RTM_ROAR     "\uddedushka\anim\ded_roar.rtm"
#define DED_RTM_RISE     "\uddedushka\anim\ded_beamrise.rtm"
#define DED_RTM_HOLD     "\uddedushka\anim\ded_beamhold.rtm"
#define DED_RTM_LOWER    "\uddedushka\anim\ded_beamlower.rtm"
#define DED_RTM_STOMP    "\uddedushka\anim\ded_stomp.rtm"
#define DED_RTM_SWAT     "\uddedushka\anim\ded_swat.rtm"
#define DED_RTM_FLINCH   "\uddedushka\anim\ded_flinch.rtm"
#define DED_RTM_DORMANT  "\uddedushka\anim\ded_dormant.rtm"
#define DED_RTM_DIE      "\uddedushka\anim\ded_die.rtm"
#define DED_RTM_DEAD     "\uddedushka\anim\ded_dead.rtm"

#define DED_SPD_STATIC   10000000000.0
// AI gait threshold, m/s. One gait only.
#define DED_LIMITFAST    5

// ManPos values (SoldierOld.hpp:473-492).
#define DED_POS_DEAD     0
#define DED_POS_LOW      3
#define DED_POS_CROUCH   6
#define DED_POS_COMBAT   8
#define DED_POS_STAND    10
#define DED_POS_UP       11

// Edge costs. Acts (roar, stomp, ...) cost more, so the path search never
// goes through one on its way somewhere else.
#define DED_COST         1
#define DED_COST_ACT     2

// Blend times are 1 / speed seconds (DED_BLEND_LOOP comes from the generator).
// Slowness is scale: nothing snaps.
#define DED_BLEND_ACT    1.0
#define DED_BLEND_FLINCH 3.0
#define DED_BLEND_SLOW   0.3
// [integration] blend into DedStride, 1 / seconds (the generator's DED_BLEND_LOOP is 1)
#define DED_BLEND_STRIDE 2.0

// The engine's own step sound and footprint fire at these phases of the walk
// when soundEnabled is 1 (SoldierOldMove.cpp:186-232). Off: the stock sound
// is a man's step. The roar feature can turn it into his footfall with
// soundOverride and a SoundEnvironExt entry (see ENGINE-WISHES.md).
#define DED_STEP_SOUND   0

class CfgManActions {};
class CfgMovesMC
{
    class Default {};
};

class CfgMovesDedushka : CfgMovesMC
{
    // vehicleActions and BlendAnims are inherited from CfgMovesMC.

    class Actions
    {
        // Required name: every move reads Actions >> NoActions. Also the dying state's map.
        class NoActions : CfgManActions
        {
            upDegree = -1;
            turnSpeed = 0;
            limitFast = 5;
        };
        class DedDeadActions : NoActions
        {
            upDegree = DED_POS_DEAD;
        };

        // ---- upright, the family an unarmed man lives in (11) -------------
        class DedActions : NoActions
        {
            upDegree = DED_POS_UP;
            turnSpeed = DED_TURN_IDLE;
            limitFast = DED_LIMITFAST;
            stop = "DedIdle";
            default = "DedIdle";
            turnL = "DedTurnL";
            turnR = "DedTurnR";
            walkF = "DedWalk";
            walkLF = "DedWalk";
            walkRF = "DedWalk";
            walkL = "DedWalk";
            walkR = "DedWalk";
            slowF = "DedWalk";
            slowLF = "DedWalk";
            slowRF = "DedWalk";
            slowL = "DedWalk";
            slowR = "DedWalk";
            fastF = "DedWalk";
            fastLF = "DedWalk";
            fastRF = "DedWalk";
            fastL = "DedWalk";
            fastR = "DedWalk";
            // He never backs off: backing means standing.
            walkB = "DedIdle";
            walkLB = "DedIdle";
            walkRB = "DedIdle";
            slowB = "DedIdle";
            slowLB = "DedIdle";
            slowRB = "DedIdle";
            fastB = "DedIdle";
            fastLB = "DedIdle";
            fastRB = "DedIdle";
            civil = "DedIdle";
            stand = "DedIdle";
            combat = "DedIdle";
            up = "DedIdle";
            crouch = "DedIdleCrouch";
            civilLying = "DedIdleLow";
            lying = "DedIdleLow";
            down = "DedIdleLow";
            die = "DedDead";
            strokeFist = "";
            strokeGun = "";
        };
        class DedWalkActions : DedActions
        {
            turnSpeed = DED_TURN_WALK;
        };
        class DedTurnActions : DedActions
        {
            turnSpeed = DED_TURN_SPOT;
        };

        // ---- "lying no weapon" (3): what the AI wants under fire. Upright. --
        class DedLowActions : DedActions
        {
            upDegree = DED_POS_LOW;
            stop = "DedIdleLow";
            default = "DedIdleLow";
            turnL = "DedTurnLLow";
            turnR = "DedTurnRLow";
            walkF = "DedWalkLow";
            walkLF = "DedWalkLow";
            walkRF = "DedWalkLow";
            walkL = "DedWalkLow";
            walkR = "DedWalkLow";
            slowF = "DedWalkLow";
            slowLF = "DedWalkLow";
            slowRF = "DedWalkLow";
            slowL = "DedWalkLow";
            slowR = "DedWalkLow";
            fastF = "DedWalkLow";
            fastLF = "DedWalkLow";
            fastRF = "DedWalkLow";
            fastL = "DedWalkLow";
            fastR = "DedWalkLow";
            walkB = "DedIdleLow";
            walkLB = "DedIdleLow";
            walkRB = "DedIdleLow";
            slowB = "DedIdleLow";
            slowLB = "DedIdleLow";
            slowRB = "DedIdleLow";
            fastB = "DedIdleLow";
            fastLB = "DedIdleLow";
            fastRB = "DedIdleLow";
        };
        class DedLowWalkActions : DedLowActions
        {
            turnSpeed = DED_TURN_WALK;
        };
        class DedLowTurnActions : DedLowActions
        {
            turnSpeed = DED_TURN_SPOT;
        };

        // ---- crouch (6): asked of an unarmed man in COMBAT below 2 m/s ----
        class DedCrouchActions : DedActions
        {
            upDegree = DED_POS_CROUCH;
            stop = "DedIdleCrouch";
            default = "DedIdleCrouch";
            turnL = "DedTurnLCrouch";
            turnR = "DedTurnRCrouch";
            walkF = "DedWalkCrouch";
            walkLF = "DedWalkCrouch";
            walkRF = "DedWalkCrouch";
            walkL = "DedWalkCrouch";
            walkR = "DedWalkCrouch";
            slowF = "DedWalkCrouch";
            slowLF = "DedWalkCrouch";
            slowRF = "DedWalkCrouch";
            slowL = "DedWalkCrouch";
            slowR = "DedWalkCrouch";
            fastF = "DedWalkCrouch";
            fastLF = "DedWalkCrouch";
            fastRF = "DedWalkCrouch";
            fastL = "DedWalkCrouch";
            fastR = "DedWalkCrouch";
            walkB = "DedIdleCrouch";
            walkLB = "DedIdleCrouch";
            walkRB = "DedIdleCrouch";
            slowB = "DedIdleCrouch";
            slowLB = "DedIdleCrouch";
            slowRB = "DedIdleCrouch";
            fastB = "DedIdleCrouch";
            fastLB = "DedIdleCrouch";
            fastRB = "DedIdleCrouch";
        };
        class DedCrouchWalkActions : DedCrouchActions
        {
            turnSpeed = DED_TURN_WALK;
        };
        class DedCrouchTurnActions : DedCrouchActions
        {
            turnSpeed = DED_TURN_SPOT;
        };

        // ---- held states: every action is the state itself ---------------
        // Upright upDegree, and every stance action answers with the state
        // too, so the AI can neither walk him out nor change his stance.
        class DedBeamActions : DedActions
        {
            turnSpeed = 0;
            stop = "DedBeamHold";
            default = "DedBeamHold";
            turnL = "DedBeamHold";
            turnR = "DedBeamHold";
            walkF = "DedBeamHold";
            walkLF = "DedBeamHold";
            walkRF = "DedBeamHold";
            walkL = "DedBeamHold";
            walkR = "DedBeamHold";
            slowF = "DedBeamHold";
            slowLF = "DedBeamHold";
            slowRF = "DedBeamHold";
            slowL = "DedBeamHold";
            slowR = "DedBeamHold";
            fastF = "DedBeamHold";
            fastLF = "DedBeamHold";
            fastRF = "DedBeamHold";
            fastL = "DedBeamHold";
            fastR = "DedBeamHold";
            walkB = "DedBeamHold";
            walkLB = "DedBeamHold";
            walkRB = "DedBeamHold";
            slowB = "DedBeamHold";
            slowLB = "DedBeamHold";
            slowRB = "DedBeamHold";
            fastB = "DedBeamHold";
            fastLB = "DedBeamHold";
            fastRB = "DedBeamHold";
            civil = "DedBeamHold";
            stand = "DedBeamHold";
            combat = "DedBeamHold";
            up = "DedBeamHold";
            crouch = "DedBeamHold";
            civilLying = "DedBeamHold";
            lying = "DedBeamHold";
            down = "DedBeamHold";
        };
        class DedDormantActions : DedActions
        {
            turnSpeed = 0;
            stop = "DedDormant";
            default = "DedDormant";
            turnL = "DedDormant";
            turnR = "DedDormant";
            walkF = "DedDormant";
            walkLF = "DedDormant";
            walkRF = "DedDormant";
            walkL = "DedDormant";
            walkR = "DedDormant";
            slowF = "DedDormant";
            slowLF = "DedDormant";
            slowRF = "DedDormant";
            slowL = "DedDormant";
            slowR = "DedDormant";
            fastF = "DedDormant";
            fastLF = "DedDormant";
            fastRF = "DedDormant";
            fastL = "DedDormant";
            fastR = "DedDormant";
            walkB = "DedDormant";
            walkLB = "DedDormant";
            walkRB = "DedDormant";
            slowB = "DedDormant";
            slowLB = "DedDormant";
            slowRB = "DedDormant";
            fastB = "DedDormant";
            fastLB = "DedDormant";
            fastRB = "DedDormant";
            civil = "DedDormant";
            stand = "DedDormant";
            combat = "DedDormant";
            up = "DedDormant";
            crouch = "DedDormant";
            civilLying = "DedDormant";
            lying = "DedDormant";
            down = "DedDormant";
        };

        // Spawn aliases. A new man starts in the default move of "stand" or
        // "combat"; without a map of those upDegrees he would have no move.
        class DedAliasStandActions : DedActions
        {
            upDegree = DED_POS_STAND;
        };
        class DedAliasCombatActions : DedActions
        {
            upDegree = DED_POS_COMBAT;
        };
    };

    // Shared state defaults, outside States so they are not moves themselves.
    class DedBase : Default
    {
        actions = "DedActions";
        file = DED_RTM_IDLE;
        looped = 1;
        preload = 1;
        speed = DED_SPD_IDLE;
        disableWeapons = 0;
        disableWeaponsLong = 0;
        enableOptics = 0;
        showWeaponAim = 0;
        limitGunMovement = 0;
        aiming = "aimingNo";
        // legsNo: no slope skew of the legs (the stock skew is for a 0.4 m stance).
        legs = "legsNo";
        head = "headNo";
        duty = -1;
        soundEnabled = 0;
        // [integration] no footfall edge unless a move sets one (phase 2 is never reached)
        soundEdge1 = 2;
        soundEdge2 = 2;
        relSpeedMin = 1;
        relSpeedMax = 1;
        // 0 = stay upright on slopes.
        onLandBeg = 0;
        onLandEnd = 0;
        interpolationSpeed = DED_BLEND_LOOP;
        interpolationRestart = 0;
        visibleSize = 1;
        variantsAI[] = {""};
        variantsPlayer[] = {};
        equivalentTo = "";
        connectFrom[] = {};
        connectTo[] = {};
        interpolateWith[] = {};
        interpolateTo[] = {};
        interpolateFrom[] = {};
    };
    // An act: played once from any of the loops, blends in from its first
    // frame and out of its last one into the idle. The AI is not asked while
    // it plays.
    class DedAct : DedBase
    {
        looped = 0;
        interpolationSpeed = DED_BLEND_ACT;
        interpolationRestart = 1;
        interpolateFrom[] = {"DedIdle", DED_COST_ACT, "DedWalk", DED_COST_ACT, "DedStride", DED_COST_ACT, "DedTurnL", DED_COST_ACT, "DedTurnR", DED_COST_ACT, "DedIdleLow", DED_COST_ACT, "DedWalkLow", DED_COST_ACT, "DedTurnLLow", DED_COST_ACT, "DedTurnRLow", DED_COST_ACT, "DedIdleCrouch", DED_COST_ACT, "DedWalkCrouch", DED_COST_ACT, "DedTurnLCrouch", DED_COST_ACT, "DedTurnRCrouch", DED_COST_ACT};
        interpolateTo[] = {"DedIdle", DED_COST};
    };

    class States
    {
        // ---- the loops ----------------------------------------------------
        class DedIdle : DedBase
        {
        };
        class DedWalk : DedBase
        {
            actions = "DedWalkActions";
            file = DED_RTM_WALK;
            speed = DED_SPD_WALK;
            relSpeedMin = DED_WALK_SLOWEST;
            relSpeedMax = 1;
            // Always from phase 0: left foot planted under him, right foot
            // lifting. A start from the stand lifts that foot and goes.
            interpolationSpeed = DED_BLEND_LOOP;
            interpolationRestart = 1;
            soundEnabled = DED_STEP_SOUND;
            soundEdge1 = DED_WALK_EDGE1;
            soundEdge2 = DED_WALK_EDGE2;
        };
        // [integration] the walk at full rate under playMove: a script move plays at relSpeedMin
        // (SoldierOldSim.cpp, Man::SimulateAnimations), and DedWalk keeps 0.6 there for the AI.
        class DedStride : DedWalk
        {
            relSpeedMin = 1;
            // [integration] a shorter blend into the stride. While a walk blends in, the engine drags
            // the standing foot back by half the blend time at walking speed (measured: 1.33 m at one
            // second). No script can cure that (integration NOTES, loose end 2); a shorter blend
            // shrinks it in proportion.
            interpolationSpeed = DED_BLEND_STRIDE;
        };
        class DedTurnL : DedBase
        {
            actions = "DedTurnActions";
            file = DED_RTM_TURNL;
            speed = DED_SPD_TURN;
            interpolationSpeed = DED_BLEND_LOOP;
            interpolationRestart = 1;
            // [integration] footfalls of the turn (edge 1 is the left foot)
            soundEdge1 = DED_TURNL_EDGE1;
            soundEdge2 = DED_TURNL_EDGE2;
        };
        class DedTurnR : DedTurnL
        {
            file = DED_RTM_TURNR;
            soundEdge1 = DED_TURNR_EDGE1;
            soundEdge2 = DED_TURNR_EDGE2;
        };

        // The same clips under the upDegrees the AI asks for.
        class DedIdleLow : DedIdle
        {
            actions = "DedLowActions";
            interpolateWith[] = {"DedIdle", DED_COST};
        };
        class DedWalkLow : DedWalk
        {
            actions = "DedLowWalkActions";
            interpolateWith[] = {"DedWalk", DED_COST};
        };
        class DedTurnLLow : DedTurnL
        {
            actions = "DedLowTurnActions";
            interpolateWith[] = {"DedTurnL", DED_COST};
        };
        class DedTurnRLow : DedTurnR
        {
            actions = "DedLowTurnActions";
            interpolateWith[] = {"DedTurnR", DED_COST};
        };
        class DedIdleCrouch : DedIdle
        {
            actions = "DedCrouchActions";
            interpolateWith[] = {"DedIdle", DED_COST};
        };
        class DedWalkCrouch : DedWalk
        {
            actions = "DedCrouchWalkActions";
            interpolateWith[] = {"DedWalk", DED_COST};
        };
        class DedTurnLCrouch : DedTurnL
        {
            actions = "DedCrouchTurnActions";
            interpolateWith[] = {"DedTurnL", DED_COST};
        };
        class DedTurnRCrouch : DedTurnR
        {
            actions = "DedCrouchTurnActions";
            interpolateWith[] = {"DedTurnR", DED_COST};
        };

        // ---- acts ----------------------------------------------------------
        class DedRoar : DedAct
        {
            file = DED_RTM_ROAR;
            speed = DED_SPD_ROAR;
        };
        class DedStomp : DedAct
        {
            file = DED_RTM_STOMP;
            speed = DED_SPD_STOMP;
            // [integration] the impact of the stomping foot is a footfall
            soundEdge1 = DED_STOMP_EDGE1;
            soundEdge2 = DED_STOMP_EDGE2;
        };
        class DedSwat : DedAct
        {
            file = DED_RTM_SWAT;
            speed = DED_SPD_SWAT;
        };
        class DedFlinch : DedAct
        {
            file = DED_RTM_FLINCH;
            speed = DED_SPD_FLINCH;
            // The one quick thing he does, and it is still a third of a second.
            interpolationSpeed = DED_BLEND_FLINCH;
        };

        // ---- the beam: rise, hold until played out, lower -----------------
        class DedBeamRise : DedAct
        {
            actions = "DedBeamActions";
            file = DED_RTM_RISE;
            speed = DED_SPD_RISE;
            interpolateTo[] = {"DedBeamHold", DED_COST};
        };
        class DedBeamHold : DedBase
        {
            actions = "DedBeamActions";
            file = DED_RTM_HOLD;
            speed = DED_SPD_HOLD;
            interpolationSpeed = 2.0;
            interpolateTo[] = {"DedBeamLower", DED_COST};
        };
        class DedBeamLower : DedBase
        {
            file = DED_RTM_LOWER;
            speed = DED_SPD_LOWER;
            looped = 0;
            interpolationSpeed = 2.0;
            interpolationRestart = 1;
            interpolateTo[] = {"DedIdle", DED_COST};
        };

        // ---- dormant: held until a script plays DedIdle -------------------
        class DedDormant : DedBase
        {
            actions = "DedDormantActions";
            file = DED_RTM_DORMANT;
            speed = DED_SPD_DORMANT;
            // Sinking into it and coming out of it take over three seconds.
            interpolationSpeed = DED_BLEND_SLOW;
            interpolateFrom[] = {"DedIdle", DED_COST_ACT, "DedWalk", DED_COST_ACT, "DedStride", DED_COST_ACT, "DedTurnL", DED_COST_ACT, "DedTurnR", DED_COST_ACT, "DedIdleLow", DED_COST_ACT, "DedWalkLow", DED_COST_ACT, "DedTurnLLow", DED_COST_ACT, "DedTurnRLow", DED_COST_ACT, "DedIdleCrouch", DED_COST_ACT, "DedWalkCrouch", DED_COST_ACT, "DedTurnLCrouch", DED_COST_ACT, "DedTurnRCrouch", DED_COST_ACT};
            interpolateTo[] = {"DedIdle", DED_COST_ACT};
        };

        // ---- death ---------------------------------------------------------
        class DedDying : DedBase
        {
            actions = "NoActions";
            file = DED_RTM_DIE;
            speed = DED_SPD_DIE;
            looped = 0;
            interpolationSpeed = 2;
            interpolationRestart = 1;
            interpolateFrom[] = {"DedIdle", 0.1, "DedWalk", 0.1, "DedStride", 0.1, "DedTurnL", 0.1, "DedTurnR", 0.1, "DedIdleLow", 0.1, "DedWalkLow", 0.1, "DedTurnLLow", 0.1, "DedTurnRLow", 0.1, "DedIdleCrouch", 0.1, "DedWalkCrouch", 0.1, "DedTurnLCrouch", 0.1, "DedTurnRCrouch", 0.1, "DedRoar", 0.1, "DedStomp", 0.1, "DedSwat", 0.1, "DedFlinch", 0.1, "DedBeamRise", 0.1, "DedBeamHold", 0.1, "DedBeamLower", 0.1, "DedDormant", 0.1, "DedAliasStand", 0.1, "DedAliasCombat", 0.1};
            connectTo[] = {"DedDead", DED_COST};
        };
        class DedDead : DedBase
        {
            // upDegree dead is what Man::IsDead() tests.
            actions = "DedDeadActions";
            file = DED_RTM_DEAD;
            speed = DED_SPD_STATIC;
            terminal = 1;
        };

        // Spawn aliases (never entered by the AI).
        class DedAliasStand : DedBase
        {
            actions = "DedAliasStandActions";
            interpolateTo[] = {"DedIdle", DED_COST};
        };
        class DedAliasCombat : DedBase
        {
            actions = "DedAliasCombatActions";
            interpolateTo[] = {"DedIdle", DED_COST};
        };
    };

    class Interpolations
    {
        dedUp[] = {DED_COST, "DedIdle", "DedWalk", "DedTurnL", "DedTurnR", "DedStride"};
        dedLow[] = {DED_COST, "DedIdleLow", "DedWalkLow", "DedTurnLLow", "DedTurnRLow"};
        dedCrouch[] = {DED_COST, "DedIdleCrouch", "DedWalkCrouch", "DedTurnLCrouch", "DedTurnRCrouch"};
    };
    transitionsInterpolated[] = {};
    transitionsSimple[] = {};
    transitionsDisabled[] = {};
};
