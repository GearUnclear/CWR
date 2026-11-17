#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Graphics/Rendering/Effects/ParticleDrop.hpp>
#include <Poseidon/Graphics/Rendering/Effects/Smokes.hpp>

namespace Poseidon
{
bool GetVector3(Vector3& ret, GameValuePar oper);
GameValue ParticleDrop(const GameState* state, GameValuePar oper1);
} // namespace Poseidon

namespace
{
GameValue Scalars(std::initializer_list<float> values)
{
    GameArrayType array;
    for (float value : values)
    {
        array.Add(GameValue(value));
    }
    return GameValue(array);
}

GameValue Colors(std::initializer_list<std::initializer_list<float>> rgbas)
{
    GameArrayType array;
    for (const auto& rgba : rgbas)
    {
        array.Add(Scalars(rgba));
    }
    return GameValue(array);
}

GameValue Text(const char* text)
{
    return GameValue(RString(text));
}

// A sloped test landscape: the surface rises 1 m per 100 m east, plus 30 m.
float SlopedSurface(float x, float z)
{
    (void)z;
    return 30.0f + x * 0.01f;
}

// The first `drop` of LoBo's UH-1 rotor wash (LoboIDFhelo.pbo,
// UH1\Engine\rotorwash.sqs, line 133, the "#dusty" branch), with the values
// its variables hold on the first pass over Sinai terrain ("standard" colours,
// _dustsize 0). The same shape is dropped by LoBoEghelo Engine/Engine2/Engine3
// rotorwash.sqs and CH53\MPscripts\MProtorwash.sqs. Element 5 is
// [getpos _unit select 0, getpos _unit select 1] (two elements) and element 18
// is the String "_unit", not the object.
GameArrayType Uh1RotorWashDrop()
{
    GameArrayType args;
    args.Add(Text("cl_basic"));                    // _ShapeName
    args.Add(Text(""));                            // _AnimationName
    args.Add(Text("Billboard"));                   // _Type
    args.Add(GameValue(1.0f));                     // _TimerPeriod
    args.Add(GameValue(1.5f));                     // _LifeTime
    args.Add(Scalars({8068.0f, 3725.0f}));         // _Position
    args.Add(Scalars({7.07f, 7.07f, 0.9f}));       // _MoveVelocity
    args.Add(GameValue(0.5f));                     // _RotationVelocity
    args.Add(GameValue(0.01f));                    // _Weight
    args.Add(GameValue(0.005f));                   // _Volume
    args.Add(GameValue(0.05f));                    // _Rubbing
    args.Add(Scalars({0.01f, 0.02f, 1.0f, 0.4f})); // _Size
    args.Add(Colors({{0.3f, 0.3f, 0.168f, 0.08f},
                     {0.34f, 0.3f, 0.168f, 0.1f},
                     {0.345f, 0.3f, 0.168f, 0.2f},
                     {0.345f, 0.3f, 0.168f, 0.04f}})); // _Color (_standard)
    args.Add(Scalars({0.0f}));                         // _AnimationPhase
    args.Add(GameValue(1.2f));                         // _RandomDirectionPeriod
    args.Add(GameValue(4.0f));                         // _RandomDirectionIntensity
    args.Add(Text(""));                                // _OnTimer
    args.Add(Text(""));                                // _BeforeDestroy
    args.Add(Text("_unit"));                           // _Object
    return args;
}
} // namespace

TEST_CASE("smokes.hpp compiles", "[rendering][effects]")
{
    SUCCEED("header included successfully");
}

TEST_CASE("drop velocity and object-relative arrays take exactly x, y, z", "[rendering][effects][drop]")
{
    GameArrayType array;
    array.Add(GameValue(100.0f));
    array.Add(GameValue(200.0f));
    array.Add(GameValue(0.0f));
    GameValue value(array);

    Vector3 position;

    REQUIRE(Poseidon::GetVector3(position, value));
    CHECK(position[0] == 100.0f);
    CHECK(position[1] == 0.0f);
    CHECK(position[2] == 200.0f);

    Vector3 untouched;
    CHECK_FALSE(Poseidon::GetVector3(untouched, Scalars({100.0f, 200.0f})));
}

TEST_CASE("drop accepts legacy no-object argument count", "[rendering][effects][drop]")
{
    CHECK(Poseidon::IsParticleDropArgumentCountSupported(18));
    CHECK(Poseidon::IsParticleDropArgumentCountSupported(19));
    CHECK_FALSE(Poseidon::IsParticleDropArgumentCountSupported(17));
    CHECK_FALSE(Poseidon::IsParticleDropArgumentCountSupported(20));
}

