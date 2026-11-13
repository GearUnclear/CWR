// DedushkaDemo camera spots, shared by kjd_cam.sqs (the mission's cameras) and
// the tests (the same spots through triSetView).
// [name, [x, y], eye height above the ground or the sea there, aim height above his feet, lens]
// (lens: the camera's field of view; 0.7 is the player's, 0.1 a long lens)
KJD_cams = [
    ["far", [4880, 2160], 1.7, 22, 0.06],          // 1.85 km out on the bay, long lens: past the object distance, inside the view distance. The furnace glows here by night too
    ["farther", [4900, 1700], 1.7, 22, 0.06],      // 2.3 km out, past the view distance: the far pass proper (by night its furnace is dark: engine request 10)
    ["bay", [4990, 3250], 1.7, 20, 0.5],           // 450 to 750 m out on the bay: the town's skyline and him over it
    ["west", [4690, 3990], 1.7, 20, 0.7],          // behind him on the road in, 100 m west of where he stands dormant
    ["street", [4985, 3984], 1.7, 22, 0.7],        // the west end of the street: he comes straight at it
    ["square", [5050, 3975], 1.7, 26, 0.95],       // among the garrison, 55 m from where he stops: looking up
    ["armour", [5128, 3958], 2.6, 24, 0.7],        // behind the tanks, down the line of their fire
    ["north", [5075, 4125], 2, 22, 0.8],           // the open ground north of the row: side on to the beam
    ["south", [5010, 3895], 1.7, 24, 0.8],         // the road south of the row, on his way to the sea
    ["high", [5080, 3900], 70, 14, 0.8],           // 70 m up, south-east of the row: the town, the cut and the fires
    ["shore", [5095, 3835], 28, 14, 0.8],          // 28 m over the shelf's edge, east of his line into the water (the bank hides him from eye height)
    ["water", [4990, 3600], 1.7, 16, 0.7]          // on the water, south of where he wades to: he comes toward it
];

// spot name call KJD_fnSpot : the spot's entry
KJD_fnSpot = {
    private ["_s"];
    _s = [];
    { if ((_x select 0) == _this) then { _s = _x } } forEach KJD_cams;
    _s
};

// spot name call KJD_fnLens : its lens
KJD_fnLens = { (_this call KJD_fnSpot) select 4 };

// [spot name, kaiju] call KJD_fnView : the spot as the array triSetView takes
// ([x, height above sea level, y, direction x, direction up, direction y]),
// aimed at him. Needs KJD_probe (a HeliHEmpty) for the ground or sea height.
KJD_fnView = {
    private ["_s", "_p", "_g", "_e", "_xy"];
    _s = (_this select 0) call KJD_fnSpot;
    _xy = _s select 1;
    KJD_probe setPos [_xy select 0, _xy select 1, 0];
    _g = (getPosASL KJD_probe) select 2;
    _e = _g + (_s select 2);
    _p = getPosASL (_this select 1);
    [_xy select 0, _e, _xy select 1, (_p select 0) - (_xy select 0), (_p select 2) + (_s select 3) - _e, (_p select 1) - (_xy select 1)]
};
