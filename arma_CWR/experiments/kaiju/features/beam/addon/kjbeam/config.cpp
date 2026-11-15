// kjbeam: the drawn parts of Dedushka's breath (the beam feature). Packed by
// ../build-kjbeam.ps1 into kjbeam.pbo; the pbo file name is the path prefix, so
// its files resolve as \kjbeam\... Mount it beside @uddedushka as @kj_beam.
//
// Nothing here is a weapon and nothing fires: the beam is a row of fullbright
// models that the script (kj\beam\) re-places every frame, the lights are
// Fire-class objects that the script switches with `inflame`, and all damage
// is script setDamage.
//
// Config rules learnt on the chupacabra: no forward declarations, base chains
// re-declared empty so they merge onto the originals.

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class kjbeam
    {
        units[] = {"KJBeamSeg", "KJBeamProbe", "KJBeamGlowA", "KJBeamGlowB", "KJBeamGlowC", "KJBeamGlowD",
                   "KJBeamGlowHit", "KJBeamGlowRed", "KJBeamFlame", "KJBeamFlameBig", "KJBeamSmoke"};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {};
    };
};

// A point light is at full strength out to 50 m * brightness and fades with the
// square of the distance to nothing at ten times that
// (Graphics/Rendering/Lighting/Lights.cpp, LightPoint::Apply; Lights.hpp:193).
// The engine keeps the 16 strongest lights of the scene and 7 per object
// (Core/Config/EngineConfig.hpp:71, World/Scene/Scene.cpp:366), so the script
// keeps few of these alight at once.

class CfgVehicles
{
    class All {};
    class Static : All {};
    class Building : Static {};
    class NonStrategic : Building {};
    class Fire : NonStrategic {};

    // One 50 m piece of the beam. No geometry LOD: it collides with nothing,
    // blocks no view and no shot.
    class KJBeamSeg : NonStrategic
    {
        scope = public;
        model = "\kjbeam\kjbeam_seg";
        displayName = "Dedushka beam";
        simulation = "house";
        destrType = "DestructNo";
        armor = 1000000;
        accuracy = 0;
        cost = 0;
        mapSize = 1;
        vehicleClass = "Objects";
        icon = "Unknown_object";
    };

    // Unseen marker the script moves about to read the ground height.
    class KJBeamProbe : KJBeamSeg
    {
        model = "\kjbeam\kjbeam_null";
        displayName = "Dedushka beam probe";
    };

