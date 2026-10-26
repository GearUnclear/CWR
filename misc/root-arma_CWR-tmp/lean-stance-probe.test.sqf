triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
probeStart = time
triSimUntil { time > probeStart + 3 }
triLeanPose player
getMove player
triKeyDown 75
triSimFrames 10
triKeyUp 75
probeStart = time
triSimUntil { time > probeStart + 3 }
triLeanPose player
getMove player
triKeyDown 78
triSimFrames 10
triKeyUp 78
probeStart = time
triSimUntil { time > probeStart + 3 }
triLeanPose player
getMove player
triKeyDown 75
triSimFrames 10
triKeyUp 75
probeStart = time
triSimUntil { time > probeStart + 3 }
triLeanPose player
getMove player
triEndTest
