#include <catch2/catch_test_macros.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QStream.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp>
#include <Poseidon/World/Entities/Infantry/LeanState.hpp>
#include <catch2/catch_approx.hpp>
#include <limits>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp>

#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace Poseidon;

TEST_CASE("Lean hold/release and side switching are independent of frame rate", "[Infantry][lean]")
{
    for (int fps : {30, 60, 144})
    {
        LeanState lean;
        for (int i = 0; i < fps; ++i)
            lean.Advance(1, 0, true, 1.0f / fps);
        CHECK(lean.amount == Catch::Approx(-1));
        for (int i = 0; i < fps; ++i)
            lean.Advance(0, 1, true, 1.0f / fps);
        CHECK(lean.amount == Catch::Approx(1));
        for (int i = 0; i < fps; ++i)
            lean.Advance(1, 1, true, 1.0f / fps);
        CHECK(lean.amount == 0);
        lean.Advance(1, 0, true, 1);
        lean.Advance(0, 0, true, 1);
        CHECK(lean.amount == 0);
    }
}

TEST_CASE("Lean interruptions require release and invalid time cannot poison the pose", "[Infantry][lean]")
{
    LeanState lean;
    CHECK(lean.Advance(0, 1, true, 1) == 1);
    CHECK(lean.Advance(0, 1, false, 0.1f) == 0);
    CHECK(lean.Advance(0, 1, true, 1) == 0);
    lean.Advance(0, 0, false, 0);
    CHECK(lean.Advance(0, 1, true, 0.1f) == Catch::Approx(0.5f));
    CHECK(lean.Advance(0, 1, true, 0) == Catch::Approx(0.5f));
    CHECK(lean.Advance(0, 1, true, std::numeric_limits<float>::quiet_NaN()) == 0);
}

TEST_CASE("Lean analog dead zone agrees with held input and interruption recovery", "[Infantry][lean]")
{
    LeanState lean;
    CHECK(lean.Advance(0, 0.1f, true, 1) == 0);
    CHECK(lean.Advance(0.05f, 0, true, 1) == 0);
    CHECK(lean.Advance(0.05f, 0.5f, true, 1) == Catch::Approx(0.5f));
    CHECK(lean.Advance(0, 0.5f, false, 1) == 0);
    CHECK(lean.Advance(0, 0.5f, true, 1) == 0);
    CHECK(lean.Advance(0, 0.05f, true, 1) == 0);
    CHECK_FALSE(lean.requireRelease);
    CHECK(lean.Advance(0, 0.5f, true, 1) == Catch::Approx(0.5f));
}

namespace
{
float Degrees(float radians)
{
    return radians * 57.2957795f;
}

Matrix3 LevelFrame()
{
    Matrix3 frame;
    frame.SetDirectionAndUp(VForward, VUp);
    return frame;
}
} // namespace

TEST_CASE("Lean roll tilts the upper body 35 degrees toward the leaning side", "[Infantry][lean]")
{
    CHECK(Degrees(LeanState::MaxAngle) == Catch::Approx(35.0f));
    const Vector3 pivot(0.03f, 0.89f, -0.02f);
    const Vector3 head = pivot + Vector3(0, 0.5f, 0);

    const Matrix4 right = LeanRollMatrix(pivot, LevelFrame(), LeanState::MaxAngle);
    const Vector3 headRight = right.FastTransform(head) - pivot;
    // Model +X is the soldier's right: the head tips right and drops.
    CHECK(headRight.X() == Catch::Approx(0.5f * std::sin(LeanState::MaxAngle)).margin(1e-5));
    CHECK(headRight.Y() == Catch::Approx(0.5f * std::cos(LeanState::MaxAngle)).margin(1e-5));
    CHECK(headRight.Z() == Catch::Approx(0).margin(1e-6));
    CHECK(Degrees(std::acos(right.DirectionUp() * VUp)) == Catch::Approx(35.0f).margin(0.01));

    const Vector3 headLeft = LeanRollMatrix(pivot, LevelFrame(), -LeanState::MaxAngle).FastTransform(head) - pivot;
    CHECK(headLeft.X() == Catch::Approx(-headRight.X()).margin(1e-5));
    CHECK(headLeft.Y() == Catch::Approx(headRight.Y()).margin(1e-5));

    // The pivot itself and the legs below a zero-weight boundary do not move.
    CHECK(right.FastTransform(pivot).Distance(pivot) == Catch::Approx(0).margin(1e-6));
    CHECK(LeanRollMatrix(pivot, LevelFrame(), 0).FastTransform(head).Distance(head) == Catch::Approx(0).margin(1e-6));
}

