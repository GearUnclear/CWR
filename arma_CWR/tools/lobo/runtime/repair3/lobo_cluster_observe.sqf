// Shared by lobo_rbk500_bomblets and lobo_mk20_rockeye through #include.
// Callers set lcCenter, lcBomb, lcDeployed and lcBomblet before a drop.
// lcReset clears the observations for one drop. lcObserve is one poll: it
// records each distinct bomblet, the largest number alive at once, the opened
// dispenser and the lowest height the parent bomb was seen at. It returns true
// once bomblets were seen and none has been alive for three polls, or after
// 30 mission seconds. The runner polls about every ten frames, so the bomblet
// count is a lower bound of what the script created, not an exact census.
lcReset = {
    lcSeen = [];
    lcMaxLive = 0;
    lcMaxDeployed = 0;
    lcQuiet = 0;
    lcPolls = 0;
    lcBombSeen = 0;
    lcBombMinZ = 100000;
    lcT0 = time
}
lcObserve = {
    lcLive = 0;
    lcDep = 0;
    {
        lcType = typeOf _x;
        if (lcType == lcBomblet) then {lcLive = lcLive + 1; if (not (_x in lcSeen)) then {lcSeen = lcSeen + [_x]}};
        if (lcType == lcDeployed) then {lcDep = lcDep + 1};
        if (lcType == lcBomb) then {lcBombSeen = lcBombSeen + 1; if (((getPos _x) select 2) < lcBombMinZ) then {lcBombMinZ = (getPos _x) select 2}}
    } forEach (nearestObjects [lcCenter, [], 400]);
    if (lcLive > lcMaxLive) then {lcMaxLive = lcLive};
    if (lcDep > lcMaxDeployed) then {lcMaxDeployed = lcDep};
    if (((count lcSeen) > 0) and (lcLive == 0)) then {lcQuiet = lcQuiet + 1};
    lcPolls = lcPolls + 1;
    (lcQuiet >= 3) or (time > lcT0 + 30)
}
