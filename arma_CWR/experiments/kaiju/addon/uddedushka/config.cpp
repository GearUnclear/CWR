// uddedushka: Dedushka, the giant, as a Man-class unit (standalone experiment,
// not wired into Guerrilla Mode). Packed by ../build-uddedushka.ps1 into
// uddedushka.pbo; the pbo file name is the path prefix, so its files resolve
// as \uddedushka\...
//
// The model keeps the stock man BONE NAMES (the engine asks for zbran, roura,
// lruka, pruka and hlava by name, and the RTM tools are built around the 25
// stock names) but none of the stock man's joints or rest pose: its rest pose
// is the standing pose, origin on the ground between the feet. So it has its
// own moves class, CfgMovesDedushka, with clips made for its own joints
// (../gen_dedushka_rtm.py). The engine names the skeleton after the moves
// class (SoldierOld.cpp:389) and fills it from the RTM headers.
//
// Config rules learnt on the chupacabra (cryptid notes, anim\moves\NOTES.md):
// no forward declarations (`class X;` aborts this parser), base chains are
// re-declared empty and merge onto the originals, never `class X : X`, edge
// costs at most 2.5, every movement action of an action class stays inside
// its own upDegree.

#define private   0
#define protected 1
#define public    2

// ---------------------------------------------------------------------------
// Clips and timing. A rename or a retime is a one-line edit here.
// Negative speed = seconds per cycle (SoldierOldSimProxy.cpp:1220-1224).
// Ground speed = step / seconds: ded_walk steps 17.6 m, so -6 is 2.9 m/s.
// Keep it under 10 m/s: a Man in ground contact takes crash damage above that
// (SoldierOldSim.cpp:840-850).
// ---------------------------------------------------------------------------
#define DED_RTM_STAND   "\uddedushka\anim\ded_stand.rtm"
#define DED_RTM_WALK    "\uddedushka\anim\ded_walk.rtm"
#define DED_RTM_DIE     "\uddedushka\anim\ded_die.rtm"
#define DED_RTM_DEAD    "\uddedushka\anim\ded_dead.rtm"

#define DED_SPD_STAND   -9.0
#define DED_SPD_WALK    -6.0
#define DED_SPD_DIE     -5.0
#define DED_SPD_STATIC  10000000000.0

// Turn rate cap, rad/s (SoldierOldSim.cpp:88-94): a slow, heavy turn.
#define DED_TURN        0.5
// AI gait thresholds, m/s (SoldierOldAI.cpp:2169-2196). One gait only.
#define DED_LIMITFAST   5

// ManPos values (SoldierOld.hpp:473-492).
#define DED_POS_DEAD    0
#define DED_POS_LOW     3
#define DED_POS_CROUCH  6
#define DED_POS_COMBAT  8
#define DED_POS_STAND   10
#define DED_POS_UP      11

#define DED_COST        1

class CfgPatches
{
    class uddedushka
    {
        units[] = {"UDDedushka"};
        weapons[] = {};
        requiredVersion = 1.30;
        // Civilian, Man, CfgModels >> Man and CfgMovesMC live in the base
        // config (no owner addon), so nothing has to be required.
        requiredAddons[] = {};
    };
};