    // ---- lights only (the furnace and the point of impact) -----------------
    class KJBeamGlowA : Fire
    {
        scope = public;
        model = "\kjbeam\kjbeam_null";
        displayName = "Dedushka furnace light";
        simulation = "fire";
        destrType = "DestructNo";
        armor = 1000000;
        accuracy = 0;
        cost = 0;
        mapSize = 1;
        sound = "Fire";
        // A smoke source that gives off nothing: these classes are only a light.
        // The classes below inherit it.
        class Smoke
        {
            interval = 100000;
            cloudletDuration = 0.1;
            cloudletAnimPeriod = 1;
            cloudletSize = 0.01;
            cloudletAlpha = 0;
            cloudletGrowUp = 0.1;
            cloudletFadeIn = 0;
            cloudletFadeOut = 0.1;
            cloudletAccY = 0;
            cloudletMinYSpeed = 0;
            cloudletMaxYSpeed = 0;
            cloudletShape = "cl_basic";
            cloudletColor[] = {0, 0, 0, 0};
            initT = 0;
            deltaT = 0;
            class Table
            {
                class T1
                {
                    maxT = 0;
                    color[] = {0, 0, 0, 0};
                };
            };
            density = 0.001;
            size = 0.01;
            initYSpeed = 0;
            timeToLive = 100000002004087734272.0;
            in = 0;
            out = 0;
        };
        class Light
        {
            color[] = {1.0, 0.42, 0.08, 1};
            ambient[] = {0.22, 0.09, 0.02, 1};
            brightness = 0.14;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamGlowB : KJBeamGlowA
    {
        class Light
        {
            color[] = {1.0, 0.58, 0.18, 1};
            ambient[] = {0.26, 0.14, 0.04, 1};
            brightness = 0.32;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamGlowC : KJBeamGlowA
    {
        class Light
        {
            color[] = {1.0, 0.80, 0.48, 1};
            ambient[] = {0.30, 0.22, 0.12, 1};
            brightness = 0.65;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamGlowD : KJBeamGlowA
    {
        class Light
        {
            color[] = {1.0, 0.96, 0.86, 1};
            ambient[] = {0.34, 0.32, 0.28, 1};
            brightness = 1.3;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamGlowHit : KJBeamGlowA
    {
        displayName = "Dedushka beam impact light";
        class Light
        {
            color[] = {1.0, 0.86, 0.62, 1};
            ambient[] = {0.30, 0.24, 0.15, 1};
            brightness = 0.55;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamGlowRed : KJBeamGlowA
    {
        class Light
        {
            color[] = {0.75, 0.13, 0.03, 1};
            ambient[] = {0.16, 0.03, 0.01, 1};
            brightness = 0.10;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };

    // ---- what the beam leaves behind ---------------------------------------
    // Flames with their own light. The engine's own emitter does the work, so
    // a fire costs no script time once it is lit.
    class KJBeamFlame : Fire
    {
        scope = public;
        model = "\kjbeam\kjbeam_null";
        displayName = "Dedushka beam fire";
        simulation = "fire";
        destrType = "DestructNo";
        armor = 1000000;
        accuracy = 0;
        cost = 0;
        mapSize = 2;
        sound = "Fire";
        class Smoke
        {
            interval = 0.035;
            cloudletDuration = 1.3;
            cloudletAnimPeriod = 1;
            cloudletSize = 1.5;
            cloudletAlpha = 0.85;
            cloudletGrowUp = 0.5;
            cloudletFadeIn = 0;
            cloudletFadeOut = 2.5;
            cloudletAccY = -0.1;
            cloudletMinYSpeed = 0.6;
            cloudletMaxYSpeed = 5;
            cloudletShape = "cl_basic";
            cloudletColor[] = {1, 1, 1, 0};
            initT = 1000;
            deltaT = -420;
            class Table
            {
                class T1
                {
                    maxT = 0;
                    color[] = {0.35, 0.33, 0.31, 1};
                };
                class T2
                {
                    maxT = 600;
                    color[] = {0.12, 0.11, 0.10, 1};
                };
                class T3
                {
                    maxT = 850;
                    color[] = {1, 0.35, 0.02, 0.8};
                };
                class T4
                {
                    maxT = 1000;
                    color[] = {1, 0.72, 0.25, 0.7};
                };
            };
            density = 0.6;
            size = 1.6;
            initYSpeed = 3.0;
            timeToLive = 100000002004087734272.0;
            in = 0;
            out = 0;
        };
        class Light
        {
            color[] = {1, 0.5, 0.08, 1};
            ambient[] = {0.3, 0.15, 0.02, 1};
            brightness = 0.22;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    class KJBeamFlameBig : KJBeamFlame
    {
        displayName = "Dedushka beam fire (large)";
        mapSize = 4;
        class Smoke
        {
            interval = 0.03;
            cloudletDuration = 2.0;
            cloudletAnimPeriod = 1;
            cloudletSize = 3.2;
            cloudletAlpha = 0.9;
            cloudletGrowUp = 0.8;
            cloudletFadeIn = 0;
            cloudletFadeOut = 4;
            cloudletAccY = -0.1;
            cloudletMinYSpeed = 0.8;
            cloudletMaxYSpeed = 7;
            cloudletShape = "cl_basic";
            cloudletColor[] = {1, 1, 1, 0};
            initT = 1000;
            deltaT = -300;
            class Table
            {
                class T1
                {
                    maxT = 0;
                    color[] = {0.30, 0.28, 0.27, 1};
                };
                class T2
                {
                    maxT = 620;
                    color[] = {0.09, 0.08, 0.08, 1};
                };
                class T3
                {
                    maxT = 850;
                    color[] = {1, 0.33, 0.02, 0.85};
                };
                class T4
                {
                    maxT = 1000;
                    color[] = {1, 0.75, 0.3, 0.75};
                };
            };
            density = 0.7;
            size = 3.5;
            initYSpeed = 4.5;
            timeToLive = 100000002004087734272.0;
            in = 0;
            out = 0;
        };
        class Light
        {
            color[] = {1, 0.52, 0.1, 1};
            ambient[] = {0.3, 0.16, 0.03, 1};
            brightness = 0.4;
            shape = "koulesvetlo";
            size = 0.5;
            position = "kjfire";
        };
    };
    // A tall dark column and no light: the smoke that stays for minutes.
    class KJBeamSmoke : KJBeamFlame
    {
        displayName = "Dedushka beam smoke";
        class Smoke
        {
            interval = 0.12;
            cloudletDuration = 7;
            cloudletAnimPeriod = 1;
            cloudletSize = 4;
            cloudletAlpha = 0.75;
            cloudletGrowUp = 2.5;
            cloudletFadeIn = 0.4;
            cloudletFadeOut = 6;
            cloudletAccY = -0.05;
            cloudletMinYSpeed = 1.2;
            cloudletMaxYSpeed = 6;
            cloudletShape = "cl_basic";
            cloudletColor[] = {1, 1, 1, 0};
            initT = 500;
            deltaT = -40;
            class Table
            {
                class T1
                {
                    maxT = 0;
                    color[] = {0.32, 0.31, 0.30, 1};
                };
                class T2
                {
                    maxT = 500;
                    color[] = {0.07, 0.07, 0.07, 1};
                };
            };
            density = 0.7;
            size = 2.2;
            initYSpeed = 4;
            timeToLive = 100000002004087734272.0;
            in = 0;
            out = 0;
        };
        class Light
        {
            color[] = {0, 0, 0, 1};
            ambient[] = {0, 0, 0, 1};
            brightness = 0.0001;
            shape = "koulesvetlo";
            size = 0.01;
            position = "kjfire";
        };
    };
};
