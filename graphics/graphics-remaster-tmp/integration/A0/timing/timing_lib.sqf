// A0 timing rig: #included after capture_lib.sqf (a verbatim copy of
// arma_CWR/tests/rendering/captures/capture_lib.sqf at 0779350).
// Two additions the hero views need and the repo rig does not have yet
// (plan section 7.2 gives them to A1's capture_motion.sqf):
//   capRain    rain density pinned with the weather (capture_lib pins 0)
//   capEyeAbs  true: capEye is metres ASL, not above the ground probe
//              (sea views: the probe over water reads the sea floor)
// Both are reset per view by the generated test before capBegin.

capRain = 0
capEyeAbs = false

capPinEnv = {
    setDate capDate;
    capWx = triCheatWeather capOvercast;
    0 setOvercast capOvercast;
    0 setRain capRain;
    0 setFog capFog;
    setViewDistance capVD;
    if (capPinTime) then { capSimRes = triSetSimTime capSimT };
}

capPinView = {
    private ["_g", "_dx", "_dy", "_dz"];
    _g = [capOX, capOZ] call capGround;
    _dx = (sin capH) * (cos capP);
    _dy = sin capP;
    _dz = (cos capH) * (cos capP);
    capCam camSetPos [capOX, capOZ, capEye];
    capCam camSetTarget [capOX + 1000 * _dx, capOZ + 1000 * _dz, capEye + 1000 * _dy];
    capCam camSetFov capFov;
    capCam camCommit 0;
    capEyeASL = _g + capEye;
    if (capEyeAbs) then { capEyeASL = capEye };
    capViewRes = triSetView [capOX, capEyeASL, capOZ, _dx, _dy, _dz];
}
