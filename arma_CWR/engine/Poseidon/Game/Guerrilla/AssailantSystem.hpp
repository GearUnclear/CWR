#pragma once

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

namespace Poseidon
{
class Person;
class EntityAI;
class AIUnit;
class AIGroup;
class AICenter;
namespace Guerrilla
{
enum AssailantMode { ASNone, ASResister, ASRogue };

// Individual hostility never changes a Target's perceived side or the
// center's exposure category. Records own identity, not perception/memory.
class AssailantSystem : public SerializeClass
{
public:
    static AssailantSystem& Instance();
    static constexpr int MaxLive = 8, MaxRogues = 2;
    static int SpareSide(int occupier, int resistance);
    static AssailantMode Mode(const char* name);
    static const char* Name(AssailantMode mode);
    static bool HasCapacity(int live, int rogues, AssailantMode mode);
    static bool Hostile(AssailantMode mode, bool personal, bool combatant, bool rogue);
    static float Interval(float roll);
    static float Advance(float remaining, float dt, bool eligible);
    void Clear();
    void InitMission();
    void Configure(); // final campaign sides; also called after the second load pass
    bool IsActive() const { return _side >= 0; }
    RString SideName() const;
    RString Diagnostic(AssailantMode mode) const;
    int Count(AssailantMode mode = ASNone) const;
    AssailantMode Classify(const EntityAI* body) const;
    bool IsGroup(const AIGroup* group) const;
    bool Eligible(Person* body, AssailantMode mode) const;
    bool Register(Person* body, AssailantMode mode, EntityAI* extorter, RString zone);
    bool Remove(EntityAI* body);
    void Simulate(float dt);
    bool IsHostile(const AIUnit* observer, const EntityAI* target, int perceivedSide, const AICenter* center) const;
    LSError Serialize(ParamArchive& ar) override;
private:
    struct Record
    {
        LLink<Person> body;
        LLink<EntityAI> extorter;
        LLink<AIGroup> group;
        RString zone;
        int mode = ASNone;
        LSError Serialize(ParamArchive& ar);
    };
    bool ResolveEquipment();
    void Prune();
    AutoArray<Record> _records;
    int _side = -1;
    bool _ready = false;
    float _accum = 0;
    RString _weapon, _magazine;
};

// Shared observer-to-target resolver for classification, selection and fire.
bool ObserverHostile(const AIUnit* observer, const EntityAI* target, int perceivedSide, const AICenter* center);
bool IndependentGroup(const AIGroup* group);
}
}
