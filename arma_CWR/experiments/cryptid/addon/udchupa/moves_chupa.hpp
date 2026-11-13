// CfgMovesChupa: the chupacabra's own move set (experiment, not wired into
// Guerrilla Mode). Included by config.cpp only when UDCHUPA_CUSTOM_MOVES is
// defined; build-udchupa.ps1 -CustomMoves inlines it into the staged config,
// so the packed pbo never relies on #include.
//
// Contract for the clips: anim\moves\SPEC.md in the cryptid notes folder.
// Engine facts and citations: anim\moves\NOTES.md beside it.
//
// Why it looks like this (short):
// - The engine names the skeleton after the moves class (SoldierOld.cpp:389),
//   so this class gets its own skeleton "CfgMovesChupa". Bones come from the
//   RTM headers by name (RtAnimation.cpp:531-535), so the stock bone names
//   keep the stock-skeleton mesh and the new clips lined up.
// - States is a fresh class: only its OWN entries become moves
//   (SoldierOld.cpp:393-397, ParamFile.hpp:281), so inheriting the stock
//   States would not add anything. Never write "class X : X" in this file:
//   the addon merge looks the base up after inserting the new class and finds
//   the class itself (ParamFile.cpp:1984-2017).
// - Unarmed men live in stance "no weapon" (ManPos 11) and "lying no weapon"
//   (ManPos 3) (SoldierOldAI.cpp:1991-2012). Here 11 is the hunched upright
//   family and 3 is the all-fours family. The AI also asks unarmed men for
//   crouch (ManPos 6) in COMBAT at low speed, so a crouch family exists too.
// - Every movement action of an action class points to a state of the SAME
//   upDegree. The stock civilian breaks that rule (lying turns use the armed
//   lying actions), which is the prone turn flicker.
// - Edge costs are stored as char(cost*50) (SoldierOld.hpp:258-266,
//   SoldierOld.cpp:132-147): keep every cost at or below 2.5.

// ---------------------------------------------------------------------------
// Clip mapping table. A rename or a retime is a one-line edit here.
// CHUPA_SPD_*: the state's speed. Negative = seconds per cycle (the engine
// uses -1/speed as cycles per second, SoldierOldSimProxy.cpp:1220-1224);
// 1e10 = a static pose.
// ---------------------------------------------------------------------------
#define CHUPA_RTM_IDLE        "\udchupa\anim\chupa_idle_twitch.rtm"
#define CHUPA_RTM_STALK       "\udchupa\anim\chupa_stalk_crouch.rtm"
#define CHUPA_RTM_STALK_TL    "\udchupa\anim\chupa_stalk_crouch_turnl.rtm"
#define CHUPA_RTM_STALK_TR    "\udchupa\anim\chupa_stalk_crouch_turnr.rtm"
#define CHUPA_RTM_LOPE        "\udchupa\anim\chupa_lope.rtm"
#define CHUPA_RTM_RUSH        "\udchupa\anim\chupa_sprint_rush.rtm"
#define CHUPA_RTM_FREEZE      "\udchupa\anim\chupa_freeze.rtm"
#define CHUPA_RTM_CRAWL       "\udchupa\anim\chupa_crawl.rtm"
#define CHUPA_RTM_CRAWL_TL    "\udchupa\anim\chupa_crawl_turnl.rtm"
#define CHUPA_RTM_CRAWL_TR    "\udchupa\anim\chupa_crawl_turnr.rtm"
#define CHUPA_RTM_SCUTTLE     "\udchupa\anim\chupa_flee_scuttle.rtm"
#define CHUPA_RTM_GALLOP      "\udchupa\anim\chupa_gallop.rtm"
#define CHUPA_RTM_POUNCE      "\udchupa\anim\chupa_pounce.rtm"
#define CHUPA_RTM_FEED        "\udchupa\anim\chupa_feed.rtm"
#define CHUPA_RTM_SNAPUP      "\udchupa\anim\chupa_snap_up.rtm"
#define CHUPA_RTM_LUNGE       "\udchupa\anim\chupa_lunge_strike.rtm"
#define CHUPA_RTM_RECOIL      "\udchupa\anim\chupa_recoil.rtm"
#define CHUPA_RTM_DIE         "\udchupa\anim\chupa_die.rtm"
// One-keyframe hold of chupa_die's last frame (the RTM writer derives it).
#define CHUPA_RTM_DEAD        "\udchupa\anim\chupa_dead.rtm"