TEST_CASE("Partial lean factors rotate rigidly instead of shearing or shrinking", "[Infantry][lean]")
{
    const Vector3 pivot(0.03f, 0.89f, -0.02f);
    const Vector3 points[] = {Vector3(0.12f, 1.07f, 0.05f), Vector3(0.14f, 1.22f, 0.03f), Vector3(0.33f, 1.33f, 0.17f),
                              Vector3(0.40f, 1.18f, 0.95f)};
    for (float factor : {0.2f, 0.5f, 0.9f, 1.0f})
    {
        const float angle = factor * LeanState::MaxAngle;
        const Matrix4 roll = LeanRollMatrix(pivot, LevelFrame(), angle);
        // An orthonormal rotation: no axis is scaled or skewed.
        CHECK(roll.DirectionAside().Size() == Catch::Approx(1).margin(1e-5));
        CHECK(roll.DirectionUp().Size() == Catch::Approx(1).margin(1e-5));
        CHECK(roll.Direction().Size() == Catch::Approx(1).margin(1e-5));
        CHECK(roll.DirectionAside() * roll.DirectionUp() == Catch::Approx(0).margin(1e-5));
        for (const Vector3& point : points)
        {
            // Every point keeps its distance to the pivot and turns by the
            // selection's own share of the roll.
            const Vector3 before = point - pivot;
            const Vector3 after = roll.FastTransform(point) - pivot;
            CHECK(after.Size() == Catch::Approx(before.Size()).margin(1e-5));
            CHECK(after.Z() == Catch::Approx(before.Z()).margin(1e-5));
            const float turned = std::atan2(before.X(), before.Y()) - std::atan2(after.X(), after.Y());
            CHECK(Degrees(-turned) == Catch::Approx(Degrees(angle)).margin(0.01));
        }
    }

    // What the first lean did instead: blending a full-angle matrix toward
    // identity by the factor shortens partially weighted parts.
    const Matrix4 full = LeanRollMatrix(pivot, LevelFrame(), LeanState::MaxAngle);
    const Matrix4 blended = full * 0.5f + MIdentity * 0.5f;
    CHECK(blended.DirectionUp().Size() < 0.96f);
}

TEST_CASE("Lean roll about the bore keeps the bore's direction", "[Infantry][lean]")
{
    // Aiming 25 degrees up and 10 degrees left: the roll axis is the bore.
    const Vector3 bore = (Matrix3(MRotationY, 0.17f) * (Matrix3(MRotationX, -0.44f) * VForward)).Normalized();
    Matrix3 frame;
    frame.SetDirectionAndUp(bore, VUp);
    const Vector3 pivot(0.03f, 0.89f, -0.02f);
    const Matrix4 roll = LeanRollMatrix(pivot, frame, LeanState::MaxAngle);
    CHECK(roll.Rotate(bore).Distance(bore) == Catch::Approx(0).margin(1e-5));
    // Points on the bore line through the pivot stay on it.
    const Vector3 onAxis = pivot + bore * 0.8f;
    CHECK(roll.FastTransform(onAxis).Distance(onAxis) == Catch::Approx(0).margin(1e-5));
    // The weapon's up vector still rolls the full 35 degrees about the bore.
    const Vector3 up = frame.DirectionUp();
    CHECK(Degrees(std::acos(roll.Rotate(up) * up)) == Catch::Approx(35.0f).margin(0.01));
}

