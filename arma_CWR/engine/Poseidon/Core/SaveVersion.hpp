
// load / save game

namespace Poseidon
{
// 14: per-object script variable banks (setVariable/getVariable)
const int WorldSerializeVersion = 14;

// Guerrilla Mode campaign state (ZoneRegistry/GarrisonCache/AlertMachine
// subclasses of the world archive).  Bump when their row formats change;
// the loaded value is handed to the campaignLoaded script event so mission
// scripts can migrate.
const int GuerrillaSaveVersion = 1;

// load / save mission
const int MissionsVersion = 11;

// UserInfo.cfg file
const int UserInfoVersion = 2;

// Campaign history
const int CampaignVersion = 3;

#ifndef SERIAL_BRANCH
const int SerializeBranch = 0xffff0000;
#define SERIAL_BRANCH(ver) ((ver) & SerializeBranch)
#define SERIAL_VERSION(ver) ((ver) & ~SerializeBranch)
#endif

// load / save unit status
#ifndef IS_UNIT_STATUS_BRANCH
const int UnitStatusBase = 0x00010000;
#define IS_UNIT_STATUS_BRANCH(ver) (SERIAL_BRANCH(ver) == UnitStatusBase)
#endif
#define MAKE_UNIT_STATUS(ver) ((ver) | UnitStatusBase)
const int UnitStatusVersion = MAKE_UNIT_STATUS(1);
} // namespace Poseidon