#define CHUPA_SPD_IDLE        -4.0
#define CHUPA_SPD_STALK       -1.6
#define CHUPA_SPD_STALK_TURN  -1.0
#define CHUPA_SPD_LOPE        -0.75
#define CHUPA_SPD_RUSH        -0.45
#define CHUPA_SPD_FREEZE      -6.0
#define CHUPA_SPD_CRAWL       -1.2
#define CHUPA_SPD_CRAWL_TURN  -1.2
#define CHUPA_SPD_SCUTTLE     -0.6
#define CHUPA_SPD_GALLOP      -0.5
#define CHUPA_SPD_POUNCE      -0.8
#define CHUPA_SPD_FEED        -3.0
#define CHUPA_SPD_SNAPUP      -0.45
#define CHUPA_SPD_LUNGE       -0.7
#define CHUPA_SPD_RECOIL      -0.9
#define CHUPA_SPD_DIE         -1.4
#define CHUPA_SPD_STATIC      10000000000.0

// AI gait thresholds (m/s): FastF above limitFast, SlowF above 0.6*limitFast,
// WalkF above 0.1 (SoldierOldAI.cpp:2169-2196).
#define CHUPA_LIMITFAST_UP    4.0
// The upright family's fast gait (AUTO or UP, moving fast: SAFE/AWARE at
// NORMAL or FULL, COMBAT at any speed mode). "ChupaRush" = the upright
// attack sprint; "ChupaGallopUp" = the all-fours gallop as the default chase.
// The low family (setUnitPos "DOWN", under fire, exposed STEALTH) always
// gallops.
#define CHUPA_UP_FAST         "ChupaRush"
#define CHUPA_LIMITFAST_LOW   4.0
// Turn rate cap (rad/s) while in a family (SoldierOldSim.cpp:88-94).
#define CHUPA_TURN_UP         5
#define CHUPA_TURN_LOW        4

// ManPos values (SoldierOld.hpp:473-492), written as numbers so the addon
// does not depend on the base config's named constants.
#define CHUPA_POS_DEAD        0
#define CHUPA_POS_LOW         3
#define CHUPA_POS_CROUCH      6
#define CHUPA_POS_COMBAT      8
#define CHUPA_POS_STAND       10
#define CHUPA_POS_UP          11

// Edge costs (at most 2.5, see the header).
#define CHUPA_COST            1
#define CHUPA_COST_ACT        2.5

// ---------------------------------------------------------------------------
// Base config classes this file derives from, re-declared empty so the
// addon parser can resolve them; empty bodies merge onto the originals
// without changing them.
// ---------------------------------------------------------------------------
class CfgManActions {};
class CfgMovesMC
{
    class Default {};
};

class CfgMovesChupa : CfgMovesMC
{
    // vehicleActions and BlendAnims (aimingNo, legsDefault, legsNo,
    // headDefault, headNo) are inherited from CfgMovesMC.

    class Actions
    {
        // Required name: every move's MoveInfo reads Actions >> NoActions
        // (SoldierOldSimProxy.cpp:1184-1187). Also the dying state's map.
        class NoActions : CfgManActions
        {
            upDegree = -1;
            turnSpeed = 0;
            limitFast = 5;
        };
        class ChupaDeadActions : NoActions
        {
            upDegree = CHUPA_POS_DEAD;
        };

