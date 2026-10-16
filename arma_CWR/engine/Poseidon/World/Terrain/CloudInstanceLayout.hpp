#pragma once
#include <cmath>
// Shared authored-cloud cell layout. The visible sky alone applies camera compression.
namespace Poseidon::clouds {
struct Phase { int xCell, zCell; float xOffset, zOffset; };
struct Instance { int type; float x, z, azimuth; };
inline Phase MakePhase(float position, int layer) {
    static constexpr float xs[3] = {.040f,.024f,.020f}, zs[3] = {.008f,-.004f,-.008f};
    float x=position*xs[layer], z=position*zs[layer];
    int xi=static_cast<int>(std::floor(x)), zi=static_cast<int>(std::floor(z));
    return {xi,zi,(x-xi)*6000.0f,(z-zi)*6000.0f};
}
template<class Random> Instance AtCell(Random& random, const Phase& phase, int x, int z, int layer) {
    int seed=random.GetSeed(x-phase.xCell,z-phase.zCell,layer);
    int type=static_cast<int>(std::floor(random.RandomValue(seed)*8));
    // Retain legacy double half-grid arithmetic, rounded to float before cell addition.
    float xOffset=phase.xOffset+random.RandomValue(seed+1)*(6000.0f*0.5);
    float zOffset=phase.zOffset+random.RandomValue(seed+2)*(6000.0f*0.5);
    return {type,x*6000.0f+xOffset,
                 z*6000.0f+zOffset,
                 random.RandomValue(seed+3)*(2*3.14159265358979323846f/2)};
}
}