TEST_CASE("drop world positions read like getPos: x, y and a height above the surface", "[rendering][effects][drop]")
{
    Vector3 position;

    // [x, y]: on the surface (OFP 1.99 ::GetPos, the form LoBo's rotor wash uses).
    REQUIRE(Poseidon::GetParticleDropWorldPosition(position, Scalars({8068.0f, 3725.0f}), &SlopedSurface));
    CHECK(position[0] == 8068.0f);
    CHECK(position[1] == SlopedSurface(8068.0f, 3725.0f));
    CHECK(position[2] == 3725.0f);

    // [x, y, z]: z above the surface, not above sea level.
    REQUIRE(Poseidon::GetParticleDropWorldPosition(position, Scalars({1000.0f, 2000.0f, 5.0f}), &SlopedSurface));
    CHECK(position[0] == 1000.0f);
    CHECK(position[1] == 5.0f + SlopedSurface(1000.0f, 2000.0f));
    CHECK(position[2] == 2000.0f);

    // Below the surface stays legal (the rotor wash's hover ripples drop at -1).
    REQUIRE(Poseidon::GetParticleDropWorldPosition(position, Scalars({0.0f, 0.0f, -1.0f}), &SlopedSurface));
    CHECK(position[1] == -1.0f + SlopedSurface(0.0f, 0.0f));

    // Neither one element nor four, nor a non-number.
    Vector3 untouched;
    CHECK_FALSE(Poseidon::GetParticleDropWorldPosition(untouched, Scalars({8068.0f}), &SlopedSurface));
    CHECK_FALSE(Poseidon::GetParticleDropWorldPosition(untouched, Scalars({1.0f, 2.0f, 3.0f, 4.0f}), &SlopedSurface));
    GameArrayType mixed;
    mixed.Add(GameValue(1.0f));
    mixed.Add(Text("2"));
    CHECK_FALSE(Poseidon::GetParticleDropWorldPosition(untouched, GameValue(mixed), &SlopedSurface));
}

TEST_CASE("drop takes LoBo's UH-1 rotor wash array: a two-element position and a String object",
          "[rendering][effects][drop]")
{
    const GameArrayType args = Uh1RotorWashDrop();
    REQUIRE(args.Size() == 19);

    Poseidon::ParticleDropRequest request;
    REQUIRE(Poseidon::ParseParticleDrop(nullptr, args, request, &SlopedSurface) == Poseidon::ParticleDropParse::Ready);
    CHECK(request.type == Poseidon::PT_Billboard);
    // The String "_unit" is not an object: the position is a world position on the ground.
    CHECK(request.position[0] == 8068.0f);
    CHECK(request.position[1] == SlopedSurface(8068.0f, 3725.0f));
    CHECK(request.position[2] == 3725.0f);
    // Script [x, y, z] velocity to engine (x, z up, y).
    CHECK(request.moveVelocity[0] == 7.07f);
    CHECK(request.moveVelocity[1] == 0.9f);
    CHECK(request.moveVelocity[2] == 7.07f);
    CHECK(request.size.Size() == 4);
    CHECK(request.color.Size() == 4);
    CHECK(request.animationPhase.Size() == 1);

    // The later revision of the same script (uh60\scripts\rotorwash.sqs):
    // [x, y, 0] and "" for the object, which lands on the same spot.
    GameArrayType revised = Uh1RotorWashDrop();
    revised[5] = Scalars({8068.0f, 3725.0f, 0.0f});
    revised[18] = Text("");
    Poseidon::ParticleDropRequest revisedRequest;
    REQUIRE(Poseidon::ParseParticleDrop(nullptr, revised, revisedRequest, &SlopedSurface) ==
            Poseidon::ParticleDropParse::Ready);
    CHECK(revisedRequest.position[1] == request.position[1]);

    // 18 elements (no object) read the position the same way.
    GameArrayType noObject = Uh1RotorWashDrop();
    noObject.Delete(18);
    REQUIRE(noObject.Size() == 18);
    Poseidon::ParticleDropRequest noObjectRequest;
    REQUIRE(Poseidon::ParseParticleDrop(nullptr, noObject, noObjectRequest, &SlopedSurface) ==
            Poseidon::ParticleDropParse::Ready);
    CHECK(noObjectRequest.position[1] == request.position[1]);
}

TEST_CASE("drop still rejects arrays outside the 1.99 contract", "[rendering][effects][drop]")
{
    Poseidon::ParticleDropRequest request;

    GameArrayType onePosition = Uh1RotorWashDrop();
    onePosition[5] = Scalars({8068.0f});
    CHECK(Poseidon::ParseParticleDrop(nullptr, onePosition, request, &SlopedSurface) ==
          Poseidon::ParticleDropParse::Rejected);

    GameArrayType twoVelocity = Uh1RotorWashDrop();
    twoVelocity[6] = Scalars({7.07f, 7.07f});
    CHECK(Poseidon::ParseParticleDrop(nullptr, twoVelocity, request, &SlopedSurface) ==
          Poseidon::ParticleDropParse::Rejected);

    GameArrayType rgbColor = Uh1RotorWashDrop();
    rgbColor[12] = Colors({{0.3f, 0.3f, 0.168f}});
    CHECK(Poseidon::ParseParticleDrop(nullptr, rgbColor, request, &SlopedSurface) ==
          Poseidon::ParticleDropParse::Rejected);

    GameArrayType lowerCaseType = Uh1RotorWashDrop();
    lowerCaseType[2] = Text("billboard");
    CHECK(Poseidon::ParseParticleDrop(nullptr, lowerCaseType, request, &SlopedSurface) ==
          Poseidon::ParticleDropParse::Rejected);

    GameArrayType tooShort = Uh1RotorWashDrop();
    tooShort.Delete(18);
    tooShort.Delete(17);
    CHECK(Poseidon::ParseParticleDrop(nullptr, tooShort, request, &SlopedSurface) ==
          Poseidon::ParticleDropParse::Rejected);
}

TEST_CASE("drop reports a rejected position to the script", "[rendering][effects][drop]")
{
    // The command itself: it fails on the position before any shape or
    // landscape is touched, and says so through the script state.
    GameArrayType onePosition = Uh1RotorWashDrop();
    onePosition[5] = Scalars({8068.0f});
    GGameState.SetError(EvalOK);
    Poseidon::ParticleDrop(&GGameState, GameValue(onePosition));
    CHECK(GGameState.GetLastError() == EvalGen);
    GGameState.SetError(EvalOK);
}