        // Hunched upright family (ManPos "no weapon").
        class ChupaActions : NoActions
        {
            upDegree = CHUPA_POS_UP;
            turnSpeed = CHUPA_TURN_UP;
            limitFast = CHUPA_LIMITFAST_UP;
            stop = "ChupaIdle";
            default = "ChupaIdle";
            turnL = "ChupaTurnL";
            turnR = "ChupaTurnR";
            walkF = "ChupaStalkF";
            walkLF = "ChupaStalkF";
            walkRF = "ChupaStalkF";
            walkL = "ChupaStalkF";
            walkR = "ChupaStalkF";
            slowF = "ChupaLope";
            slowLF = "ChupaLope";
            slowRF = "ChupaLope";
            slowL = "ChupaLope";
            slowR = "ChupaLope";
            fastF = CHUPA_UP_FAST;
            fastLF = CHUPA_UP_FAST;
            fastRF = CHUPA_UP_FAST;
            fastL = CHUPA_UP_FAST;
            fastR = CHUPA_UP_FAST;
            // No backward clip: backing off means holding still.
            walkB = "ChupaIdle";
            walkLB = "ChupaIdle";
            walkRB = "ChupaIdle";
            slowB = "ChupaIdle";
            slowLB = "ChupaIdle";
            slowRB = "ChupaIdle";
            fastB = "ChupaIdle";
            fastLB = "ChupaIdle";
            fastRB = "ChupaIdle";
            // Stance changes.
            civil = "ChupaIdle";
            stand = "ChupaIdle";
            combat = "ChupaIdle";
            up = "ChupaIdle";
            crouch = "ChupaCrouch";
            civilLying = "ChupaFreeze";
            lying = "ChupaFreeze";
            down = "ChupaFreeze";
            die = "ChupaDead";
            // The AI tries strokeGun first (SoldierOldAI.cpp:887-891): keep it empty.
            strokeFist = "ChupaLungeStrike";
            strokeGun = "";
        };
        // Lope and rush: a strike while running is the pounce.
        class ChupaRunActions : ChupaActions
        {
            strokeFist = "ChupaPounce";
        };

        // All-fours family (ManPos "lying no weapon").
        class ChupaLowActions : NoActions
        {
            upDegree = CHUPA_POS_LOW;
            turnSpeed = CHUPA_TURN_LOW;
            limitFast = CHUPA_LIMITFAST_LOW;
            stop = "ChupaFreeze";
            default = "ChupaFreeze";
            turnL = "ChupaCrawlTurnL";
            turnR = "ChupaCrawlTurnR";
            walkF = "ChupaCrawlF";
            walkLF = "ChupaCrawlF";
            walkRF = "ChupaCrawlF";
            walkL = "ChupaCrawlF";
            walkR = "ChupaCrawlF";
            slowF = "ChupaScuttle";
            slowLF = "ChupaScuttle";
            slowRF = "ChupaScuttle";
            slowL = "ChupaScuttle";
            slowR = "ChupaScuttle";
            fastF = "ChupaGallop";
            fastLF = "ChupaGallop";
            fastRF = "ChupaGallop";
            fastL = "ChupaGallop";
            fastR = "ChupaGallop";
            walkB = "ChupaFreeze";
            walkLB = "ChupaFreeze";
            walkRB = "ChupaFreeze";
            slowB = "ChupaFreeze";
            slowLB = "ChupaFreeze";
            slowRB = "ChupaFreeze";
            fastB = "ChupaFreeze";
            fastLB = "ChupaFreeze";
            fastRB = "ChupaFreeze";
            civil = "ChupaIdle";
            stand = "ChupaIdle";
            combat = "ChupaIdle";
            up = "ChupaIdle";
            crouch = "ChupaCrouch";
            civilLying = "ChupaFreeze";
            lying = "ChupaFreeze";
            down = "ChupaFreeze";
            die = "ChupaDead";
            strokeFist = "ChupaLungeStrike";
            strokeGun = "";
        };
        // Gallop and scuttle: a strike at speed is the pounce.
        class ChupaLowRunActions : ChupaLowActions
        {
            strokeFist = "ChupaPounce";
        };
        // Pinned on a victim: holding still means feeding.
        class ChupaPinActions : ChupaLowActions
        {
            stop = "ChupaFeed";
            default = "ChupaFeed";
        };

        // Crouch family (ManPos 6), same hunched clips. Only reached when the
        // AI asks an unarmed man to crouch (COMBAT, below 2 m/s).
        class ChupaCrouchActions : ChupaActions
        {
            upDegree = CHUPA_POS_CROUCH;
            stop = "ChupaCrouch";
            default = "ChupaCrouch";
            turnL = "ChupaCrouchTurnL";
            turnR = "ChupaCrouchTurnR";
            walkF = "ChupaCrouchStalkF";
            walkLF = "ChupaCrouchStalkF";
            walkRF = "ChupaCrouchStalkF";
            walkL = "ChupaCrouchStalkF";
            walkR = "ChupaCrouchStalkF";
            slowF = "ChupaCrouchStalkF";
            slowLF = "ChupaCrouchStalkF";
            slowRF = "ChupaCrouchStalkF";
            slowL = "ChupaCrouchStalkF";
            slowR = "ChupaCrouchStalkF";
            fastF = "ChupaCrouchStalkF";
            fastLF = "ChupaCrouchStalkF";
            fastRF = "ChupaCrouchStalkF";
            fastL = "ChupaCrouchStalkF";
            fastR = "ChupaCrouchStalkF";
            walkB = "ChupaCrouch";
            walkLB = "ChupaCrouch";
            walkRB = "ChupaCrouch";
            slowB = "ChupaCrouch";
            slowLB = "ChupaCrouch";
            slowRB = "ChupaCrouch";
            fastB = "ChupaCrouch";
            fastLB = "ChupaCrouch";
            fastRB = "ChupaCrouch";
        };