class CfgModels
{
    class Default {};
    class Head : Default {};
    class Man : Default {};
    // Looked up by the model's file stem (dedushka.p3d).
    class dedushka : Man {};
};

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
        // Required name: every move reads Actions >> NoActions
        // (SoldierOldSimProxy.cpp:1184-1187). Also the dying state's map.
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

        // Upright family. An unarmed man lives in ManPos "no weapon" (11).
        class DedActions : NoActions
        {
            upDegree = DED_POS_UP;
            turnSpeed = DED_TURN;
            limitFast = DED_LIMITFAST;
            stop = "DedStand";
            default = "DedStand";
            // No turn clip: the engine turns the body in any move.
            turnL = "DedStand";
            turnR = "DedStand";
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
            // No backward clip: backing off means standing still.
            walkB = "DedStand";
            walkLB = "DedStand";
            walkRB = "DedStand";
            slowB = "DedStand";
            slowLB = "DedStand";
            slowRB = "DedStand";
            fastB = "DedStand";
            fastLB = "DedStand";
            fastRB = "DedStand";
            // Stance changes. There is no prone and no crouch: the low and
            // crouch families below are the same upright clips under the
            // upDegree the AI asks for, so it never loops on a stance action.
            civil = "DedStand";
            stand = "DedStand";
            combat = "DedStand";
            up = "DedStand";
            crouch = "DedStandCrouch";
            civilLying = "DedStandLow";
            lying = "DedStandLow";
            down = "DedStandLow";
            die = "DedDead";
            strokeFist = "";
            strokeGun = "";
        };
        // "Lying no weapon" (3): what the AI wants under fire or with
        // setUnitPos "DOWN" (SoldierOldAI.cpp:1991-2012). Still upright.
        class DedLowActions : DedActions
        {
            upDegree = DED_POS_LOW;
            stop = "DedStandLow";
            default = "DedStandLow";
            turnL = "DedStandLow";
            turnR = "DedStandLow";
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
            walkB = "DedStandLow";
            walkLB = "DedStandLow";
            walkRB = "DedStandLow";
            slowB = "DedStandLow";
            slowLB = "DedStandLow";
            slowRB = "DedStandLow";
            fastB = "DedStandLow";
            fastLB = "DedStandLow";
            fastRB = "DedStandLow";
        };
        // Crouch (6): asked of an unarmed man in COMBAT below 2 m/s.
        class DedCrouchActions : DedActions
        {
            upDegree = DED_POS_CROUCH;
            stop = "DedStandCrouch";
            default = "DedStandCrouch";
            turnL = "DedStandCrouch";
            turnR = "DedStandCrouch";
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
            walkB = "DedStandCrouch";
            walkLB = "DedStandCrouch";
            walkRB = "DedStandCrouch";
            slowB = "DedStandCrouch";
            slowLB = "DedStandCrouch";
            slowRB = "DedStandCrouch";
            fastB = "DedStandCrouch";
            fastLB = "DedStandCrouch";
            fastRB = "DedStandCrouch";
        };
        // Spawn aliases. A new man starts in the default move of "stand"
        // (SAFE, CARELESS) or "combat" (the rest) (SoldierOldCommon.hpp:137-148,
        // SoldierOld.cpp:345-370); without a map of those upDegrees it would
        // start with no move at all.
        class DedAliasStandActions : DedActions
        {
            upDegree = DED_POS_STAND;
        };
        class DedAliasCombatActions : DedActions
        {
            upDegree = DED_POS_COMBAT;
        };
    };

    // Shared state defaults, outside States so they are not moves themselves
    // (only the OWN entries of States become moves, SoldierOld.cpp:393-397).
    class DedBase : Default
    {
        actions = "DedActions";
        file = DED_RTM_STAND;
        looped = 1;
        preload = 1;
        speed = DED_SPD_STAND;
        disableWeapons = 0;
        disableWeaponsLong = 0;
        enableOptics = 0;
        showWeaponAim = 0;
        limitGunMovement = 0;
        aiming = "aimingNo";
        // legsNo: no slope skew of the legs. The stock skew is tuned for a
        // 0.4 m stance; on a 10 m stance it shears the legs visibly.
        legs = "legsNo";
        head = "headNo";
        duty = -1;
        soundEnabled = 0;
        relSpeedMin = 1;
        relSpeedMax = 1;
        // 0 = stay upright on slopes instead of tilting to the terrain normal
        // (SoldierOldSim.cpp:672-680).
        onLandBeg = 0;
        onLandEnd = 0;
        interpolationSpeed = 1;
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

    class States
    {
        class DedStand : DedBase
        {
        };
        class DedWalk : DedBase
        {
            file = DED_RTM_WALK;
            speed = DED_SPD_WALK;
            relSpeedMin = 0.5;
            relSpeedMax = 1;
        };
        class DedStandLow : DedStand
        {
            actions = "DedLowActions";
            interpolateWith[] = {"DedStand", DED_COST};
        };
        class DedWalkLow : DedWalk
        {
            actions = "DedLowActions";
            interpolateWith[] = {"DedWalk", DED_COST};
        };
        class DedStandCrouch : DedStand
        {
            actions = "DedCrouchActions";
            interpolateWith[] = {"DedStand", DED_COST};
        };
        class DedWalkCrouch : DedWalk
        {
            actions = "DedCrouchActions";
            interpolateWith[] = {"DedWalk", DED_COST};
        };

        class DedDying : DedBase
        {
            actions = "NoActions";
            file = DED_RTM_DIE;
            speed = DED_SPD_DIE;
            looped = 0;
            interpolationSpeed = 2;
            interpolationRestart = 1;
            interpolateFrom[] = {"DedStand", 0.1, "DedWalk", 0.1, "DedStandLow", 0.1, "DedWalkLow", 0.1, "DedStandCrouch", 0.1, "DedWalkCrouch", 0.1, "DedAliasStand", 0.1, "DedAliasCombat", 0.1};
            connectTo[] = {"DedDead", DED_COST};
        };
        class DedDead : DedBase
        {
            // upDegree dead is what Man::IsDead() tests (SoldierOld.cpp:1244-1247).
            actions = "DedDeadActions";
            file = DED_RTM_DEAD;
            speed = DED_SPD_STATIC;
            terminal = 1;
        };

        // Spawn aliases (never entered by the AI).
        class DedAliasStand : DedBase
        {
            actions = "DedAliasStandActions";
            interpolateTo[] = {"DedStand", DED_COST};
        };
        class DedAliasCombat : DedBase
        {
            actions = "DedAliasCombatActions";
            interpolateTo[] = {"DedStand", DED_COST};
        };
    };

    class Interpolations
    {
        dedUp[] = {DED_COST, "DedStand", "DedWalk"};
        dedLow[] = {DED_COST, "DedStandLow", "DedWalkLow"};
        dedCrouch[] = {DED_COST, "DedStandCrouch", "DedWalkCrouch"};
    };
    transitionsInterpolated[] = {};
    transitionsSimple[] = {};
    transitionsDisabled[] = {};
};