namespace
{
// The stock rifleman's combat stance, measured in game (model space, metres):
// a bladed torso hunched toward his right, the eye out over the stock.
LeanRig CombatStanceRig()
{
    LeanRig rig;
    rig.frame = LevelFrame();
    rig.pivot = Vector3(0.026f, 0.891f, -0.017f); // belly centroid
    rig.neck = Vector3(0.224f, 1.258f, 0.100f);   // neck centroid
    rig.ankle[0] = Vector3(-0.001f, 0.018f, 0.295f);
    rig.ankle[1] = Vector3(0.246f, 0.030f, -0.340f);
    rig.sway = true;
    rig.lowerMax = 0.9f;
    return rig;
}

const Vector3 CombatEye(0.320f, 1.339f, 0.161f);

bool IsRigid(const Matrix4& m)
{
    return std::fabs(m.DirectionAside().Size() - 1) < 1e-4f && std::fabs(m.DirectionUp().Size() - 1) < 1e-4f &&
           std::fabs(m.Direction().Size() - 1) < 1e-4f && std::fabs(m.DirectionAside() * m.DirectionUp()) < 1e-4f &&
           std::fabs(m.DirectionUp() * m.Direction()) < 1e-4f;
}
} // namespace

TEST_CASE("Lean rig rolls the head 35 degrees and keeps the bore", "[Infantry][lean]")
{
    const LeanRig rig = CombatStanceRig();
    for (float amount : {-1.0f, -0.5f, 0.5f, 1.0f})
    {
        const Matrix4 head = rig.Spine(amount, 1);
        CHECK(Degrees(std::acos(head.DirectionUp() * VUp)) == Catch::Approx(35.0f * std::fabs(amount)).margin(0.01));
        // Right lean (positive) tips the head toward +X, the soldier's right.
        CHECK((head.DirectionUp().X() > 0) == (amount > 0));
        for (float factor : {0.2f, 0.5f, 0.9f, 1.0f})
        {
            const Matrix4 bone = rig.Spine(amount, factor);
            CHECK(IsRigid(bone));
            CHECK(bone.Rotate(VForward).Distance(VForward) == Catch::Approx(0).margin(1e-5));
        }
        for (int side : {0, 1})
        {
            CHECK(IsRigid(rig.Leg(amount, side)));
            CHECK(rig.Leg(amount, side).Rotate(VForward).Distance(VForward) == Catch::Approx(0).margin(1e-5));
        }
    }
    CHECK(rig.Spine(0, 1).Distance2(MIdentity) == Catch::Approx(0).margin(1e-10));
}

TEST_CASE("Lean rig moves every part rigidly and keeps the joints together", "[Infantry][lean]")
{
    const LeanRig rig = CombatStanceRig();
    for (float amount : {-1.0f, 1.0f})
    {
        const Vector3 shift = rig.Shift(amount);
        // Hips sway toward the lean, about 0.12 m, staying nearly level.
        CHECK(shift.X() * amount > 0.1f);
        CHECK(std::fabs(shift.Y()) < 0.04f);
        // The lower spine turns about the belly, which only rides the sway.
        for (float factor : {0.2f, 0.5f, 0.9f})
            CHECK(rig.Spine(amount, factor).FastTransform(rig.pivot).Distance(rig.pivot + shift) ==
                  Catch::Approx(0).margin(0.002));
        // Chest and head meet at the neck.
        CHECK(rig.Spine(amount, 0.9f).FastTransform(rig.neck).Distance(rig.Spine(amount, 1).FastTransform(rig.neck)) ==
              Catch::Approx(0).margin(1e-4));
        // Each leg turns about its planted ankle, and its hip end follows the
        // pelvis to within a couple of centimetres.
        for (int side : {0, 1})
        {
            const Matrix4 leg = rig.Leg(amount, side);
            CHECK(leg.FastTransform(rig.ankle[side]).Distance(rig.ankle[side]) == Catch::Approx(0).margin(1e-5));
            CHECK(leg.FastTransform(rig.pivot).Distance(rig.pivot + shift) < 0.03f);
        }
    }
}