        // Spawn aliases. A new man starts in the default move of the stance
        // its combat mode names: stand (SAFE, CARELESS) or combat (the rest)
        // (SoldierOldCommon.hpp:137-148, SoldierOld.cpp:345-370). Without a
        // map of those upDegrees the creature would start with no move.
        class ChupaAliasStandActions : ChupaActions
        {
            upDegree = CHUPA_POS_STAND;
        };
        class ChupaAliasCombatActions : ChupaActions
        {
            upDegree = CHUPA_POS_COMBAT;
        };
    };

    // Shared state defaults, outside States so they are not moves themselves
    // (only the own entries of States become moves). Default (CfgMovesMC)
    // supplies every key MoveInfo reads; this changes what the creature needs.
    class ChupaBase : Default
    {
        actions = "ChupaActions";
        file = CHUPA_RTM_IDLE;
        looped = 1;
        preload = 1;
        speed = CHUPA_SPD_IDLE;
        disableWeapons = 0;
        disableWeaponsLong = 0;
        enableOptics = 0;
        showWeaponAim = 0;
        limitGunMovement = 0;
        aiming = "aimingNo";
        legs = "legsDefault";
        head = "headDefault";
        duty = -1;
        // Footstep sound edges stay at the Default 0.5 / 1.0 (SPEC.md).
        soundEnabled = 0;
        relSpeedMin = 1;
        relSpeedMax = 1;
        onLandBeg = 0;
        onLandEnd = 0;
        interpolationSpeed = 6;
        interpolationRestart = 0;
        visibleSize = 0.8;
        variantsAI[] = {""};
        variantsPlayer[] = {};
        equivalentTo = "";
        connectFrom[] = {};
        connectTo[] = {};
        interpolateWith[] = {};
        interpolateTo[] = {};
        interpolateFrom[] = {};
    };
    class ChupaLowBase : ChupaBase
    {
        actions = "ChupaLowActions";
        legs = "legsNo";
        onLandBeg = 1;
        onLandEnd = 1;
        interpolationSpeed = 5;
        visibleSize = 0.3;
    };

    class States
    {
        // --- hunched upright family -------------------------------------
        class ChupaIdle : ChupaBase
        {
            file = CHUPA_RTM_IDLE;
            speed = CHUPA_SPD_IDLE;
            interpolationSpeed = 5;
            // Dropping to all fours has no clip: a quick blend.
            interpolateTo[] = {"ChupaFreeze", CHUPA_COST};
        };
        class ChupaStalkF : ChupaBase
        {
            file = CHUPA_RTM_STALK;
            speed = CHUPA_SPD_STALK;
            relSpeedMin = 0.5;
            relSpeedMax = 1;
            soundEnabled = 1;
            visibleSize = 0.7;
            interpolateTo[] = {"ChupaCrawlF", CHUPA_COST};
        };
        class ChupaTurnL : ChupaBase
        {
            file = CHUPA_RTM_STALK_TL;
            speed = CHUPA_SPD_STALK_TURN;
            visibleSize = 0.7;
        };
        class ChupaTurnR : ChupaTurnL
        {
            file = CHUPA_RTM_STALK_TR;
        };
        class ChupaLope : ChupaBase
        {
            actions = "ChupaRunActions";
            file = CHUPA_RTM_LOPE;
            speed = CHUPA_SPD_LOPE;
            relSpeedMin = 0.7;
            relSpeedMax = 1;
            duty = 0.3;
            soundEnabled = 1;
            interpolateTo[] = {"ChupaScuttle", CHUPA_COST};
        };
        class ChupaRush : ChupaBase
        {
            actions = "ChupaRunActions";
            file = CHUPA_RTM_RUSH;
            speed = CHUPA_SPD_RUSH;
            relSpeedMin = 0.85;
            relSpeedMax = 1;
            duty = 0.8;
            soundEnabled = 1;
            visibleSize = 1;
            interpolateTo[] = {"ChupaGallop", CHUPA_COST};
        };