class CfgVehicles
{
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Civilian : Man {};

    // Values follow kaiju-notes\survey-engine-giant.md ("Recommended approach").
    class UDDedushka : Civilian
    {
        scope = public;
        side = 0;                       // EAST by default; a script may put it in any group
        displayName = "Dedushka";
        model = "\uddedushka\dedushka";
        moves = "CfgMovesDedushka";
        hiddenSelections[] = {};
        // Civilian carries Throw and Put; the creature carries nothing.
        weapons[] = {};
        magazines[] = {};

        // Toughness. Direct-hit damage already falls with the square of the
        // geometry radius (Dammage.cpp:149-175), about 1/600 of a man's at
        // this size. armor 1000 on top: rifles are not even considered
        // (hit/armor under 0.1, Target.cpp:1739-1747), tank guns and
        // launchers are. The zone factors keep head and body hits from
        // killing through the 0.9 zone rule (SoldierOldMove.cpp:1341-1348).
        armor = 1000;
        armorStructural = 4;
        armorHead = 2;
        armorBody = 2;
        armorHands = 1;
        armorLegs = 2;

        // AI value and perception of it.
        cost = 10000000;
        irTarget = 1;                   // AT missiles and gunships can lock it (VehicleAI.cpp:671-700)
        audible = 8;
        camouflage = 8;
        accuracy = 0.1;                 // recognised at once

        // km/h. Caps the AI's wanted speed (SoldierOldAI.cpp:1786-1789);
        // the walk clip is 10.6 km/h at full rate.
        maxSpeed = 11;
        precision = 10;
        brakeDistance = 20;
        formationX = 60;
        formationZ = 60;
        mapSize = 30;
        extCameraPosition[] = {0, 15, -90};
    };
};
