#ifndef _RANDOMGEN_HPP
#define _RANDOMGEN_HPP

#include <Poseidon/Foundation/platform.hpp>

class RandomTable
{
  public:
    enum
    {
        Size = 32768
    };

  private:
    short int _table[Size];

  public:
    RandomTable(int seed = 1);
    // returned number is always in range 0..Size-1
    int operator()(int i) const { return _table[i & (Size - 1)]; }
    int Seed(int x, int z, int y) const;
    //! seed combiner - fast (unrolled) version
    int Seed(int x, int z) const;
    //! seed combiner - reference (slow, unoptimized C source) version
    int SeedRef(int x, int z) const;
};

class RandomGenerator
{
    mutable int _seed;
    RandomTable _seedTable;
    RandomTable _valueTable;

  public:
    RandomGenerator(int seed1, int seed2);
    RandomGenerator();
    float RandomValue(int seed) const;
    float RandomValue(int x, int z) const { return RandomValue(GetSeed(x, z)); }
    float RandomValue(int x, int z, int y) const { return RandomValue(GetSeed(x, z, y)); }
    void SetSeed(int seed) { _seed = seed; }
    float RandomValue() const;
    float Gauss(float min, float mid, float max) const;
    float PlusMinus(float a, float b) const;

    __forceinline int GetSeed(int x, int z) const { return _seedTable.Seed(x, z); }
    __forceinline int GetSeedRef(int x, int z) const { return _seedTable.SeedRef(x, z); }
    __forceinline int GetSeed(int x, int z, int y) const { return _seedTable.Seed(x, z, y); }
};

// Asset-only rendering can construct visual proxies (e.g. flag cloth) through
// normal model loading without drawing from the campaign's random stream.
inline RandomGenerator*& RandomGeneratorOverride()
{
    static thread_local RandomGenerator* current = nullptr;
    return current;
}

class ScopedRandomGenerator
{
    RandomGenerator* _previous;

  public:
    explicit ScopedRandomGenerator(RandomGenerator& generator) : _previous(RandomGeneratorOverride())
    {
        RandomGeneratorOverride() = &generator;
    }
    ~ScopedRandomGenerator() { RandomGeneratorOverride() = _previous; }
    ScopedRandomGenerator(const ScopedRandomGenerator&) = delete;
    ScopedRandomGenerator& operator=(const ScopedRandomGenerator&) = delete;
};

// Meyers singleton accessor — constructed on first use, no static-init-order hazard.
inline RandomGenerator& GRandGen()
{
    if (auto* local = RandomGeneratorOverride())
        return *local;
    static RandomGenerator instance;
    return instance;
}

// Let call sites spell the singleton as `GRandGen.Foo()`; expands to `GRandGen().Foo()`.
#define GRandGen GRandGen()

#endif