        // --- all-fours family -------------------------------------------
        class ChupaFreeze : ChupaLowBase
        {
            file = CHUPA_RTM_FREEZE;
            speed = CHUPA_SPD_FREEZE;
            // Going still is instant.
            interpolationSpeed = 8;
            visibleSize = 0.25;
            // Rising always goes through the snap (crouch too: snap, then crouch).
            interpolateTo[] = {"ChupaSnapUp", CHUPA_COST};
        };
        class ChupaCrawlF : ChupaLowBase
        {
            file = CHUPA_RTM_CRAWL;
            speed = CHUPA_SPD_CRAWL;
            relSpeedMin = 0.6;
            relSpeedMax = 1;
            soundEnabled = 1;
            interpolateTo[] = {"ChupaSnapUp", CHUPA_COST};
        };
        class ChupaCrawlTurnL : ChupaLowBase
        {
            file = CHUPA_RTM_CRAWL_TL;
            speed = CHUPA_SPD_CRAWL_TURN;
            interpolateTo[] = {"ChupaSnapUp", CHUPA_COST};
        };
        class ChupaCrawlTurnR : ChupaCrawlTurnL
        {
            file = CHUPA_RTM_CRAWL_TR;
        };
        class ChupaScuttle : ChupaLowBase
        {
            actions = "ChupaLowRunActions";
            file = CHUPA_RTM_SCUTTLE;
            speed = CHUPA_SPD_SCUTTLE;
            relSpeedMin = 0.7;
            relSpeedMax = 1;
            duty = 0.5;
            soundEnabled = 1;
            visibleSize = 0.35;
            // Rising while moving: blend straight into the lope.
            interpolateTo[] = {"ChupaLope", CHUPA_COST};
        };
        class ChupaGallop : ChupaLowBase
        {
            actions = "ChupaLowRunActions";
            file = CHUPA_RTM_GALLOP;
            speed = CHUPA_SPD_GALLOP;
            // Never slower than its natural pace: it always outruns a runner.
            relSpeedMin = 1;
            relSpeedMax = 1;
            duty = 0.8;
            soundEnabled = 1;
            visibleSize = 0.45;
            interpolateTo[] = {"ChupaRush", CHUPA_COST};
        };
        // The gallop clip in the upright family, for CHUPA_UP_FAST.
        class ChupaGallopUp : ChupaGallop
        {
            actions = "ChupaRunActions";
            interpolateWith[] = {"ChupaGallop", CHUPA_COST};
            interpolateTo[] = {};
        };
        class ChupaFeed : ChupaLowBase
        {
            actions = "ChupaPinActions";
            file = CHUPA_RTM_FEED;
            speed = CHUPA_SPD_FEED;
            interpolationSpeed = 4;
            visibleSize = 0.4;
            interpolateWith[] = {"ChupaFreeze", CHUPA_COST};
            interpolateTo[] = {"ChupaSnapUp", CHUPA_COST};
        };

        // --- crouch family (same clips as the upright family) ------------
        class ChupaCrouch : ChupaBase
        {
            actions = "ChupaCrouchActions";
            file = CHUPA_RTM_IDLE;
            speed = CHUPA_SPD_IDLE;
            visibleSize = 0.6;
            interpolateWith[] = {"ChupaIdle", CHUPA_COST};
            interpolateTo[] = {"ChupaFreeze", CHUPA_COST};
        };
        class ChupaCrouchStalkF : ChupaStalkF
        {
            actions = "ChupaCrouchActions";
            visibleSize = 0.6;
            interpolateWith[] = {"ChupaStalkF", CHUPA_COST};
            interpolateTo[] = {};
        };
        class ChupaCrouchTurnL : ChupaTurnL
        {
            actions = "ChupaCrouchActions";
            visibleSize = 0.6;
            interpolateWith[] = {"ChupaTurnL", CHUPA_COST};
        };
        class ChupaCrouchTurnR : ChupaTurnR
        {
            actions = "ChupaCrouchActions";
            visibleSize = 0.6;
            interpolateWith[] = {"ChupaTurnR", CHUPA_COST};
        };