TEST_CASE("Lean rig moves the eye about the same distance either way", "[Infantry][lean]")
{
    const LeanRig rig = CombatStanceRig();
    const Vector3 left = rig.Spine(-1, 1).FastTransform(CombatEye) - CombatEye;
    const Vector3 right = rig.Spine(1, 1).FastTransform(CombatEye) - CombatEye;
    CHECK(left.X() < -0.2f);
    CHECK(right.X() > 0.2f);
    CHECK(std::min(-left.X(), right.X()) / std::max(-left.X(), right.X()) > 0.7f);
    CHECK(left.Y() > -0.15f);
    CHECK(right.Y() > -0.15f);

    // Turning the whole hunched torso 35 degrees about the belly instead folds
    // it into a bow on the right and stands it upright on the left.
    const Vector3 bowRight =
        LeanRollMatrix(rig.pivot, rig.frame, LeanState::MaxAngle).FastTransform(CombatEye) - CombatEye;
    const Vector3 bowLeft =
        LeanRollMatrix(rig.pivot, rig.frame, -LeanState::MaxAngle).FastTransform(CombatEye) - CombatEye;
    CHECK(bowRight.Y() < -0.2f);
    CHECK(bowLeft.Y() > 0.05f);
}

namespace
{
ParamFile ParseConfig(const char* text)
{
    ParamFile pf;
    QIStream in(text, static_cast<int>(strlen(text)));
    pf.Parse(in);
    return pf;
}

WeaponType MakeWeapon(const ParamEntry& entry, int weaponType)
{
    WeaponType weapon;
    weapon._parClass = &entry;
    weapon._weaponType = weaponType;
    return weapon;
}

Ref<WeaponType> MakeWeaponRef(const ParamEntry& entry, int weaponType)
{
    Ref<WeaponType> weapon = new WeaponType();
    weapon->_parClass = &entry;
    weapon->_weaponType = weaponType;
    return weapon;
}
} // namespace

TEST_CASE("soldierOld.hpp compiles", "[Infantry][soldierOld]")
{
    SUCCEED("header included successfully");
}

TEST_CASE("WeaponType::IsBinocular centralizes canonical binocular detection", "[Infantry][soldierOld][binocular]")
{
    ParamFile pf = ParseConfig("class Binocular {};\n"
                               "class CustomBinocular {};\n"
                               "class NVGoggles {};\n");

    const WeaponType binocular = MakeWeapon(pf >> "Binocular", MaskSlotBinocular);
    REQUIRE(binocular.IsBinocular());

    const WeaponType customBinocular = MakeWeapon(pf >> "CustomBinocular", MaskSlotBinocular);
    REQUIRE_FALSE(customBinocular.IsBinocular());

    const WeaponType nvgInBinocularSlot = MakeWeapon(pf >> "NVGoggles", MaskSlotBinocular);
    REQUIRE_FALSE(nvgInBinocularSlot.IsBinocular());

    const WeaponType binocularWithoutSlot = MakeWeapon(pf >> "Binocular", MaskSlotPrimary);
    REQUIRE_FALSE(binocularWithoutSlot.IsBinocular());
}

TEST_CASE("FindBinocularWeapon prefers the Binocular over other slot items", "[Infantry][soldierOld][binocular]")
{
    ParamFile pf = ParseConfig("class Binocular {};\n"
                               "class NVGoggles {};\n"
                               "class Phone {};\n"
                               "class M16 {};\n");

    Ref<WeaponType> nvg = MakeWeaponRef(pf >> "NVGoggles", MaskSlotBinocular);
    Ref<WeaponType> phone = MakeWeaponRef(pf >> "Phone", MaskSlotBinocular);
    Ref<WeaponType> binocular = MakeWeaponRef(pf >> "Binocular", MaskSlotBinocular);
    Ref<WeaponType> rifle = MakeWeaponRef(pf >> "M16", MaskSlotPrimary);

    RefArray<WeaponType> weapons;
    weapons.Add(nvg);
    weapons.Add(phone);
    weapons.Add(binocular);

    REQUIRE(FindBinocularWeapon(weapons) == binocular);

    RefArray<WeaponType> phoneOnly;
    phoneOnly.Add(rifle);
    phoneOnly.Add(phone);

    REQUIRE(FindBinocularWeapon(phoneOnly) == phone);

    RefArray<WeaponType> noSlotItems;
    noSlotItems.Add(rifle);

    REQUIRE(FindBinocularWeapon(noSlotItems) == nullptr);
}

