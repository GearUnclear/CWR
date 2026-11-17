// kj_roar: Dedushka's voice. Sounds only, no models and no units; packed by
// ../build-kj-roar.ps1 into kj_roar.pbo (the pbo name is the path prefix, so
// the files resolve as \kj_roar\snd\...). Mount it beside @uddedushka.
//
// Every file is synthesised here by ../synth_sounds.py; nothing is sampled.
//
// Volumes. A 3D sound's reference distance is sqrt(volume) * 30 m (at ear
// accommodation 1) and the gain past it is reference / distance
// (Audio/Shared/AudioMath.hpp:8-21, PoseidonOpenAL/SoundSystemOAL.cpp:153),
// and "db+N" is 10^(N/20) (IO/ParamFile/ParamFile.cpp:718-733). So:
//   db+50  reference 533 m   -11 dB at 2 km   -19 dB at 5 km
//   db+56  reference 754 m   -8 dB at 2 km    -16 dB at 5 km   -22 dB at 10 km
//   db+40  reference 300 m   -10 dB at 1 km   -20 dB at 3 km
//   db+20  reference 95 m    -10 dB at 300 m  -16 dB at 600 m
// A wave is only dropped when volume < 1e-8 * distance^2
// (Audio/Core/VoiceBudget.cpp:36), which none of these reach on any island.
//
// A script picks the near or the far class by the listener's distance
// (kj\roar\init.sqs holds the switch distances).

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
    // The roar, heard from his feet out to a kilometre or so.
    class KJ_Roar
    {
        name = "Dedushka roar";
        sound[] = {"\kj_roar\snd\roar.wss", db+50, 1};
        titles[] = {};
    };
    // The same roar after kilometres of air and a valley's echo.
    class KJ_RoarFar
    {
        name = "Dedushka roar (far)";
        sound[] = {"\kj_roar\snd\roar_far.wss", db+56, 1};
        titles[] = {};
    };
    // One footfall: the blow, then the ground still moving.
    class KJ_Step
    {
        name = "Dedushka footfall";
        sound[] = {"\kj_roar\snd\step.wss", db+40, 1};
        titles[] = {};
    };
    class KJ_StepFar
    {
        name = "Dedushka footfall (far)";
        sound[] = {"\kj_roar\snd\step_far.wss", db+46, 1};
        titles[] = {};
    };
    // The furnace in his chest. A seamless 8 s loop; kj\roar\presence.sqs
    // restarts it while the listener is near.
    class KJ_Furnace
    {
        name = "Dedushka furnace";
        sound[] = {"\kj_roar\snd\furnace.wss", db+20, 1};
        titles[] = {};
    };
    // The beam. Charge whine (4.2 s, ends on a hard edge), the beam itself
    // (crack, 4.4 s of hold, fall-off), and a seamless 3 s body for a beam
    // that is held longer.
    class KJ_BeamCharge
    {
        name = "Dedushka beam charge";
        sound[] = {"\kj_roar\snd\beam_charge.wss", db+42, 1};
        titles[] = {};
    };
    class KJ_Beam
    {
        name = "Dedushka beam";
        sound[] = {"\kj_roar\snd\beam.wss", db+50, 1};
        titles[] = {};
    };
    class KJ_BeamLoop
    {
        name = "Dedushka beam (loop)";
        sound[] = {"\kj_roar\snd\beam_loop.wss", db+50, 1};
        titles[] = {};
    };
    // Spent: the long wheezing breath after a beam.
    class KJ_Exhale
    {
        name = "Dedushka exhale";
        sound[] = {"\kj_roar\snd\exhale.wss", db+36, 1};
        titles[] = {};
    };
    // Presence. Static is played 2D (playSound), so its volume is a plain
    // gain; the faint one is for when he is still far off.
    class KJ_Static
    {
        name = "Radio static";
        sound[] = {"\kj_roar\snd\static.wss", db-3, 1};
        titles[] = {};
    };
    class KJ_StaticFaint
    {
        name = "Radio static (faint)";
        sound[] = {"\kj_roar\snd\static.wss", db-16, 1};
        titles[] = {};
    };
    // A flock going up; played from the place the birds leave.
    class KJ_Birds
    {
        name = "Birds taking flight";
        sound[] = {"\kj_roar\snd\birds.wss", db+14, 1};
        titles[] = {};
    };
};