        // --- one-shot beats ----------------------------------------------
        // Non-looped: it plays once and holds its last frame until a
        // connectTo edge takes over. Exits are connectTo (wait for the end)
        // except death and recoil, so the AI cannot cut a strike short.
        class ChupaSnapUp : ChupaBase
        {
            file = CHUPA_RTM_SNAPUP;
            speed = CHUPA_SPD_SNAPUP;
            looped = 0;
            head = "headNo";
            onLandBeg = 1;
            onLandEnd = 0;
            interpolationSpeed = 10;
            interpolationRestart = 1;
            connectTo[] = {"ChupaIdle", CHUPA_COST, "ChupaStalkF", CHUPA_COST, "ChupaLope", CHUPA_COST, "ChupaRush", CHUPA_COST};
        };
        class ChupaLungeStrike : ChupaBase
        {
            // A stroke weapon's hit lands when this move ENDS
            // (SoldierOldMove.cpp:291-299, 95-99; SoldierOld.cpp:1532-1538).
            file = CHUPA_RTM_LUNGE;
            speed = CHUPA_SPD_LUNGE;
            looped = 0;
            head = "headNo";
            duty = 0.8;
            visibleSize = 1;
            interpolationSpeed = 10;
            interpolationRestart = 1;
            // The rush is the attack run: without its own edge a strike from
            // it walked rush -> lope (or idle) -> lunge, two blends.
            interpolateFrom[] = {"ChupaIdle", CHUPA_COST_ACT, "ChupaStalkF", CHUPA_COST_ACT, "ChupaTurnL", CHUPA_COST_ACT, "ChupaTurnR", CHUPA_COST_ACT, "ChupaLope", CHUPA_COST_ACT, "ChupaRush", CHUPA_COST_ACT, "ChupaCrouch", CHUPA_COST_ACT, "ChupaCrouchStalkF", CHUPA_COST_ACT, "ChupaFreeze", CHUPA_COST_ACT, "ChupaCrawlF", CHUPA_COST_ACT, "ChupaFeed", CHUPA_COST_ACT};
            connectTo[] = {"ChupaIdle", CHUPA_COST};
        };
        class ChupaPounce : ChupaLowBase
        {
            // 3 to 5 m leap that ends pinning the victim. Hit lands at the end.
            actions = "ChupaPinActions";
            file = CHUPA_RTM_POUNCE;
            speed = CHUPA_SPD_POUNCE;
            looped = 0;
            head = "headNo";
            duty = 0.8;
            visibleSize = 0.6;
            interpolationSpeed = 10;
            interpolationRestart = 1;
            interpolateFrom[] = {"ChupaGallop", CHUPA_COST_ACT, "ChupaGallopUp", CHUPA_COST_ACT, "ChupaScuttle", CHUPA_COST_ACT, "ChupaRush", CHUPA_COST_ACT, "ChupaLope", CHUPA_COST_ACT, "ChupaStalkF", CHUPA_COST_ACT, "ChupaCrawlF", CHUPA_COST_ACT, "ChupaIdle", CHUPA_COST_ACT, "ChupaFreeze", CHUPA_COST_ACT, "ChupaCrouch", CHUPA_COST_ACT};
            // The last frame is the PIN pose, equal to the feed's frame 0.
            // No edge to the lunge: it starts on CROUCH, 95 cm off at the
            // claws. Recoil leaves the pin through its own interpolateFrom.
            connectTo[] = {"ChupaFeed", CHUPA_COST, "ChupaFreeze", CHUPA_COST, "ChupaSnapUp", CHUPA_COST};
        };
        class ChupaRecoil : ChupaBase
        {
            // Shoved off: thrown back, then straight back into the attack.
            file = CHUPA_RTM_RECOIL;
            speed = CHUPA_SPD_RECOIL;
            looped = 0;
            head = "headNo";
            visibleSize = 0.9;
            interpolationSpeed = 12;
            interpolationRestart = 1;
            interpolateFrom[] = {"ChupaPounce", CHUPA_COST_ACT, "ChupaFeed", CHUPA_COST_ACT, "ChupaLungeStrike", CHUPA_COST_ACT, "ChupaIdle", CHUPA_COST_ACT, "ChupaStalkF", CHUPA_COST_ACT, "ChupaLope", CHUPA_COST_ACT, "ChupaRush", CHUPA_COST_ACT, "ChupaFreeze", CHUPA_COST_ACT, "ChupaCrawlF", CHUPA_COST_ACT, "ChupaCrouch", CHUPA_COST_ACT};
            connectTo[] = {"ChupaIdle", CHUPA_COST, "ChupaLope", CHUPA_COST, "ChupaRush", CHUPA_COST, "ChupaGallop", CHUPA_COST, "ChupaGallopUp", CHUPA_COST, "ChupaFreeze", CHUPA_COST};
        };

