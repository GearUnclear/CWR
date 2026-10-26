triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
probeStart = time
triSimUntil { time > probeStart + 2 }
player addMagazine "LAWLauncher"
player addWeapon "LAWLauncher"
player selectWeapon "LAWLauncher"
probeStart = time
triSimUntil { time > probeStart + 3 }
getMove player
triLeanPose player
player selectWeapon "M16"
probeStart = time
triSimUntil { time > probeStart + 4 }
getMove player
triLeanPose player
triKeyDown 8
probeStart = time
triSimUntil { time > probeStart + 1 }
triLeanPose player
getPos player
getMove player
triEndTest