TEST_CASE("stealth stance exposure tolerates soldiers without a group", "[Infantry][soldierOld]")
{
    REQUIRE(Poseidon::SoldierStealthStanceExposure(nullptr, false, 0, 0) == 0.0f);
    REQUIRE(Poseidon::SoldierStealthStanceExposure(nullptr, true, 0, 0) == 0.0f);
}

TEST_CASE("Man flare lookup validates the selected weapon slot", "[Infantry][soldierOld]")
{
    const std::filesystem::path source =
        std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine/Poseidon/World/Entities/Infantry/SoldierOld.cpp";
    std::ifstream input(source);
    REQUIRE(input.is_open());

    std::stringstream stream;
    stream << input.rdbuf();
    const std::string body = stream.str();
    const std::size_t begin = body.find("bool Man::HasFlares");
    REQUIRE(begin != std::string::npos);
    const std::size_t end = body.find("Matrix4 Man::InsideCamera", begin);
    REQUIRE(end != std::string::npos);

    const std::string function = body.substr(begin, end - begin);
    CHECK(function.find("_currentWeapon >= 0") != std::string::npos);
    CHECK(function.find("_currentWeapon < NMagazineSlots()") != std::string::npos);
    CHECK(function.find("if (slot._muzzle)") != std::string::npos);
}

TEST_CASE("MotionType rejects an out-of-range path destination", "[Infantry][soldierOld][motion]")
{
    ParamFile pf = ParseConfig("class EmptyActions {};\n"
                               "class TestMoves {\n"
                               "  class States {\n"
                               "    class Idle {};\n"
                               "    class Walk {};\n"
                               "  };\n"
                               "  class Interpolations {};\n"
                               "  transitionsInterpolated[] = {};\n"
                               "  transitionsSimple[] = {};\n"
                               "  transitionsDisabled[] = {};\n"
                               "  vehicleActions = EmptyActions;\n"
                               "};\n");
    MotionType motion;
    motion.Load(pf >> "TestMoves");
    MotionPath path;

    REQUIRE_FALSE(motion.FindPath(path, motion.GetMoveId("Idle"), MotionPathItem(static_cast<MoveId>(32758))));
    REQUIRE(path.Size() == 0);
}

TEST_CASE("MotionType ignores an out-of-range animation edge", "[Infantry][soldierOld][motion]")
{
    ParamFile pf = ParseConfig("class EmptyActions {};\n"
                               "class TestMoves {\n"
                               "  class States {\n"
                               "    class Idle {};\n"
                               "    class Walk {};\n"
                               "  };\n"
                               "  class Interpolations {};\n"
                               "  transitionsInterpolated[] = {};\n"
                               "  transitionsSimple[] = {};\n"
                               "  transitionsDisabled[] = {};\n"
                               "  vehicleActions = EmptyActions;\n"
                               "};\n");
    MotionType motion;
    motion.Load(pf >> "TestMoves");
    MoveId idle = motion.GetMoveId("Idle");
    MoveId walk = motion.GetMoveId("Walk");
    motion.AddEdge(idle, static_cast<MoveId>(32758), MEdgeSimple, 1.0f);
    MotionPath path;

    REQUIRE_FALSE(motion.FindPath(path, idle, MotionPathItem(walk)));
    REQUIRE(path.Size() == 0);
}