        // --- death -------------------------------------------------------
        class ChupaDying : ChupaBase
        {
            actions = "NoActions";
            file = CHUPA_RTM_DIE;
            speed = CHUPA_SPD_DIE;
            looped = 0;
            legs = "legsNo";
            head = "headNo";
            onLandBeg = 0;
            onLandEnd = 1;
            interpolationSpeed = 8;
            interpolationRestart = 1;
            soundEnabled = 1;
            soundOverride = "fallbody";
            soundEdge1 = 0.45;
            soundEdge2 = 0.45;
            interpolateFrom[] = {"ChupaIdle", 0.1, "ChupaStalkF", 0.1, "ChupaTurnL", 0.1, "ChupaTurnR", 0.1, "ChupaLope", 0.1, "ChupaRush", 0.1, "ChupaFreeze", 0.1, "ChupaCrawlF", 0.1, "ChupaCrawlTurnL", 0.1, "ChupaCrawlTurnR", 0.1, "ChupaScuttle", 0.1, "ChupaGallop", 0.1, "ChupaGallopUp", 0.1, "ChupaFeed", 0.1, "ChupaCrouch", 0.1, "ChupaCrouchStalkF", 0.1, "ChupaCrouchTurnL", 0.1, "ChupaCrouchTurnR", 0.1, "ChupaSnapUp", 0.1, "ChupaLungeStrike", 0.1, "ChupaPounce", 0.1, "ChupaRecoil", 0.1, "ChupaAliasStand", 0.1, "ChupaAliasCombat", 0.1};
            connectTo[] = {"ChupaDead", CHUPA_COST};
        };
        class ChupaDead : ChupaBase
        {
            // upDegree dead is what Man::IsDead() tests (SoldierOld.cpp:1244-1247).
            actions = "ChupaDeadActions";
            file = CHUPA_RTM_DEAD;
            speed = CHUPA_SPD_STATIC;
            looped = 1;
            legs = "legsNo";
            head = "headNo";
            onLandBeg = 1;
            onLandEnd = 1;
            terminal = 1;
            visibleSize = 0.3;
        };

        // --- spawn aliases (never entered by the AI) ---------------------
        class ChupaAliasStand : ChupaBase
        {
            actions = "ChupaAliasStandActions";
            interpolateTo[] = {"ChupaIdle", CHUPA_COST};
        };
        class ChupaAliasCombat : ChupaBase
        {
            actions = "ChupaAliasCombatActions";
            interpolateTo[] = {"ChupaIdle", CHUPA_COST};
        };
    };

    // Phase-synced blends within each gait family (all pairs, both ways).
    // A blend keeps the current phase (interpolationRestart = 0), so the
    // clips of one family must share their foot timing (SPEC.md).
    class Interpolations
    {
        chupaUp[] = {CHUPA_COST, "ChupaIdle", "ChupaStalkF", "ChupaTurnL", "ChupaTurnR", "ChupaLope", "ChupaRush", "ChupaGallopUp"};
        chupaLow[] = {CHUPA_COST, "ChupaFreeze", "ChupaCrawlF", "ChupaCrawlTurnL", "ChupaCrawlTurnR", "ChupaScuttle", "ChupaGallop"};
        chupaCrouch[] = {CHUPA_COST, "ChupaCrouch", "ChupaCrouchStalkF", "ChupaCrouchTurnL", "ChupaCrouchTurnR"};
    };
    transitionsInterpolated[] = {};
    transitionsSimple[] = {};
    transitionsDisabled[] = {};
};
