#pragma once
#include <Poseidon/Foundation/Math/Math3D.hpp>
namespace Poseidon { class Shape; }
struct CloudRegistrationInstance { const Poseidon::Shape* shape; Matrix4 transform; int layer, cellX, cellZ, type; };
struct CloudRegistrationInput {
    const CloudRegistrationInstance* instances = nullptr;
    int count = 0;
    Vector3 camera, towardSun;
    float centreX = 0, centreZ = 0, extent = 4096, alpha = 0, phase = 0;
    bool complete = false;
};
