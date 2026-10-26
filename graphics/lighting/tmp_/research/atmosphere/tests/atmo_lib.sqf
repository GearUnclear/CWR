// Atmosphere research track: helpers on top of capture_lib.sqf.
// Every knob the prototype adds, back to its default (off).
capAtmoReset = {
    capR0 = triSetLighting "atmosphere=0";
    capR1 = triSetLighting "atmoFog=0";
    capR2 = triSetLighting "atmoLight=0";
    capR3 = triSetLighting "atmoSun=0";
    capR4 = triSetLighting "atmoHaze=1";
    capR5 = triSetLighting "atmoExposure=1";
    capR6 = triSetLighting "atmoClouds=1";
    capR7 = triSetLighting "atmoDebug=0";
}

// ["label", ["key=value", ...]] call capVariant: Remastered with the atmosphere
// knobs reset and then the given ones applied; restarts the timers.
capVariant = {
    capModeName = _this select 0;
    capLit = triSetLighting "remastered";
    [] call capAtmoReset;
    capKnobRes = [];
    { capKnobRes = capKnobRes + [triSetLighting _x] } forEach (_this select 1);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3;knobs=%4;res=%5", capView, capModeName, capLit, _this select 1, capKnobRes] call capLog;
}
