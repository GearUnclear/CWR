// Remaster config shim for running the modernized Poseidon engine against the
// ORIGINAL full "ARMA Cold War Assault" (OFP 1.99) data install.
//
// The engine's remaster reads (Remaster >> "Cfg...") normally come from the
// Remaster/Demo data package's Bin/remaster.bin. The original 1.99 install has
// no such file, and Landscape::DrawSky / DrawClouds dereference the sky and
// cloud objects unconditionally -> segfault at the main menu without this.
//
// All models below are the original 2001 assets shipped in DTA\Data3D.pbo.
// Slots with no 1.99 equivalent (dev/diagnostic visuals) are left unset;
// ScenePreloader tolerates missing entries (nullptr) at Initialize.
//
// Uslu dur! compatibility configuration (GPL-3.0-or-later).
// Model references resolve against the user's separately supplied Classic data.

class CfgLandscapeSky
{
	class sky      { model = "data3d\obloha.p3d"; };
	class stars    { model = "data3d\stars.p3d"; };
	class horizont { model = "data3d\horizont.p3d"; };
	class sunHalo  { model = "data3d\sunhalo.p3d"; };
	class sun      { model = "data3d\sun.p3d"; };
	class moon     { model = "data3d\moon.p3d"; };
};

class CfgScenePreload
{
	// Clouds are drawn unconditionally every frame (Landscape::DrawClouds) -
	// these four are required.
	class Cloud1 { model = "data3d\mrak1.p3d"; };
	class Cloud2 { model = "data3d\mrak2.p3d"; };
	class Cloud3 { model = "data3d\mrak3.p3d"; };
	class Cloud4 { model = "data3d\mrak4.p3d"; };

	class CraterShell   { model = "data3d\krater.p3d"; };
	class CloudletBasic { model = "data3d\cl_basic.p3d"; };
	class CloudletFire  { model = "data3d\cl_fire.p3d"; };
	class CloudletWater { model = "data3d\cl_water.p3d"; };
	class CobraLight    { model = "data3d\cobrasvetlo.p3d"; };
	class SphereLight   { model = "data3d\koulesvetlo.p3d"; };
	class HalfLight     { model = "data3d\halflight.p3d"; };
	class FootStepL     { model = "data3d\stopa_l.p3d"; };
	class FootStepR     { model = "data3d\stopa_p.p3d"; };
	class SphereModel   { model = "data3d\koule.p3d"; };

	// No 1.99 equivalents found in Data3D.pbo - dev/diag visuals, left null:
	// SlopBlood, CinemaBorder, Marker, ForceArrowModel, RectangleModel,
	// CollisionStar
};
