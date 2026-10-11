triSimUntil { alive player }
logInfo "LOBO_ACCEPTANCE baseline mission ready, installed @LoBo only"
triAssert [triAddonActive "LoBoWeapons"]
triAssert [triAddonActive "LoBoM60"]
laTank = "LoBo_M60A1_jor" createVehicle [7525,5790,0]
triAssert [not isNull laTank]
triAssert ["LoBo_105Sabot2" in magazines laTank]
logInfo format ["LOBO_ACCEPTANCE Jordanian M60A1 loadout: %1 %2", weapons laTank, magazines laTank]
laAmmo = "LoBo_105Sabot2" camCreate [7535,5790,10]
triAssert [not isNull laAmmo]
laAmmo setVelocity [0,0,-300]
triSimFrames 60
logInfo "LOBO_ACCEPTANCE M833 projectile created and simulated"
deleteVehicle laTank
triEndTest
