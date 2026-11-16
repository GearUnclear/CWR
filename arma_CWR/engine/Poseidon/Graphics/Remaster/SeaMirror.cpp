#include <Poseidon/Graphics/Remaster/SeaMirror.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Poseidon::remaster
{

float ParseSeaMirrorXp(const char* value)
{
    if (!value || !*value)
        return 0.0f;
    char* end = nullptr;
    const float v = std::strtof(value, &end);
    if (end == value || !std::isfinite(v) || v <= 0.0f)
        return 0.0f;
    return std::min(v, 4.0f);
}

float ParseSeaMirrorWeight(const char* value)
{
    if (!value || !*value)
        return 1.0f;
    char* end = nullptr;
    const float v = std::strtof(value, &end);
    if (end == value || !std::isfinite(v))
        return 1.0f;
    return std::clamp(v, 0.0f, 1.0f);
}

float SeaMirrorXpWeight()
{
    static const float weight = ParseSeaMirrorWeight(std::getenv("POSEIDON_XP_SEAMIRROR_WEIGHT"));
    return weight;
}

float SeaMirrorXpScale()
{
    static const float scale = ParseSeaMirrorXp(std::getenv("POSEIDON_XP_SEAMIRROR"));
    return scale;
}

} // namespace Poseidon::remaster
