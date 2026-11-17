// udchupa: the chupacabra creature as a Man-class unit (experiment, not wired
// into Guerrilla Mode). Packed by ../build-udchupa.ps1 into udchupa.pbo; the
// pbo file name is the path prefix, so its files resolve as \udchupa\...
//
// The model keeps the 25 stock man bone selections and the stock rest pose,
// so the stock CfgMovesMC animations (inherited from Civilian) drive it.
// No forward declarations anywhere: `class X;` aborts this parser, so the
// base chains are re-declared empty, which merges onto the originals.

#define private   0
#define protected 1
#define public    2

// Custom horror moves (CfgMovesChupa, class UDChupacabraAnim). Off by default
// so the stock-moves creature keeps working until the RTMs exist. Turn it on
// with build-udchupa.ps1 -CustomMoves, which defines this and inlines
// moves_chupa.hpp into the staged config (no #include inside the pbo).
// #define UDCHUPA_CUSTOM_MOVES
#ifdef UDCHUPA_CUSTOM_MOVES
#include "moves_chupa.hpp"
#endif

class CfgPatches
{
    class udchupa
    {
#ifdef UDCHUPA_CUSTOM_MOVES
        units[] = {"UDChupacabra", "UDChupacabraAnim"};
#else
        units[] = {"UDChupacabra"};
#endif
        weapons[] = {};
        requiredVersion = 1.30;
        // Civilian, Man and CfgModels >> Man live in the base config (no owner
        // addon), so nothing has to be required.
        requiredAddons[] = {};
    };
};

class CfgModels
{
    class Default {};
    class Head : Default {};
    class Man : Default {};
    // Looked up by the model's file stem (chupacabra.p3d). Inheriting Man
    // gives the man's section splits (wound selections, hlava, krk); the
    // creature has none of the wound selections yet, so they simply match
    // nothing.
    class chupacabra : Man {};
};

class CfgVehicles
{
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Civilian : Man {};

    class UDChupacabra : Civilian
    {
        scope = public;
        side = 3;                       // civilian by default; a script may put it in any group
        displayName = "Chupacabra";
        model = "\udchupa\chupacabra";  // moves stays CfgMovesMC from Civilian
        hiddenSelections[] = {};
        // Tougher than a man (Man armor = 3), not a tank: about two rifle
        // hits to the body.
        armor = 6;
        armorStructural = 2;
        cost = 50000;
        // Civilian carries Throw and Put; the creature carries nothing.
        weapons[] = {};
        magazines[] = {};
    };
#ifdef UDCHUPA_CUSTOM_MOVES
    // The same creature on its own move set. A separate class, so the
    // stock-moves UDChupacabra and its spawn test stay as they are.
    class UDChupacabraAnim : UDChupacabra
    {
        displayName = "Chupacabra (horror moves)";
        moves = "CfgMovesChupa";
        // km/h. Caps the AI's wanted speed (SoldierOldAI.cpp:1786-1789) and
        // scales LIMITED (0.22x, AISubgroup.cpp:2289-2296). Above the rush
        // and gallop clip speeds so the AI never asks them to slow down.
        maxSpeed = 30;
    };
#endif
};
