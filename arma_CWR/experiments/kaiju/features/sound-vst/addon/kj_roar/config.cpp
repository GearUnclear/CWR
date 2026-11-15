// kj_roar (VST set, round 2): Dedushka's sounds. A drop-in replacement for the first
// set's kj_roar addon: the same CfgPatches class and the same twelve CfgSounds classes the
// roar scripts use, plus one class per roar take for the two creatures, the other footfall
// takes and the other bird take. Mount @kj_roar_vst INSTEAD of @kj_roar, never both.
// This file is written by ../../make_round2.py (write_config); edit the table there.
// Volumes as in the first set: db+50 has a 533 m reference distance, db+56 754 m,
// db+40 300 m, db+20 95 m (Audio/Shared/AudioMath.hpp:8-21).

class CfgPatches
{
    class kj_roar
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {};
    };
};

class CfgSounds
{
    // the default roar (a copy of the Starved Bear, take 1), close
    class KJ_Roar
    {
        name = "KJ_Roar";
        sound[] = {"\kj_roar\snd\roar.wss", db+50, 1};
        titles[] = {};
    };
    // its far version
    class KJ_RoarFar
    {
        name = "KJ_RoarFar";
        sound[] = {"\kj_roar\snd\roar_far.wss", db+56, 1};
        titles[] = {};
    };
    // round 1 roar (take A, with its own reverb): a middle distance
    class KJ_RoarMid
    {
        name = "KJ_RoarMid";
        sound[] = {"\kj_roar\snd\roar_mid.wss", db+50, 1};
        titles[] = {};
    };
    // Starved Bear, take 1, close
    class KJ_RoarBear1
    {
        name = "KJ_RoarBear1";
        sound[] = {"\kj_roar\snd\roar_bear1.wss", db+50, 1};
        titles[] = {};
    };
    // Starved Bear, take 2, close
    class KJ_RoarBear2
    {
        name = "KJ_RoarBear2";
        sound[] = {"\kj_roar\snd\roar_bear2.wss", db+50, 1};
        titles[] = {};
    };
    // Starved Bear, take 3, close
    class KJ_RoarBear3
    {
        name = "KJ_RoarBear3";
        sound[] = {"\kj_roar\snd\roar_bear3.wss", db+50, 1};
        titles[] = {};
    };
    // Starved Bear, take 4, close
    class KJ_RoarBear4
    {
        name = "KJ_RoarBear4";
        sound[] = {"\kj_roar\snd\roar_bear4.wss", db+50, 1};
        titles[] = {};
    };
    // Starved Bear, take 1, far
    class KJ_RoarBear1Far
    {
        name = "KJ_RoarBear1Far";
        sound[] = {"\kj_roar\snd\roar_bear1_far.wss", db+56, 1};
        titles[] = {};
    };
    // Starved Bear, take 2, far
    class KJ_RoarBear2Far
    {
        name = "KJ_RoarBear2Far";
        sound[] = {"\kj_roar\snd\roar_bear2_far.wss", db+56, 1};
        titles[] = {};
    };
    // Starved Bear, take 3, far
    class KJ_RoarBear3Far
    {
        name = "KJ_RoarBear3Far";
        sound[] = {"\kj_roar\snd\roar_bear3_far.wss", db+56, 1};
        titles[] = {};
    };
    // Starved Bear, take 4, far
    class KJ_RoarBear4Far
    {
        name = "KJ_RoarBear4Far";
        sound[] = {"\kj_roar\snd\roar_bear4_far.wss", db+56, 1};
        titles[] = {};
    };
    // dog and lizard, take 1, close
    class KJ_RoarDog1
    {
        name = "KJ_RoarDog1";
        sound[] = {"\kj_roar\snd\roar_dog1.wss", db+50, 1};
        titles[] = {};
    };
    // dog and lizard, take 2, close
    class KJ_RoarDog2
    {
        name = "KJ_RoarDog2";
        sound[] = {"\kj_roar\snd\roar_dog2.wss", db+50, 1};
        titles[] = {};
    };
    // dog and lizard, take 3, close
    class KJ_RoarDog3
    {
        name = "KJ_RoarDog3";
        sound[] = {"\kj_roar\snd\roar_dog3.wss", db+50, 1};
        titles[] = {};
    };
    // dog and lizard, take 4, close
    class KJ_RoarDog4
    {
        name = "KJ_RoarDog4";
        sound[] = {"\kj_roar\snd\roar_dog4.wss", db+50, 1};
        titles[] = {};
    };
    // dog and lizard, take 1, far
    class KJ_RoarDog1Far
    {
        name = "KJ_RoarDog1Far";
        sound[] = {"\kj_roar\snd\roar_dog1_far.wss", db+56, 1};
        titles[] = {};
    };
    // dog and lizard, take 2, far
    class KJ_RoarDog2Far
    {
        name = "KJ_RoarDog2Far";
        sound[] = {"\kj_roar\snd\roar_dog2_far.wss", db+56, 1};
        titles[] = {};
    };
    // dog and lizard, take 3, far
    class KJ_RoarDog3Far
    {
        name = "KJ_RoarDog3Far";
        sound[] = {"\kj_roar\snd\roar_dog3_far.wss", db+56, 1};
        titles[] = {};
    };
    // dog and lizard, take 4, far
    class KJ_RoarDog4Far
    {
        name = "KJ_RoarDog4Far";
        sound[] = {"\kj_roar\snd\roar_dog4_far.wss", db+56, 1};
        titles[] = {};
    };
    // footfall, take 1 (earth), close
    class KJ_Step
    {
        name = "KJ_Step";
        sound[] = {"\kj_roar\snd\step.wss", db+40, 1};
        titles[] = {};
    };
    // footfall, take 2 (street), close
    class KJ_Step2
    {
        name = "KJ_Step2";
        sound[] = {"\kj_roar\snd\step_2.wss", db+40, 1};
        titles[] = {};
    };
    // footfall, take 3 (heel and sole), close
    class KJ_Step3
    {
        name = "KJ_Step3";
        sound[] = {"\kj_roar\snd\step_3.wss", db+40, 1};
        titles[] = {};
    };
    // footfall, far (from take 1)
    class KJ_StepFar
    {
        name = "KJ_StepFar";
        sound[] = {"\kj_roar\snd\step_far.wss", db+46, 1};
        titles[] = {};
    };
    // furnace drone, close, seamless 8 s loop
    class KJ_Furnace
    {
        name = "KJ_Furnace";
        sound[] = {"\kj_roar\snd\furnace.wss", db+20, 1};
        titles[] = {};
    };
    // beam charge, close
    class KJ_BeamCharge
    {
        name = "KJ_BeamCharge";
        sound[] = {"\kj_roar\snd\beam_charge.wss", db+42, 1};
        titles[] = {};
    };
    // beam, close
    class KJ_Beam
    {
        name = "KJ_Beam";
        sound[] = {"\kj_roar\snd\beam.wss", db+50, 1};
        titles[] = {};
    };
    // beam body, close, seamless 3 s loop
    class KJ_BeamLoop
    {
        name = "KJ_BeamLoop";
        sound[] = {"\kj_roar\snd\beam_loop.wss", db+50, 1};
        titles[] = {};
    };
    // spent exhale, close
    class KJ_Exhale
    {
        name = "KJ_Exhale";
        sound[] = {"\kj_roar\snd\exhale.wss", db+36, 1};
        titles[] = {};
    };
    // radio static (round 1, unchanged)
    class KJ_Static
    {
        name = "KJ_Static";
        sound[] = {"\kj_roar\snd\static.wss", db-3, 1};
        titles[] = {};
    };
    // radio static, faint
    class KJ_StaticFaint
    {
        name = "KJ_StaticFaint";
        sound[] = {"\kj_roar\snd\static.wss", db-16, 1};
        titles[] = {};
    };
    // birds: mostly wings (the recommended take)
    class KJ_Birds
    {
        name = "KJ_Birds";
        sound[] = {"\kj_roar\snd\birds.wss", db+14, 1};
        titles[] = {};
    };
    // birds: wings and alarm calls
    class KJ_Birds1
    {
        name = "KJ_Birds1";
        sound[] = {"\kj_roar\snd\birds_1.wss", db+14, 1};
        titles[] = {};
    };
};
