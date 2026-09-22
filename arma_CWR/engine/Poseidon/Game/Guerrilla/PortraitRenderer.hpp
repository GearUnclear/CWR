#pragma once
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace Poseidon::Guerrilla
{
struct PortraitAppearance
{
    RString body;
    RString face;
};

// A graphics-thread-owned, unregistered mannequin. Preparation resolves the
// dependencies before cache lookup; Capture alone issues portrait draws.
class PortraitRenderer
{
  public:
    PortraitRenderer();
    ~PortraitRenderer();
    bool Prepare(const PortraitAppearance& appearance, std::string& error);
    bool Capture(std::vector<uint8_t>& rgb, std::string& error);
    const std::vector<std::string>& Dependencies() const;
    const std::string& Configuration() const;

  private:
    struct State;
    std::unique_ptr<State> _state;
};
} // namespace Poseidon::Guerrilla
